// ------------------------------------------------------------------------- //
// SHIM CODE                                                                 // 
// ------------------------------------------------------------------------- //
/**@file    SHIM_TEMPLATE.H
 * @brief   Primary shim method
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    2/6/2025
 *
 * ------------------------------------------------------------------------- 
 * The only difference between a CONSOLE and GUI shim is the entry point
 * (i.e wmain and APIENTRY wWinMain respectively). These are chosen in the
 * shim_xxx.cpp files which both simply need to call shim() as defined
 * below.
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

#include "shim_resources.h"
#include "log.h"
#include "resource_functions.h"
#include "get_argument.h"
#include "utility_functions.h"

#pragma comment(lib, "SHELL32.LIB")


#ifndef ERROR_ELEVATION_REQUIRED
#define ERROR_ELEVATION_REQUIRED 740
#endif

#define BUFSIZE 4096
#define SHIM_ARG_PREFIX L"--shim"

bool GetShimArg(vector<wstring> &args, wstring pattern) {
  wstring argFormat = SHIM_ARG_PREFIX;
  argFormat += L"[a-z]*-";
  argFormat += pattern;
  argFormat += L"[a-z]*";
  return GetArgument(args, argFormat);
}


// --------------------------- Process Creation ---------------------------- // 
BOOL WINAPI CtrlHandler(DWORD ctrlType) {
  switch (ctrlType) {
    // Ignore all events, and let the child process handle them.
  case CTRL_C_EVENT:
  case CTRL_CLOSE_EVENT:
  case CTRL_LOGOFF_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_SHUTDOWN_EVENT:
    return TRUE;

  default:
    return FALSE;
  }
}

struct HandleDeleter {
  typedef HANDLE pointer;
  void operator()(HANDLE handle) {
    if (handle) {
      CloseHandle(handle);
    }
  }
};

namespace std {
  typedef unique_ptr<HANDLE, HandleDeleter> unique_handle;
}

tuple<unique_handle, unique_handle> MakeProcess(
    const wstring &path,
    const wstring &args,
    const wstring &workingDirectory) {
  STARTUPINFOW        startInfo     = {};
  PROCESS_INFORMATION processInfo   = {};
  unique_handle       threadHandle;
  unique_handle       processHandle;

  // Build the Command Line
  wstring cmd = path;
  if(!args.empty())
    cmd += L" " + args;
  
  // Set the Working Directory
  LPCWSTR workingDirectoryCSTR = nullptr;
  if (!workingDirectory.empty()) {
      workingDirectoryCSTR = workingDirectory.c_str();

      if (!PathFileExistsW(workingDirectoryCSTR))
        LOG(2) <<
          "Working directory does not exist, process may fail to start";
  }
  
  // Create the Process
  if (CreateProcessW(
          nullptr,                 // No module name (use command line)       
          cmd.data(),              // Command Line
          nullptr, nullptr, TRUE,  // Inheritance (Process, Thread, Handle)
          CREATE_SUSPENDED,        // Suspend threads on creation
          nullptr,                 // Use parent's environment block          
          workingDirectoryCSTR,    // Starting directory         
          &startInfo, &processInfo)) {
    // Set the handles
    threadHandle.reset(processInfo.hThread);
    processHandle.reset(processInfo.hProcess);
    
    // Start the thread
    ResumeThread(threadHandle.get());
  }
  else if (GetLastError() == ERROR_ELEVATION_REQUIRED) {
    // We must elevate the process, which is (basically) impossible with
    // CreateProcess, and therefore we fallback to ShellExecuteEx, which CAN
    // create elevated processes, at the cost of opening a new separate
    // window.  Theoretically, this could be fixed (or rather, worked around)
    // using pipes and IPC, but... this is a question for another day.

    SHELLEXECUTEINFOW sei = {};

    sei.cbSize = sizeof(SHELLEXECUTEINFOW);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpFile = path.c_str();
    sei.lpParameters = args.c_str();
    sei.nShow = SW_SHOW;
    sei.lpDirectory = workingDirectoryCSTR;

    if (!ShellExecuteExW(&sei)) {
      LOG(1) << "Unable to create elevated process: error ";
      LOG(-1) << GetLastError();
      return {move(processHandle), move(threadHandle)};
    }

    processHandle.reset(sei.hProcess);
  }
  else {
    LOG(1) << "Could not create process with command: ";
    LOG(-1) << "'" << cmd << "'";
    return {move(processHandle), move(threadHandle)};
  }

  // Ignore Ctrl-C and other signals
  if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) 
    LOG(2) << "Could not set control handler; Ctrl-C behavior may be invalid";

  return {move(processHandle), move(threadHandle)};
}



// ----------------------------- Help Message ------------------------------ // 
void ShowHelp() {
  LOG() << horizontal_line_bold;
  LOG() << " INFO";
  LOG() << horizontal_line_bold;
  LOG() << R"V0G0N(This is an application 'shim' that will execute another application (typically
named the same located elsewhere). Execute with --shim-NoOp to identify its
target.

Execute SHIM_EXEC -h or visit https://github.com/jphilbert/shim_executable for
additional information.

)V0G0N";

  
  LOG() << horizontal_line_bold;
  LOG() << " ARGUMENTS";
  LOG() << horizontal_line_bold;
  LOG() << R"V0G0N(The arguments below can be passed to the shim prior to it executing its parent.
They are not case-sensitive and have an equivilent shimgen alias for
Chocolately compatibility.

Technically the arguments need only match "--shim[a-z]*-[hlwegun][a-z]*".

All other argument are passed to the parent executable.

    --shim-Help     Shows this help menu and exits without running the target

    --shim-Log      Turns on diagnostic messaging in the console. If a windows
                        application executed without a console, a file (<shim
                        path>.SHIM.LOG) will be generated instead.
                        (alias --shimgen-log)

    --shim-Wait     Explicitly tell the shim to wait for target to exit. Useful
                        when something is calling a GUI and wanting to block
                        command line programs. This is the default behavior
                        unless the shim was created with the --GUI flag. Cannot
                        be used with --shim-Exit or --shim-GUI.
                        (alias --shimgen-waitforexit)

    --shim-Exit     Explicitly tell the shim to exit immediately after creating
                        the application process. This is the default behavior
                        when the shim was created with the --GUI flag. Cannot
                        be used with --shim-Wait.
                        (alias --shimgen-exit)

    --shim-GUI      Explicitly behave as if the target is a GUI application.
                        This is helpful in situations where the package did not
                        have a proper .gui file. This technically has the same
                        effect as --shim-Exit and is kept for legacy purposes.
                        (alias --shimgen-gui)

    --shim-UseTargetWorkingDirectory
                    Set the working directory to the target path if omitted. By
                        default the application will use the shim's path.
                        Useful when programs need to be running from where they
                        are located (usually indicates programs that have
                        issues being run globally).
                        (alias --shimgen-UseTargetWorkingDirectory)

    --shim-NoOp     Executes the shim without calling the target application.
                        Logging is implicitly turned on.
                        (alias --shimgen-noop))V0G0N";
  
  exit(0);
}


// ----------------------------- Main Function ----------------------------- // 
int shim() {
  DWORD exitCode            = 1;
  
  // --------------------- Get Command Line Arguments ---------------------- //
  filesystem::path thisExecPath  = GetExecPath();
  wstring shimExe           = thisExecPath.filename().c_str();
  UpperCase(shimExe);
  wstring shimDir           = thisExecPath.parent_path().c_str();
  wstring currDir           = filesystem::current_path().c_str();

  wstring calling_cmd       = GetCommandLineW();
  vector<wstring> arg_list  = ParseArguments(calling_cmd);
  GetArgument(arg_list, 0, calling_cmd);
      
  bool shimArgLog           = GetShimArg(arg_list, L"l");    
  bool shimArgWait          = GetShimArg(arg_list, L"w");
  bool shimArgExit          = GetShimArg(arg_list, L"e");
  bool isWindowsApp         = GetShimArg(arg_list, L"g");
  bool shimArgUseTarget     = GetShimArg(arg_list, L"u");
  bool shimArgNoop          = GetShimArg(arg_list, L"n");

  // If there still exists an argument starting with "--shim" and just run help 
  if(GetArgument(arg_list, L"--shim.*")) ShowHelp();

  // Any arguments left, save to pass to parent executable
  wstring calling_args      = CollapseArguments(arg_list);
      
  // Print useful info
  if (shimArgLog || shimArgNoop) {
    LOG() << horizontal_line_bold;
    LOG() << shimExe << " - SHIM";
    LOG() << horizontal_line_bold;

    LOG() << "This is a shim for " << shimExe
          << " built with SHIM EXECUTABLE "
          << "(v" << VER_FILEVERSION_STR << ")";
    LOG() << "See https://github.com/jphilbert/shim_executable "
          << "for additional information";
    
    LOG();
    LOG() << "Shim Path:      " << "'" << shimDir << "'";
    LOG() << "Current Path:   " << "'" << currDir << "'";
    LOG();
    
    LOG() << "Command Line Parameters:";
    LOG() << "  GUI:          " << isWindowsApp; 
    LOG() << "  Log:          " << shimArgLog  ; 
    LOG() << "  NoOp:         " << shimArgNoop ;
    LOG() << "  Exit:         " << shimArgExit ; 
    LOG() << "  Wait:         " << shimArgWait ; 
    LOG() << "  Use Target:   " << shimArgUseTarget;

    if(calling_args.empty()) {
      LOG() << "  App Args:     "
            << "<NONE>";    }
    else {
      LOG() << "  App Args:     "
            << "'" << calling_args << "'";
    }
    LOG();
  }

  shimArgLog =  shimArgLog || shimArgNoop;    
  shimArgExit = shimArgExit || isWindowsApp;

  if (shimArgExit && shimArgWait) {
    LOG(1) << "SHIM-WAIT cannot be used with SHIM-EXIT or SHIM-GUI";
    return exitCode;
  }

  
  // ------------------------- Get Exec Arguments -------------------------- // 
  wstring appPath   = L"";
  wstring appDir    = L"";
  wstring appArgs   = L"";
  wstring shimType  = L"";
  
  if (!GetResourceData("SHIM_PATH", appPath)) {
    LOG(1)  << "Shim has no application path. ";
    LOG(-1) << "Shim is no longer valid and must be regenerated.";
    return exitCode;
  }
  else if (!filesystem::exists(appPath)) {
    LOG(1) << "Shim application path does not exist. ";
    LOG(-1) << "Shim is no longer valid and must be regenerated.";
    return exitCode;
  }
  else if (filesystem::equivalent(thisExecPath, appPath)) {
    LOG(1) << "Shim points to itself. ";
    LOG(-1) << "Shim is no longer valid and must be regenerated.";
    return exitCode;
  }
  else
    exitCode = 0;

  appDir = filesystem::path(appPath).parent_path().c_str();
  GetResourceData("SHIM_ARGS", appArgs);
  GetResourceData("SHIM_TYPE", shimType);

  // Here forward we'll just use shimArgWait
  if (shimType == L"CONSOLE")
    shimArgWait = !shimArgExit;

  // Print useful info
  if (shimArgLog) {
    LOG() << "Embedded Parameters:";
    LOG() << "  Shim Type:    " << shimType; 
    LOG() << "  App Name:     " << filesystem::path(appPath).stem();
    LOG() << "  App Path:     " << "'" << appDir << "'";
    if(appArgs.empty()) 
      LOG() << "  App Args:     " << "<NONE>";
    else 
      LOG() << "  App Args:     " << "'" << appArgs << "'";
    LOG();

    if (shimArgWait) {
      LOG(3) << "Waiting for process to finish ";
      if (shimType == L"CONSOLE")
        LOG(-3) << "(default for CONSOLE shim)";
      else
        LOG(-3) << "(overridden for GUI shim)";
    }
    else {
      LOG(3) << "Exiting immediately once started ";
      if (shimType == L"CONSOLE")
        LOG(-3) << "(overridden for CONSOLE shim)";
      else
        LOG(-3) << "(default for GUI shim)";
    }
    LOG();
  }

  // Combine the calling and embedded arguments
  if (!calling_args.empty() && !appArgs.empty())
    appArgs += L" ";
  appArgs += calling_args;

  wstring working_dir = shimArgUseTarget ? appDir : shimDir;
  
  
  
  // ----------------------------- Execute App ----------------------------- //
  if (shimArgLog) {
    LOG() << "Creating process for application";
    LOG() << "  APP: " << "'" << appPath << "'";
    LOG() << "  ARG: " << "'" << appArgs << "'";
    LOG() << "  DIR: " << "'" << working_dir << "'";
    LOG() << horizontal_line;
  }
  
  if (shimArgNoop) {
    LOG() << "Shim Exiting: NoOp";
    LOG() << horizontal_line;
    return exitCode;
  }
  
  auto [processHandle, threadHandle] =
    MakeProcess(move(appPath), move(appArgs), working_dir);
  
  exitCode = processHandle ? 0 : 1;

  // Wait for app to finish when
  if (processHandle && shimArgWait) {
    // A job object allows groups of processes to be managed as a unit.
    // Operations performed on a job object affect all processes associated with
    // the job object. Specifically here we attach to child processes to make
    // sure they terminate when the parent terminates as well.   
    unique_handle jobHandle(CreateJobObject(nullptr, nullptr));
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobInfo = {};

    jobInfo.BasicLimitInformation.LimitFlags = 
      JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE |
      JOB_OBJECT_LIMIT_SILENT_BREAKAWAY_OK;
    
    SetInformationJobObject(
      jobHandle.get(),
      JobObjectExtendedLimitInformation,
      &jobInfo,
      sizeof(jobInfo));
    
    AssignProcessToJobObject(jobHandle.get(), processHandle.get());
    
    // Wait till end of process
    WaitForSingleObject(processHandle.get(), INFINITE);

    // Get the exit code
    GetExitCodeProcess(processHandle.get(), &exitCode);
  }

  if (shimArgLog) {
    LOG() << "Shim Exiting: " << exitCode;
    LOG() << horizontal_line;
  }
  
  return exitCode;
}
