#pragma once
#include <SafetyHook.hpp>
#include <filesystem>
#include <ini.h>

namespace utility
{
std::string DLSS_PresetEnumToName(unsigned int val);
unsigned int DLSS_PresetNameToEnum(const std::string& val);
std::pair<int, int> ParseResolution(std::string_view val);
inline bool ValidResolution(const std::pair<int, int> val)
{
	return val.first > 0 && val.second > 0;
}

std::string ini_get_string_safe(inih::INIReader& ini, const std::string& section, const std::string& name, std::string&& default_v);
float stof_nolocale(std::string_view s, bool strict = false);

// exists can cause exception under certain apps (UWP?), grr...
inline bool exists_safe(const std::filesystem::path& path)
{
	bool exists = false;
	try
	{
		exists = std::filesystem::exists(path);
	}
	catch (const std::exception&)
	{
		exists = false;
	}
	return exists;
}

BOOL HookIAT(HMODULE callerModule, char const* targetModule, void* targetFunction, void* detourFunction);

std::string ModuleVersion(const std::filesystem::path& module_path);
inline void* ModuleEntryPoint(HMODULE hmod)
{
	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)hmod;
	PIMAGE_NT_HEADERS nt_headers = (PIMAGE_NT_HEADERS)((PBYTE)hmod + dos_header->e_lfanew);
	return (PBYTE)hmod + nt_headers->OptionalHeader.AddressOfEntryPoint;
}
};

// Matches the order of NVSDK_NGX_Parameter vftable inside _nvngx.dll (which should never change unless they want to break compatibility)
struct NVSDK_NGX_Parameter_vftable
{
	/* 0x00 */ void* SetVoidPointer;
	/* 0x08 */ void* SetD3d12Resource;
	/* 0x10 */ void* SetD3d11Resource;
	/* 0x18 */ void* SetI;
	/* 0x20 */ void* SetUI;
	/* 0x28 */ void* SetD;
	/* 0x30 */ void* SetF;
	/* 0x38 */ void* SetULL;
	/* 0x40 */ void* GetVoidPointer;
	/* 0x48 */ void* GetD3d12Resource;
	/* 0x50 */ void* GetD3d11Resource;
	/* 0x58 */ void* GetI;
	/* 0x60 */ void* GetUI;
	/* 0x68 */ void* GetD;
	/* 0x70 */ void* GetF;
	/* 0x78 */ void* GetULL;
	/* 0x80 */ void* Reset;
};

// from safetyhook: https://github.com/cursey/safetyhook/blob/35d28aab6d10f9ed17499df8461c92721f0db025/src/InlineHook.cpp#LL15
class UnprotectMemory {
public:
	UnprotectMemory(uintptr_t address, size_t size) : m_address{ address }, m_size{ size } {
		VirtualProtect((LPVOID)m_address, m_size, PAGE_EXECUTE_READWRITE, &m_protect);
	}

	~UnprotectMemory() { VirtualProtect((LPVOID)m_address, m_size, m_protect, &m_protect); }

private:
	uintptr_t m_address{};
	size_t m_size{};
	DWORD m_protect{};
};

// defs below from chromium: https://source.chromium.org/chromium/chromium/src/+/main:chrome/common/conflicts/module_watcher_win.cc

// These structures and functions are documented in MSDN, see
// http://msdn.microsoft.com/en-us/library/gg547638(v=vs.85).aspx
// there are however no headers or import libraries available in the
// Platform SDK. They are declared outside of the anonymous namespace to
// allow them to be forward declared in the header file.
enum {
	// The DLL was loaded. The NotificationData parameter points to a
	// LDR_DLL_LOADED_NOTIFICATION_DATA structure.
	LDR_DLL_NOTIFICATION_REASON_LOADED = 1,
	// The DLL was unloaded. The NotificationData parameter points to a
	// LDR_DLL_UNLOADED_NOTIFICATION_DATA structure.
	LDR_DLL_NOTIFICATION_REASON_UNLOADED = 2,
};

// Structure that is used for module load notifications.
struct LDR_DLL_LOADED_NOTIFICATION_DATA {
	// Reserved.
	ULONG Flags;
	// The full path name of the DLL module.
	PCUNICODE_STRING FullDllName;
	// The base file name of the DLL module.
	PCUNICODE_STRING BaseDllName;
	// A pointer to the base address for the DLL in memory.
	PVOID DllBase;
	// The size of the DLL image, in bytes.
	ULONG SizeOfImage;
};
using PLDR_DLL_LOADED_NOTIFICATION_DATA = LDR_DLL_LOADED_NOTIFICATION_DATA*;

// Structure that is used for module unload notifications.
struct LDR_DLL_UNLOADED_NOTIFICATION_DATA {
	// Reserved.
	ULONG Flags;
	// The full path name of the DLL module.
	PCUNICODE_STRING FullDllName;
	// The base file name of the DLL module.
	PCUNICODE_STRING BaseDllName;
	// A pointer to the base address for the DLL in memory.
	PVOID DllBase;
	// The size of the DLL image, in bytes.
	ULONG SizeOfImage;
};
using PLDR_DLL_UNLOADED_NOTIFICATION_DATA = LDR_DLL_UNLOADED_NOTIFICATION_DATA*;

// Union that is used for notifications.
union LDR_DLL_NOTIFICATION_DATA {
	LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
	LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
};
using PLDR_DLL_NOTIFICATION_DATA = LDR_DLL_NOTIFICATION_DATA*;

// Signature of the notification callback function.
using PLDR_DLL_NOTIFICATION_FUNCTION =
VOID(CALLBACK*)(ULONG notification_reason,
	const LDR_DLL_NOTIFICATION_DATA* notification_data,
	PVOID context);

// Signatures of the functions used for registering DLL notification callbacks.
using LdrRegisterDllNotificationFunc =
NTSTATUS(NTAPI*)(ULONG flags,
	PLDR_DLL_NOTIFICATION_FUNCTION notification_function,
	PVOID context,
	PVOID* cookie);
using LdrUnregisterDllNotificationFunc = NTSTATUS(NTAPI*)(PVOID cookie);
