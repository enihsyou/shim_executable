// ------------------------- General Version Info -------------------------- //
#define STRR(X) #X              // to stringify file version 
#define STR(X) STRR(X)          // because one is just never enough  
#define VERSION_MAJOR           2
#define VERSION_MINOR           3
#define VERSION_PATCH           0
#define VER_FILEVERSION         VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH
#define VER_FILEVERSION_STR     STR(VERSION_MAJOR.VERSION_MINOR.VERSION_PATCH)
#define VER_PRODUCT             "Shim Executable"
#define VER_COPYRIGHT           "MIT License - John P. Hilbert (2025) | TheCakeIsNaOH (2021) | Grégoire Geis (2019)"


// ---------------------------- Shim Executable ---------------------------- // 
#define VER_FILENAME            "SHIM_EXEC"
#define VER_DESC                "Creates shortcut-like 'shims' for an executable"


// --------------------------------- Shim ---------------------------------- // 
#define VER_SHIM_FILENAME       "SHIM"
#define VER_SHIM_DESC           "An executable 'shim' created by " VER_FILENAME ".EXE"
