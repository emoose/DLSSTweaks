#define WIN32_LEAN_AND_MEAN
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>
#include <tchar.h>

#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/msvc_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <ini.h>

#include "DLSSTweaks.hpp"
#include "Proxy.hpp"

#include "resource.h" // TWEAKS_VER_STR

const wchar_t* NgxFileName = L"_nvngx.dll";
const wchar_t* DlssFileName = L"nvngx_dlss.dll";
const char* DlssFileNameA = "nvngx_dlss.dll";
const wchar_t* DlssgFileName = L"nvngx_dlssg.dll";
const char* DlssgFileNameA = "nvngx_dlssg.dll";
const wchar_t* DlssdFileName = L"nvngx_dlssd.dll";
const char* DlssdFileNameA = "nvngx_dlssd.dll";

const wchar_t* LogFileName = L"dlsstweaks.log";
const wchar_t* IniFileName = L"dlsstweaks.ini";
const wchar_t* ErrorFileName = L"dlsstweaks_error.log";

std::filesystem::path ExePath;
std::filesystem::path DllPath;
std::filesystem::path LogPath;
std::filesystem::path IniPath;

UserSettings settings;
DlssSettings dlss;

std::mutex initThreadFinishedMutex;
std::condition_variable initThreadFinishedVar;
bool initThreadFinished = false;

// In nvngx.dll wrapper mode the game might call DLSS functions immediately after loading DLL (ie. right after DllMain)
// before our InitThread has actually finished reading settings etc
// This func will just check if finished & wait for it if not, seems to be safe to let the NVNGX calls wait for this
void WaitForInitThread()
{
	if (initThreadFinished)
		return;

	std::unique_lock lock(initThreadFinishedMutex);
	initThreadFinishedVar.wait(lock, [] { return initThreadFinished; });
}

// LoadLibraryXXX hook so we can redirect DLSS library load to users choice
SafetyHookInline LoadLibraryExW_Orig;
SafetyHookInline LoadLibraryExA_Orig;
SafetyHookInline LoadLibraryA_Orig;
SafetyHookInline LoadLibraryW_Orig;

std::once_flag dlssOverrideMessage;
HMODULE __stdcall LoadLibraryExW_Hook(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	std::filesystem::path libPath = lpLibFileName;
	const auto filenameStr = libPath.filename().string();

	// Check if filename matches any DLL user has requested to override, change to the user-specified path if so
	for (auto& [overrideDllName, overridePath] : settings.dllPathOverrides)
	{
		if (_stricmp(filenameStr.c_str(), overrideDllName.c_str()) != 0)
			continue;

		if (!overridePath.empty())
		{
			spdlog::info("DLLPathOverrides: redirecting {} to new path {}", libPath.string(), overridePath.string());

			if (utility::exists_safe(overridePath))
				libPath = overridePath;
			else
				spdlog::error("DLLPathOverrides: override DLL no longer exists, skipping... (path: {})", overridePath.string());
		}
		break;
	}

	const std::wstring libPathStr = libPath.wstring();
	return LoadLibraryExW_Orig.stdcall<HMODULE>(libPathStr.c_str(), hFile, dwFlags);
}

HMODULE __stdcall LoadLibraryExA_Hook(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
{
	const std::filesystem::path libPath = lpLibFileName;
	const auto libPathStr = libPath.wstring();
	return LoadLibraryExW_Hook(libPathStr.c_str(), hFile, dwFlags);
}

HMODULE __stdcall LoadLibraryW_Hook(LPCWSTR lpLibFileName)
{
	return LoadLibraryExW_Hook(lpLibFileName, 0, 0);
}

HMODULE __stdcall LoadLibraryA_Hook(LPCSTR lpLibFileName)
{
	return LoadLibraryExA_Hook(lpLibFileName, 0, 0);
}

// LoaderNotificationCallback is called whenever a DLL is loaded into the process
// This is used to watch for _nvngx / nvngx_dlss loads, and if found then our XXX::init function will hook the DllMain entrypoint of them
// In that entrypoint hook we can handle applying/removing our hooks from the DLL via ATTACH/DETACH events
// Seems to be the easiest method for us to detect module unload, otherwise we'd need to hook a bunch of FreeLibrary funcs etc
// (since LoaderNotificationCallback for unloads is only called after the DLL has unloaded, making resetting our InlineHooks more difficult...)
void* dll_notification_cookie = nullptr;
void __stdcall LoaderNotificationCallback(unsigned long notification_reason, const LDR_DLL_NOTIFICATION_DATA* notification_data, void* context)
{
	if (notification_reason == LDR_DLL_NOTIFICATION_REASON_LOADED)
	{
		// If user has overridden the nvngx_dlss path, use the filename they specified in our comparisons below
		std::wstring dlssName = DlssFileName;
		std::wstring dlssgName = DlssgFileName;
		std::wstring dlssdName = DlssdFileName;

		if (settings.dllPathOverrides.contains(DlssFileNameA))
			dlssName = settings.dllPathOverrides[DlssFileNameA].filename().wstring();
		if (settings.dllPathOverrides.contains(DlssgFileNameA))
			dlssgName = settings.dllPathOverrides[DlssgFileNameA].filename().wstring();
		if (settings.dllPathOverrides.contains(DlssdFileNameA))
			dlssdName = settings.dllPathOverrides[DlssdFileNameA].filename().wstring();

		// A module was loaded in, check if NGX/DLSS and apply hooks if so
		const std::wstring dllName(notification_data->Loaded.BaseDllName->Buffer, notification_data->Loaded.BaseDllName->Length / sizeof(WCHAR));
		if (!_wcsicmp(dllName.c_str(), NgxFileName))
		{
			nvngx::init((HMODULE)notification_data->Loaded.DllBase);
		}
		else if (!_wcsicmp(dllName.c_str(), dlssName.c_str()))
		{
			nvngx_dlss::init((HMODULE)notification_data->Loaded.DllBase);
		}
		else if (!_wcsicmp(dllName.c_str(), dlssgName.c_str()))
		{
			nvngx_dlssg::init((HMODULE)notification_data->Loaded.DllBase);
		}
		else if (!_wcsicmp(dllName.c_str(), dlssdName.c_str()))
		{
			nvngx_dlssd::init((HMODULE)notification_data->Loaded.DllBase);
		}
	}
}

HMODULE ourModule = 0;
bool attachResult = false;

unsigned int __stdcall InitThread(void* param)
{
	WCHAR modulePath[4096];
	GetModuleFileNameW(GetModuleHandleA(0), modulePath, 4096);
	ExePath = std::filesystem::path(modulePath);
	GetModuleFileNameW(ourModule, modulePath, 4096);
	DllPath = std::filesystem::path(modulePath);

	if (settings.disableAllTweaks)
	{
		// Try warning user via error log file
		// (this used to use a Win32 MessageBox too, but some fullscreen games had issues when MessageBox showed, even in separate thread, so that was cut)
		std::string warningText =
			"Warning: multiple versions of DLSSTweaks have attempted to load into the game process.\n\nIf you recently tried to update the DLL, an older version may still be present & loaded in.\n\nCheck your game folder for files such as dxgi.dll / xinput1_3.dll / nvngx.dll and remove any extra versions.";

		try
		{
			std::ofstream warningFile(ExePath.parent_path() / ErrorFileName);
			warningFile << warningText << "\r\n";
			warningFile.close();

			std::ofstream warningFileDll(DllPath.parent_path() / ErrorFileName);
			warningFileDll << warningText << "\r\n";
			warningFileDll.close();
		}
		catch (const std::exception&)
		{
		}

		// Skip InitThread if disableAllTweaks was set...
		{
			std::lock_guard lock(initThreadFinishedMutex);
			initThreadFinished = true;
			initThreadFinishedVar.notify_all();
		}

		return 0;
	}

	// spdlog setup
	{
		// Log is always written next to our DLL
		LogPath = DllPath.parent_path() / LogFileName;

		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back(std::make_shared<spdlog::sinks::msvc_sink_mt>(true));
		try
		{
			sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(LogPath.string(), true));
		}
		catch (const std::exception&)
		{
			// spdlog failed to open log file for writing (happens in some WinStore apps)
			// let's just try to continue instead of crashing
		}

		auto combined_logger = std::make_shared<spdlog::logger>("", begin(sinks), end(sinks));
		combined_logger->set_level(spdlog::level::info);
		spdlog::set_default_logger(combined_logger);
		spdlog::flush_on(spdlog::level::debug);
	}

	spdlog::info("DLSSTweaks v{}, by emoose: {} wrapper loaded", TWEAKS_VER_STR, DllPath.filename().string());
	spdlog::info("Game path: {}", ExePath.string());
	spdlog::info("DLL path: {}", DllPath.string());

	spdlog::info("---");

	// Read config from next to DLL first, and then from next to EXE
	// So with a global injector, you could keep a global config stored next to the DLL, and then per-game overrides kept next to the EXE
	{
		if (DllPath.parent_path() != ExePath.parent_path())
		{
			IniPath = DllPath.parent_path() / IniFileName;
			if (utility::exists_safe(IniPath))
			{
				if (settings.read(IniPath))
					spdlog::info("Config read from {}", IniPath.string());
				else
					spdlog::error("Failed to read config from {}", IniPath.string());
			}
		}

		IniPath = ExePath.parent_path() / IniFileName;
		if (utility::exists_safe(IniPath))
		{
			if (settings.read(IniPath))
				spdlog::info("Config read from {}", IniPath.string());
			else
				spdlog::error("Failed to read config from {}", IniPath.string());
		}

		// IniPath will point to the INI next to game EXE after this, so that we can monitor any updates to that INI
	}

	// print msg about wrapping to log here, as nvngx wrap stuff was setup before spdlog was inited
	if (proxy::is_wrapping_nvngx)
		spdlog::info("Wrapped nvngx.dll, using funcptrs from original dll");
	else
		spdlog::info("Wrapped system DLL, watching for DLSS module load");

	// Register notification so we can learn of DLL loads/unloads
	auto LdrRegisterDllNotification =
		(LdrRegisterDllNotificationFunc)GetProcAddress(GetModuleHandle("ntdll.dll"), "LdrRegisterDllNotification");
	if (LdrRegisterDllNotification &&
		LdrRegisterDllNotification(0, &LoaderNotificationCallback, nullptr, &dll_notification_cookie) == STATUS_SUCCESS)
	{
		spdlog::debug("LdrRegisterDllNotification callback set");
	}
	else
	{
		spdlog::error("Failed to locate LdrRegisterDllNotification function address?"); // shouldn't happen
	}

	// Hook LoadLibrary so we can override DLSS path if desired
	if (!settings.dllPathOverrides.empty())
	{
		spdlog::debug("LoadLibrary hook set");

		auto* kernel32 = GetModuleHandleA("kernel32.dll");
		auto* LoadLibraryExW_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryExW") : nullptr;
		auto* LoadLibraryExA_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryExA") : nullptr;
		auto* LoadLibraryW_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryW") : nullptr;
		auto* LoadLibraryA_addr = kernel32 ? GetProcAddress(kernel32, "LoadLibraryA") : nullptr;
		if (!LoadLibraryExW_addr || !LoadLibraryExA_addr || !LoadLibraryW_addr || !LoadLibraryA_addr)
		{
			spdlog::error("Failed to find LoadLibrary address?");
		}
		else
		{
			LoadLibraryExW_Orig = safetyhook::create_inline(LoadLibraryExW_addr, LoadLibraryExW_Hook);
			LoadLibraryExA_Orig = safetyhook::create_inline(LoadLibraryExA_addr, LoadLibraryExA_Hook);
			LoadLibraryW_Orig = safetyhook::create_inline(LoadLibraryW_addr, LoadLibraryW_Hook);
			LoadLibraryA_Orig = safetyhook::create_inline(LoadLibraryA_addr, LoadLibraryA_Hook);
		}
	}

	// Init finished, allow any pending DLSS calls to continue
	{
		std::lock_guard lock(initThreadFinishedMutex);
		initThreadFinished = true;
		initThreadFinishedVar.notify_all();
	}

	if (!settings.disableIniMonitoring)
		settings.watch_for_changes(IniPath);

	return 0;
}

HANDLE processUniqueMutex;
std::mutex processUniqueMutexGuard;

// Tries to create a process-unique Win32 mutex
// If mutex already exists then another instance of this module must have already been loaded in
BOOL CheckDllAlreadyLoaded()
{
	std::scoped_lock lock{ processUniqueMutexGuard };

	if (processUniqueMutex) // If already set we must be the owner, so exit out
		return FALSE;

	// Generate unique name for the mutex based on the current process ID
	TCHAR szMutexName[MAX_PATH];
	_sntprintf_s(szMutexName, MAX_PATH, _T("Local\\DLSSTweaksMutex_%lu"), GetCurrentProcessId());

	processUniqueMutex = CreateMutex(NULL, TRUE, szMutexName);
	if (processUniqueMutex == NULL)
		return FALSE;

	DWORD dwError = GetLastError();
	if (dwError != ERROR_ALREADY_EXISTS)
	{
		// Mutex was created successfully, no other instance of the DLL is loaded in this process
		return FALSE;
	}

	// Another instance of the DLL must already be loaded in this process
	CloseHandle(processUniqueMutex); // Not using ReleaseMutex since we want to keep the mutex alive, just closing the handle
	processUniqueMutex = NULL;
	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, int ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		ourModule = hModule;
		attachResult = proxy::on_attach(ourModule);
		if (proxy::is_wrapping_nvngx)
			nvngx::init_from_proxy();

		// Check if another instance/version of this module has already loaded in
		if (CheckDllAlreadyLoaded())
		{
			// Disable tweaks to hopefully let game continue...
			settings.disableAllTweaks = true;

			// We'll alert user to the issue during InitThread, to prevent us from blocking game init
		}

		_beginthreadex(NULL, 0, InitThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH)
	{
		proxy::on_detach();
		if (processUniqueMutex)
			ReleaseMutex(processUniqueMutex);
	}

	return TRUE;
}
