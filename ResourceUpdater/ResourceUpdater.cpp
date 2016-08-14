#include <Windows.h>
#include <iostream>

int main(int argc, char* argv[])
{
    printf("====== Resource Updater ======\n\n");

    printf("> Openening input file...\n");

    HANDLE hFile = CreateFile("UnFairplayKD.sys", 
        GENERIC_READ, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("[!] Failed to open input file\n");
        return 0;
    }

    DWORD dwSize = GetFileSize(hFile, NULL);

    char* pData = new char[dwSize];

    DWORD dwBytesRead;
    BOOL bResult = ReadFile(hFile,
        pData,
        dwSize,
        &dwBytesRead,
        NULL);

    printf("> Read input file: Size = %d, Bytes Read = %d, Result = %d\n", dwSize, dwBytesRead, bResult);

    if (!bResult) {
        printf("[!] Failed to read input file\n");
        return 0;
    }

    HANDLE hUpdateRes = BeginUpdateResource("netc.dll", FALSE);

    if (hUpdateRes == NULL) {
        printf("[!] Could not open netc.dll\n");
        return 0;
    }

    printf("> Opened netc.dll for resource update\n");

    bResult = UpdateResource(hUpdateRes,
        "BINARY",
        MAKEINTRESOURCE(122),
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_UK),
        pData,
        dwSize);

    if (!bResult) {
        printf("[!] UpdateResource failed\n");
        return 0;
    }

    if (!EndUpdateResource(hUpdateRes, FALSE)) {
        printf("[!] EndUpdateResource failed\n");
        return 0;
    }

    system("pause");
    return 0;
}

