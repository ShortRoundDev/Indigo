#include "stdafx.h"

bool wideStrToMbStr(const wstring& in, string& out)
{
    // Get buffer size
    i32 bufferSize = WideCharToMultiByte(
        CP_UTF8, 0,
        in.c_str(), (i32)in.size(),
        nullptr, 0,
        nullptr, nullptr
    );

    // Set outstring to new buffer size
    out = string(bufferSize, '\0');

    //Convert
    i32 res = WideCharToMultiByte(
        CP_UTF8, 0,
        in.c_str(), (i32)in.size(),
        &out[0], (i32)out.size(),
        nullptr, nullptr
    );
    if (!res)
    {
        wcerr << L"Failed to convert Wide string [" << in << L"] to ANSI string!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << L" Got: " << str << endl;
        }
        wcerr << endl;
        return false;
    }
    return true;
}

bool mbStrToWideChar(const string& in, wstring& out)
{
    bool status = true;
    i32 bufferSize = MultiByteToWideChar(
        CP_ACP, 0, in.c_str(), -1, nullptr, 0
    );

    WCHAR* buffer = new WCHAR[bufferSize];
    i32 conversionResult = MultiByteToWideChar(
        CP_ACP, 0,
        in.c_str(), -1,
        buffer, bufferSize
    );

    if (!conversionResult)
    {
        cerr << "Failed to convert ANSI string [" << in << "] to Wide string!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
        status = false;
        goto Delete;
    }

    out = wstring(buffer);

Delete:
    if (buffer)
    {
        delete[] buffer;
    }
    return status;
}