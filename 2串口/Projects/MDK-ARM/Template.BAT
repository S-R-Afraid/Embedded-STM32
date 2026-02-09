SET PATH=D:\Keil_v5\ARM\ARMCLANG\Bin;C:\Program Files (x86)\Common Files\Oracle\Java\java8path;C:\Program Files (x86)\Common Files\Oracle\Java\javapath;C:\Program Files\Microsoft MPI\Bin\;C:\Program Files (x86)\Common Files\Intel\Shared Libraries\redist\intel64\compiler;C:\Ruby32-x64\bin;C:\Program Files\Microsoft\jdk-11.0.16.101-hotspot\bin;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Windows\System32\OpenSSH\;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\Program Files\Microsoft SQL Server\120\Tools\Binn\;C:\Program Files\Microsoft SQL Server\150\Tools\Binn\;D:\MATLAB\R2022a\runtime\win64;D:\MATLAB\R2022a\bin;C:\Program Files\dotnet\;C:\Program Files\Git\cmd;C:\Program Files\NVIDIA Corporation\NVIDIA app\NvDLISR;C:\Program Files\MySQL\MySQL Shell 8.0\bin\;C:\Users\LENOVO\.cargo\bin;E:\Python\Scripts\;E:\Python\;C:\Users\LENOVO\AppData\Local\Microsoft\WindowsApps;E:\Python\PyCharm 2023.3.1\bin;;E:\RubyMine\RubyMine 2023.3.1\bin;;E:\Microsoft VS Code\bin;C:\ProgramData\miniconda3\Scripts;C:\ProgramData\miniconda3\condabin;C:\Users\LENOVO\.dotnet\tools;c:\users\lenovo\.local\bin;c:\users\lenovo\appdata\roaming\python\python311\scripts;D:\PyCharm Community Edition 2024.2.1\bin;;E:\CLion 2023.3.4\bin;
SET CPU_TYPE=STM32F103ZE
SET CPU_VENDOR=STMicroelectronics
SET UV2_TARGET=Template
SET CPU_CLOCK=0x00B71B00
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"..\..\output\startup_stm32f103xe._ac"
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"..\..\output\stm32f1xx_it.__i"
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"..\..\output\main.__i"
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"..\..\output\system_stm32f1xx.__i"
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmClang.exe" @"..\..\output\sys.__i"
"D:\Keil_v5\ARM\ARMCLANG\Bin\ArmLink" --Via "..\..\Output\atk_f103.lnp"
"D:\Keil_v5\ARM\ARMCLANG\Bin\fromelf.exe" "..\..\Output\atk_f103.axf" --i32combined --output "..\..\Output\atk_f103.hex"
