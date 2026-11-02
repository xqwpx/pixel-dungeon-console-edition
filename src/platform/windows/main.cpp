#include "core/Application.h"
#include "platform/windows/WindowsInput.h"

#include <windows.h>

int main() {
    // Prevent two independent dungeon windows (and two writers for the same
    // save file) from being started accidentally.
    HANDLE singleInstance = CreateMutexW(nullptr, TRUE, L"PixelDungeonConsoleEdition.SingleInstance");
    if (!singleInstance || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (singleInstance) CloseHandle(singleInstance);
        return 0;
    }

    pdv::WindowsInput input;
    pdv::Application app(input);
    const int result = app.initialize() ? (app.run(), app.shutdown(), 0) : 1;
    CloseHandle(singleInstance);
    return result;
}
