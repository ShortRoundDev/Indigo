#include "stdafx.h"

#include "SystemManager.h"

using namespace std;

template<typename T>
T* LocalAllocator<T>::allocate(sz n)
{
    if (n > numeric_limits<sz>::max() / sizeof(T))
    {
        throw bad_alloc();
    }
    void* p = LocalAlloc(LMEM_FIXED, n * sizeof(T));
    if (p == nullptr)
    {
        throw bad_alloc();
    }
    return static_cast<T*>(p);
}

template<typename T>
void LocalAllocator<T>::deallocate(T* p, sz) noexcept
{
    LocalFree(p);
}


bool TryGetLastErrorMessageA(LocalAllocString& message)
{
    DWORD err = GetLastError();
    if (!err)
    {
        return false;
    }

    return TryGetErrorMessageA(message, err);
}

bool TryGetErrorMessageA(LocalAllocString& message, DWORD errorCode)
{
    LPSTR str = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        LANG_SYSTEM_DEFAULT,
        (LPSTR)&str,
        0,
        NULL
    );

    message = LocalAllocString(move(str));
    str = nullptr;
    return true;
}

bool TryGetErrorMessageW(LocalAllocWString& message, DWORD errorCode)
{
    LPWSTR str = NULL;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        GetLastError(),
        LANG_SYSTEM_DEFAULT,
        (LPWSTR)&str,
        0,
        NULL
    );

    message = LocalAllocWString(move(str));
    str = nullptr;
    return true;

}

bool TryGetLastErrorMessageW(LocalAllocWString& message)
{
    DWORD err = GetLastError();
    if (!err)
    {
        return false;
    }

    return TryGetErrorMessageW(message, err);
}

void PrintDxInfoQueue(ID3D11InfoQueue* queue)
{
    if (!SYS->getVars().m_dxDebug)
    {
        return;
    }
    for (u64 i = 0; i < queue->GetNumStoredMessages(); i++)
    {
        sz msgSize = 0;
        auto res = queue->GetMessage(i, NULL, &msgSize);
        D3D11_MESSAGE* msg = (D3D11_MESSAGE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, msgSize);
        if (msg == NULL)
        {
            cerr << "Failed to allocate message!";
            continue;
        }

        res = queue->GetMessage(i, msg, &msgSize);
        if (res == S_OK)
        {
            cerr << msg->pDescription << endl;
        }
        HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, msg);
    }
    queue->ClearStoredMessages();
}