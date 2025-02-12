#include <shim_template.h>

int APIENTRY wWinMain(
    HINSTANCE hInst, HINSTANCE hInstPrev,
    PWSTR pCmdLine, int nCmdShow) {
  return shim();
}
