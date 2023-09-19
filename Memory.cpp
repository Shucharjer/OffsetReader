#include "Memory.h"

Memory::Process::~Process()
{
	if (handle)
	{
		CloseHandle(handle);
		handle = NULL;
	}
}

Memory::SnapshotReader::SnapshotReader(Process& process, const char* processName)
{
	// free
	if (process.handle) CloseHandle(process.handle);

	// initialize
	process.id = NULL;
	process.handle = NULL;

	// create snapshot
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) return;

	// create process entry
	PROCESSENTRY32 processEntry{ };
	processEntry.dwSize = sizeof(PROCESSENTRY32);
	
	// traverse
	bool flag = Process32First(hSnapshot, &processEntry);
	while (flag)
	{
		if (strcmp(processName, processEntry.szExeFile))
			flag = Process32Next(hSnapshot, &processEntry);
		else
		{
			process.id = processEntry.th32ProcessID;
			process.handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.id);
			break;
		}
	}

	// fress
	if (hSnapshot)
		CloseHandle(hSnapshot);
}

pAddr Memory::SnapshotReader::getModuleAddress(Process& process, const char* moduleName)
{
	// the same as Memory::SnapshotReader::SnapshotReader(Process& process, const char* processName)

	if (!process.avalible()) return NULL;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, process.id);
	if (hSnapshot == INVALID_HANDLE_VALUE) return NULL;

	MODULEENTRY32 moduleEntry = { };
	moduleEntry.dwSize = sizeof(::MODULEENTRY32);

	pAddr result = 0;

	bool flag = Module32First(hSnapshot, &moduleEntry);
	while (flag)
	{
		if (strcmp(moduleName, moduleEntry.szModule))
			flag = Module32Next(hSnapshot, &moduleEntry);
		else
			return reinterpret_cast<pAddr>(moduleEntry.modBaseAddr);
	}

	if (hSnapshot)
		CloseHandle(hSnapshot);

	return result;
}

void Memory::SnapshotReader::getBasicSectionInfo(Process& process, pAddr baseAddress, pAddr& numberOfSections, PIMAGE_SECTION_HEADER& section)
{
	// a const value
	constexpr USHORT valid_dos_header{ 0x5A4D };

	// get dos header and check it
	IMAGE_DOS_HEADER dosHeader = read<IMAGE_DOS_HEADER>(process, baseAddress);
	if (dosHeader.e_magic != valid_dos_header) return ;

	// get PE header
	PIMAGE_NT_HEADERS pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + dosHeader.e_lfanew);
	IMAGE_NT_HEADERS ntHeaders = read<IMAGE_NT_HEADERS>(process, (pAddr)pNtHeaders);

	// return value
	numberOfSections = ntHeaders.FileHeader.NumberOfSections;
	section = reinterpret_cast<PIMAGE_SECTION_HEADER>((pAddr)pNtHeaders + sizeof(ntHeaders));
}

bool Memory::SnapshotReader::findMemory(Process& process, pAddr address, pAddr endAddress, char* memoryToFind, pAddr lengthToFind, pAddr& returnAddress, bool flag)
{
	char *m = new char[lengthToFind];
	while (address < endAddress)
	{
		pAddr temp = address;
		char* mTemp = m;
		// read m
		for (pAddr i = 0; i < lengthToFind; i++)
		{
			*m = read<char>(process, temp);
			m++;
			temp++;
		}
		// relocate m
		m = mTemp;

		// compare directly
		// this could be change to KMP
		if (memcmp(memoryToFind, m, lengthToFind))
			if (flag)
				return false;
			else
				address++;
		else
		{
			returnAddress = address + lengthToFind;
			return true;
		}
	}
	return false;
}

pAddr Memory::SnapshotReader::findOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, const char* commandAfterOffset)
{
	if (!process.avalible() || !moduleBaseAddress) return NULL;

	// get section header and number of sections
	pAddr numberOfSections = 0;
	PIMAGE_SECTION_HEADER pSectionHeader = nullptr;
	getBasicSectionInfo(process, moduleBaseAddress, numberOfSections, pSectionHeader);
	
	// traverse
	for (pAddr i = 0; i < numberOfSections; i++)
	{
		// if the section contains code
		IMAGE_SECTION_HEADER sectionHeader = read<IMAGE_SECTION_HEADER>(process, (pAddr)pSectionHeader);
		if ((sectionHeader.Characteristics >> 5) & 1)
		{
			// tansform value in string into value in memory
			char *before = new char[8], * after = new char[8];
			ZeroMemory(before, 8); ZeroMemory(after, 8);
			pAddr len1, len2;
			Byte::ValueToBYTES(commandBeforeOffset, before, len1);
			Byte::ValueToBYTES(commandAfterOffset, after, len2);

			// compare memory in section(between moduleBaseAddress+sectionHeader.VirtualAddress and moduleBaseAddress+sectionHeader.VirtualAddress+sectionHeader->Misc.VirtualSize)
			bool flag1 = false, flag2 = false;
			pAddr findAddress = moduleBaseAddress + sectionHeader.VirtualAddress, endAddress = findAddress + sectionHeader.Misc.VirtualSize, returnAddress1, returnAddress2;
			while (!flag1 || !flag2)
			{
				flag1 = findMemory(process, findAddress, endAddress, before, len1, returnAddress1, false);
				flag2 = findMemory(process, returnAddress1 + sizeof(pAddr), endAddress, after, len2, returnAddress2, true);
				if (flag1 && !flag2) findAddress = returnAddress1 + 1;
				if (flag1 && flag2) return returnAddress1;
				if (endAddress <= findAddress) return NULL;
			}
		}
		// find the next section
		pSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>((pAddr)pSectionHeader + sizeof(sectionHeader));
	}

	return NULL;
}

bool Memory::SnapshotReader::findOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, pAddr& offset, const char* commandAfterOffset)
{
	if (!process.avalible() || !moduleBaseAddress) return false;

	pAddr numberOfSections = 0;
	PIMAGE_SECTION_HEADER pSectionHeader = nullptr;
	getBasicSectionInfo(process, moduleBaseAddress, numberOfSections, pSectionHeader);

	for (pAddr i = 0; i < numberOfSections; i++)
	{
		IMAGE_SECTION_HEADER sectionHeader = read<IMAGE_SECTION_HEADER>(process, (pAddr)pSectionHeader);
		if ((sectionHeader.Characteristics >> 5) & 1)
		{
			char* before = new char[8], * after = new char[8];
			ZeroMemory(before, 8); ZeroMemory(after, 8);
			pAddr len1, len2;
			Byte::ValueToBYTES(commandBeforeOffset, before, len1);
			Byte::ValueToBYTES(commandAfterOffset, after, len2);

			bool flag1 = false, flag2 = false;
			pAddr findAddress = moduleBaseAddress + sectionHeader.VirtualAddress, endAddress = findAddress + sectionHeader.Misc.VirtualSize, returnAddress1, returnAddress2;
			while (!flag1 || !flag2)
			{
				flag1 = findMemory(process, findAddress, endAddress, before, len1, returnAddress1, false);
				flag2 = findMemory(process, returnAddress1 + sizeof(pAddr), endAddress, after, len2, returnAddress2, true);
				if (flag1 && !flag2) findAddress = returnAddress1 + 1;
				if (flag1 && flag2) { offset = returnAddress1; return true; }
				if (endAddress <= findAddress) return false;
			}
		}
		// find the next section
		pSectionHeader = reinterpret_cast<PIMAGE_SECTION_HEADER>((pAddr)pSectionHeader + sizeof(sectionHeader));
	}

	offset = 0;
	return false;
}

pAddr Memory::SnapshotReader::getOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, const char* commandAfterOffset)
{
	pAddr offset;
	if (findOffset(process, moduleBaseAddress, commandBeforeOffset, offset, commandAfterOffset))
		return read<pAddr>(process, offset);
	else
		return NULL;
}

bool Memory::SnapshotReader::getOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, pAddr& offset, const char* commandAfterOffset)
{
	if (findOffset(process, moduleBaseAddress, commandBeforeOffset, offset, commandAfterOffset))
	{
		offset = read<pAddr>(process, offset);
		return true;
	}
	else
	{
		return false;
	}
}