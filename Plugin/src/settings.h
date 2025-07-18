#include <Windows.h>
#include <string>

class Settings
{
public:
    void LoadSettings();
    int max_tick{ 500 };
    float scaling_factor{ 0.998f };
    int realignKey{ -1 }; // Default key (e.g., Space key)
    int altControls{ 0 }; // Default key (e.g., Space key)
    float altSens{ 6.0f };
    float reticleOffset{ 1.0f };
};

void Settings::LoadSettings()
{
    const wchar_t* path = L"Data/SFSE/Plugins/FlightControlsAutoalign.ini";
    wchar_t buffer[256];

    max_tick = GetPrivateProfileIntW(L"Settings", L"WaitBeforeRealign", 500, path);

    GetPrivateProfileStringW(L"Settings", L"RealignSpeed", L"0.998", buffer, sizeof(buffer) / sizeof(wchar_t), path);
    scaling_factor = static_cast<float>(_wtof(buffer));

    GetPrivateProfileStringW(L"Settings", L"Offset", L"1.0", buffer, sizeof(buffer) / sizeof(wchar_t), path);
    reticleOffset = static_cast<float>(_wtof(buffer));

    realignKey = GetPrivateProfileIntW(L"Settings", L"RealignKey", -1, path);

    altControls = GetPrivateProfileIntW(L"Settings", L"AlternativeControls", 0, path);

    GetPrivateProfileStringW(L"Settings", L"MouseSensitivityForAltControls", L"6.0", buffer, sizeof(buffer) / sizeof(wchar_t), path);
    altSens = static_cast<float>(_wtof(buffer));

}