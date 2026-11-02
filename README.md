# Pixel Dungeon Console Edition

Native C++ port of **Pixel Dungeon** for classic consoles and handhelds.

The project recreates the game's systems in portable C++ and uses platform
backends for input, rendering, audio and packaging.  It is based on the
open-source Pixel Dungeon by Oleg Dolya.

## Platforms

| Platform | Build entry point |
| --- | --- |
| Windows | `build_pc.bat` |
| PlayStation Vita / PS TV | `build_vpk_psvita.bat` |
| PlayStation Portable | CMake target `psp` |
| PlayStation 1 | `build_ps1.bat` |
| PlayStation 2 | `build_ps2.bat` |
| PlayStation 3 / PS3DK | `build_ps3dk.bat` |

## Build prerequisites

The project is built with CMake and each console target requires its own SDK
and libraries.  The Windows batch files expect the corresponding toolchain to
be installed and configured locally.  Review the relevant build script before
running it, then use the entry point from the table above.

## Assets

This repository deliberately contains **no graphic or audio assets**.  To
build a runnable game, place only assets that you are authorised to use in the
`assets/` directory.  Attribution for the original project is preserved in
[NOTICE_UPSTREAM.md](NOTICE_UPSTREAM.md).

## Repository layout

```text
src/       Game code and platform backends
cmake/     CMake modules and toolchain files
configs/   Platform metadata
tests/     Host-side tests
tools/     Source code for supporting build tools
```

Generated output, SDKs, editor files and third-party assets are intentionally
not tracked.

## Licence and attribution

Pixel Dungeon Console Edition is distributed under the
[GNU General Public License v3.0](LICENSE) (`GPL-3.0-only`).

The project incorporates and ports code from Pixel Dungeon and uses additional
third-party components.  Their copyright notices and licences are retained in
[NOTICE_UPSTREAM.md](NOTICE_UPSTREAM.md) and in the relevant component
directories.  See [COPYRIGHT.md](COPYRIGHT.md) for project-level attribution.

## Status

This is an active work in progress.  Contributions, portability fixes and
platform testing reports are welcome.
