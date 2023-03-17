#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#include <cstdint>
#include "DLSSTweaks.hpp"

namespace utility
{

std::string DLSS_PresetEnumToName(unsigned int val)
{
	if (val <= NVSDK_NGX_DLSS_Hint_Render_Preset_Default || val > NVSDK_NGX_DLSS_Hint_Render_Preset_F)
		return "Default";
	int charVal = int(val) - 1 + 'A';
	std::string ret(1, charVal);
	return ret;
}

unsigned int DLSS_PresetNameToEnum(const std::string& val)
{
	if (!stricmp(val.c_str(), "Default"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	if (!stricmp(val.c_str(), "A"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_A;
	if (!stricmp(val.c_str(), "B"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_B;
	if (!stricmp(val.c_str(), "C"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_C;
	if (!stricmp(val.c_str(), "D"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_D;
	if (!stricmp(val.c_str(), "E"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_E;
	if (!stricmp(val.c_str(), "F"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_F;

	return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
}

BOOL HookIAT(HMODULE callerModule, char const* targetModule, void* targetFunction, void* detourFunction)
{
	uint8_t* base = (uint8_t*)callerModule;
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)base;
	IMAGE_NT_HEADERS* nt_headers = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew);

	IMAGE_IMPORT_DESCRIPTOR* imports = (IMAGE_IMPORT_DESCRIPTOR*)(base + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (int i = 0; imports[i].Characteristics; i++)
	{
		const char* name = (const char*)(base + imports[i].Name);

		if (lstrcmpiA(name, targetModule) != 0)
			continue;

		void** thunk = (void**)(base + imports[i].FirstThunk);

		for (; *thunk; thunk++)
		{
			void* import = *thunk;

			if (import != targetFunction)
				continue;

			DWORD oldState;
			if (!VirtualProtect(thunk, sizeof(void*), PAGE_READWRITE, &oldState))
				return FALSE;

			*thunk = (void*)detourFunction;

			VirtualProtect(thunk, sizeof(void*), oldState, &oldState);

			return TRUE;
		}
	}

	return FALSE;
}

};
