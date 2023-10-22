#include "stdafx.h"

_Success_(return)
bool TryReadFile(
    _In_ char const* path,
    _Out_ u8 * *buffer,
    _Out_opt_ sz * outSize
)
{
    if (buffer == NULL)
    {
        return false;
    }
    HANDLE handle = CreateFileA(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        cerr << "Failed to open file " << path << "!";
        LocalAllocString str;
        if (TryGetLastErrorMessageA(str))
        {
            cerr << " Got: " << str;
        }
        cerr << endl;
    }
    bool result = TryReadFileHandle(handle, buffer, outSize);
    CloseHandle(handle);
    return result;
}

_Success_(return)
bool TryReadFile(
    _In_ WCHAR const* path,
    _Out_ u8 * *buffer,
    _Out_opt_ sz * outSize
)
{
    if (buffer == NULL)
    {
        return false;
    }
    HANDLE handle = CreateFileW(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        wcerr << L"Failed to open file " << path << "!";
        LocalAllocWString str;
        if (TryGetLastErrorMessageW(str))
        {
            wcerr << L" Got: " << str;
        }
        cerr << endl;
        return false;
    }
    bool result = TryReadFileHandle(handle, buffer, outSize);
    CloseHandle(handle);
    return result;
}

_Success_(return)
bool TryReadFileHandle(
    HANDLE handle,
    _Out_ u8 * *buffer,
    _Out_opt_ sz * outSize
)
{
    u32 size = GetFileSize(handle, NULL);
    if (outSize != NULL)
    {
        *outSize = size;
    }

    if (size == 0)
    {
        return false;
    }

    *buffer = new u8[size + 1];
    ZeroMemory(*buffer, size + 1);
    ReadFile(handle, *buffer, size + 1, NULL, NULL);

    return true;
}