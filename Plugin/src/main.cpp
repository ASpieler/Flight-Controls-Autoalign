#include "SFSE/Stub.h"
#include "sfse_common/Relocation.h"
#include "sfse_common/SafeWrite.h"
#include <thread>
#include <atomic> 
#include "Settings.h"

Settings settings;

const RelocAddr<uintptr_t*> PLAYER_FLYING = 0x50D59E0;
const RelocAddr<uintptr_t*> RETICLE_X = 0x5A5CE20;
const RelocAddr<uintptr_t*> RETICLE_Y = 0x5A5CE24;
const RelocAddr<uintptr_t*> MOUSE_X = 0x5A5CE3C;
const RelocAddr<uintptr_t*> MOUSE_Y = 0x5A5CE40;
const RelocAddr<uintptr_t*> FLIGHT_SENS = 0x55FA120;

std::thread monitorThread;


static float g_scaling_factor = 0.998f;
static int g_max_tick = 500;
static float g_reticle_offset = 1.0f;
static int g_user_key = -1;
static int g_alt_controls = 0;
static float g_flight_sensitivity = 3.6;


void SetReticle(float fNewReticleX, float fNewReticleY)
{
    safeWriteBuf(RETICLE_X.getUIntPtr(), &fNewReticleX, sizeof(float));
    safeWriteBuf(RETICLE_Y.getUIntPtr(), &fNewReticleY, sizeof(float));
}



void MonitorInFlight() noexcept
{

    int onUserPress = 0;
    int tick = 0;
    bool initializeTick = false;
    bool shouldRecenter = false;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
        bool isPlayerFlying = *reinterpret_cast<bool*>(PLAYER_FLYING.getUIntPtr());
        if (isPlayerFlying)
        {

            if (g_alt_controls)
            {
                safeWriteBuf(FLIGHT_SENS.getUIntPtr(), &g_flight_sensitivity, sizeof(float));
                float reticlePosX = *reinterpret_cast<float*>(RETICLE_X.getUIntPtr());
                float reticlePosY = *reinterpret_cast<float*>(RETICLE_Y.getUIntPtr());
                if (abs(reticlePosX) > 0.4f || abs(reticlePosY) > 0.4f)
                {
                    reticlePosX *= 0.8;
                    reticlePosY *= 0.85;
                }
                else
                {
                    reticlePosX = 0;
                    reticlePosY = 0;
                }
                SetReticle(reticlePosX, reticlePosY);
                continue;
            }


            bool bMouseMoved = (*reinterpret_cast<float*>(MOUSE_X.getUIntPtr()) || *reinterpret_cast<float*>(MOUSE_Y.getUIntPtr()));
            if (bMouseMoved)
            {
                onUserPress = 0;
                initializeTick = true;
                shouldRecenter = false;
                tick = 0;
            }
            else if (initializeTick)
            {
                tick++;
                if (tick > g_max_tick)
                {
                    initializeTick = false;
                    shouldRecenter = true;
                }
            }


            if (g_user_key != -1 && GetAsyncKeyState(g_user_key) & 0x8000)
            {
                onUserPress = 3;
                shouldRecenter = false;
            }
            else if (onUserPress)
            {
                shouldRecenter = true;
            }



            if (shouldRecenter)
            {
                float reticlePosX = *reinterpret_cast<float*>(RETICLE_X.getUIntPtr());
                float reticlePosY = *reinterpret_cast<float*>(RETICLE_Y.getUIntPtr());
                float ooga = reticlePosX / 200.0f;
                float booga = reticlePosY / 200.0f;
                if ((ooga * ooga + booga * booga) <= g_reticle_offset || onUserPress)
                {
                    reticlePosX *= (g_scaling_factor - 0.05 * onUserPress);
                    reticlePosY *= (g_scaling_factor - 0.05 * onUserPress);
                }

                if (abs(reticlePosX) < 0.4f && abs(reticlePosY) < 0.4f)
                {
                    reticlePosX = 0;
                    reticlePosY = 0;
                    onUserPress = 0;
                    shouldRecenter = false;
                }
                SetReticle(reticlePosX, reticlePosY); 
            }
        }

    }
}



namespace
{
    void MessageCallback(SFSE::MessagingInterface::Message* a_msg) noexcept
    {
        switch (a_msg->type) {
        case SFSE::MessagingInterface::kPostLoad:
            {
                settings.LoadSettings();
                g_max_tick = settings.max_tick;
                g_scaling_factor = settings.scaling_factor;
                g_reticle_offset = settings.reticleOffset;
                g_user_key = settings.realignKey;
                g_alt_controls = settings.altControls;
                g_flight_sensitivity = settings.altSens;
                monitorThread = std::thread(MonitorInFlight);
                monitorThread.detach();
            }
            break;
        default:
            break;
        }
    }
}


DLLEXPORT constinit auto SFSEPlugin_Version = []() noexcept {
	SFSE::PluginVersionData data{};
	
	data.PluginVersion(Plugin::Version);
	data.PluginName(Plugin::NAME);
	data.AuthorName(Plugin::AUTHOR);
	data.UsesSigScanning(true);
	//data.UsesAddressLibrary(true);
	data.HasNoStructUse(true);
	//data.IsLayoutDependent(true);
	data.CompatibleVersions({ RUNTIME_VERSION_1_8_88 });

	return data;
}();

DLLEXPORT bool SFSEAPI SFSEPlugin_Load(SFSEInterface* a_sfse)
{
    SFSE::Init(a_sfse);
    DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
    INFO("{} v{} loaded", Plugin::NAME, Plugin::Version);
    SFSE::GetMessagingInterface()->RegisterListener(MessageCallback);
    return true;
}