#!/usr/bin/env python3
"""PIXEL DUNGEON CONSOLE EDITION build frontend (CMake is the build authority)."""
import argparse, json, os, shutil, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CONFIGS = ROOT / "configs" / "platforms"
ACTIVE = {"windows", "psvita", "psp"}

def platform_data():
    return [json.loads(p.read_text(encoding="utf-8")) for p in sorted(CONFIGS.glob("*.json"))]
def build_dir(platform, config): return ROOT / "build" / platform / config
def windows_executable(config):
    root = build_dir("windows", config)
    direct = root / "PIXEL DUNGEON CONSOLE EDITION.exe"
    return direct if direct.exists() else root / config.capitalize() / "PIXEL DUNGEON CONSOLE EDITION.exe"
def run(cmd, **kw): return subprocess.run(cmd, cwd=ROOT, check=False, **kw).returncode
def require_tool(name):
    if not shutil.which(name): print(f"ERROR: {name} was not found in PATH."); return False
    return True
def configure(a):
    if a.platform not in ACTIVE: print(f"ERROR: {a.platform} is planned; only Windows, PS Vita and PSP are active."); return 2
    if not require_tool("cmake"): return 2
    cmd=["cmake","-S",str(ROOT),"-B",str(build_dir(a.platform,a.config)),f"-DPD_PLATFORM={a.platform}",f"-DCMAKE_BUILD_TYPE={a.config.capitalize()}"]
    if a.platform == "windows":
        vcpkg_root = os.environ.get("VCPKG_ROOT")
        if vcpkg_root:
            toolchain = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
            if toolchain.is_file():
                cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
            else:
                print(f"WARNING: VCPKG_ROOT is set but no vcpkg toolchain was found: {toolchain}")
    if a.platform == "psvita":
        vita=os.environ.get("VITASDK")
        if not vita: print("ERROR: VITASDK is not set."); return 2
        cmd.insert(1, f"-DCMAKE_TOOLCHAIN_FILE={Path(vita) / 'share' / 'vita.toolchain.cmake'}")
    if a.platform == "psp":
        pspdev=os.environ.get("PSPDEV")
        if not pspdev: print("ERROR: PSPDEV is not set."); return 2
        cmd.insert(1, f"-DCMAKE_TOOLCHAIN_FILE={ROOT / 'cmake' / 'toolchains' / 'psp.cmake'}")
    return run(cmd)
def build(a):
    code=configure(a)
    if code: return code
    command=["cmake","--build",str(build_dir(a.platform,a.config))]
    # vcpkg's app-local deployment writes runtime DLLs into one Visual Studio
    # output directory. Building every test target in parallel races there.
    if a.platform == "windows": command += ["--target","PixelDungeonConsoleEdition"]
    else: command += ["--parallel"]
    return run(command)
def clean(a):
    path=build_dir(a.platform,a.config)
    if path.exists(): shutil.rmtree(path)
    print(f"Removed {path.relative_to(ROOT)}")
    return 0
def main():
    p=argparse.ArgumentParser(prog="pd.py"); sub=p.add_subparsers(dest="cmd",required=True)
    for name in ("configure","build","rebuild","clean","run","test","package"):
        q=sub.add_parser(name); q.add_argument("--platform",default="windows"); q.add_argument("--config",default="debug",choices=("debug","release")); q.add_argument("--build",action="store_true")
    select=sub.add_parser("platform"); select.add_argument("id", choices=sorted(d["id"] for d in platform_data()))
    sub.add_parser("platforms"); sub.add_parser("status"); sub.add_parser("doctor"); sub.add_parser("validate")
    a=p.parse_args()
    if a.cmd in ("platforms","status"):
        print("PIXEL DUNGEON CONSOLE EDITION Platforms\n\nACTIVE\n------")
        for d in platform_data():
            if d["status"] == "development": print(f"{d['name']}\nStatus: DEVELOPMENT\nSDK: {d['sdk']}\n")
        print("PLANNED STANDARD\n----------------")
        for d in platform_data():
            if d["status"] == "planned" and d["profile"] == "standard": print(d["name"])
        print("\nPLANNED TINY\n------------")
        for d in platform_data():
            if d["profile"] == "tiny": print(d["name"])
        return 0
    if a.cmd == "platform":
        data=next(d for d in platform_data() if d["id"] == a.id)
        print(json.dumps(data, indent=2)); return 0
    if a.cmd == "doctor":
        print("Python:", sys.executable); print("CMake:", shutil.which("cmake") or "NOT FOUND"); print("VITASDK:", os.environ.get("VITASDK", "NOT SET")); return 0
    if a.cmd == "validate":
        banned=("psp2/","scekernel","scectrl","sdl","windows.h","libnx","libogc","ps2sdk","psl1ght","kallistios","jni")
        failures=[]
        for path in (ROOT/"src"/"game").rglob("*.[ch]pp"):
            text=path.read_text(encoding="utf-8",errors="ignore")
            if any(token.lower() in text.lower() for token in banned): failures.append(str(path.relative_to(ROOT)))
        print("Core platform-boundary validation " + ("FAILED: " + ", ".join(failures) if failures else "passed")); return 1 if failures else 0
    if a.cmd == "configure": return configure(a)
    if a.cmd == "build": return build(a)
    if a.cmd == "rebuild": return clean(a) or build(a)
    if a.cmd == "clean": return clean(a)
    if a.cmd == "test":
        if configure(a): return 1
        if run(["cmake","--build",str(build_dir(a.platform,a.config))]): return 1
        return run(["ctest","--test-dir",str(build_dir(a.platform,a.config)),"--output-on-failure"])
    if a.cmd == "package":
        if build(a): return 1
        source=build_dir(a.platform,a.config)
        target=ROOT / "dist" / a.platform
        target.mkdir(parents=True, exist_ok=True)
        if a.platform == "psvita":
            package_file=source / "PixelDungeonConsoleEdition.vpk"
            if not package_file.exists(): print("ERROR: Vita VPK was not produced."); return 1
            shutil.copy2(package_file, target / package_file.name)
            print(f"Packaged: {target / package_file.name}"); return 0
        if a.platform == "windows":
            executable=windows_executable(a.config)
            if not executable.exists(): print("ERROR: Windows executable was not produced."); return 1
            shutil.copy2(executable, target / executable.name)
            for dll in executable.parent.glob("*.dll"): shutil.copy2(dll, target / dll.name)
            shutil.copytree(source / "assets", target / "assets", dirs_exist_ok=True)
            # Save data is deliberately kept beside the portable game rather
            # than in a system profile.  Create it even for a fresh package so
            # every save-related file has a writable destination immediately.
            (target / "userdata").mkdir(parents=True, exist_ok=True)
            print(f"Packaged: {target}"); return 0
        return 2
    if a.cmd == "run":
        exe=windows_executable(a.config) if os.name == "nt" else build_dir("windows",a.config)/"PixelDungeon"
        if not exe.exists() and not a.build: print("Build is missing. Re-run with --build or use play_pc.bat."); return 2
        if not exe.exists() and build(a): return 1
        return subprocess.run([str(exe)],cwd=exe.parent).returncode
if __name__ == "__main__": sys.exit(main())
