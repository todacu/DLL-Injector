#include "stdafx.h"
using namespace std;

HANDLE current_console;
const char* dll_file = "D:\\Project\\File.dll";
const wchar_t* process = L"game.exe";

#pragma warning(disable : 4996)


/* Console Colour */
void set_colour(int id)
{
	SetConsoleTextAttribute(current_console, id);
}

/* Injection Process */
bool inject_dll(DWORD process_id, const char* dll_name)
{
	char buf[50] = { 0 };
	LPVOID RemoteString, LoadLibAddy;

	if (!process_id)
	{
		set_colour(12);
		cout << "Error: Injection code 1 (IS GTA OPEN?)\n";
		return false;
	}

	const auto process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
	if (!process)
	{
		set_colour(12);
		cout << "Error: Injection code 2\n";
		return false;
	}

	LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
	RemoteString = (LPVOID)VirtualAllocEx(process, NULL, strlen(dll_name), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(process, (LPVOID)RemoteString, dll_name, strlen(dll_name), NULL);
	CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);

	CloseHandle(process);
	return true;
}

DWORD get_target_thread_id_from_proc(const PCWSTR process_name)
{
	PROCESSENTRY32 pe;
	HANDLE thSnapShot;
	BOOL retval, ProcFound = false;

	thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (thSnapShot == INVALID_HANDLE_VALUE)
	{
		set_colour(12);
		cout << "Error: Injection code 3\n";
		return false;
	}

	pe.dwSize = sizeof(PROCESSENTRY32);

	retval = Process32First(thSnapShot, &pe);
	while (retval)
	{
		if (StrStrI(pe.szExeFile, process_name))
			return pe.th32ProcessID;
		retval = Process32Next(thSnapShot, &pe);
	}
	return 0;
}

/* Administrator Perms */
bool EnableDebugPrivilege()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return false;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
		return false;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, false, &tkp, sizeof(tkp), NULL, NULL))
		return false;

	if (!CloseHandle(hToken))
		return false;

	return true;
}


/* Injects the DLL */
bool inject_file()
{
	set_colour(14);
	cout << "Preparing File Stream!\n";
	set_colour(10);
	cout << "Successfully Streamed DLL (";
	set_colour(14);
	cout << "Version 0.1";
	set_colour(10);
	cout << ")\n";

	return inject_dll(get_target_thread_id_from_proc(process), dll_file);
}


/* Main Program */
int main()
{
	ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
	while (true)
	{
		if (GetAsyncKeyState(VK_F10))
		{
			// This should be done by getting the appropriate path rather than assuming they are using 'C' as their OS drive 
			PlaySound(L"C:\\WINDOWS\\Media\\tada.wav", nullptr, SND_ASYNC);

			bool admin = EnableDebugPrivilege();
			if (!admin)
			{
				set_colour(12);
				cout << "Error: No Admin Perms!\n";
				string nothing;
				cin >> nothing;
				exit(1);
			}

			SetConsoleTitle(L"Injector");
			current_console = GetStdHandle(STD_OUTPUT_HANDLE);


			const auto injection_result = inject_file();
			if (injection_result)
			{
				set_colour(10);
				cout << "Injected Successfully!\n";
			}

			set_colour(15);
			Sleep(1000);
		}
		Sleep(10);
	}
	return 0;
}

