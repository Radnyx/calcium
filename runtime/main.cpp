/**
 * Loads the standard library and executes the calcium code.
 * TODO: Pack linking this into one command.
*/

#include <stdio.h>
#include <cstdint>
#include "window.h"

#ifdef _WIN32
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern DLLEXPORT int main();