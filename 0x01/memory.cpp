#include "memory.h"
#include <iostream>

Memory::Memory() : hProcess(NULL), processID(0), handle_(NULL) 
{
	base_client_.base = 0;
	base_client_.size = 0;
	base_hw_.base = 0;
	base_hw_.size = 0;
}

Memory::~Memory() 
{
	if (hProcess) 
	{
		CloseHandle(hProcess);
	}
}

DWORD Memory::GetProcessID(const std::string& processName)
{
	DWORD pid = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (snapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	PROCESSENTRY32 processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(snapshot, &processEntry))
	{
		do 
		{
			if (_stricmp(processEntry.szExeFile, processName.c_str()) == 0)
			{
				pid = processEntry.th32ProcessID;
				break;
			}
		} 
		while (Process32Next(snapshot, &processEntry));
	}

	CloseHandle(snapshot);
	return pid;
}

DWORD64 Memory::GetModuleBase(DWORD pid, const std::string& moduleName)
{
	DWORD64 base = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

	if (snapshot == INVALID_HANDLE_VALUE) 
	{
		return 0;
	}

	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(snapshot, &moduleEntry)) {
		do 
		{
			if (_stricmp(moduleEntry.szModule, moduleName.c_str()) == 0)
			{
				base = (DWORD64)moduleEntry.modBaseAddr;
				break;
			}
		} 
		while (Module32Next(snapshot, &moduleEntry));
	}

	CloseHandle(snapshot);
	return base;
}

bool Memory::AttachProcess(const std::string& processName)
{
	processID = GetProcessID(processName);

	if (!processID)
	{
		return false;
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
	handle_ = hProcess;

	return hProcess != NULL;
}

ModuleInfo Memory::GetModule(const std::string& moduleName)
{
	ModuleInfo info = { 0, 0 };

	if (!processID) 
	{
		return info;
	}

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processID);

	if (snapshot == INVALID_HANDLE_VALUE) 
	{
		return info;
	}

	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(snapshot, &moduleEntry)) 
	{
		do
		{
			if (_stricmp(moduleEntry.szModule, moduleName.c_str()) == 0)
			{
				info.base = (DWORD64)moduleEntry.modBaseAddr;
				info.size = moduleEntry.modBaseSize;
				break;
			}
		} 
		while (Module32Next(snapshot, &moduleEntry));
	}

	CloseHandle(snapshot);
	return info;
}

DWORD64 Memory::GetModuleAddress(const std::string& moduleName)
{
	return GetModule(moduleName).base;
}

bool Memory::read(DWORD64 address, void* buffer, size_t size)
{
	SIZE_T bytesRead;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) != 0;
}

bool Memory::ReadHugeMemory(DWORD64 address, void* buffer, size_t size)
{
	SIZE_T bytesRead;
	return ReadProcessMemory(hProcess, (LPCVOID)address, buffer, size, &bytesRead) != 0;
}

bool Memory::write(DWORD64 address, void* buffer, size_t size)
{
	SIZE_T bytesWritten;
	return WriteProcessMemory(hProcess, (LPVOID)address, buffer, size, &bytesWritten) != 0;
}