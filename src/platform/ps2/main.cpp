#include "core/Application.h"
#include "platform/ps2/PS2Input.h"

extern "C" {
#include <debug.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
}

int main() {
    SifInitRpc(0);
    SifLoadModule("rom0:SIO2MAN", 0, nullptr);
    SifLoadModule("rom0:PADMAN", 0, nullptr);
    pdv::PS2Input input;
    pdv::Application app(input);
    if (!app.initialize()) {
        // Do not silently return to the BIOS when an early startup stage fails.
        // This screen is also visible when PCSX2's IOP console is disabled.
        init_scr();
        scr_printf("PIXEL DUNGEON PS2\n\nStartup failed:\n%s\n", app.initializationError());
        while (true) SleepThread();
    }
    app.run();
    app.shutdown();
    return 0;
}
