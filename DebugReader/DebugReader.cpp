#include <Windows.h>
#include <iostream>

#define IOCTL_FAIRPLAY_CONTROL_CODE          \
    CTL_CODE(FILE_DEVICE_UNKNOWN, \
    0x802, \
    METHOD_BUFFERED, \
    FILE_READ_DATA | FILE_WRITE_DATA)

void hexdump(void *pAddressIn, long  lSize)
{
    char szBuf[100];
    long lIndent = 1;
    long lOutLen, lIndex, lIndex2, lOutLen2;
    long lRelPos;
    struct { char *pData; unsigned long lSize; } buf;
    unsigned char *pTmp, ucTmp;
    unsigned char *pAddress = (unsigned char *)pAddressIn;

    buf.pData = (char *)pAddress;
    buf.lSize = lSize;

    while (buf.lSize > 0)
    {
        pTmp = (unsigned char *)buf.pData;
        lOutLen = (int)buf.lSize;
        if (lOutLen > 16)
            lOutLen = 16;

        // create a 64-character formatted output line:
        sprintf(szBuf, " >                            "
            "                      "
            "    %08lX", pTmp - pAddress);
        lOutLen2 = lOutLen;

        for (lIndex = 1 + lIndent, lIndex2 = 53 - 15 + lIndent, lRelPos = 0;
            lOutLen2;
            lOutLen2--, lIndex += 2, lIndex2++
            )
        {
            ucTmp = *pTmp++;

            sprintf(szBuf + lIndex, "%02X ", (unsigned short)ucTmp);
            if (!isprint(ucTmp))  ucTmp = '.'; // nonprintable char
            szBuf[lIndex2] = ucTmp;

            if (!(++lRelPos & 3))     // extra blank after 4 bytes
            {
                lIndex++; szBuf[lIndex + 2] = ' ';
            }
        }

        if (!(lRelPos & 3)) lIndex--;

        szBuf[lIndex] = '<';
        szBuf[lIndex + 1] = ' ';

        printf("%s\n", szBuf);

        buf.pData += lOutLen;
        buf.lSize -= lOutLen;
    }
}


int ComputeHash(unsigned char* buffer, int length)
{
    int hash = 0x4661223;
    for (int i = 0; i < length; i++)
    {
        hash = (hash + buffer[i]) ^ (buffer[i] << ((i & 7) + 8));
    }
    return hash;
}

int main(int argc, char* argv[])
{
    printf("====== Fairplay Debug Reader ======\n\n");

    HANDLE hDevice = CreateFile(L"\\\\.\\FairplayKD0", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    printf("> Device Handle: 0x%X\n", hDevice);
    printf("> IO Control Code: %d\n", IOCTL_FAIRPLAY_CONTROL_CODE);

    //unsigned char sendBuffer[] = { 0x7D, 0x00, 0x00, 0x00, 0x68, 0x01, 0x00, 0x00, 0x09, 0xD0, 0x60, 0x04 };
    unsigned char sendBuffer[] = { 0x7E, 0x00, 0x00, 0x00, 0x68, 0x01, 0x00, 0x00, 0x0A, 0xCD, 0x60, 0x04 };
    //unsigned char sendBuffer[] = { 0x01, 0x00, 0x00, 0x00 };
    unsigned char readBuffer[1000] = { 0 };

    /*
    int result = ComputeHash(sendBuffer, sizeof(sendBuffer)-4);
    printf("> Hash = 0x%X\n", result);
    printf("> Hash2 = 0x%X\n", ((int*)sendBuffer)[2]);
    */

    if (hDevice == INVALID_HANDLE_VALUE)
    {
        printf(">> ERROR: Could not open Fairplay device\n");
        printf(">> Error Code: %d\n", GetLastError());
        system("pause");
        return 0;
    }

    while (true)
    {
        DWORD dwBytesRead = 0;
        BOOL result = DeviceIoControl(hDevice, IOCTL_FAIRPLAY_CONTROL_CODE, sendBuffer, sizeof(sendBuffer), readBuffer, sizeof(readBuffer), &dwBytesRead, NULL);
        printf("> DeviceIoControl Result = %d, %d\n", result, dwBytesRead);

        if (result == 1 && dwBytesRead >= 4)
        {
            hexdump(readBuffer, dwBytesRead);
        }

        Sleep(2000);
    }

    CloseHandle(hDevice);

    system("pause");
	return 0;
}

