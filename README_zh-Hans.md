# OffsetReader

[English](README.md)|简体中文  

#### 简介

这个项目可以读取偏移。使用它只需要clone它然后更改`processName`，`moduleName`和你想要读取的偏移的命令。  

#### 注意

-   `reader.find()`返回的是偏移的地址。获取偏移需要使用`reader.read<pAddr>(process, `偏移的地址`)`！  
-   指令的最大长度为8！  