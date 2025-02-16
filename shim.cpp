#include <shim_template.h>

// Console Application Entry Point
int wmain(int argc, wchar_t* argv[]) {
  return shim();
}

// GUI Application Entry Point
int APIENTRY wWinMain(
    HINSTANCE hInst, HINSTANCE hInstPrev,
    PWSTR pCmdLine, int nCmdShow) {
  return shim();
}
