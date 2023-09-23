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
	if (val <= NVSDK_NGX_DLSS_Hint_Render_Preset_Default || val > NVSDK_NGX_DLSS_Hint_Render_Preset_G)
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
	if (!stricmp(val.c_str(), "G"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_G;

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

};
