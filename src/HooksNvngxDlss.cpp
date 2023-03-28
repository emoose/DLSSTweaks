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
	if ((settings.watchIniUpdates || settings.overrideDlssHud == 2) && indicatorValueCheck.size())
	{
		if (settings.watchIniUpdates)
			spdlog::debug("nvngx_dlss: WatchIniUpdates enabled, applying hud hook via vftable hook...");
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

		spdlog::debug("nvngx_dlss: applied hud hook via vftable hook");
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
			spdlog::debug("nvngx_dlss: applied hud hook via registry");
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
// Currently only patches out watermark text on select builds
void init(HMODULE ngx_module)
{
	if (settings.disableAllTweaks)
		return;
	if (!settings.disableDevWatermark)
		return;

	// Check if we support this dlssg build, currently only 3.1.10 / 1.10.0 dev supported
	// (unfortunately haven't found a safe way to genericise this patch atm...)
	uint8_t* base = (uint8_t*)ngx_module;
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)base;
	IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew);
	if (nt_headers->FileHeader.TimeDateStamp != 1678805300)
	{
		spdlog::warn("DisableDevWatermark: dlssg patch not applied as version is unknown");
		return;
	}

	uint8_t expectedBytes[] = {
		0x40, 0x55, 
		0x56, 
		0x57, 
		0x41, 0x54,
		0x41, 0x55, 
		0x41, 0x56, 
		0x41, 0x57, 
		0x48, 0x8D, 0xAC, 0x24, 0x30, 0xFF, 0xFF, 0xFF,
		0x48, 0x81, 0xEC, 0xD0, 0x01, 0x00, 0x00,
		0x48, 0xC7, 0x45, 0x60, 0xFE, 0xFF, 0xFF, 0xFF
	};
	uint8_t* patchAddress = base + 0xB5E30;
	if (memcmp(patchAddress, expectedBytes, sizeof(expectedBytes)) != 0)
	{
		spdlog::warn("DisableDevWatermark: dlssg patch not applied as expected bytes weren't found");
		return;
	}

	{
		UnprotectMemory unprotect{ (uintptr_t)patchAddress, 1 };
		*patchAddress = 0xC3; // ret
	}

	spdlog::info("DisableDevWatermark: dlssg patch applied");
}
};
