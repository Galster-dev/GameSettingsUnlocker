#include "main.hpp"

void logError(std::string const& msg, bool showerrorcode = false)
{
    std::string message("ERROR: " + msg);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
    if (showerrorcode)
    {
        message += " Error code - " + std::to_string(GetLastError());
    }
    std::cout << message << std::endl;
    SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
    system("pause > nul");
    exit(1);
}

void logInfo(std::string const& message, const WORD color = NULL)
{
    if (color != NULL)
    {
        SetConsoleTextAttribute(hConsole, color);
    }
    std::cout << message << std::endl;
    if (color != NULL)
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY);
    }
}

HANDLE createMap(std::wstring& str)
{
    HANDLE file = CreateFileW(
        str.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (file == INVALID_HANDLE_VALUE) return NULL;

    HANDLE map = CreateFileMappingA(file, NULL, PAGE_READWRITE, NULL, NULL, NULL);
    CloseHandle(file);

    return map;
}

inline uintptr_t getMapBase(HANDLE& map)
{
    return reinterpret_cast<uintptr_t>(MapViewOfFile(
            map,
            FILE_MAP_READ | FILE_MAP_WRITE,
            NULL,
            NULL,
            0
    ));
}

inline bool freeMap(uintptr_t base, HANDLE& map)
{
    return UnmapViewOfFile(reinterpret_cast<LPCVOID>(base)) && CloseHandle(map);
}

std::wstring getProcessPath()
{

    HWND hwnd = FindWindowA("UnityWndClass", "Among Us");
    logInfo("Waiting for Among Us...", WHITE_TEXT);

    while (!hwnd)
    {
        Sleep(500);
        hwnd = FindWindowA("UnityWndClass", "Among Us");
    }

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, NULL, pid);
    if (process == INVALID_HANDLE_VALUE)
    {
        logError("Cannot get process info.", true);
    }

    constexpr uint32_t MAX_UNICODE_PATH = 32767u;
    wchar_t buf[MAX_UNICODE_PATH];
    DWORD len = GetModuleFileNameEx(process, NULL, buf, MAX_UNICODE_PATH);
    if (!len)
    {
        logError("Cannot find process path.");
    }
    std::wstring path(buf, buf + len);
    logInfo("Close the game to continue", WHITE_TEXT);
    WaitForSingleObject(process, INFINITE);
    return path;
}

int main()
{
    SetConsoleTitleA("Among Us Game settings unlocker");
    logInfo("Game settings unlocker patch by Galster\n", GREEN_TEXT);

    std::wstring aupath = getProcessPath();
    aupath = aupath.substr(0, aupath.find_last_of(L"\\")) + L"\\GameAssembly.dll"; // safety 100

    std::wcout << std::endl << L"Trying to open \"" << aupath << "\"..." << std::endl;
    HANDLE map = createMap(aupath);
    if (!map)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            logError("Cannot find GameAssembly.dll");
        }
        logError("Failed mapping GameAssembly.dll", true);
    }

    uintptr_t base = getMapBase(map);
    if(!base) logError("Failed mapping GameAssembly.dll", true);

    uintptr_t address = base + PATCH.offset;
    std::vector<byte> buf(PATCH.off.size());
    memcpy(buf.data(), reinterpret_cast<void*>(address), buf.size());
    bool reversePatch = false;

    if (buf == PATCH.on)
    {
        reversePatch = true;
        logInfo("Looks like you already have this patch. Do you want to remove it? (y/n)", WHITE_TEXT);
        std::string answer;
        std::cin >> answer;
        if (answer != "y" && answer != "yes")
        {
            logInfo("Then i have nothing to do, cleaning...");
            freeMap(base, map);
            logInfo("done", WHITE_TEXT);
            system("pause");
            return 0;
        }
    }
    else if (buf != PATCH.off)
    {
        logError("Byte signature is incorrect. Are you on v2021.3.5s Steam?");
    }

    logInfo("Patching...");
    memcpy((void*)address, (reversePatch ? PATCH.off : PATCH.on).data(), PATCH.on.size());

    if (!FlushViewOfFile(reinterpret_cast<LPCVOID>(address), PATCH.on.size()))
    {
        logError("Patching failed", true);
    }

    if (!freeMap(base, map))
    {
        logError("Failed to load map out. Patch may be failed", true);
    }

    logInfo("\nSuccessfully patched!", GREEN_TEXT);

    system("pause");
}