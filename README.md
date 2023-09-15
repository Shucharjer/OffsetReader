# OffsetReader

[简体中文](README_zh-Hans.md)|English  

#### Description
This project could reader some offsets. To use it, you only need clone it and change `processName`, `moduleName` and commands to what you want to.  
#### Notice
- `reader.find()`returns the address of the offset. To get the offset, you need to use `reader.read<pAddr>(process, `the address`)`!  
- The longest length of the command could be only 8 bytes!  
