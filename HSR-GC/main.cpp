#define _CRT_SECURE_NO_WARNINGS

#include "core.hpp"

namespace anti_cheat {
	static HMODULE h_module = 0;

	typedef HANDLE(WINAPI* CREATE_FILE_W)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

	CREATE_FILE_W p_CreateFileW = nullptr;
	CREATE_FILE_W t_CreateFileW;

	HANDLE WINAPI h_CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	{
		if (memcmp(lpFileName, L"\\\\.\\ACE-BASE", 24) == 0) {
			wprintf(L"[>] Thread (%i) attempting to communicate with anti-cheat driver -> %s\n", GetCurrentThreadId(), lpFileName);

			SuspendThread(GetCurrentThread()); // 200iq bypass for memory protection
		}

		return p_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	void Setup(uint64_t srbase) {
		if (MH_Initialize() != MH_OK)
		{
			puts("[-] Failed to initialize MinHook library");

			return;
		}

		if (MH_CreateHookApiEx(L"kernelbase", "CreateFileW", &anti_cheat::h_CreateFileW, reinterpret_cast<void**>(&p_CreateFileW), reinterpret_cast<void**>(&t_CreateFileW)) != MH_OK)
		{
			puts("[-] Failed to create hook for CreateFileW function");

			return;
		}

		if (MH_EnableHook(t_CreateFileW) != MH_OK)
		{
			puts("[-] Failed to enable hook for CreateFileW function");

			return;
		}

		utils::Write<uint32_t>(srbase + 0xFCDC0, 0xCCC3C031);
		utils::Write<uint8_t>(srbase + 0xF9940, 0xC3);
		utils::Write<uint16_t>(srbase + 0x1BCBA0, 0xFEEB);
	}
}

void Setup()
{
	if (!config::Load(anti_cheat::h_module))
	{
		config::Save();
		Sleep(200);

		if (!config::Load(anti_cheat::h_module)) {
			puts("[!] Failed to create config file");
			return;
		}
	}

	auto base_address = reinterpret_cast<uint64_t>(GetModuleHandleA("starrailbase.dll"));

	if (utils::GetTextSectionChecksum(base_address) != 0x1434A1A0) {

		globals::china_version = true;

		puts("[>] China game version detected\n[>] If you don't have the Chinese game version, please create a GitHub issue");
	}

	anti_cheat::Setup(base_address);

	Sleep(15000);

	if (!Direct3D.Initialization())
		puts("[-] Failed to setup Direct3D!");
	else
	{
		puts("[+] Direct3D setup successfully!");
		printf("[>] Direct3D Present: %p\n[>] Direct3D ResizeBuffers: %p\n", Direct3D.Present, Direct3D.ResizeBuffers);
	}
	if (!device::Setup())
		puts("[-] Failed to setup device hooks!");
	else
		puts("[+] device hooks setup successfully!");

	return overlay::Main();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		anti_cheat::h_module = hModule;

		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		freopen("CONIN$", "r", stdin);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Setup, 0, 0, 0);
	}

	return true;
}