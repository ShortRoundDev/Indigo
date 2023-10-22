#pragma once

using namespace std;

template<typename T>
using Initializer = bool(T::*)(void);

bool wideStrToMbStr(const wstring& in, string& out);
bool mbStrToWideChar(const string& in, wstring& out);

template<typename T>
bool initWaterfall(T* object, initializer_list<Initializer<T>> initializers)
{
    for (auto const& initializer : initializers)
    {
        if (!(object->*initializer)())
        {
            return false;
        }
    }
    return true;
}