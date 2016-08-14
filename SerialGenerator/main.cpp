#include <Windows.h>
#include <string>
#include <iostream>

std::string RegReadStringValue(const std::string& key, const std::string& value)
{
    BYTE keyData[100];
    HKEY hKey;
    std::string result("");

    if (RegOpenKeyExA(HKEY_CURRENT_USER, key.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dataCount = 100;
        if (RegQueryValueExA(hKey, value.c_str(), NULL, NULL, keyData, &dataCount) == ERROR_SUCCESS)
        {
            result = (char*)keyData;
        }

        RegCloseKey(hKey);
    }

    return result;
}

std::string GetFakeIEShellAttributes()
{
    return RegReadStringValue("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID2\\{871C5380-42A0-1069-A2EA-08002B30309D}\\ShellFolder", "Attributes");
}

bool IsValidBase36Char(char input)
{
    return (input >= '0' && input <= '9') || (input >= 'A' && input <= 'Z');
}

std::string sub_10038320(const std::string& input1, const std::string& input2)
{
    std::string transformed;

    if (input1.size() > 0)
    {
        for (size_t i = 0; i < input1.size(); i++)
        {
            char v6 = input2[i % input2.size()];
            char v7 = input1[i];

            if (!IsValidBase36Char(v6) || !IsValidBase36Char(v7))
            {
                return "e";
            }
            
            v6 = v6 > '9' ? v6 - '7' : v6 - '0'; // Convert from base36 ASCII to decimal
            v7 = v7 > '9' ? v7 - '7' : v7 - '0'; // Convert from base36 ASCII to decimal

            char newChar = ((v7 - v6 + 36) % 36);
            char v10 = newChar > 9 ? newChar + '7' : newChar + '0'; // Convert from decimal to base36 ASCII (seems to only result in 0-F though)
            transformed += v10;
        }
    }

    if (transformed.size() > 8)
    {
        // Get last 8 characters
        std::string lastChars = transformed.substr(transformed.size() - 8);
        std::string otherChars = transformed.substr(0, transformed.size() - 8);

        transformed = otherChars;
    
        // This appears to be some kind of checksum?
        int lastAsInt;
        sscanf(lastChars.c_str(), "%x", &lastAsInt);

        int unkHash = 0;

        if (transformed.size() > 0)
        {
            for (size_t i = 0; i < transformed.size(); i++)
            {
                char v15 = transformed[i];
                v15 = v15 > '9' ? v15 - '7' : v15 - '0';
                unkHash = v15 + (unkHash ^ 2 * unkHash);
            }
        }

        if (lastAsInt == unkHash)
        {
            return transformed;
        }
        else
        {
            return "";
        }
    }
    else
    {
        return "e";
    }
}

std::string ReadAppDataAFSData()
{
    char* buf = (char*)malloc(10); // TODO: Leaked
    DWORD size = GetEnvironmentVariableA("APPDATA", buf, 1);

    buf = (char*)realloc(buf, size + 1);
    GetEnvironmentVariableA("APPDATA", buf, size);

    std::string envVar = buf;
    envVar += ":NT"; // DODGY ALTERNATE FILE STREAMS

    FILE* f = fopen(envVar.c_str(), "rb");

    if (f)
    {
        char readBuf[128];
        size_t read = fread(readBuf, 1, 127, f);
        readBuf[read] = 0;

        return readBuf;
    }
    else
    {
        return "";
    }
}

std::string GetAFSData()
{
    std::string afsData = ReadAppDataAFSData();
    if (sub_10038320(afsData, "Q3G9").size() <= 1)
    {
        // TODO: IMPLEMENT (This probably generates the AFS data, looks really complex)
    }

    return afsData;
}

std::string sub_10039E00()
{
    std::string fakeShellAttr = GetFakeIEShellAttributes();
    std::string fakeShellTransform = sub_10038320(fakeShellAttr, "CA2X");
    
    std::string afsData = GetAFSData();
    std::string afsDataTransform = sub_10038320(afsData, "Q3G9");

    std::string thatHash = "1B3811D156AB87B2377394498E470A93";

    if (thatHash != fakeShellTransform)
    {
        if (fakeShellTransform.size() > 1 && afsDataTransform.size() > 1 && fakeShellTransform == afsDataTransform)
        {
            // TODO: DO IMPLEMENT THING
            return "TODO";
        }
    }
    else
    {

    }



    std::cout << fakeShellTransform << std::endl;
}

/*
void TestWrite()
{
    char* buf = (char*)malloc(10);
    DWORD size = GetEnvironmentVariableA("APPDATA", buf, 1);

    buf = (char*)realloc(buf, size + 1);
    GetEnvironmentVariableA("APPDATA", buf, size);

    std::string envVar = buf;
    envVar += ":NT";

    FILE* f = fopen(envVar.c_str(), "wb");

    if (f)
    {
        fwrite("3FJHQ8UOU6KLQ5OKQHSOU5KFYALHS6PC5CPMY5T9", 1, 40, f);
        fclose(f);
    }
}
*/

int main()
{
    sub_10039E00();
}