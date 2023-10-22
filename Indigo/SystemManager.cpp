#include "stdafx.h"

#include "SystemManager.h"

void SystemManager::init()
{
    initWaterfall(this, {
        &SystemManager::initVars
        });
}

const SystemVariables& SystemManager::getVars() const
{
    return m_vars;
}


/* ---------- PRIVATE ---------- */

bool SystemManager::initVars()
{
    return initConsoleVars();
}

bool SystemManager::initConsoleVars()
{
    auto _cmd = GetCommandLine();
    i32 length = lstrlenW(_cmd) + 1;
    auto cmd = new WCHAR[length];
    ZeroMemory(cmd, length);
    CopyMemory(cmd, _cmd, sizeof(WCHAR) * length);

    WCHAR* key = NULL;
    WCHAR* iterator;
    wcstok_s(cmd, L" ", &iterator); // call once to skip program name
    while (WCHAR* token = wcstok_s(NULL, L" ", &iterator))
    {
        if (token == NULL || token[0] == L'\0')
        {
            wcout << "Warning: Null or empty argument!" << endl;
            continue;
        }
        if (key == NULL)
        {
            if (token[0] != L'-')
            {
                //warning
                wcout << L"Warning: Config key '" << token << L"' Doesn't begin with '-'!" << endl;
            }
            key = token;
            continue;
        }
        else if (token[0] == L'-')
        {
            m_args.emplace(move(key), L"true");
            key = token;
        }
        else
        {
            m_args.emplace(move(key), move(token));
            key = NULL;
        }
    }
    if (key != NULL && key[0] == L'-')
    {
        m_args.emplace(key, L"true");
    }
    m_vars.m_configLocation = getArg(L"-c", wstring(L"config.yaml"));
    if (!m_vars.m_configLocation.empty())
    {
        initConfigVars(); // call this here so that commandline
        // vars can override config file
    }

    m_vars.m_width = getArg(L"-w", m_vars.m_width.value_or((u16)1024));
    m_vars.m_height = getArg(L"-h", m_vars.m_height.value_or((u16)768));
    m_vars.m_fullscreen = getArg(L"-f", m_vars.m_fullscreen.value_or(false));
    m_vars.m_vsync = getArg(L"-v", m_vars.m_vsync.value_or(true));
    m_vars.m_dxDebug = getArg(L"-dd", m_vars.m_vsync.value_or(false));
    return true;
}

bool SystemManager::initConfigVars()
{
    // get bufferSize required after conversion
    string path;
    wideStrToMbStr(m_vars.m_configLocation, path);
    YAML::Node node;
    try
    {
        node = YAML::LoadFile(path);
    }
    catch (exception& e)
    {
        cout << "Failed to load config file at [" << path << "]. Got " << e.what() << endl;
        return true;
    }

    if (node["width"])
    {
        m_vars.m_width = node["width"].as<u16>();
    }
    if (node["height"])
    {
        m_vars.m_height = node["height"].as<u16>();
    }
    if (node["fullscreen"])
    {
        m_vars.m_fullscreen = node["fullscreen"].as<bool>();
    }
    if (node["vsync"])
    {
        m_vars.m_vsync = node["vsync"].as<bool>();
    }
    if (node["dxDebug"])
    {
        m_vars.m_dxDebug = node["dxDebug"].as<bool>();
    }
    if (node["fov"])
    {
        m_vars.m_fov = node["fov"].as<f32>();
    }
    return true;
}

u16 SystemManager::getArg(const wstring& key, u16 fallback)
{
    auto iter = m_args.find(key);
    if (iter == m_args.end())
    {
        return fallback;
    }
    try
    {
        return stoi(iter->second);
    }
    catch (exception&)
    {
        wcout << L"Value '" << key << "' is not a valid u16!" << endl;
    }
    return fallback;
}

bool SystemManager::getArg(const wstring& key, bool fallback)
{
    auto iter = m_args.find(key);
    if (iter == m_args.end())
    {
        return fallback;
    }

    transform(
        iter->second.begin(), iter->second.end(),
        iter->second.begin(),
        towlower
    );
    return iter->second == L"true";
}

const wstring& SystemManager::getArg(const wstring& key, const wstring& fallback)
{
    auto iter = m_args.find(key);
    if (iter == m_args.end())
    {
        return fallback;
    }
    return iter->second;
}
