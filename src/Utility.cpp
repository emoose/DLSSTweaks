#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdint>

namespace utility
{

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

		for (; thunk; thunk++)
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
