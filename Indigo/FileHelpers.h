#pragma once

_Success_(return)
bool TryReadFile(
    _In_ char const* path,
    _Out_ u8 * *buffer,
    _Out_ sz * outSize
);

_Success_(return)
bool TryReadFile(
    _In_ WCHAR const* path,
    _Out_ u8 * *buffer,
    _Out_ sz * outSize
);


_Success_(return)
bool TryReadFileHandle(
    HANDLE handle,
    _Out_ u8 * *buffer,
    _Out_ sz * outSIze
);