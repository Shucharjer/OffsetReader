#include "Memory.h"

int main()
{
	// declare
	Memory::Process process;
	Memory::SnapshotReader reader;
	char processName[] = "csgo.exe";
	char moduleName[] = "client.dll";

	// initialize
	reader = Memory::SnapshotReader(process, processName);
	if (!process.avalible()) return 1;
	// get module base
	pAddr client = reader.getModuleAddress(process, moduleName);

	// get offset
	std::cout << std::hex << reader.getOffset(process, client, "FF D0 89 87", "89 87") << '\n';
	// another way
	/*pAddr val;
	std::cout << std::hex << reader.getOffset(process, client, "FF D0 89 87", val, "89 87") << '\n';*/
	return 0;
}