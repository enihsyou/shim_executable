// ------------------------------------------------------------------------- //
// Shim Executable                                                           // 
// ------------------------------------------------------------------------- //
/**@file    SHIM_EXECUTABLE.CPP
 * @brief   Main code for creating shims
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    2/6/2025
 *
 * ------------------------------------------------------------------------- 
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 * ------------------------------------------------------------------------- */

#include <version.h>
#include <log.h>
#include <resource_functions.h>
#include <get_argument.h>
#include <utility_functions.h>

#pragma comment(lib, "SHELL32.LIB")


// ----------------- Unpack the Shim from this Application ----------------- // 
BOOL UnpackShim(const filesystem::path& path, wstring shim_type) {
  return GetResourceFile("SHIM_" + NarrowString(shim_type), path);
}


// ----------------------------- Help Message ------------------------------ // 
void ShowHelp(string exec_name, bool is_shimgen) {
  cout.clear();
  cout << horizontal_line_bold << endl;
  cout << "SHIM CREATOR - v" << VER_FILEVERSION_STR << endl;
  cout << horizontal_line_bold << endl;
  
  string cmd = "    " + exec_name;
  cout << cmd + " [-? | -h | --help]" << endl;
  if(!is_shimgen)
    cout << cmd + " PATH [OUTPUT] [...]" << endl;
  cout << cmd + " -p PATH -o OUTPUT [...]" << endl;
  cout << cmd + " --path=PATH --output=OUTPUT [...]" << endl << endl << endl;

  // ---------- INFO ---------- //
  cout << horizontal_line << endl;
  cout << " INFO" << endl;
  cout << horizontal_line;

  string help_text(R"V0G0N(
Generates a 'shim' file that has the sole purpose of executing another file,
similar to a shortcut, yet is a full fledged executable. During creation, the
resources of the source executable such as version info and icons are copied to
the shim. In addition to the source path, specific command line arguments can
be embedded and hence editable by a resource editor (e.g.
https://www.angusj.com/resourcehacker).

One specific option to take note of is denoting if the source application has a
GUI. Typically, this simply denotes if the shim process should end immediately
after starting the process for the source or to wait (these two options are
also selectable in the shim). In either case, the shim, originally built as a
console application, will utilize the current console when executed from the
command line. Otherwise it will spawn a console window to generate the child
process for the source executable. For GUI applications, where waiting is
unneeded, this console is immediately destroyed, albeit still noticeable. To
remedy this adverse effect, the --GUI option generates a shim built with the
GUI subsystem as opposed to the console subsystem. This in effect removes the
creation of a console for GUI source applications.

Shims created with and without the --GUI option still include the options to
wait or exit immediately and for the most part are indistinguishable. One
important yet practical difference is that GUI shims by default will exit
immediately after creating its child process whereas console shims will default
to wait. Of course GUI shims can be called from a console and flagged to wait
which should function similarly to console shims. Issues only arise if GUI
shims are called with any type of console logging turned on (i.e. --shim-help
or --shim-log). The type of CLI (e.g., powershell, cmd.exe, etc.) appears to
impact the output. If run outside of a console, in which case a console would
need to be created for the output stream, the shim instead writes to a
.SHIM.LOG file with the same path.

For additional information, execute the shim with --shim-help flag or visit
https://github.com/jphilbert/shim_executable.
)V0G0N");
  cout << help_text << endl << endl;

  
  // ---------- EXAMPLES ---------- //
  cout << horizontal_line << endl;
  cout << " EXAMPLES" << endl;
  cout << horizontal_line;
  if(is_shimgen) {
    help_text = R"V0G0N(
Paths are always expanded and vary on the argument. The output is relative to
the SHIMGEN executable directory whereas the application path is relative to the
output. Therefore, assuming SHIMGEN.exe is located in C:\SHIMGEN_DIR the
following examples are equivalent:

    SHIMGEN --output .\SHIM\app.exe --path ..\..\APP_PATH\app.exe 

    SHIMGEN --output C:\SHIMGEN_DIR\SHIM\app.exe --path C:\APP_PATH\app.exe 
)V0G0N";
    cout << help_text << endl << endl;
  }
  else {
    help_text = R"V0G0N(
Only the path to the application to shim is required. In which case, the shim
will be created in with the same name in the current directory. Thus, the
following all have the same behavior:
)V0G0N";
    cout << help_text << endl;
    cout << cmd + " --path=C:\\APP_PATH\\app.exe --output=app.exe"
         << endl << endl;
    cout << cmd + " -p \"C:\\APP_PATH\\app.exe\" -o app.exe"
         << endl << endl;
    cout << cmd + " C:\\APP_PATH\\app.exe app.exe"
         << endl << endl;
    cout << cmd + " C:\\APP_PATH\\app.exe" << endl;
    
    help_text = R"V0G0N(
Paths are always expanded and relative to the current directory. Assuming CD is
C:\CURRENT, the following examples are equivalent:
)V0G0N";
    cout << help_text << endl;

    cout << cmd + " --path ..\\APP_PATH\\app.exe --output .\\SHIM\\app.exe"
         << endl << endl;
    cout << cmd +
      " --path C:\\APP_PATH\\app.exe --output C:\\CURRENT\\SHIM\\app.exe"
         << endl << endl << endl;
  }



  // ---------- ARGUMENTS ---------- //
  cout << horizontal_line << endl;
  cout << " ARGUMENTS" << endl;
  cout << horizontal_line;
  help_text = R"V0G0N(
The application accepts the following arguments and they are not
case-sensitive. Argument flags can be shortened to a single dash and initial
letter (except for --GUI and --DEBUG) and values can be separated by either a
space or equal sign.
)V0G0N";
  cout << help_text;
  
  help_text = R"V0G0N(
Since PATH is required, it need not be denoted by a flag if it is the first
argument. Similarly, if the second argument is also not denoted by a flag, it
will be assumed to be OUTPUT.
)V0G0N";
  if(!is_shimgen) cout << help_text;
  
  help_text = R"V0G0N(
    --help              Show this help message and exit.
)V0G0N";
  cout << help_text;

  if(is_shimgen) {
    help_text = R"V0G0N(
    --path PATH         [REQUIRED] The path to the executable to shim. This can
                            be relative from the OUTPUT path and will be
                            expanded.

    --output OUTPUT     [REQUIRED] The path to the shim to create. This can be
                            relative from the SHIMGEN executable and will be
                            expanded.
)V0G0N";
    cout << help_text;
  }
  else {
    help_text = R"V0G0N(
    --path PATH         [REQUIRED] The path to the executable to shim. This can
                            be relative from the current directory and will be
                            expanded.

    --output OUTPUT     The path to the shim to create. This can be relative
                            from the current directory and will be expanded. If
                            only a valid directory is given, the name of the
                            executable is used for the shim. If omitted
                            completely, the current directory with the
                            executable name is used. This cannot be equal to
                            PATH.
)V0G0N";
    cout << help_text;
  }
  
  help_text = R"V0G0N(
    --command ARGS      Additional arguments the shim should pass to the
                            original executable automatically. Should be quoted
                            for multiple arguments.

    --iconpath ICON     [UNIMPLEMENTED] Path to a file to use for the shim's
                            icon. By default, the executable's icon resources
                            are used.

    --gui               Explicitly sets shim to be created using the GUI or
    --console               console subsystem. GUI shims exit as soon as the
                            child process for the executable is created where
                            as console shims will wait. If neither is set, by
                            default the subsystem will be infered by the
                            executable, thus these options likely would be need
                            only for special cases.

    --debug             Print additional information when creating the shim to
                            the console.
)V0G0N";
  cout << help_text << endl;
  cout << horizontal_line_bold;
  exit(0);
}


// ------------------------------------------------------------------------- //
// MAIN METHOD                                                               // 
// ------------------------------------------------------------------------- //
int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
  int exitcode              = 1;

  // ----------------------------------------------------------------------- //
  // Get Command Line Arguments                                              // 
  // ----------------------------------------------------------------------- //
  filesystem::path thisExecPath  = GetExecPath();
  wstring exec_name         = thisExecPath.stem().c_str();
  UpperCase(exec_name);
  filesystem::path exec_dir = thisExecPath.parent_path();
  filesystem::path curr_dir = filesystem::current_path();

  // The original SHIMGEN.EXE worked slightly different, so by simply having the
  // executable named as such, we'll handle the magic for the user
  bool is_shimgen           = exec_name.compare(L"SHIMGEN") == 0;

  wstring calling_cmd       = GetCommandLineW();
  vector<wstring> arg_list  = ParseArguments(calling_cmd);
  GetArgument(arg_list, 0, calling_cmd);

  wstring output            = L"";
  wstring input             = L"";
  wstring icon              = L"";
  wstring command_args      = L"";
  wstring shim_type         = L"";
  bool debug                = false;

  
  // -------------------------- //
  // Standard SHIMGEN Arguments //
  // -------------------------- //
  // Help
  //   -?, --help, -h
  if(GetArgument(arg_list, L"-(\\?|h|-help)"))
    ShowHelp(NarrowString(exec_name), is_shimgen);

  // Get Input Path
  //   -p, --path=VALUE
  GetArgument(arg_list, L"-(p|-path)", input);
  
  // Get Output Path  
  //   -o, --output=VALUE
  GetArgument(arg_list, L"-(o|-output)", output);
  
  // Get Additional Arguments for Application             
  //   -c, --command=VALUE
  GetArgument(arg_list, L"-(c|-command)", command_args);

  // Get Icon Path             
  //   -i, --iconpath=VALUE
  GetArgument(arg_list, L"-(i|-iconpath)", icon);
  
  // Force GUI
  //       --gui
  if(GetArgument(arg_list, L"--gui"))
    shim_type = L"GUI";
  
  // Debug Info
  //       --debug
  debug = GetArgument(arg_list, L"--debug");
  if (!debug) LOGCFG.level = 1;
  else LOGCFG.level = 3;      // ignore level 4+

  
  // ------------------------------------------ //
  // Supplemental Arguments and Passing Methods //
  // ------------------------------------------ //
  if(!is_shimgen) {
    // Force Console 
    //       --console
    // since GUI and CONSOLE shims are significantly different than those
    // created by SHIMGEN, this allows forcing GUI apps to use the CONSOLE shim
    // if needed 
    if(GetArgument(arg_list, L"--console")) {
      if(shim_type.empty())
        shim_type = L"CONSOLE";
      else {
        LOG(2) << "CONSOLE and GUI flags cannot be used together,";
        LOG(-2) << "assuming GUI was intended";
      }
    }

    // Additional Input Path Methods
    //   --input=VALUE
    if(input.empty())
      GetArgument(arg_list, L"--input", input);
    //   ... or if all else fails, use the first argument
    if(input.empty()) {
      ReparseArguments(arg_list);
      GetArgument(arg_list, 0, input);
    }
  
    // Additital Output Path Method  
    // technically we have parsed all the valid arguments, so we'll assume if
    // there is one left, it is the output path 
    if(output.empty()) {
      ReparseArguments(arg_list);
      GetArgument(arg_list, 0, output);
    }
  }

  if (!CollapseArguments(arg_list).empty()) {
    LOG(2) << "Additional arguments ignored: ";
    LOG(-2) << CollapseArguments(arg_list);
  }
  
  TrimQuotes(input);
  TrimQuotes(output);
  TrimQuotes(icon);
  TrimQuotes(command_args);
  command_args = UnquoteString(command_args);

  // Debug Info
  LOG(4) << "exec_name:       " << exec_name;
  LOG(4) << "exec_dir:        " << exec_dir;
  LOG(4) << "curr_dir:        " << curr_dir;
  LOG(4) << "is_shimgen:      " << is_shimgen;
   
  LOG(4) << "output:          " << output;
  LOG(4) << "input:           " << input;
  LOG(4) << "icon:            " << icon;
  LOG(4) << "command_args:    " << command_args;
  LOG(4) << "shim_type:       " << shim_type;
  LOG(4) << "debug:           " << debug;


  // ----------------------------------------------------------------------- //
  // Validate / Transform Arguments                                          // 
  // ----------------------------------------------------------------------- //
  filesystem::path input_path =     input;
  filesystem::path output_path =    output;

  // Check if INPUT is  EMPTY
  if (input.empty()) {
    LOG(1) << "SOURCE executable must be specified.";
    return exitcode;
  }

  // ---------- Expand Paths ---------- // 
  // NOTE: SHIMGEN requires OUTPUT to be explicitly given and possibly relative
  // to this EXECUTABLE. INPUT then can be relative to the OUTPUT. 
  if(is_shimgen) {
    // Check if OUTPUT is EMPTY
    if (output.empty()) {
      LOG(1) << "OUTPUT path must be specified.";
      return exitcode;
    }

    // Expand OUTPUT from EXEC_DIR if necessary 
    if (output_path.is_relative()) {
      LOG(3) << "OUTPUT path is relative, expanding from "
             << exec_name << " path";
      LOG(-4) << exec_dir;
      output_path = exec_dir / output_path;
      output_path = filesystem::weakly_canonical(output_path);
    }
    
    // Expand INPUT from OUTPUT_DIR if necessary 
    if (input_path.is_relative()) {
      LOG(3) << "SOURCE path is relative, expanding from OUTPUT path";
      LOG(-4) << output_path.parent_path();
      input_path = output_path.parent_path() / input_path;
      input_path = filesystem::weakly_canonical(input_path);
    }
  }
  // New version does not require OUTPUT to be set and relative paths are assume
  // to be from the CURRENT directory
  else {
    // Expand INPUT if necessary 
    if (input_path.is_relative()) {
      LOG(3) << "SOURCE path is relative, expanding from CURRENT path";
      LOG(-4)  << curr_dir;
      input_path = filesystem::weakly_canonical(curr_dir / input_path);
    }

    // Check if OUTPUT is empty
    if (output.empty()) {
      output_path = curr_dir;
      LOG(2) << "OUTPUT path was not specified, using CURRENT path";
      LOG(-4) << output_path;
    }
    
    // Expand OUTPUT if necessary
    if (output_path.is_relative()) {
      LOG(3) << "OUTPUT path is relative, expanding from CURRENT path";
      LOG(-4) << curr_dir;
      output_path = filesystem::weakly_canonical(curr_dir / output_path);
    }
  }


  
  // ---------- INPUT File ---------- // 
  // Check if EXISTS
  if (!filesystem::exists(input_path)) {
    LOG(1) << "SOURCE path, " << input_path << ", does not exist";
    return exitcode;      
  }
  
  // Check if its a REGULAR FILE
  if (!filesystem::is_regular_file(input_path)) {
    LOG(1) << "SOURCE, " << input_path.filename() << ", must be a regular file";
    return exitcode;
  }

  // Check if EXECUTABLE
  DWORD execType =
    SHGetFileInfoW(input_path.c_str(), NULL, NULL, NULL, SHGFI_EXETYPE);
  if (execType == 0) {
    LOG(1) << "SOURCE, " << input_path.filename() << ", must be an executable";
    return exitcode;
  }

  // Print the Application Info
  LOG(3)  << "SOURCE APPLICATION: ";
  LOG(-3) << input_path;

  LOG(3)  << "APPLICATION TYPE: ";
  if (HIWORD(execType) != 0)
    LOG(-3) << "Windows GUI application";
  else if (LOWORD(execType) == 0x5A4D)
    LOG(-3) << "MS-DOS application";
  else
    LOG(-3) << "Windows Console application (or .bat)";    


  
  
  // ---------- Output Path ---------- // 
  // If only a directory is given, add the filename to it
  if (filesystem::is_directory(output_path)) {
    output_path /= input_path.filename();
    LOG(2) << "OUTPUT filename not specified, using "
           << input_path.filename();
  }

  // Check if its directory EXISTS
  if (!filesystem::is_directory(output_path.parent_path())) {
    LOG(1) << "OUTPUT directory, " << output_path.parent_path()
           << ", does not exist";
    return exitcode;
  }

  // Check if it EXISTS
  if (filesystem::exists(output_path)) {
    
    // ... it cannot be EQUAL to the SOURCE
    if (filesystem::equivalent(output_path, input_path)) {
      LOG(1) << "Cannot overwrite SOURCE.";
      LOG(-1) << "Choose a different filename or directory";
      return exitcode;
    }

    // ... it must be a regular file
    if ((!filesystem::is_regular_file(output_path))) {
      LOG(1) << "OUTPUT already exists but is not a regular file.";
      LOG(-1) << "Choose a different filename or directory";
      return exitcode;
    }

    LOG(2) << "OUTPUT already exists and will be overwritten.";
  }


  // Print the Application Info
  LOG(3) << "OUTPUT SHIM: ";
  LOG(-3) << output_path;

  // Set the Shim Type
  if (shim_type.empty()) {
    if (HIWORD(execType) != 0)
      shim_type = L"GUI";
    else
      shim_type = L"CONSOLE";
    LOG(3)  << "SHIM TYPE: ";
    LOG(-3) << shim_type << " (automatically selected)";
  }
  else {
    LOG(3)  << "SHIM TYPE: ";
    LOG(-3) << shim_type << " (manually selected)";
  }
  
  // ---------- Icon Path ---------- // 
  if (!icon.empty())
    LOG(2) << "Specifying alternative icon not implemented, ignoring";


  // ---------- Additional Application Commands ---------- // 
  if (!command_args.empty()) {
    LOG(3) << "SHIM ARGUMENTS: " << command_args;
  }


  
  // ----------------------------------------------------------------------- //
  // Build Shim                                                              // 
  // ----------------------------------------------------------------------- //

  // ---------- Unpack / Create Shim ---------- // 
  if (!UnpackShim(output_path, shim_type)) {
    LOG(1) << "Could not unpack shim";
    return exitcode;
  }
  
  LOG(3) << "Created shim, " << output_path.filename()
         << ", from SHIM_" << shim_type << ".EXE";


  // ---------- Copy and Add Resources ---------- // 
  CopyResources(output_path, input_path);    

  // Add Shim Arguments
  AddResourceData(output_path, "SHIM_PATH", input_path);
  AddResourceData(output_path, "SHIM_TYPE", shim_type);  
  if (!command_args.empty()) 
    AddResourceData(output_path, "SHIM_ARGS", command_args);


  // -------------------------------- Done --------------------------------- // 
  LOG() << exec_name << " has successfully created " << output_path;
  return exitcode;
}
 
