#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#include <spdlog/spdlog.h>
#include <Patterns.h>

#include "DLSSTweaks.hpp"

namespace nvngx_dlss
{
// Allow force enabling/disabling the DLSS debug display via RegQueryValueExW hook
// (fallback method if we couldn't find the indicator value check pattern in the DLL)
LSTATUS(__stdcall* RegQueryValueExW_Orig)(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LSTATUS __stdcall RegQueryValueExW_Hook(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LSTATUS ret = RegQueryValueExW_Orig(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
	if (settings.overrideDlssHud != 0 && !_wcsicmp(lpValueName, L"ShowDlssIndicator"))
		if (lpcbData && *lpcbData >= 4 && lpData)
		{
			DWORD* outData = (DWORD*)lpData;
			if (settings.overrideDlssHud >= 1)
			{
				*outData = 0x400;
				return ERROR_SUCCESS; // always return success for DLSS to accept the result
			}
			else if (settings.overrideDlssHud < 0)
				*outData = 0;
		}

	return ret;
}

SafetyHookInline DLSS_GetIndicatorValue_Hook;
uint32_t __fastcall DLSS_GetIndicatorValue(void* thisptr, uint32_t* OutValue)
{
	auto ret = DLSS_GetIndicatorValue_Hook.thiscall<uint32_t>(thisptr, OutValue);
	if (settings.overrideDlssHud == 0)
		return ret;
	*OutValue = settings.overrideDlssHud > 0 ? 0x400 : 0;
	return ret;
}

SafetyHookMid dlssIndicatorHudHook{};
bool hook(HMODULE ngx_module)
{
	// This pattern finds 2 matches in latest DLSS, but only 1 in older ones
	// The one we're trying to find seems to always be first match
	// TODO: scan for RegQueryValue call and grab the offset from that, use offset instead of wildcards below

	auto indicatorValueCheck = hook::pattern((void*)ngx_module, "8B 81 ? ? ? ? 89 02 33 C0 C3");
	if ((!settings.disableIniMonitoring || settings.overrideDlssHud == 2) && indicatorValueCheck.size())
	{
		if (!settings.disableIniMonitoring)
			spdlog::debug("nvngx_dlss: applying hud hook via vftable hook...");
		else if (settings.overrideDlssHud == 2)
			spdlog::debug("nvngx_dlss: OverrideDlssHud == 2, applying hud hook via vftable hook...");

		auto indicatorValueCheck_addr = indicatorValueCheck.get(0).get<void>();
		{
			DLSS_GetIndicatorValue_Hook = safetyhook::create_inline(indicatorValueCheck_addr, DLSS_GetIndicatorValue);
		}

		// Unfortunately it's not enough to just hook the function, HUD render code seems to have an optimization where it checks funcptr and inlines code if it matches
		// So we also need to search for the address of the function, find vftable that holds it, and overwrite entry to point at our hook

		// Ugly way of converting our address to a pattern string...
		uint8_t* p = (uint8_t*)&indicatorValueCheck_addr;

		std::stringstream ss;
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)p[0];
		for (int i = 1; i < 8; ++i)
			ss << " " << std::setw(2) << std::setfill('0') << (int)p[i];

		auto pattern = ss.str();

		auto indicatorValueCheckMemberVf = hook::pattern((void*)ngx_module, pattern);
		for (int i = 0; i < indicatorValueCheckMemberVf.size(); i++)
		{
			auto vfAddr = indicatorValueCheckMemberVf.get(i).get<uintptr_t>(0);
			UnprotectMemory unprotect{ (uintptr_t)vfAddr, sizeof(uintptr_t) };
			*vfAddr = (uintptr_t)&DLSS_GetIndicatorValue;
		}

		spdlog::info("nvngx_dlss: applied debug hud overlay hook via vftable hook");
	}
	else
	{
		// Failed to locate indicator value check inside DLSS
		// Fallback to RegQueryValueExW hook so we can override the ShowDlssIndicator value
		HMODULE advapi = GetModuleHandleA("advapi32.dll");
		RegQueryValueExW_Orig = advapi ? (decltype(RegQueryValueExW_Orig))GetProcAddress(advapi, "RegQueryValueExW") : nullptr;
		if (!RegQueryValueExW_Orig || !utility::HookIAT(ngx_module, "advapi32.dll", RegQueryValueExW_Orig, RegQueryValueExW_Hook))
			spdlog::warn("nvngx_dlss: failed to hook DLSS HUD functions, OverrideDlssHud might not be available.");
		else
			spdlog::info("nvngx_dlss: applied debug hud overlay hook via registry");
	}

	return true;
}

void unhook(HMODULE ngx_module)
{
	dlssIndicatorHudHook.reset();

	spdlog::debug("nvngx_dlss: finished unhook");
}

SafetyHookInline dllmain;
BOOL APIENTRY hooked_dllmain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		hook(hModule);

	BOOL res = dllmain.stdcall<BOOL>(hModule, ul_reason_for_call, lpReserved);

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		unhook(hModule);
		dllmain.reset();
	}

	return res;
}

// Installs DllMain hook onto nvngx_dlss
void init(HMODULE ngx_module)
{
	if (settings.disableAllTweaks)
		return;

	dllmain = safetyhook::create_inline(utility::ModuleEntryPoint(ngx_module), hooked_dllmain);
}
};

namespace nvngx_dlssg
{
std::mutex module_handle_mtx;
HMODULE module_handle = NULL;

// Currently only patches out watermark text on select builds
void settings_changed()
{
	std::scoped_lock lock{ module_handle_mtx };

	if (!module_handle)
		return;

	char patch = settings.disableDevWatermark ? 0 : 0x4E;

	// Search for DLSSG watermark text and null it if found
	auto pattern = hook::pattern((void*)module_handle,
		"56 49 44 49 41 20 43 4F 4E 46 49 44 45 4E 54 49 41 4C 20 2D 20 50 52 4F 56 49 44 45 44 20 55 4E 44 45 52 20 4E 44 41");

	int count = pattern.size();
	if (!count)
	{
		spdlog::warn("nvngx_dlssg: DisableDevWatermark failed, couldn't locate watermark string inside module");
		return;
	}

	int changed = 0;
	for (int i = 0; i < count; i++)
	{
		char* result = pattern.get(i).get<char>(-1);

		if (result[0] != patch)
		{
			UnprotectMemory unprotect{ (uintptr_t)result, 1 };
			*result = patch;
			changed++;
		}
	}

	if (changed)
		spdlog::info("nvngx_dlssg: DisableDevWatermark patch {} ({}/{} strings patched)", settings.disableDevWatermark ? "applied" : "removed", changed, count);
}
	
SafetyHookInline dllmain;
BOOL APIENTRY hooked_dllmain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		{
			std::scoped_lock lock{module_handle_mtx};
			module_handle = hModule;
		}
		settings_changed();
	}

	BOOL res = dllmain.stdcall<BOOL>(hModule, ul_reason_for_call, lpReserved);

	if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		std::scoped_lock lock{module_handle_mtx};
		module_handle = NULL;
		dllmain.reset();
	}

	return res;
}

void init(HMODULE ngx_module)
{
	if (settings.disableAllTweaks)
		return;

	dllmain = safetyhook::create_inline(utility::ModuleEntryPoint(ngx_module), hooked_dllmain);
}
};
