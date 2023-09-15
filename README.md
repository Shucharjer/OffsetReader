# Dumper  
#### Description  
This project could reader some offsets. To use it, you only need clone it and change `processName`, `moduleName` and commands to what you want to.  
#### Notice  
- `reader.find()`returns the address of the offset, to get the offset, you need to use `reader.read<pAddr>(process, `the address`)`!  
- The longest length of the command could be only 8 bytes!  
