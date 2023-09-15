#include "Memory.h"

int main()
{
	Memory::Process process;
	Memory::SnapshotReader reader;
	char processName[] = "csgo.exe";
	reader = Memory::SnapshotReader(process, processName);
	if (process.avalible()) std::cout << "avalible!\n";
	pAddr client = reader.getModuleAddress(process, "client.dll");
	//std::cout << std::hex << reader.read<pAddr>(process, reader.find(process, client, "FF D0 89 87", "89 87")) << '\n';
	pAddr val;
	if (reader.find(process, client, "FF D0 89 87", val, "89 87"))
	{
		std::cout << std::hex << reader.read<pAddr>(process, val) << '\n';
	}
	return 0;
}