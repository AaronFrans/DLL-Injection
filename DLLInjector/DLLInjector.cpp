#include <iostream>
#include <string>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlstr.h>
#include <VersionHelpers.h>

#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM)
{
	constexpr int bufferSize = 1024; // Adjust the buffer size as needed
	char String[bufferSize];
	if (!hWnd)
		return TRUE; //Not a Window

	if (!IsWindowVisible(hWnd))
		return TRUE; //Not Visible

	if (!SendMessage(hWnd, WM_GETTEXT, sizeof(String), reinterpret_cast<LPARAM>(String)))
		return TRUE; //No window title

#ifdef _WIN64
	// 64-bit code
	GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
#else
	// 32-bit code
	GetWindowLong(hWnd, GWL_HINSTANCE);
#endif

	DWORD  dwProcessId;
	GetWindowThreadProcessId(hWnd, &dwProcessId);


	std::cout << "PID: " << dwProcessId << '\t' << String << '\t' << std::endl;

	return TRUE;
}

BOOL InjectDLL(DWORD ProcessId)
{
	Sleep(50);

	std::string dllPath;
	do {
		std::cout << "Enter the path to the DLL: ";
		std::getline(std::cin, dllPath);

		// Validate the path (you may want to add more sophisticated validation)
		if (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
			std::cout << "Entered path: " << dllPath << std::endl;
			std::cout << "Invalid path. Try again.\n";
		}
	} while (GetFileAttributesA(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES);

	LPVOID LoadLibAddy, RemoteString;

	if (!ProcessId)
		return false;

	HANDLE Proc = OpenProcess(CREATE_THREAD_ACCESS, FALSE, ProcessId);

	if (!Proc)
	{
		std::cout << "OpenProcess() failed: " << GetLastError() << std::endl;
		return false;
	}

	LoadLibAddy = reinterpret_cast<LPVOID>(GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"));

	RemoteString = reinterpret_cast<LPVOID>(VirtualAllocEx(Proc, NULL, strlen(dllPath.c_str()) + 1, MEM_COMMIT, PAGE_READWRITE));
	WriteProcessMemory(Proc, RemoteString, reinterpret_cast<LPCVOID>(dllPath.c_str()), strlen(dllPath.c_str()) + 1, NULL);
	CreateRemoteThread(Proc, NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibAddy), RemoteString, NULL, NULL);

	CloseHandle(Proc);

	return true;

}

int main()
{
	if (!IsWindowsXPOrGreater())
	{
		std::cout << "Method not supported by OS. Terminating\n";
		return 0;
	}

	std::cout << "Available Targets: \n\n" << std::endl;
	EnumWindows(EnumWindowsProc, NULL);
	std::cout << "\nChoose you PID:" << std::endl;
	DWORD PID;
	std::cin >> PID;

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.clear();


	InjectDLL(PID);

}