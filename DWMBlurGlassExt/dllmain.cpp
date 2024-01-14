// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "framework.h"
#include "DWMBlurGlass.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if(ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);

        MDWMBlurGlassExt::CreateNotifyThread();

        std::thread([hModule] 
        {
        	if (!MDWMBlurGlassExt::Startup())
			{
				MDWMBlurGlassExt::CloseNotifyThread();
				FreeLibrary(hModule);
			}
        }).detach();
    }
    else if(ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        MDWMBlurGlassExt::Shutdown();
        MDWMBlurGlassExt::CloseNotifyThread();
    }
    return TRUE;
}

