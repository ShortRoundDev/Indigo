#pragma once

using namespace std;

struct SystemVariables
{
    wstring m_configLocation;
    optional<u16> m_width;
    optional<u16> m_height;
    optional<bool> m_vsync;
    optional<bool> m_fullscreen;
    optional<bool> m_dxDebug;
    optional<f32> m_fov;
};

class SystemManager
{
public:
    void init();

    const SystemVariables& getVars() const;

private:
    map<wstring, wstring> m_args;
    SystemVariables m_vars;

    bool initVars();
    bool initConsoleVars();
    bool initConfigVars();

    u16 getArg(const wstring& key, u16 fallback);
    bool getArg(const wstring& key, bool fallback);
    const wstring& getArg(const wstring& key, const wstring& fallback);
};

extern SystemManager* g_system;
#define SYS (g_system)