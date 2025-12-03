#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>

struct ModuleInfo {
	DWORD64 base;
	DWORD64 size;
};

class Memory 
{
private:
	HANDLE hProcess;
	DWORD processID;
	HANDLE handle_;

	DWORD GetProcessID(const std::string& processName);
	DWORD64 GetModuleBase(DWORD pid, const std::string& moduleName);

public:
	ModuleInfo base_client_;
	ModuleInfo base_hw_;

	Memory();
	~Memory();

	bool AttachProcess(const std::string& processName);
	ModuleInfo GetModule(const std::string& moduleName);
	DWORD64 GetModuleAddress(const std::string& moduleName);

	template<typename T>
	T read(DWORD64 address) 
	{
		T buffer;
		ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
		return buffer;
	}

	bool read(DWORD64 address, void* buffer, size_t size);

	template<typename T>
	T ReadModuleBuffer(void* buffer, DWORD offset) 
	{
		return *reinterpret_cast<T*>(static_cast<char*>(buffer)+offset);
	}

	bool ReadHugeMemory(DWORD64 address, void* buffer, size_t size);

	template<typename T>
	bool write(DWORD64 address, T value) {
		return WriteProcessMemory(hProcess, (LPVOID)address, &value, sizeof(T), NULL) != 0;
	}

	bool write(DWORD64 address, void* buffer, size_t size);
};