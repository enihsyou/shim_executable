// ------------------------------------------------------------------------- //
// Resource Functions                                                        // 
// ------------------------------------------------------------------------- //
/**@file    RESOURCE_FUNCTIONS.H
 * @brief   Methods for getting, adding, and copying resources
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

#ifndef RESOURCE_FUNCTIONS_H
#define RESOURCE_FUNCTIONS_H

// ------------------------------------------------------------------------- //
#include <string>
#include "log.h"


// ---------------------------- Read Resources ----------------------------- // 
bool HasResourceData(LPCSTR name) {
  if (FindResource(NULL, name, RT_RCDATA)) 
    return true;
  return false;
}

bool GetResourceData(LPCSTR name, wstring& arg) {
  // Get the resource handle if it exists 
  HRSRC     resource    = FindResource(NULL, name, RT_RCDATA);
  if (!resource) return false;
  
  // Load the data
  HGLOBAL   data        = LoadResource(NULL, resource);
  LPVOID    data_ptr    = LockResource(data);
  DWORD     data_size   = SizeofResource(NULL, resource);

  // Its assumed to be a WSTRING so convert
  arg = wstring((LPCWSTR)data_ptr, data_size / sizeof(WCHAR));
  return true;
}

bool GetResourceFile(string name, const filesystem::path& path) {
  // Get the resource handle if it exists 
  HRSRC     resource    = FindResource(NULL, name.c_str(), RT_RCDATA);
  if (!resource) return false;
  
  // Load the data
  HGLOBAL   data        = LoadResource(NULL, resource);
  LPVOID    data_ptr    = LockResource(data);
  DWORD     data_size   = SizeofResource(NULL, resource);

  // Create the file
  HANDLE    file        =
    CreateFileW(path.c_str(),
                GENERIC_WRITE | GENERIC_READ, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) return false;
  
  // Save the resource to it

  DWORD bytes_written   = 0;
  WriteFile(file, data_ptr, data_size, &bytes_written, NULL);
  CloseHandle(file);
  return true;
}


// ----------------------------- Add Resources ----------------------------- // 
BOOL AddResourceData (filesystem::path target, LPCSTR name, wstring arg) {
  HANDLE resource   = BeginUpdateResourceW(target.c_str(), FALSE);
  BOOL bUpdate      = UpdateResource(
      resource,                 // Handle
      RT_RCDATA,                // Resource Type
      name,                     // Resource Name
      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
      (LPVOID)arg.c_str(),      // Resource String
      arg.size() * sizeof(wchar_t));
  
  if (!bUpdate)
    LOG(1) << "Failed to add resource: " << name;
  else 
    LOG(3) << "Added resource: " << name << " = " << arg;

  EndUpdateResource(resource, FALSE);
  
  return bUpdate;
}


// ---------------------------- Copy Resources ----------------------------- //
HANDLE resource_handle;

BOOL CALLBACK enumLangsFunc(HMODULE hModule, LPCTSTR lpType, LPCTSTR lpName,
                   WORD wLang, LONG lParam) {
    
  HRSRC hRes =          FindResourceEx(hModule, lpType, lpName, wLang);
  HGLOBAL hResLoad =    LoadResource(hModule, hRes);
  LPVOID lpResLock =    LockResource(hResLoad);

  UpdateResource(resource_handle, lpType, lpName, wLang,
                 lpResLock,                        // ptr to resource info
                 SizeofResource(hModule, hRes));   // size of resource info

  string log_str = "Copied ";
  
  if (lpType == RT_ICON)
    log_str += "ICON ";
  else if (lpType == RT_VERSION)
    log_str += "VERSION ";
  else if (lpType == RT_GROUP_ICON)
    log_str += "ICON GROUP ";

  log_str += "resource ";
  if (!IS_INTRESOURCE(lpName))
    log_str += lpName;
  else
    log_str += to_string((USHORT)lpName);

  LOG(3) << log_str;

  return TRUE; 
}

BOOL CALLBACK enumNamesFunc(HMODULE hModule, LPCTSTR lpType, LPTSTR lpName,
                   LONG lParam) {
  EnumResourceLanguages(hModule, lpType, lpName, enumLangsFunc, 0);
  return TRUE;
}

BOOL CALLBACK enumTypesFunc(HMODULE hModule, LPTSTR lpType, LONG lParam) {
  // Only Copy Icons and Version Info
  if(lpType == RT_ICON || lpType == RT_VERSION || lpType == RT_GROUP_ICON)
    EnumResourceNames(hModule, lpType, enumNamesFunc, 0);
  return TRUE;
}

BOOL CopyResources(filesystem::path target, filesystem::path source) {
  resource_handle = BeginUpdateResourceW(target.c_str(), FALSE);
  
  HMODULE hExe =
    LoadLibraryExW(source.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
  
  if (!hExe) {
    LOG(1) << "Could not open " << source;
    return false;
  }

  EnumResourceTypes(hExe, enumTypesFunc, 0);

  EndUpdateResource(resource_handle, FALSE);
  
  if (!FreeLibrary(hExe)){ 
    LOG(2) << "Could not free application library";
    return false;
  }

  return true;
}  


// ------------------------------------------------------------------------- //
#endif  /* RESOURCE_FUNCTIONS_H */
