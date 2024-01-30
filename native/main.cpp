/**
 * Loads the standard library and executes the calcium code.
 * TODO: Pack linking this into one command.
*/

#include <stdio.h>
#include <cstdint>

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

extern DLLEXPORT int main();

struct Window;

struct Kernel { 
    const uint8_t * code;
    size_t size;
};

Window * createWindow(const char * title, uint32_t width, uint32_t height) {
    // TODO
    return nullptr;
}

void createDemoPipeline(Window * window, Kernel kernel) {
    // TODO
}

void update(Window * window) {
    // TODO
}

bool closed(Window * window) {
    // TODO
    return true;
}

void destroyWindow(Window * window) {
    // TODO
}