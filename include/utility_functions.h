// ------------------------------------------------------------------------- //
// General Methods Used Across the Apps                                      // 
// ------------------------------------------------------------------------- //
/**@file    UTILITY_FUNCTIONS.H    
 * @brief   General methods used here and there
 * @author  John P. Hilbert
 * @email   jphilbert@gmail.com
 * @date    2/6/2025
 *
 * -------------------------------------------------------------------------
 * Defines the following:
 *
 *  horizontal_line_bold
 *      - a string of "=" x 79
 *
 *  horizontal_line
 *      - a string of "-" x 79
 *
 *  UnquoteString
 *      (just noticed this is bad choice of name) this un-escapes quotes,
 *      i.e., simply replacing /" with ". Used when command line arguments
 *      are given to be stored inside the shim since the argument must be
 *      quoted hence internal quotes must be escaped.
 *          
 *  TrimQuotes
 *      removes quotes. GetArgument methods will return input verbatim so need
 *      to remove quotes for paths and such.
 *      ??? Should this just be automatic ???
 *          
 *  UpperCase
 *      upper cases a string
 *  
 *  NarrowString
 *      converts a wstring -> string
 *  
 *  GetExecPath
 *      gets the path of the executable
 *  
 * ------------------------------------------------------------------------- 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ------------------------------------------------------------------------- */

#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

// ------------------------------------------------------------------------- //
#include <windows.h>
#include <vector>
#include <string>
#include <filesystem>

using namespace std;

const string horizontal_line_bold(79, '=');
const string horizontal_line(79, '-');

wstring UnquoteString(const wstring& input) {
  wstring output;

  // Reserve space to avoid frequent re-allocations
  output.reserve(input.size());
  
  for (size_t i = 0; i < input.length(); i++) {
    if (i + 1 < input.length() &&
        input[i] == '\\' && input[i + 1] == '"') {
      output += '"';
      i++;
    }
    else
      output += input[i];
  }

  return output;
}


bool TrimQuotes(wstring& s) {
  if(s.front() == '"' && s.back() == '"') {
    s.erase(s.begin());
    s.erase(s.end()-1);
    return true;
  }
  return false;
}


bool UpperCase(wstring& s) {
  transform(s.begin(), s.end(), s.begin(), ::toupper);
  return true;
}


// Convert Wide --> Narrow
string NarrowString(const wstring& wstr) {
  if (wstr.empty())
    return string();
  
  int sz = WideCharToMultiByte(
      CP_UTF8, 0, &wstr[0], (int)wstr.size(),
      0, 0, 0, 0);
  
  string res(sz, 0);
  WideCharToMultiByte(
      CP_UTF8, 0, &wstr[0], (int)wstr.size(),
      &res[0], sz, 0, 0);
  
  return res;
}


filesystem::path GetExecPath() {
  CHAR cExePath[MAX_PATH];  
  GetModuleFileName(NULL, cExePath, MAX_PATH);
  return filesystem::path(cExePath);
}  


// ------------------------------------------------------------------------- //
#endif // UTILITY_FUNCTIONS_H
