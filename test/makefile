CPPFLAGS = -nologo -std:c++17 -DNDEBUG -MD -O2 -GF -GR- -GL -EHsc

all: gui_app.exe console_app.exe cleanup

.SILENT:

console_app.exe: $*.cpp
	$(CPP) $(CPPFLAGS) $*.cpp

gui_app.exe: $*.cpp
	$(CPP) $(CPPFLAGS) $*.cpp

cleanup: 
	echo Removing intermediate files
	rm *.obj
