#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include "Byte.h"

namespace Memory
{
	class Process
	{
	public:
		pAddr id = NULL;
		HANDLE handle = NULL;
		// is is a avalible Process?
		bool avalible() { return id && handle && handle != INVALID_HANDLE_VALUE; }
		~Process();
	};

	// Reader via snapshot
	class SnapshotReader
	{
	private:
		bool findMemory(Process& process, pAddr address, pAddr size, char* memoryToFind, pAddr lengthToFind, pAddr& returnAddress, bool flag);
	public:
		SnapshotReader() { }
		SnapshotReader(Process& process, const char* processName);
		~SnapshotReader() { }
		// get module base address
		pAddr getModuleAddress(Process& process, const char* moduleName);
		// get basic section information
		void getBasicSectionInfo(Process& process, pAddr baseAddress, pAddr& sectionNumbers, PIMAGE_SECTION_HEADER& section);
		// find the offset
		// the longest command could be 8 bytes
		pAddr findOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, const char* commandAfterOffset);
		// find the offset
		// the longest command could be 8 bytes
		bool findOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, pAddr& offset, const char* commandAfterOffset);
		//
		pAddr getOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, const char* commandAfterOffset);
		bool getOffset(Process& process, pAddr moduleBaseAddress, const char* commandBeforeOffset, pAddr& offset, const char* commandAfterOffset);
		// read memory
		template <typename T> T read(Process& process, pAddr address)
		{
			T returnValue = { };
			ReadProcessMemory(process.handle, reinterpret_cast<void*>(address), &returnValue, sizeof(T), NULL);
			return returnValue;
		}
		// read memory
		template <typename T> void read(Process& process, pAddr address, T& returnValue)
		{
			ReadProcessMemory(process.handle, reinterpret_cast<void*>(address), returnValue, sizeof(T), NULL);
		}
		// write memory
		template <typename T> void write(Process& process, pAddr address, T& valueToWrite)
		{
			WriteProcessMemory(process.handle, reinterpret_cast<void*>(address), valueToWrite, sizeof(T), NULL);
		}
	};
}