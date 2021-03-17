#include "main.hpp"

HANDLE process = NULL;

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
    CloseHandle(process);
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

uintptr_t getAssemblyBase()
{
    DWORD out;
    HMODULE hmods[0x1000];
    uintptr_t base = NULL;
    if (EnumProcessModulesEx(process, hmods, 0x1000, &out, LIST_MODULES_ALL))
    {
        for (uint32_t i = 0; i < out / 4; ++i)
        {
            char path[MAX_PATH];
            if (GetModuleBaseNameA(process, hmods[i], path, MAX_PATH))
            {
                if (!strcmp(path, "GameAssembly.dll"))
                {
                    return reinterpret_cast<uintptr_t>(hmods[i]);
                }
            }
        }
    }
    else {
        logError("Cannot enumerate modules. Do i have admin perms?", true);
    }

    logError("Cannot get module base");
    return 0; // never gonna be executed but this stupid VS doesn't know this
}

int main()
{
    SetConsoleTitleA("Among Us Game settings unlocker");
    logInfo("Game settings unlocker patch by Galster\n", GREEN_TEXT);
    
    HWND hwnd = FindWindowA("UnityWndClass", "Among Us");
    logInfo("Waiting for Among Us...", WHITE_TEXT);

    while (!hwnd)
    {
        Sleep(500);
        hwnd = FindWindowA("UnityWndClass", "Among Us");
    }
    Sleep(1000);

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, FALSE, pid);

    if (process == INVALID_HANDLE_VALUE)
    {
        logError("Cannot hook to a process. Do i have admin perms?", true);
    }

    uintptr_t base = getAssemblyBase();
    uintptr_t addr = base + PATCH.offset;
    std::vector<byte> buf(PATCH.off.size());
    if(!ReadProcessMemory(process, reinterpret_cast<LPCVOID>(addr), buf.data(), buf.size(), NULL))
    {
        logError("Cannot read memory", true);
    }

    bool reversePatch = false;
    if (buf == PATCH.on)
    {
        reversePatch = true;
        logInfo("Looks like this patch is already enabled. Do you want to disable it? (y/n)", WHITE_TEXT);
        std::string answer;
        std::cin >> answer;
        if (answer != "y" && answer != "yes")
        {
            logInfo("Then i have nothing to do, cleaning...");
            CloseHandle(process);
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
    if (!WriteProcessMemory(process, reinterpret_cast<LPVOID>(addr), (reversePatch ? PATCH.off : PATCH.on).data(), PATCH.on.size(), NULL))
    {
        logError("Cannot write memory", true);
    }

    logInfo("\nSuccessfully patched!", GREEN_TEXT);
    CloseHandle(process);
    system("pause");
}