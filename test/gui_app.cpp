#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#pragma comment(lib, "SHELL32.LIB")
#pragma comment(lib, "user32.lib")

using namespace std;
using namespace filesystem;

path GetExecPath() {
  CHAR cExePath[MAX_PATH];  
  GetModuleFileName(NULL, cExePath, MAX_PATH);
  return path(cExePath);
}


int WINAPI WinMain(
   _In_ HINSTANCE       hInstance,
   _In_opt_ HINSTANCE   hPrevInstance,
   _In_ LPSTR           lpCmdLine,
   _In_ int             nCmdShow
                   ) {
  string exec_name      = GetExecPath().stem().string();
  string exec_dir       = GetExecPath().parent_path().string();

  string message_text   = "";
  message_text =  "EXE NAME:\t'" + exec_name + "'\n";
  message_text += "EXE DIR:\t\t'" + exec_dir + "'\n";
  message_text += "CUR DIR:\t\t'" + current_path().string() + "'\n\n";
  message_text += "CMD LINE:\t'" + (string)GetCommandLineA() + "'\n";

  MessageBox(NULL, message_text.c_str(), "GUI APP", MB_OK);
  return 0;
}
