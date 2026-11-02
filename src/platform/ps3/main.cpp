#include "core/Application.h"
#include "platform/ps3/PS3Input.h"

#include <sys/process.h>

// The PS3 loader reads this process record from the ELF.  The native PS3DK
// cellGcm renderer needs the larger PPU heap for command buffers and assets.
SYS_PROCESS_PARAM(1001, 0x200000)

int main() {
    pdv::PS3Input input;
    pdv::Application app(input);
    if (!app.initialize()) return 1;
    app.run();
    app.shutdown();
    return 0;
}
