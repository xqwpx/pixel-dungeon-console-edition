#include "core/Application.h"
#include "platform/VitaInput.h"

#ifdef __vita__
unsigned int _newlib_heap_size_user = 192 * 1024 * 1024;
#endif

int main() {
    pdv::VitaInput input;
    pdv::Application app(input);

    if (!app.initialize()) {
        return -1;
    }

    app.run();
    app.shutdown();
    return 0;
}
