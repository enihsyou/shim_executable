# Shimming Executables 
This application generates an executable "[shim](https://en.wikipedia.org/wiki/Shim_(computing))" that will in turn execute another file relative to its location. This is an attempt at a drop in replacement for the RealDimensions Software LLC's (RDS) `shimgen.exe` that comes with [Chocolately](https://chocolatey.org/), yet has various other possible uses. Per their [site](https://docs.chocolatey.org/en-us/features/shim/):
>Shimming is like symlinking, but it works much better. It's a form of redirection, where you create a "shim" that redirects input to the actual binary process and shares the output. It can also work to simply call the actual binary when it shims GUI applications.
>
>We like to call this "batch redirection that works".
>
>This also allows applications and tools to be on the "PATH" without cluttering up the PATH environment variable.

The generator and in turn, the shims themselves, have evolved from [kiennq's c++ shim implementation](https://github.com/kiennq/scoop-better-shimexe) for Scoop and [TheCakeIsNaOH's scoop style shimgen](https://github.com/TheCakeIsNaOH/scoop-style-shimgen) for Chocolatey. Hence many features are attributed to or inspired by their great work.

## Features and Improvements
### Generator
  - **Free and Open Source** - can be used outside of "official" Chocolatey builds without a paid license
  - **Backward compatible** - drop-in replacement of `shimgen.exe` (see [Compatibility](#Compatibility) section for more details)
  - **Smaller size** - almost 50% smaller :exclamation:
  - **Increased speed** - I assume RDS `shimgen.exe` compiles a new executable using `csc.exe` for every shim which is a significant bottleneck
  
  
### Shims:
  - **More transparent execution** - GUI applications launch without the brief, yet annoying, console window display (see [Motivation](#Motivation) section below)
  - **Automatic use of parent version info and icons** - in Windows, shims will appear to be equivalent to their parent
  - **Smaller size** - now 28% smaller :exclamation:
  - **Increased speed** - now 50% faster :exclamation: (no *precise* speed tests were actually done)
  - Better support for `crtl+c`, as shim passes signal to child process
  - Terminates child processes if parent process is killed
  - Consistent checksum for all shims


## To Do
  - More testing
  - Impliment changing of icon (i.e., `--iconpath` function) - does anyone actually use this?
  - More testing
  - :question: Add support for embedding working directory into shim
  - :question: Add option to create Scoop-style shims (`.shim` file next to each shim `.exe`)


# Using 
1. Download a release or build it yourself
2. Run `shim_exec.exe <source>`

This will create an executable in the current directory named the same as `<source>` that will in turn execute it. More options can be viewed using the [help](doc/shimgen-h.txt) flag `-?`, `-h`, or `--help`. The shim itself has additional options and can be viewed using it's [help](doc/shim-help.txt) flag `--shim-help`.




# Motivation
Why reinvent the wheel? Ever since I found [Chocolately](https://chocolatey.org/), I thought it would solve all my problems with various app installs across home and work computers. Specifically, keeping track of where things are and the Windows path. As I began embracing the ecosystem more I found `shimgen.exe` was key, though had a small quirk I just could not get past.

If you live in the terminal most of the time, you wouldn't notice this, which is why it took me a while to become annoyed. Shims that launch a GUI executed from the command line seem no different from executing their parent, however run it from elsewhere (e.g., Windows Explorer) you'll probably notice a terminal briefly popping up. You have to be quick as its fast (250ms). 

Here is an example that was slowed down 25%:

![launching a GUI shim from windows (RDS)](doc/gui%20shim%20launching%20(RDS).gif)

Is it *THAT* big of a problem? For most, probably not, but I thought so :P and as usual, more I got into it other little improvements were done. Apart from ridding the terminal popup, it seems much quicker (600ms, by my imprecise screen capture editor).

![launching a GUI shim from windows](doc/gui%20shim%20launching.gif)

# Methodology
As mentioned earlier, this application was built off the the foundation laid by 
[kiennq](https://github.com/kiennq/scoop-better-shimexe) and [TheCakeIsNaOH](https://github.com/TheCakeIsNaOH/scoop-style-shimgen), both whom used C++. 

> [!WARNING]
> Though I appreciate the advantages using C++, I had not used it for over 20 years (this was the days of MFC, if anyone remembers) so forgive me for my poor implimentation and antiquated syntax.

As an example of my unfamiliarity, both of them also made use of `wstring` which I continued, though consistently complicated code. I believe in the end I compartmentalized it enough to keep confusion to a minimum.

The great thing I found from [TheCakeIsNaOH](https://github.com/TheCakeIsNaOH/scoop-style-shimgen) code was embedding the shim executable into the actual generator executable. In a way, generating a shim is simply like unzipping itself. Because the shim itself is already compiled, no external resources are needed, however it does require the shim executable to be generic enough to run any parent application. This actually was solved earlier by [kiennq](https://github.com/kiennq/scoop-better-shimexe) since they designed it to replace shims in [Scoop](https://github.com/ScoopInstaller/Scoop).

[Scoop](https://github.com/ScoopInstaller/Scoop), unlike [Chocolately](https://chocolatey.org/), uses text files to store shim information such as the parent executable and its location. The shim executable simply reads this information and spawns a new process. These text files do provide an easy way to inspect and alter a shim, however it does double the number of files since each shim has a `.exe` and `.txt`. 

Prefering to keep things neat and tidy, I wanted to rid myself of these files and was inspired by [TheCakeIsNaOH](https://github.com/TheCakeIsNaOH/scoop-style-shimgen). Why not embed this information in the actual shim executable? I found that it wasn't difficult to insert data into an `.exe`. Furthermore, version info and icons are similar resources stored in the parent, so may as well copy those over too.

The final task was to solve the initial annoyance of that console window popping up. This, I some how found, was due to Windows *actually* having two seperate subsystems for console and GUI apps. Basically, all shim executables were built to run off the console subsystem (i.e. console apps), which in turn would spawn the parent executable process. You should be able to see the issue by now: for GUI shims executing in the Windows subsystem, a process is created breifly in the console subsystem (i.e. the console shim) which is killed after it spawns the window subsystem process for the app.

In short, we want GUI apps to be shimmed with GUI shims and console apps to be shimmed with console shims. Thus two shims need to be built and embedded into the creator. The only difference between the two is the entry point which most would recognize as:

``` c++
// Console Application Entry Point
int wmain(int argc, wchar_t* argv[])
```

and

``` c++
// GUI Application Entry Point
int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PWSTR pCmdLine, int nCmdShow)
```

As seperate code files, the compiler handles all the details, however diving deeper into this recently, I found actually that the linker does the bifurcating and thus simplify the code and build process immensely (see the [makefile](makefile) for more details). 


# Compatibility
For being a simple command line utility, much of the code is for user I/O and backwards compatibility with RDS's `shimgen.exe`. I actually started writing most of this prior to fully  understanding some its quirks and hence, some options function as I would have *expected* them to. To remedy these nuances, the generator executable is built to behave as close to possible to the original **IF** its actually named `shimgen.exe`. 

> [!CAUTION]
> To be explicit, renaming `shim_exec.exe` to `shimgen.exe` changes how it functions.

That being said, this only changes how its options are interpreted and not the final shim. A shim made with either will be identical. The specific differences are:

| Option | Function | RDS `shimgen.exe` | `shimgen.exe` | `shim_exec.exe` |
|--:|:--|:--|:--|:--|
| `help` | Prints help to the console | | _(same)_ | _(same)_ |
| `path` | Path to the executable to be shimmed |  :bangbang: **[REQUIRED]** :bangbang:<br/> - Paths relative to `output`<br/> - Will <ins>not</ins> be expanded. | :bangbang: **[REQUIRED]** :bangbang:<br/> - Paths relative to `output`<br/> - <ins>Will be</ins> expanded. | :bangbang: **[REQUIRED]** :bangbang:<br/> - Can be positional, :one:<br/> - Relative to current directory. |
| `output` | Path to the output shim | :bangbang: **[REQUIRED]** :bangbang:<br/> - Paths relative to `shimgen.exe` | _(same)_ | :grey_question: **[OPTIONAL]** :grey_question:<br/> - Can be positional, :two:<br/> - Relative to current directory<br/> - Default: `\[current_directory]\[parent_executable].exe`. 
| `command` | Adds arguments to executable | :grey_question: **[OPTIONAL]** :grey_question:<br/> - string w/o spaces or quoted escaped string | _(same)_ | _(same)_ |
| `iconpath` | Icon to be used for shim | :grey_question: **[OPTIONAL]** :grey_question:| _(not implemented)_ | _(not implemented)_ |
| `gui` | Forces GUI shim | :grey_question: **[OPTIONAL]** :grey_question:<br/> - forces shim to exit immediately after running parent | :grey_question: **[OPTIONAL]** :grey_question:<br/> - forces creation of a GUI shim which by default exits immediately | _(same)_ |
| `debug` | Prints additional info | :grey_question: **[OPTIONAL]** :grey_question: | _(same)_ | _(same)_ |

For more detail you can compare the help text of the three: [shimgen-h (RDS).txt](doc/shimgen-h%20(RDS).txt), [shimgen-h.txt](doc/shimgen-h.txt), and [shim_exec-h.txt](doc/shim_exec-h.txt).

The shims themselves behave more similarly. There are no differences between shims created with `shim_exec.exe` and the compatible version, `shimgen.exe`. Shim arguments are prefixed with `--shimgen-` however those created with `shim_exec.exe` have been relaxed to requiring only `--shim.*-`. The differences between the shim arguments are: 

| Option | RDS `shimgen.exe` | `shim_exec.exe` |
|--:|:--|:--|
| `help` | -prints help to the console | _(same)_ |
| `log` | - print additional diagnostic info to console | - if console shim, prints to consol<br/> - if gui shim executed from console, print to console<br/> -if gui shim executed from Windows, print to log file |
| `waitforexit` | - force shim to wait for parent to exit | _(same)_ |
| `exit` | - force shim to exit after executing for parent | _(same)_ |
| `gui` | - force shim to behave as a GUI shim | since it is impractical to convert a console shim to a GUI shim, this behaves exactly like `exit`|
| `usetargetworkingdirectory` | - set the working directory to the target path **_ONLY_** if the shim was created with relative paths | - set the working directory to the target path **_REGARDLESS_** |
| `noop` | - stop prior to executing parent | _(same)_ |

## Replacement
If you choose to use this instead of the `shimgen.exe` provided by Chocolately a PowerShell script, [./tools/replace_shimgen.ps1](./tools/replace_shimgen.ps1), is available to toggle between the two.


# Building
1. Get a copy of [Visual Studio](https://visualstudio.microsoft.com/downloads/). Installing just the "Desktop Development w/ C++" should suffice however the minimum I installed was:
    - Microsoft Visual Studio Workload - Core Editor
    - Microsoft Visual Studio Component - Core Editor
    - Microsoft Visual Studio Workload - Native Desktop
    - Microsoft Visual Studio Component Group - Native Desktop Core
    - Microsoft Visual Studio Component - Roslyn Compiler
    - Microsoft Visual Studio Component - Text Templating
    - Microsoft Visual Studio Component - VC Core IDE
    - Microsoft Visual Studio Component - VC Tools x86 x64
    - Microsoft Visual Studio Component - VC Redist 14 Latest 
    - Microsoft Visual Studio Component - Windows 11 SDK 22000
    - Microsoft Component - MSBuild
2. Build using `nmake`. Visual Studio should have installed `nmake` however other flavors should be able to process the included [makefile](./makefile).


# Thanks
If I didn't mention it enough, this app directly follows from the great work of [@TheCakeIsNaOH](https://github.com/TheCakeIsNaOH) and [@kiennq](https://github.com/kiennq) (I owe them :beer:, :coffee:, :cake:). Also thanks to those working on Scoop and [Chocolately](https://chocolatey.org/) and being open with its code. It doesn't make us rich but do it for the [feels](https://media.tenor.com/Ibkju4TwMWIAAAAe/feels-good-good.png).


# License
`SPDX-License-Identifier: MIT OR Unlicense`
