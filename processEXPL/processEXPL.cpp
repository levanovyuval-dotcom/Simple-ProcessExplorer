#include <windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <conio.h>
#include <thread>
#include <chrono> 
#include <atomic> 
#include <string>
#include <psapi.h>
#include <iomanip>
#pragma comment(lib, "psapi.lib")
#include <vector> 
#include <algorithm>

struct ProcessInfo {
	DWORD pid;
	std::wstring name;
	SIZE_T ramUsageMB;
};


#define TH32CS_SNAPTHREAD 0x00000004
#define TH32CS_SNAPMODULE 0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define THREAD_SUSPEND_RESUME 0x0002

std::atomic<bool> keepRunning(true);
std::atomic<bool> stopped(false);
std::atomic<int> processCount(0);
void properties();

void instructions() {
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}
	std::wcout << L"Instructions:" << std::endl;
	std::wcout << L"Press 's' to stop the process list and access the menu." << std::endl;
	std::wcout << L"Press 'f' to find a process by name." << std::endl;
	std::wcout << L"Press 'p' to view properties of a process." << std::endl;
	std::wcout << L"Press 's' to suspend a process." << std::endl;
	std::wcout << L"Press 'r' to resume a suspended process." << std::endl;
	std::wcout << L"Press 'k' to kill a process." << std::endl;
	std::wcout << L"Press ESC to go back or exit the program." << std::endl;
	while (true) {
		if (_kbhit()) {
			if (_getch() == 27) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	Sleep(100);
	std::cout << "\nresuming";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(900);
	stopped = false;
}

void suspend() {
	int64_t targetPID;
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}

	std::wcout << L"Write the PID of the process you want to suspend: ";

	bool typing = true;
	std::wstring typeterm = L"";
	while (typing) {

		char key = _getch();
		if (key == 27) {
			Sleep(100);
			std::cout << "\nresuming";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(900);
			stopped = false;
			return;
		}
		else if (key == 8) {
			if (!typeterm.empty()) {
				typeterm.pop_back();
				std::wcout << L"\rWrite the PID of the process you want to suspend: " << typeterm << L" \b";
			}
		}
		else if (key == 13 || key == 10) {
			if (!typeterm.empty()) {
				typing = false;
			}
		}
		else if (isdigit(key)) {
			typeterm += key;
			std::wcout << key;
		}
	}

	targetPID = _wtoi(typeterm.c_str());
	std::wcout << std::endl;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, targetPID);
	if (hProcess != NULL) {
		wchar_t filePath[MAX_PATH];
		DWORD filePathSize = MAX_PATH;
		if (QueryFullProcessImageNameW(hProcess, 0, filePath, &filePathSize)) {
			std::wstring fullPath(filePath);
			size_t lastSlash = fullPath.find_last_of(L"\\");
			std::wstring justTheName = fullPath.substr(lastSlash + 1);

			std::wcout << L"You sure you want to suspend: " << justTheName << std::endl;
			char key = _getch();

			if (key == 13 || key == 10) {
				if (Thread32First(hThreadSnap, &te32)) {
					do {
						if (te32.th32OwnerProcessID == targetPID) {
							HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
							if (hThread != NULL) {
								SuspendThread(hThread);
								CloseHandle(hThread);
							}
						}
					} while (Thread32Next(hThreadSnap, &te32));
				}
				std::wcout << L"Process Suspended!" << std::endl;
			}
			else if (key == 27) {
				CloseHandle(hProcess);
				CloseHandle(hThreadSnap);
				Sleep(100);
				std::cout << "\nresuming";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(900);
				stopped = false;
				return;
			}
		}
		CloseHandle(hProcess);
	}
	CloseHandle(hThreadSnap);

	Sleep(100);
	std::cout << "\nresuming";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(900);
	stopped = false;
}

void resume() {
	int64_t targetPID;
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}

	std::wcout << L"Write the PID of the process you want to resume: ";

	bool typing = true;
	std::wstring typeterm = L"";
	while (typing) {

		char key = _getch();
		if (key == 27) {
			Sleep(100);
			std::cout << "\nresuming";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(900);
			stopped = false;
			return;
		}
		else if (key == 8) {
			if (!typeterm.empty()) {
				typeterm.pop_back();
				std::wcout << L"\rWrite the PID of the process you want to resume: " << typeterm << L" \b";
			}
		}
		else if (key == 13 || key == 10) {
			if (!typeterm.empty()) {
				typing = false;
			}
		}
		else if (isdigit(key)) {
			typeterm += key;
			std::wcout << key;
		}
	}

	targetPID = _wtoi(typeterm.c_str());
	std::wcout << std::endl;

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te32;
	te32.dwSize = sizeof(THREADENTRY32);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, targetPID);
	if (hProcess != NULL) {
		wchar_t filePath[MAX_PATH];
		DWORD filePathSize = MAX_PATH;
		if (QueryFullProcessImageNameW(hProcess, 0, filePath, &filePathSize)) {
			std::wstring fullPath(filePath);
			size_t lastSlash = fullPath.find_last_of(L"\\");
			std::wstring justTheName = fullPath.substr(lastSlash + 1);

			std::wcout << L"You sure you want to resume: " << justTheName << std::endl;
			char key = _getch();

			if (key == 13 || key == 10) {
				if (Thread32First(hThreadSnap, &te32)) {
					do {
						if (te32.th32OwnerProcessID == targetPID) {
							HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
							if (hThread != NULL) {
								ResumeThread(hThread);
								CloseHandle(hThread);
							}
						}
					} while (Thread32Next(hThreadSnap, &te32));
				}
				std::wcout << L"Process Resumed!" << std::endl;
			}
			else if (key == 27) {
				CloseHandle(hProcess);
				CloseHandle(hThreadSnap);
				Sleep(100);
				std::cout << "\nresuming";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(900);
				stopped = false;
				return;
			}
		}
		CloseHandle(hProcess);
	}
	CloseHandle(hThreadSnap);

	Sleep(100);
	std::cout << "\nresuming";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(900);
	stopped = false;
}

void find() {
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}
	std::cout << "\nFind process by name: ";
	std::wcout << L"----------------------------------------" << std::endl;

	bool searching = true;
	std::wstring searchTerm = L"";

	while (searching) {
		char key = _getch();

		if (key == 27) {
			searching = false;
		}
		else if (key == 8) {
			if (!searchTerm.empty()) {
				searchTerm.pop_back();
			}
		}
		else {
			searchTerm += key;
		}

		system("cls");
		std::wcout << L"Find process by name: " << searchTerm << L"_" << std::endl;

		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap != INVALID_HANDLE_VALUE) {
			PROCESSENTRY32 pe32;
			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hProcessSnap, &pe32)) {
				do {
					std::wstring exeName = pe32.szExeFile;

					std::wstring lowerExe = exeName;
					std::wstring lowerSearch = searchTerm;
					for (auto& c : lowerExe) c = towlower(c);
					for (auto& c : lowerSearch) c = towlower(c);

					if (lowerSearch.empty() || lowerExe.find(lowerSearch) != std::wstring::npos) {
						std::wcout << L"PID: " << pe32.th32ProcessID << L" Name: " << exeName << std::endl;
					}

				} while (Process32Next(hProcessSnap, &pe32));
			}
			CloseHandle(hProcessSnap);
		}
	}

	

		std::cout << "resuming";
		Sleep(300); std::cout << ".";
		Sleep(300); std::cout << ".";
		Sleep(300); std::cout << ".";
		Sleep(900);

		stopped = false;
	
}

void kill() {
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}

	int targetPID;
	std::wcout << L"Write the PID of the process you want to kill: ";

	char key = _getch();
	if (key == 27) {
		Sleep(100);

		std::cout << "\nresuming";
		Sleep(300); std::cout << ".";
		Sleep(300); std::cout << ".";
		Sleep(300); std::cout << ".";
		Sleep(900);

		stopped = false;
		return;
	}
	bool typing = true;
	std::wstring typeterm = L"";

	while (typing) {
		if (key == 27) {
			typing = false;
			Sleep(100);
			std::cout << "\nresuming";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(300); std::cout << ".";
			Sleep(900);
			stopped = false;
			return;
		}
		else if (key == 8) {
			if (!typeterm.empty()) {
				typeterm.pop_back();
				std::wcout << L"\rWrite the PID of the process you want to kill: " << typeterm << L" \b";
			}
		}
		else if (key == 13 || key == 10) {
			if (!typeterm.empty()) {
				typing = false;
			}
		}
		else if (isdigit(key)) {
			typeterm += key;
			std::wcout << key;
		}
		key = _getch();
	}


	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, _wtoi(typeterm.c_str()));

	if (hProcess == NULL) {
		std::cout << "\nFailed to open process! (Maybe it requires Admin rights or doesn't exist?)" << std::endl;
	}
	else {
		if (TerminateProcess(hProcess, 0)) {
			std::cout << "\nProcess " << _wtoi(typeterm.c_str()) << " was successfully terminated!" << std::endl;
		}
		else {
			std::cout << "\nFailed to terminate process. (Error code: " << GetLastError() << ")" << std::endl;
		}
		CloseHandle(hProcess);
	}
	Sleep(100);

	std::cout << "resuming";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(300); std::cout << ".";
	Sleep(900);

	stopped = false;
}

void stop() {
	stopped = true;
	while (stopped) {
		if (_kbhit()) {
			char key = _getch();
			if (key == 27) {
				std::cout << "resuming";
				Sleep(300);
				std::cout << ".";
				Sleep(300);
				std::cout << ".";
				Sleep(300);
				std::cout << ".";
				Sleep(900);
				stopped = false;
			}
			else
				if (key == 'k') {
					std::cout << "\n" << key << " key pressed - kill " << std::endl;
					Sleep(100);
					kill();
				}
				else
					if (key == 'f') {
						find();
					}
					else
						if (key == 'p') {
							std::cout << "\n" << key << " key pressed - properties " << std::endl;
							Sleep(100);
							properties();
						}
						else
							if (key == 's') {
								std::cout << "\n" << key << " key pressed - suspend  " << std::endl;
								Sleep(100);
								suspend();
							}
							else
								if (key == 'r') {
									std::cout << "\n" << key << " key pressed - resume  " << std::endl;
									Sleep(100);
									resume();
								}
								else
									if (key == 'i') {
										std::cout << "\n" << key << " key pressed - instructions " << std::endl;
										Sleep(100);
										instructions();
									}


		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

void keyboardListener() {
	while (keepRunning) {
		if (_kbhit()) {
			char key = _getch();
			if (key == 27) {
				std::cout << "\nquiting";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(300); std::cout << ".";
				Sleep(900);
				keepRunning = false;
				FreeConsole(); 
				ExitProcess(0); 
				break;
			}
			else
				if (key == 's') {
					
					stop();
				}

		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
}

void properties() {
	for (int i = 0; i < processCount; i++) {
		std::cout << "\033[A\033[2K";
	}

	int targetPID;
	std::wcout << L"Write the PID of the process you want to open properties for: ";

	char key = _getch();
	if (key == 27) {
		Sleep(1000);
		std::cout << "\nresuming";
		Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(900);
		stopped = false;
		return;
	}
	bool typing = true;
	std::wstring typeterm = L"";

	while (typing) {
		if (key == 27) {
			typing = false;
			Sleep(1000);
			std::cout << "\nresuming";
			Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(900);
			stopped = false;
			return;
		}
		else if (key == 8) {
			if (!typeterm.empty()) {
				typeterm.pop_back();
				std::wcout << L"\rWrite the PID of the process you want to open properties for: " << typeterm << L" \b";
			}
		}
		else if (key == 13 || key == 10) {
			if (!typeterm.empty()) {
				typing = false;
			}
		}
		else if (isdigit(key)) {
			typeterm += key;
			std::wcout << key;
		}
		key = _getch();
	}

	int TargetPID = _wtoi(typeterm.c_str());

	
	std::wstring processName = L"Unknown";
	HANDLE hProcessSnapForName = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnapForName != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pe32Name;
		pe32Name.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnapForName, &pe32Name)) {
			do {
				if (pe32Name.th32ProcessID == TargetPID) {
					processName = pe32Name.szExeFile;
					break;
				}
			} while (Process32Next(hProcessSnapForName, &pe32Name));
		}
		CloseHandle(hProcessSnapForName);
	}

	system("cls");

	
	std::wcout << L"--- Properties for: " << processName << L" (PID: " << TargetPID << L") ---" << std::endl;

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, TargetPID);

	if (hProcess != NULL) {
		wchar_t filePath[MAX_PATH];
		DWORD filePathSize = MAX_PATH;

		if (QueryFullProcessImageNameW(hProcess, 0, filePath, &filePathSize)) {
			std::wcout << L"File Path: " << filePath << std::endl;
		}
		else {
			std::wcout << L"File Path: [Access Denied or Not Found]" << std::endl;
		}

		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
			std::wcout << L"Current RAM: " << pmc.WorkingSetSize / (1024 * 1024) << L" MB" << std::endl;
			std::wcout << L"Peak RAM: " << pmc.PeakWorkingSetSize / (1024 * 1024) << L" MB" << std::endl;
		}
		
		HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		THREADENTRY32 te32;
		te32.dwSize = sizeof(THREADENTRY32);

		if (Thread32First(hThreadSnap, &te32)) {
			bool headerPrinted = false; 

			do {
				if (te32.th32OwnerProcessID == TargetPID) {
					if (!headerPrinted) {
						std::cout << "\n________________________________________________________________________________" << std::endl;
						std::cout << "\nThreads list: " << std::endl;
						headerPrinted = true;
					}

					
					std::wcout << std::left << L"TID: " << std::setw(6) << te32.th32ThreadID
						<< L" | Priority: " << std::setw(3) << te32.tpBasePri;

					HANDLE hThread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, te32.th32ThreadID);
					if (hThread != NULL) {
						PWSTR threadName = nullptr;
						HRESULT hr = GetThreadDescription(hThread, &threadName);

						if (SUCCEEDED(hr) && threadName != nullptr && threadName[0] != L'\0') {
							std::wcout << L" | Name: " << threadName;
							LocalFree(threadName);
						}
						else { 
							std::wcout << L" | No name";
						}
						CloseHandle(hThread);
					}
					else {
						std::wcout << L" | No name (Access Denied)";
					}
					std::wcout << std::endl;
				}
			} while (Thread32Next(hThreadSnap, &te32));
		}
		CloseHandle(hThreadSnap);

		
		HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, TargetPID);
		if (hModuleSnap != INVALID_HANDLE_VALUE) {
			MODULEENTRY32 Me32;
			Me32.dwSize = sizeof(MODULEENTRY32);

			if (Module32First(hModuleSnap, &Me32)) { 
				std::cout << "\n________________________________________________________________________________" << std::endl;
				std::cout << "\nDLL list: " << std::endl;
				do {
					std::wcout << L"DLL: " << Me32.szModule
						<< L", Base: " << (void*)Me32.modBaseAddr
						<< L", Path: " << Me32.szExePath << std::endl;

				} while (Module32Next(hModuleSnap, &Me32));
			}
			CloseHandle(hModuleSnap);
		}
		else {
			
			
		}

		CloseHandle(hProcess);
	}
	else {
		std::cout << "Failed to open process! (Error: " << GetLastError() << ")" << std::endl;
	}

	std::cout << "\nPress ESC to return..." << std::endl;
	while (true) {
		if (_kbhit()) {
			if (_getch() == 27) break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}

	std::cout << "\nresuming";
	Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(300); std::cout << "."; Sleep(900);
	stopped = false;
}

int main()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD out_mode;
	if (GetConsoleMode(hOut, &out_mode)) {
		out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(hOut, out_mode);
	}

	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
	DWORD prev_mode;
	if (GetConsoleMode(hInput, &prev_mode)) {
		prev_mode &= ~ENABLE_QUICK_EDIT_MODE;
		SetConsoleMode(hInput, prev_mode);
	}

	processCount = 0;
	bool printing = false;

	std::thread inputThread(keyboardListener);

	while (keepRunning) {
		processCount = 0;

		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			std::cout << "Failed to take snapshot!" << std::endl;
			return 1;
		}

		printing = true;

		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (!Process32First(hProcessSnap, &pe32)) {
			std::cout << "Failed to get first process." << std::endl;
			CloseHandle(hProcessSnap);
			return 1;
		}

		std::vector<ProcessInfo> processList;

		do {
			SIZE_T ramUsageMB = 0;

			HANDLE hMemProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);

			if (hMemProcess != NULL) {
				PROCESS_MEMORY_COUNTERS pmc;
				if (GetProcessMemoryInfo(hMemProcess, &pmc, sizeof(pmc))) {
					ramUsageMB = pmc.WorkingSetSize / (1024 * 1024);
				}
				CloseHandle(hMemProcess);
			}

			ProcessInfo currentProcess;

			
			currentProcess.pid = pe32.th32ProcessID;
			currentProcess.name = pe32.szExeFile;
			currentProcess.ramUsageMB = ramUsageMB;

			
			processList.push_back(currentProcess);

			processCount++;

		} while (Process32Next(hProcessSnap, &pe32));

		
		std::sort(processList.begin(), processList.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
			return a.ramUsageMB > b.ramUsageMB; 
			});

		
		for (int i = 0; i < processList.size(); i++) {
			std::wcout << std::left
				<< L"PID: " << std::setw(8) << processList[i].pid
				<< L" RAM: " << std::setw(4) << processList[i].ramUsageMB << L" MB    "
				<< L"Name: " << processList[i].name << std::endl;
		}

		CloseHandle(hProcessSnap);

		std::wcout << L" " << std::endl;
		std::wcout << processCount << L" processes were found" << std::endl;

		printing = false;

		for (int i = 0; i < 25; i++) {
			if (stopped) {
				break;
			}
			Sleep(100);
		}

		if (stopped) {
			std::wcout << L" " << std::endl;
			std::wcout << L" f - to find a process" << std::endl;
			std::wcout << L" s - to suspend a process" << std::endl;
			std::wcout << L" r - to resume a process" << std::endl;
			std::wcout << L" k - to kill a process" << std::endl;
			std::wcout << L" i - to view instructions" << std::endl;


			while (stopped) {
				Sleep(100);
			}

			for (int l = 0; l < 5; l++) {
				std::cout << "\033[A\033[2K";
			}
		}

		for (int l = 0; l < 2; l++) {
			std::cout << "\033[A\033[2K";
		}

		for (int i = 0; i < processCount; i++) {
			std::cout << "\033[A\033[2K";
		}
	}

	if (inputThread.joinable()) {
		inputThread.join();
	}

	return 0;
}