#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>

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

// Based on code from https://github.com/neosmart/msvcrt.lib/blob/679d9aeddb20eb48aec3a58cf5ed99271cb09f68/GetFileVersionInfo/GetFileVersionInfo.c
std::string ModuleVersion(const std::filesystem::path& module_path)
{
	DWORD unused = 0;
	DWORD size = GetFileVersionInfoSizeExW(0 | FILE_VER_GET_NEUTRAL | FILE_VER_GET_LOCALISED, module_path.wstring().c_str(), &unused);
	if (size == 0)
		return "";

	auto data = std::make_unique<BYTE[]>(size);
	DWORD result = GetFileVersionInfoExW(0 | FILE_VER_GET_NEUTRAL | FILE_VER_GET_LOCALISED, module_path.wstring().c_str(), unused, size, data.get());
	if (result == 0)
		return "";

	// Retrieve a list of valid localizations to query the version table against
	struct Translation {
		int16_t Language;
		int16_t Codepage;
	} *translations = NULL;
	uint32_t translationLength = 0;
	TCHAR blockBuffer[MAX_PATH];
	_tcscpy_s(blockBuffer, _countof(blockBuffer), _T("\\VarFileInfo\\Translation"));
	VerQueryValue(data.get(), blockBuffer, (LPVOID*)&translations, &translationLength);

	for (size_t i = 0; i < (translationLength / sizeof(struct Translation)); i++)
	{
		TCHAR key[MAX_PATH] = { 0 };
		_stprintf_s(key, _countof(key), _T("\\StringFileInfo\\%04x%04x\\FileVersion"), translations[i].Language, translations[i].Codepage);

		const TCHAR* ffInfo = NULL;
		uint32_t ffiLength = 0;

		result = VerQueryValue(data.get(), key, (LPVOID*)&ffInfo, &ffiLength);
		if (result != TRUE) {
			continue;
		}

		return std::string(ffInfo);
	}

	return "";
}

};
