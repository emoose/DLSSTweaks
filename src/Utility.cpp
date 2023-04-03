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

std::pair<int, int> ParseResolution(std::string_view val)
{
	std::pair<int, int> result = { 0,0 };
	if (val.size() < 3) // minimum is "0x0"
		return result;

	size_t seperator = val.find('x');
	if (seperator == std::string::npos || val.size() <= seperator + 1)
		return result;

	std::string width_str = std::string(val.substr(0, seperator));
	std::string height_str = std::string(val.substr(seperator + 1));
	result.first = std::stoi(width_str);
	result.second = std::stoi(height_str);

	return result;
}

std::string trim_quotes(const std::string& str)
{
	// Find the first non-whitespace (and non-quote) character
	auto start = std::find_if_not(str.begin(), str.end(), [](int c) {
		return std::isspace(c) || c == '"' || c == '\'';
		});

	// Find the last non-whitespace (and non-quote) character
	auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
		return std::isspace(c) || c == '"' || c == '\'';
		});

	// Convert iterators to indices
	auto first = std::distance(str.begin(), start);
	auto last = std::distance(str.begin(), end.base());

	// Return the trimmed substring
	return str.substr(first, last - first);
}

// Strips any improperly quoted string & any useless whitespace
std::string ini_get_string_safe(inih::INIReader& ini, const std::string& section, const std::string& name, std::string&& default_v)
{
	std::string ret = ini.Get<std::string>(section, name, std::move(default_v));
	return trim_quotes(ret);
}

// Converts string to float using std::from_chars (which uses "C" locale), and also converts comma formatted numbers to use period
// (this way we don't need to bother with std::setlocale shenanigans, which may have conflicts with the hooked app)
float stof_nolocale(std::string_view s, bool strict)
{
	std::string conv = std::string(s);

	// In case user uses commas for decimals, switch out first comma found to a period instead
	auto comma_pos = conv.find(',');
	if (comma_pos != std::string::npos)
		conv[comma_pos] = '.';

	if (strict)
	{
		// Disallow any characters other than numeric / period / whitespace
		auto charIsNonNumeric = [](char c) { return !std::isdigit(c) && c != '-' && c != '.' && c != ' ' && c != '\t'; };
		if (std::any_of(conv.begin(), conv.end(), charIsNonNumeric))
			throw std::invalid_argument{ "invalid_argument" };
	}

	float result{};
	auto [ptr, ec] { std::from_chars(conv.data(), conv.data() + conv.size(), result) };
	if (ec == std::errc::invalid_argument)
		throw std::invalid_argument{ "invalid_argument" };
	else if (ec == std::errc::result_out_of_range)
		throw std::out_of_range{ "out_of_range" };

	return result;
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
