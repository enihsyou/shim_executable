CPPFLAGS = -nologo -std:c++17 -DNDEBUG -MD -O2 -GF -GR- -GL -EHsc -I include
RCFLAGS = -nologo -I include
LINKFLAGS = -nologo -LTCG shim.obj shim.res
HEADERS = include\*.h 
SHIMS = shim_gui.exe shim_console.exe

all: shim_executable.exe cleanup

.SILENT:


# ---------------------------------- SHIMS ----------------------------------- #
shim.res: shim.rc include/version.h
	echo Compiling shim.res
	$(RC) $(RCFLAGS)  $*.rc

shim.obj: shim.cpp
	echo Compiling shim.cpp
	$(CPP) $(CPPFLAGS) -c shim.cpp

shim_console.exe: shim.res shim.obj
	echo Building $*.exe
	link -out:$*.exe -SUBSYSTEM:CONSOLE $(LINKFLAGS)
	echo.

shim_gui.exe: shim.res shim.obj
	echo Building $*.exe
	link -out:$*.exe -SUBSYSTEM:WINDOWS $(LINKFLAGS)
	echo.


# ----------------------------- Main Application ----------------------------- #
shim_executable.exe: $*.cpp $*.rc $(SHIMS)
	echo Building $*.exe
	$(RC) $(RCFLAGS) $*.rc
	$(CPP) $(CPPFLAGS) $*.cpp $*.res
	echo.


# --------------------------- Post Build Clean-Up ---------------------------- #
cleanup:
	echo Removing intermediate files
	del *.obj
	del *.res
	del $(SHIMS)

	echo Created checksum
	certutil -hashfile shim_executable.exe SHA256 | findstr /R "[0-9][a-z]" > shim_executable.sha256

	echo Renamed and moved to .\bin
	if not exist bin mkdir bin
	if exist shim_executable.exe    move shim_executable.exe    .\bin\shim_exec.exe
	if exist shim_executable.sha256 move shim_executable.sha256 .\bin\shim_exec.sha256