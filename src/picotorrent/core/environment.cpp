#include "environment.hpp"

#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>

#include <loguru.hpp>

#include "utils.hpp"

namespace fs = std::filesystem;
using pt::Core::Environment;

Environment::Environment()
    : m_startupTime(std::chrono::system_clock::now())
{
}

std::shared_ptr<Environment> Environment::Create()
{
    auto env = new Environment();
    loguru::add_file(
        Utils::toStdString(env->GetLogFilePath().generic_wstring()).c_str(),
        loguru::Truncate,
        loguru::Verbosity_INFO);

    LOG_F(INFO, "PicoTorrent starting up...");

    return std::shared_ptr<Environment>(env);
}

fs::path Environment::GetApplicationDataPath()
{
    if (IsInstalled() || IsAppContainerProcess())
    {
        return fs::path(GetKnownFolderPath(KnownFolder::LocalAppData)) / "PicoTorrent";
    }

    return GetApplicationPath();
}

fs::path Environment::GetApplicationPath()
{
    TCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, ARRAYSIZE(path));
    PathRemoveFileSpec(path);

    return path;
}

std::string Environment::GetCrashpadReportUrl()
{
    if (const char* url = std::getenv("PICOTORRENT_CRASHPAD_URL"))
    {
        return url;
    }

    return "https://o314884.ingest.sentry.io/api/1794272/minidump/?sentry_key=722163e5427c4a8892d2c158b5ffe954";
}

fs::path Environment::GetCoreDbFilePath()
{
    return GetApplicationPath() / "coredb.sqlite";
}

std::string Environment::GetCurrentLocale()
{
    TCHAR loc[512];
    int res = GetUserDefaultLocaleName(loc, 512);

    if (res == 0)
    {
        LOG_F(ERROR, "Failed to get current locale - defaulting to 'en'");
        return "en";
    }

    return Utils::toStdString(
        std::wstring(loc, res));
}

fs::path Environment::GetDatabaseFilePath()
{
    return GetApplicationDataPath() / "PicoTorrent.sqlite";
}

fs::path Environment::GetKnownFolderPath(Environment::KnownFolder knownFolder)
{
    KNOWNFOLDERID fid = { 0 };

    switch (knownFolder)
    {
    case KnownFolder::LocalAppData:
        fid = FOLDERID_LocalAppData;
        break;

    case KnownFolder::UserDownloads:
        fid = FOLDERID_Downloads;
        break;

    default:
        LOG_F(FATAL, "Unknown KnownFolder specified: %d", knownFolder);
        throw std::runtime_error("Unknown folder");
    }

    PWSTR result;
    HRESULT hr = SHGetKnownFolderPath(fid, 0, nullptr, &result);

    if (SUCCEEDED(hr))
    {
        fs::path p(result);
        CoTaskMemFree(result);
        return p;
    }

    LOG_F(FATAL, "Failed to get KnownFolder: %d", knownFolder);

    throw std::runtime_error("Could not get known folder path");
}

fs::path Environment::GetLogFilePath()
{
    std::time_t tim = std::chrono::system_clock::to_time_t(m_startupTime);
    tm t;
    localtime_s(&t, &tim);

    char frmt[100] = { 0 };
    snprintf(frmt,
        ARRAYSIZE(frmt),
        "PicoTorrent.%d%02d%02d%02d%02d%02d.log",
        t.tm_year + 1900,
        t.tm_mon + 1,
        t.tm_mday,
        t.tm_hour,
        t.tm_min,
        t.tm_sec);

    return GetApplicationDataPath() / "logs" / frmt;
}

bool Environment::IsAppContainerProcess()
{
    TCHAR path[MAX_PATH];
    GetModuleFileName(NULL, path, ARRAYSIZE(path));
    PathRemoveFileSpec(path);
    PathCombine(path, path, TEXT("appx.dummy"));
    DWORD dwAttr = GetFileAttributes(path);

    return (dwAttr != INVALID_FILE_ATTRIBUTES && !(dwAttr & FILE_ATTRIBUTE_DIRECTORY));
}

bool Environment::IsInstalled()
{
    HKEY hKey = NULL;

    LSTATUS lStatus = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("Software\\PicoTorrent"),
        0,
        KEY_READ,
        &hKey);

    if (lStatus != ERROR_SUCCESS)
    {
        if (hKey != NULL) { RegCloseKey(hKey); }
        return false;
    }

    TCHAR installDirectory[MAX_PATH];
    DWORD bufSize = MAX_PATH;

    lStatus = RegQueryValueEx(
        hKey,
        L"InstallDirectory",
        NULL,
        NULL,
        (LPBYTE)installDirectory,
        &bufSize);

    if (lStatus != ERROR_SUCCESS)
    {
        if (hKey != NULL) { RegCloseKey(hKey); }
        return false;
    }

    TCHAR currentLocation[MAX_PATH];
    GetModuleFileName(NULL, currentLocation, ARRAYSIZE(currentLocation));

    TCHAR installedFile[MAX_PATH];
    PathCombine(installedFile, installDirectory, TEXT("PicoTorrent.exe"));

    if (StrCmp(currentLocation, installedFile) == 0)
    {
        if (hKey != NULL) { RegCloseKey(hKey); }
        return true;
    }

    if (hKey != NULL) { RegCloseKey(hKey); }

    return false;
}
