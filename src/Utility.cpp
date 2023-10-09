#define WIN32_LEAN_AND_MEAN
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
	const int charVal = int(val) - 1 + 'A';
	std::string ret(1, charVal);
	return ret;
}

unsigned int DLSS_PresetNameToEnum(const std::string& val)
{
	if (!_stricmp(val.c_str(), "Default"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
	if (!_stricmp(val.c_str(), "A"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_A;
	if (!_stricmp(val.c_str(), "B"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_B;
	if (!_stricmp(val.c_str(), "C"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_C;
	if (!_stricmp(val.c_str(), "D"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_D;
	if (!_stricmp(val.c_str(), "E"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_E;
	if (!_stricmp(val.c_str(), "F"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_F;
	if (!_stricmp(val.c_str(), "G"))
		return NVSDK_NGX_DLSS_Hint_Render_Preset_G;

	return NVSDK_NGX_DLSS_Hint_Render_Preset_Default;
}

std::pair<int, int> ParseResolution(std::string_view val)
{
	std::pair result = { 0,0 };
	if (val.size() < 3) // minimum is "0x0"
		return result;

	const size_t separator = val.find('x');
	if (separator == std::string::npos || val.size() <= separator + 1)
		return result;

	const auto width_str = std::string(val.substr(0, separator));
	const auto height_str = std::string(val.substr(separator + 1));
	result.first = std::stoi(width_str);
	result.second = std::stoi(height_str);

	return result;
}

std::string trim_quotes(const std::string& str)
{
	// Find the first non-whitespace (and non-quote) character
	const auto start = std::find_if_not(str.begin(), str.end(), [](int c) {
		return std::isspace(c) || c == '"' || c == '\'';
		});

	// Find the last non-whitespace (and non-quote) character
	const auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
		return std::isspace(c) || c == '"' || c == '\'';
		});

	// Convert iterators to indices
	const auto first = std::distance(str.begin(), start);
	const auto last = std::distance(str.begin(), end.base());

	// Return the trimmed substring
	return str.substr(first, last - first);
}

// Strips any improperly quoted string & any useless whitespace
std::string ini_get_string_safe(inih::INIReader& ini, const std::string& section, const std::string& name, std::string&& default_v)
{
	const auto ret = ini.Get<std::string>(section, name, std::move(default_v));
	return trim_quotes(ret);
}

// Converts string to float using std::from_chars (which uses "C" locale), and also converts comma formatted numbers to use period
// (this way we don't need to bother with std::setlocale shenanigans, which may have conflicts with the hooked app)
float stof_nolocale(std::string_view s, bool strict)
{
	std::string conv = std::string(s);

	// In case user uses commas for decimals, switch out first comma found to a period instead
	if (const auto comma_pos = conv.find(','); comma_pos != std::string::npos)
		conv[comma_pos] = '.';

	if (strict)
	{
		// Disallow any characters other than numeric / period / whitespace
		auto charIsNonNumeric = [](char c) { return !std::isdigit(c) && c != '-' && c != '.' && c != ' ' && c != '\t'; };
		if (std::ranges::any_of(conv, charIsNonNumeric))
			throw std::invalid_argument{ "invalid_argument" };
	}

	float result{};
	auto [ptr, ec] { std::from_chars(conv.data(), conv.data() + conv.size(), result) };
	if (ec == std::errc::invalid_argument)
		throw std::invalid_argument{ "invalid_argument" };
	if (ec == std::errc::result_out_of_range)
		throw std::out_of_range{ "out_of_range" };

	return result;
}

BOOL HookIAT(HMODULE callerModule, char const* targetModule, const void* targetFunction, void* detourFunction)
{
	auto* base = (uint8_t*)callerModule;
	const auto* dos_header = (IMAGE_DOS_HEADER*)base;
	const auto nt_headers = (IMAGE_NT_HEADERS*)(base + dos_header->e_lfanew);
	const auto* imports = (IMAGE_IMPORT_DESCRIPTOR*)(base + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (int i = 0; imports[i].Characteristics; i++)
	{
		const char* name = (const char*)(base + imports[i].Name);
		if (lstrcmpiA(name, targetModule) != 0)
			continue;

		void** thunk = (void**)(base + imports[i].FirstThunk);

		for (; *thunk; thunk++)
		{
			const void* import = *thunk;

			if (import != targetFunction)
				continue;

			DWORD oldState;
			if (!VirtualProtect(thunk, sizeof(void*), PAGE_READWRITE, &oldState))
				return FALSE;

			*thunk = detourFunction;

			VirtualProtect(thunk, sizeof(void*), oldState, &oldState);

			return TRUE;
		}
	}

	return FALSE;
}

};
