#include "core/Application.h"
#include "platform/psp/PSPInput.h"

#include <cstdio>

extern "C" {
#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdebug.h>
#include <pspthreadman.h>
}

PSP_MODULE_INFO("Pixel Dungeon", PSP_MODULE_USER, 1, 70);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU);
// Application owns the dungeon scene graph on the main thread.  The default
// PSPSDK stack is too small once procedural level generation allocates its
// temporary map buffers, which made a real PSP return to XMB on New Game.
PSP_MAIN_THREAD_STACK_SIZE_KB(1024);
PSP_HEAP_SIZE_KB(18000);

namespace {
void writeBootLog(const char* message) {
    FILE* file = std::fopen("ms0:/PSP/GAME/PixelDungeon/psp_boot.log", "wb");
    if (file == nullptr) return;
    std::fputs(message, file);
    std::fclose(file);
}
}

namespace {
int runGame(SceSize, void*) {
    writeBootLog("starting\n");
    pdv::PSPInput input;
    pdv::Application app(input);
    if (!app.initialize()) {
        char message[256];
        std::snprintf(message, sizeof(message), "initialization failed: %s\n", app.initializationError());
        writeBootLog(message);
        pspDebugScreenInit();
        pspDebugScreenPrintf("PIXEL DUNGEON PSP\n\nStartup failed:\n%s\n\nPress START to exit.",
                              app.initializationError());
        while (true) {
            SceCtrlData pad{};
            sceCtrlPeekBufferPositive(&pad, 1);
            if ((pad.Buttons & PSP_CTRL_START) != 0) break;
            sceKernelDelayThread(10000);
        }
        return 1;
    }
    writeBootLog("initialized\n");
    app.run();
    app.shutdown();
    return 0;
}
}

int main() {
    return runGame(0, nullptr);
}
