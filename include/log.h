// ------------------------------------------------------------------------- //
// Simple Logging                                                            // 
// ------------------------------------------------------------------------- //
/**@file    LOG.H
 * @brief   A bare-bones logging library
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    2/2/2025
 *
 * ------------------------------------------------------------------------- 
 * Was considering https://github.com/SergiusTheBest/plog but opted for size
 * over functionality. 
 *
 * Modified from:
 *      Source: https://stackoverflow.com/a/32262143/3140099
 *      Author: Alberto Lepe <dev@alepe.com>
 *      Create: 2015-12-01, 6:00 PM
 *
 * Added:
 *  - indent instead of header
 *  - automatic conversion of FILESYSTEM::PATH objects
 *  - automatic conversion of BOOL
 *  - automatic attaching to console or stream to file
 *
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
#ifndef LOG_H
#define LOG_H

// ------------------------------------------------------------------------- //
#include <iostream>
#include <fstream>
#include <filesystem>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


using namespace std;

struct structlog {
  bool      headers =       true;       // Add headers
  int       level =         100;        // Log everything
  string    true_value =    "Yes";
  string    false_value =   "No";
  string    file_ext =      ".log";
  filesystem::path log_file;
};

structlog LOGCFG;

class LOG {
public:
  LOG(int level) {
    openStream();
    
    // Positive LEVEL - print out the header first
    msglevel = level >= 0 ? level : -level;
    if(LOGCFG.headers) 
      operator<<(getHeader(level));
  }

  // Same as LOG(0) w/o HEADER
  LOG() {
    openStream();
  }
  
  ~LOG() {
    closeStream();
  }
  
  template <class T>
  LOG &operator<<(T const & msg) {
    if(msglevel > LOGCFG.level)
      return *this;

    // String
    if constexpr ( is_same_v<T, string> ) {
      return printString(msg);
    }

    // Character
    if constexpr ( is_convertible_v<T, char const *>) {
      string cmsg(msg);
      return printString(cmsg);
    }

    // Wide String
    if constexpr ( is_same_v<T, wstring> ) {
      return printWString(msg);
    }

    // Wide Character
    if constexpr ( is_convertible_v<T, wchar_t const *> ) {
      wstring wmsg(msg);
      return printWString(wmsg);
    }

    // Path
    if constexpr ( is_same_v<T, filesystem::path> ) {
      return printString("'" + msg.string() + "'");
    }

    // Boolean
    if constexpr ( is_same_v<T, bool> ) {
      return printString(msg ? LOGCFG.true_value : LOGCFG.false_value);
    }

    return printString("[could not output variable]");
  }
  
  
private:
  int   msglevel =      0;
  ofstream stream;
  DWORD stream_type =   0;
  // 0 - not open
  // 1 - console app                  - write to console
  // 2 - windows app  - found console - write to console
  // 3 - windows app  - no console    - write to file
  
  inline string getHeader(int level) {
    return
      level == 1 ? "ERROR - " :
      level == 2 ? "WARN  - " :
      level == 3 ? "INFO  - " :
      level == 4 ? "DEBUG - " :
      "        ";
  }

  // ---------- Print String ---------- // 
  LOG &printString(const string msg) {
    cerr << msg;
    return *this;
  }
  
  // ---------- Print Wide String ---------- // 
  LOG &printWString(const wstring msg) {
    if(msg.empty())
      return *this;
    
    int sz = WideCharToMultiByte(CP_UTF8, 0, &msg[0], (int)msg.size(),
                                 0, 0, 0, 0);
  
    string converted_msg(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, &msg[0], (int)msg.size(),
                        &converted_msg[0], sz, 0, 0);

    return printString(converted_msg);
  }

  
  void openStream() {
    if(stream_type > 0)
      return;

    stream_type = 1;
  
    if(AttachConsole(ATTACH_PARENT_PROCESS)) {
      stream_type = 2;
      stream = ofstream("CONOUT$", ios::out);
      stream << endl;        // don't remember why
    }
    else if(GetLastError() == ERROR_INVALID_HANDLE) {
      stream_type = 3;
      if (LOGCFG.log_file.empty())
        setLogFile();
      stream.open(LOGCFG.log_file, ios::out | ios::app);
    }
    else 
      return;

    cerr.rdbuf(stream.rdbuf());
  }

  
  void closeStream() {
    if(stream_type == 0)
      return;

    operator<<("\n");

    // Cleanup and Close the Stream
    if(stream_type == 2) {
      // Mimics a console application to some extent...
      // ...from cmd.exe console, appears to add an extra new line
      // ...from powershell.exe, does not move the cursor (returns to position
      // prior to any output)
      DWORD entityWritten;
      INPUT_RECORD inputs;
      inputs.EventType = KEY_EVENT;
      inputs.Event.KeyEvent.bKeyDown = TRUE;
      inputs.Event.KeyEvent.dwControlKeyState = 0;
      inputs.Event.KeyEvent.uChar.UnicodeChar = '\r';
      inputs.Event.KeyEvent.wRepeatCount = 1;
      // inputs.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
      // inputs.Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, 0);
      WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE),
                        &inputs, 1, &entityWritten);
    
      FreeConsole();
    }
    else if(stream_type == 3) 
      stream.close();

    stream_type = 0;
  }


  void setLogFile() {
    wchar_t applicationPath[MAX_PATH];
    const auto applicationPathSize =
      GetModuleFileNameW(nullptr, applicationPath, MAX_PATH);
  
    LOGCFG.log_file = applicationPath;
    LOGCFG.log_file.replace_extension(LOGCFG.file_ext);
    return;
  }
  
};

// ------------------------------------------------------------------------- //
#endif // LOG_H
