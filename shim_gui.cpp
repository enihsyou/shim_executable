#include "shim_template.h"

// ------------------------------------------------------------------------- //
// MAIN METHOD                                                               // 
// ------------------------------------------------------------------------- //
int APIENTRY wWinMain(
    HINSTANCE hInst, HINSTANCE hInstPrev,
    PWSTR pCmdLine, int nCmdShow) {
  return shim();
}
