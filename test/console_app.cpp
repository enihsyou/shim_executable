#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#pragma comment(lib, "SHELL32.LIB")

using namespace std;
using namespace filesystem;

path GetExecPath() {
  CHAR cExePath[MAX_PATH];  
  GetModuleFileName(NULL, cExePath, MAX_PATH);
  return path(cExePath);
}

int main() {
  string exec_name      = GetExecPath().stem().string();
  string exec_dir       = GetExecPath().parent_path().string();

  cout << "EXE NAME:  '" << exec_name << "'\n";
  cout << "EXE DIR:   '" << exec_dir << "'\n";
  cout << "CUR DIR:   '" << current_path().string() << "'\n\n";
  cout << "CMD LINE:  '" << GetCommandLineA() << "'\n";

  system("pause");
  return 0;
}

