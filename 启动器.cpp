#include <windows.h>
#include <string>
#include <vector>

constexpr size_t 缓冲区大小 = 512;
wchar_t 原程序路径[缓冲区大小] = L"__MAGIC_ORIGINAL_EXE_PATH_BUFFER_0123456789ABCDEF_PLACEHOLDER__";
wchar_t 目标硬链接路径[缓冲区大小] = L"__MAGIC_TARGET_LINK_PATH_BUFFER_0123456789ABCDEF_PLACEHOLDER__";

bool 检查是否同一文件(const wchar_t* path1, const wchar_t* path2) {
    HANDLE h1 = CreateFileW(path1, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (h1 == INVALID_HANDLE_VALUE) return false;

    HANDLE h2 = CreateFileW(path2, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);
    if (h2 == INVALID_HANDLE_VALUE) {
        CloseHandle(h1);
        return false;
    }

    BY_HANDLE_FILE_INFORMATION info1 = {};
    BY_HANDLE_FILE_INFORMATION info2 = {};
    bool same = false;
    if (GetFileInformationByHandle(h1, &info1) && GetFileInformationByHandle(h2, &info2)) {
        same = (info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
                info1.nFileIndexHigh == info2.nFileIndexHigh &&
                info1.nFileIndexLow == info2.nFileIndexLow);
    }
    CloseHandle(h1);
    CloseHandle(h2);
    return same;
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR lpCmdLine, int nCmdShow) {
    if (!检查是否同一文件(原程序路径, 目标硬链接路径)) {
        DeleteFileW(目标硬链接路径);
        CreateHardLinkW(目标硬链接路径, 原程序路径, NULL);
    }

    std::wstring 完整命令行 = GetCommandLineW();
    size_t idx = 0;
    while (idx < 完整命令行.size() && 完整命令行[idx] == L' ') idx++;
    if (idx < 完整命令行.size() && 完整命令行[idx] == L'"') {
        idx++;
        while (idx < 完整命令行.size() && 完整命令行[idx] != L'"') idx++;
        if (idx < 完整命令行.size() && 完整命令行[idx] == L'"') idx++;
    } else {
        while (idx < 完整命令行.size() && 完整命令行[idx] != L' ') idx++;
    }
    while (idx < 完整命令行.size() && 完整命令行[idx] == L' ') idx++;

    std::wstring 外部参数 = 完整命令行.substr(idx);
    std::wstring 新命令行 = L"\"";
    新命令行 += 目标硬链接路径;
    新命令行 += L"\"";
    if (!外部参数.empty()) {
        新命令行 += L" ";
        新命令行 += 外部参数;
    }

    std::wstring 工作目录 = 原程序路径;
    size_t pos = 工作目录.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        工作目录 = 工作目录.substr(0, pos);
    }

    HMODULE hMod = GetModuleHandleW(NULL);
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dos->e_lfanew);
    bool 是控制台 = (nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI);

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};
    std::vector<wchar_t> cmdBuf(新命令行.begin(), 新命令行.end());
    cmdBuf.push_back(L'\0');

    if (CreateProcessW(目标硬链接路径, cmdBuf.data(), NULL, NULL, TRUE, 0, NULL, 工作目录.c_str(), &si, &pi)) {
        if (是控制台) {
            WaitForSingleObject(pi.hProcess, INFINITE);
            DWORD exitCode = 0;
            GetExitCodeProcess(pi.hProcess, &exitCode);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            return (int)exitCode;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    return 0;
}
