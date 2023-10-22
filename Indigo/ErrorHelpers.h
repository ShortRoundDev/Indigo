#pragma once

using namespace std;

template<typename T>
struct LocalAllocator
{
    using value_type = T;

    LocalAllocator() noexcept = default;

    template<class U>
    LocalAllocator(const LocalAllocator<U>&) noexcept {}

    T* allocate(sz n);

    void deallocate(T* p, sz) noexcept;
};

using LocalAllocString = basic_string<char, char_traits<char>, LocalAllocator<char>>;
using LocalAllocWString = basic_string<WCHAR, char_traits<WCHAR>, LocalAllocator<WCHAR>>;

bool TryGetLastErrorMessageA(LocalAllocString& message);
bool TryGetErrorMessageA(LocalAllocString& message, DWORD errorCode);

bool TryGetLastErrorMessageW(LocalAllocWString& message);
bool TryGetErrorMessageW(LocalAllocWString& message, DWORD errorCode);

void PrintDxInfoQueue(ID3D11InfoQueue* queue);