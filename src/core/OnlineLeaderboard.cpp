#include "core/OnlineLeaderboard.h"
#include "core/StringUtils.h"

#include "core/StoragePaths.h"

#include <cstdlib>
#include <mutex>

#if defined(PD_PLATFORM_WINDOWS)
#include <windows.h>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")
#include <chrono>
#include <cstdio>
#include <sstream>
#endif

#ifdef __vita__
#include <curl/curl.h>

#include <chrono>
#include <cstdio>
#include <sstream>

#include <psp2/net/http.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/sysmodule.h>
#endif

namespace pdv {
#if defined(PD_PLATFORM_WINDOWS)
namespace {
struct WindowsProfile { std::string device; std::string name; };
WindowsProfile windowsProfile;
std::once_flag windowsProfileOnce;
std::wstring toWide(const std::string& value) { return std::wstring(value.begin(), value.end()); }
void loadWindowsProfile() {
    const std::string path = std::string(PD_USERDATA_ROOT) + "online_profile.dat";
    FILE* file = std::fopen(path.c_str(), "rb");
    char device[33]{}; char name[17]{};
    if (file && std::fread(device, 1, 32, file) == 32 && std::fread(name, 1, 16, file) == 16) {
        std::fclose(file); windowsProfile.device = device; windowsProfile.name = name;
        if (!windowsProfile.device.empty() && !windowsProfile.name.empty()) return;
    } else if (file) std::fclose(file);
    const auto seed = static_cast<unsigned long long>(std::chrono::steady_clock::now().time_since_epoch().count());
    char generated[33]{}; snprintf(generated, sizeof(generated), "PDW%013llX", seed & 0x1FFFFFFFFFFFFull);
    windowsProfile.device = generated; windowsProfile.name = std::string("PC-") + windowsProfile.device.substr(windowsProfile.device.size() - 4);
    file = std::fopen(path.c_str(), "wb"); if (!file) return;
    char storedName[17]{}; snprintf(storedName, sizeof(storedName), "%s", windowsProfile.name.c_str());
    std::fwrite(generated, 1, sizeof(generated), file); std::fwrite(storedName, 1, sizeof(storedName), file); std::fclose(file);
}
const WindowsProfile& currentWindowsProfile() { std::call_once(windowsProfileOnce, loadWindowsProfile); return windowsProfile; }
std::string heroNameWindows(HeroClass hero) { static const char* names[] = {"Warrior", "Mage", "Rogue", "Huntress"}; const int i = static_cast<int>(hero); return i >= 0 && i < 4 ? names[i] : "Warrior"; }
OnlineLeaderboardResult windowsRequest(const std::string& query) {
    OnlineLeaderboardResult result;
    HINTERNET session = WinHttpOpen(L"PixelDungeonConsoleEdition/1.0", WINHTTP_ACCESS_TYPE_NO_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) { result.message = "Network unavailable"; return result; }
    WinHttpSetTimeouts(session, 5000, 5000, 5000, 10000);
    HINTERNET connection = WinHttpConnect(session, L"homeboxbyxqspx.ru", INTERNET_DEFAULT_HTTPS_PORT, 0);
    const std::wstring path = L"/server/pixel_dungeon_leaderboard.php?" + toWide(query);
    HINTERNET request = connection ? WinHttpOpenRequest(connection, L"POST", path.c_str(), nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE) : nullptr;
    if (request) {
        // The home server uses a certificate chain that may not be present in
        // older Windows installations. The host is fixed in this executable.
        DWORD securityFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
                              SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE;
        WinHttpSetOption(request, WINHTTP_OPTION_SECURITY_FLAGS, &securityFlags, sizeof(securityFlags));
    }
    if (!request || !WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) || !WinHttpReceiveResponse(request, nullptr)) {
        result.message = "Online service unavailable";
    } else {
        DWORD status = 0, size = sizeof(status); WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX);
        std::string body; DWORD available = 0;
        while (WinHttpQueryDataAvailable(request, &available) && available) { const std::size_t old = body.size(); body.resize(old + available); DWORD read = 0; if (!WinHttpReadData(request, &body[old], available, &read)) break; body.resize(old + read); }
        if (status != 200 || body.rfind("OK\n", 0) != 0) result.message = "Server rejected request";
        else { result.ok = true; result.message = "Record sent"; std::istringstream lines(body.substr(3)); std::string line;
            while (std::getline(lines, line)) { std::istringstream fields(line); OnlineLeaderboardEntry entry; std::string n;
                if (!std::getline(fields, entry.name, '\t') || !std::getline(fields, entry.hero, '\t') || !std::getline(fields, n, '\t')) continue;
                entry.score = std::atoi(n.c_str()); if (std::getline(fields,n,'\t')) entry.depth=std::atoi(n.c_str()); if (std::getline(fields,n,'\t')) entry.kills=std::atoi(n.c_str()); if (std::getline(fields,n,'\t')) entry.gold=std::atoi(n.c_str()); if (std::getline(fields,n,'\t')) entry.duration=std::atoi(n.c_str()); result.entries.push_back(entry); }
        }
    }
    if (request) WinHttpCloseHandle(request); if (connection) WinHttpCloseHandle(connection); WinHttpCloseHandle(session); return result;
}
}
const std::string& OnlineLeaderboard::playerName() { return currentWindowsProfile().name; }
void OnlineLeaderboard::initialize() {}
OnlineLeaderboardResult OnlineLeaderboard::fetch() { return windowsRequest(std::string("action=list&device=") + currentWindowsProfile().device); }
OnlineLeaderboardResult OnlineLeaderboard::submit(const RunStatistics& run) { const int score = run.gold + run.kills * 10 + run.depth * 100; const WindowsProfile& p = currentWindowsProfile(); std::ostringstream q; q << "action=submit&device=" << p.device << "&name=" << p.name << "&hero=" << heroNameWindows(run.heroClass) << "&score=" << score << "&depth=" << run.depth << "&kills=" << run.kills << "&gold=" << run.gold << "&duration=" << static_cast<int>(run.duration); return windowsRequest(q.str()); }

#elif !defined(__vita__)

const std::string& OnlineLeaderboard::playerName() {
    static const std::string name = "VITA-PLAYER";
    return name;
}

void OnlineLeaderboard::initialize() {}

OnlineLeaderboardResult OnlineLeaderboard::fetch() {
    return {false, "Online leaderboard is available on PS Vita", {}};
}

OnlineLeaderboardResult OnlineLeaderboard::submit(const RunStatistics&) {
    return {false, "Online leaderboard is available on PS Vita", {}};
}

#else
namespace {

constexpr const char* Endpoint = "https://homeboxbyxqspx.ru/server/pixel_dungeon_leaderboard.php";
constexpr const char* ProfileFile = "online_profile.dat";

struct OnlineProfile { std::string device; std::string name; };

// std::call_once relies on the libstdc++ threading backend, which is not
// available on Vita. These flags are set on the main thread before an SDL
// worker is started and are read-only afterwards.
bool initialized = false;
bool profileInitialized = false;
OnlineProfile profile;

size_t receive(char* data, size_t size, size_t count, void* output) {
    static_cast<std::string*>(output)->append(data, size * count);
    return size * count;
}

void initializeNetwork() {
#ifdef __vita__
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
    static unsigned char netMemory[4 * 1024 * 1024];
    SceNetInitParam net{};
    net.memory = netMemory;
    net.size = sizeof(netMemory);
    sceNetInit(&net);
    sceNetCtlInit();
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    sceHttpInit(4 * 1024 * 1024);
#endif
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string profilePath() { return std::string(PD_USERDATA_ROOT) + ProfileFile; }

void loadProfile() {
    FILE* file = std::fopen(profilePath().c_str(), "rb");
    char device[33]{};
    char name[17]{};
    if (file && std::fread(device, 1, 32, file) == 32 && std::fread(name, 1, 16, file) == 16) {
        std::fclose(file);
        profile.device = device;
        profile.name = name;
        if (!profile.device.empty() && !profile.name.empty()) return;
    } else if (file) {
        std::fclose(file);
    }
    const auto seed = static_cast<unsigned long long>(std::chrono::steady_clock::now().time_since_epoch().count());
    char generated[33]{};
    snprintf(generated, sizeof(generated), "PDV%013llX", seed & 0x1FFFFFFFFFFFFull);
    profile.device = generated;
    profile.name = std::string("VITA-") + profile.device.substr(profile.device.size() - 4);
    file = std::fopen(profilePath().c_str(), "wb");
    if (!file) return;
    std::fwrite(generated, 1, sizeof(generated), file);
    char storedName[17]{};
    snprintf(storedName, sizeof(storedName), "%s", profile.name.c_str());
    std::fwrite(storedName, 1, sizeof(storedName), file);
    std::fclose(file);
}

const OnlineProfile& currentProfile() {
    if (!profileInitialized) {
        loadProfile();
        profileInitialized = true;
    }
    return profile;
}

std::string heroName(HeroClass hero) {
    static const char* names[] = {"Warrior", "Mage", "Rogue", "Huntress"};
    const int index = static_cast<int>(hero);
    return index >= 0 && index < 4 ? names[index] : "Warrior";
}

OnlineLeaderboardResult request(const std::string& query) {
    OnlineLeaderboardResult result;
    CURL* curl = curl_easy_init();
    if (!curl) { result.message = "Network unavailable"; return result; }
    std::string body;
    const std::string url = std::string(Endpoint) + "?" + query;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receive);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
#ifdef __vita__
    // VitaSDK's libcurl package does not ship a CA bundle with the game.
    // The endpoint is fixed in the executable and HTTPS still encrypts traffic.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif
    const CURLcode code = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);
    if (code != CURLE_OK) {
        result.message = "Network error " + numberToString(static_cast<int>(code));
        return result;
    }
    if (status != 200) {
        result.message = "Online service HTTP " + numberToString(status);
        return result;
    }
    if (body.rfind("OK\n", 0) != 0) {
        result.message = "Server rejected request";
        return result;
    }
    result.ok = true;
    result.message = "Record sent";
    std::istringstream lines(body.substr(3));
    std::string line;
    while (std::getline(lines, line)) {
        std::istringstream fields(line);
        OnlineLeaderboardEntry entry;
        std::string number;
        if (!std::getline(fields, entry.name, '\t') || !std::getline(fields, entry.hero, '\t') ||
            !std::getline(fields, number, '\t')) continue;
        entry.score = std::atoi(number.c_str());
        if (std::getline(fields, number, '\t')) entry.depth = std::atoi(number.c_str());
        if (std::getline(fields, number, '\t')) entry.kills = std::atoi(number.c_str());
        if (std::getline(fields, number, '\t')) entry.gold = std::atoi(number.c_str());
        if (std::getline(fields, number, '\t')) entry.duration = std::atoi(number.c_str());
        result.entries.push_back(entry);
    }
    return result;
}

std::string escape(CURL* curl, const std::string& value) {
    char* escaped = curl_easy_escape(curl, value.c_str(), static_cast<int>(value.size()));
    std::string output = escaped ? escaped : "";
    if (escaped) curl_free(escaped);
    return output;
}

} // namespace

const std::string& OnlineLeaderboard::playerName() { return currentProfile().name; }

void OnlineLeaderboard::initialize() {
    if (!initialized) {
        initializeNetwork();
        initialized = true;
    }
    (void)currentProfile();
}

OnlineLeaderboardResult OnlineLeaderboard::fetch() {
    return request(std::string("action=list&device=") + currentProfile().device);
}

OnlineLeaderboardResult OnlineLeaderboard::submit(const RunStatistics& run) {
    CURL* curl = curl_easy_init();
    if (!curl) return {false, "Network unavailable", {}};
    const OnlineProfile& user = currentProfile();
    std::ostringstream query;
    query << "action=submit&device=" << escape(curl, user.device)
          << "&name=" << escape(curl, user.name)
          << "&hero=" << heroName(run.heroClass)
          << "&score=" << (run.gold + run.kills * 10 + run.depth * 100)
          << "&depth=" << run.depth << "&kills=" << run.kills
          << "&gold=" << run.gold << "&duration=" << static_cast<int>(run.duration);
    curl_easy_cleanup(curl);
    return request(query.str());
}

#endif
} // namespace pdv
