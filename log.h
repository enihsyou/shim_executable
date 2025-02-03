/**@file    LOG.H
 * @brief   A bare-bones logging library
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    2/2/2025
 *
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
 */
#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <filesystem>

using namespace std;

struct structlog {
  bool      headers =       true;       // Add headers
  int       level =         100;        // Log everything
  string    true_value =    "Yes";
  string    false_value =   "No";
};

structlog LOGCFG;

class LOG {
public:
  LOG(int level) {
    // Positive LEVEL - print out the header first
    msglevel = level >= 0 ? level : -level;
    if(LOGCFG.headers) 
      operator<<(getHeader(level));
  }

  // Same as LOG(0) w/o HEADER
  LOG() {}
  
  ~LOG() {
    if(opened) cerr << endl;
    opened = false;
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
  bool  opened =    false;
  int   msglevel =  0;
  
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
    opened = true;
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
};

#endif  /* LOG_H */
