

## System V Application Binary Interface
AMD64 Architecture Processor Supplement
(With LP64 and ILP32 Programming Models)
## Version 1.0
Edited by
H.J. Lu
## 1
## , Michael Matz
## 2
## , Milind Girkar
## 3
## , Jan Hubi
## ˇ
cka
## 4
## ,
## Andreas Jaeger
## 5
## , Mark Mitchell
## 6
## May 20, 2021
## 1
hongjiu.lu@intel.com
## 2
matz@suse.de
## 3
milind.girkar@intel.com
## 4
jh@suse.cz
## 5
aj@suse.de
## 6
mark@codesourcery.com
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Contents
## 1    Introduction12
## 2    Software Installation13
## 3    Low Level System Information14
3.1Machine Interface  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .14
3.1.1Processor Architecture  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .14
3.1.2Data Representation   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .16
3.2Function Calling Sequence  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .20
3.2.1Registers   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .20
3.2.2The Stack Frame   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .21
3.2.3Parameter Passing    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .22
3.3Operating System Interface  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .29
3.3.1Exception Interface  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .29
3.3.2Virtual Address Space   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .29
3.3.3Page Size  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .29
3.3.4Virtual Address Assignments .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .29
3.4Process Initialization   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .32
3.4.1Initial Stack and Register State  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .32
3.4.2Thread State    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .34
3.4.3Auxiliary Vector   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .35
3.5Coding Examples  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .38
3.5.1Architectural Constraints  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .38
3.5.2Conventions    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .41
3.5.3Position-Independent Function Prologue .  .  .  .  .  .  .  .  .  .  .  .  .  .41
3.5.4Data Objects   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .42
3.5.5Function Calls    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .48
3.5.6Branching .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .51
## 1
AMD64 ABI 1.0 – May 20, 2021 – 11:08

3.5.7Variable Argument Lists   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .54
3.6DWARF Definition   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .58
3.6.1DWARF Release Number    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .59
3.6.2DWARF Register Number Mapping  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .59
3.7Stack Unwind Algorithm  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .59
## 4    Object Files62
4.1ELF Header  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .62
4.1.1Machine Information  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .62
4.1.2Number of Program Headers  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .63
4.2Sections  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .63
4.2.1Section Flags  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .63
4.2.2Section types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .64
4.2.3Special Sections    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .64
4.2.4EH_FRAME sections    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .65
4.3Symbol Table   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .69
4.4Relocation .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .70
4.4.1Relocation Types  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .70
4.4.2Large Models  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .75
5    Program Loading and Dynamic Linking76
5.1Program Loading   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .76
5.1.1Program header  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .77
5.2Dynamic Linking  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .77
5.2.1Program Interpreter .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .82
5.2.2Initialization and Termination Functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .83
5.3Program Property  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .83
## 6    Libraries89
6.1C Library   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .89
6.1.1Global Data Symbols  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .89
6.1.2Floating Point Environment Functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .89
6.2Unwind Library Interface  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .89
6.2.1Exception Handler Framework  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .90
6.2.2Data Structures  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .92
6.2.3Throwing an Exception .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .94
6.2.4Exception Object Management .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .97
6.2.5Context Management  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .97
6.2.6Personality Routine .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   100
## 2
AMD64 ABI 1.0 – May 20, 2021 – 11:08

6.3Unwinding Through Assembler Code   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   103
## 7    Development Environment106
## 8    Execution Environment107
## 9    Conventions108
9.1C++   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   109
9.2Fortran .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   110
9.2.1Names    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   110
9.2.2Representation of Fortran Types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   111
9.2.3Argument Passing    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   112
9.2.4Functions  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   113
9.2.5COMMON blocks   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   114
9.2.6Intrinsics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   115
10  ILP32 Programming Model125
10.1  Parameter Passing .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   125
10.2  Address Space .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   125
10.3  Thread-Local Storage Support   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   125
10.3.1   Global Thread-Local Variable   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   125
10.3.2   Static Thread-Local Variable  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   127
10.3.3   TLS Linker Optimization .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   129
10.4  Kernel Support   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   132
10.5  Coding Examples  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   132
10.5.1   Indirect Branch  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   132
## 11  Alternate Code Sequences For Security133
11.1  Code Sequences without PLT  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   133
11.1.1   Indirect Call via the GOT Slot   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   133
11.1.2   Thread-Local Storage without PLT .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   134
12  Intel MPX Extension139
12.1  Parameter Passing and Returning of Values    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   139
12.1.1   Classification  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   139
12.1.2   Passing   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   140
12.1.3   Returning of Values .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   141
12.1.4   Variable Argument Lists   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   143
12.2  Program Loading and Dynamic Linking   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   143
## 3
AMD64 ABI 1.0 – May 20, 2021 – 11:08

13  Intel CET Extension146
13.1  Program Loading   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   146
13.1.1   ProcessGNU_PROPERTY_X86_FEATURE_1_IBT.  .  .  .  .  .  .  .  .  .  .  .  .   146
13.1.2   ProcessGNU_PROPERTY_X86_FEATURE_1_SHSTK.  .  .  .  .  .  .  .  .  .  .  .   147
13.2  Dynamic Linking  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   147
13.2.1   IBT-enabled PLTs For ILP32 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   148
14  Intel LAM Extension150
14.1  Intel LAM  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   150
14.2  Program Loading   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   150
14.2.1   ProcessGNU_PROPERTY_X86_FEATURE_1_LAM_U48.  .  .  .  .  .  .  .  .  .   150
14.2.2   ProcessGNU_PROPERTY_X86_FEATURE_1_LAM_U57.  .  .  .  .  .  .  .  .  .   151
## A   Linux Conventions152
A.1   Execution of 32-bit Programs .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   152
A.2   AMD64 Linux Kernel Conventions .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   152
A.2.1    Calling Conventions   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   152
A.2.2    Stack Layout   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   153
A.2.3    Miscellaneous Remarks    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   153
## B   Linker Optimization154
B.1    Combine GOTPLT and GOT Slots  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   154
B.2    Optimize GOTPCRELX Relocations  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   155
## 4
AMD64 ABI 1.0 – May 20, 2021 – 11:08

List of Tables
3.1Micro-Architecture Levels   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .15
3.2Hardware Exceptions and Signals    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .30
3.3Floating-Point Exceptions    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .30
3.4x87 Floating-Point Control Word  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .32
3.5MXCSR Status Bits  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .32
3.6rFLAGSBits    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .33
4.1AMD64 Specific Section Header Flag,sh_flags.  .  .  .  .  .  .  .  .  .  .  .  .  .63
4.2Section Header Types  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .64
4.3Special sections  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .64
4.4Additional Special Sections for the Large Code Model    .  .  .  .  .  .  .  .  .  .65
4.5Common Information Entry (CIE)   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .66
4.6CIE Augmentation Section Content    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .67
4.7Frame Descriptor Entry (FDE)   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .68
4.8FDE Augmentation Section Content   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .69
4.9Relocation Types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .72
4.10  Large Model Relocation Types  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .75
5.1Program Header Types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .77
5.2Program Property Type Ranges  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .83
5.3Program Property Types    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .84
5.4GNU_PROPERTY_X86_FEATURE_1_AND Bit Flags .  .  .  .  .  .  .  .  .  .85
5.5Bit Flags For X86 Instruction Sets   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .86
5.6Bit Flags For X86 Processor Features    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .87
7.1Predefined Pre-Processor Symbols  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   106
9.1Mil intrinsics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   115
9.2F77 intrinsics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   117
9.3F90 intrinsics   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   118
## 5
AMD64 ABI 1.0 – May 20, 2021 – 11:08

9.4Math intrinsics    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   118
9.5Unix intrinsics .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   120
10.1  General Dynamic Model Code Sequence .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   126
10.2  General Dynamic Model Code Sequence with TLSDESC  .  .  .  .  .  .  .  .  .   126
10.3  Initial Exec Model Code Sequence  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   126
10.4  Initial Exec Model Code Sequence, II   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   127
10.5  Local Dynamic Model Code Sequence With Lea    .  .  .  .  .  .  .  .  .  .  .  .  .   127
10.6  Local Dynamic Model Code Sequence With Add   .  .  .  .  .  .  .  .  .  .  .  .  .   127
10.7  General Dynamic Model Code Sequence with TLSDESC  .  .  .  .  .  .  .  .  .   128
10.8  Local Dynamic Model Code Sequence, II   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   128
10.9  Local Exec Model Code Sequence With Lea  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   128
10.10Local Exec Model Code Sequence With Add    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   128
10.11Local Exec Model Code Sequence, II    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   129
10.12Local Exec Model Code Sequence, III   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   129
10.13GD -> IE Code Transition    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   129
10.14GDesc -> IE Code Transition  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   129
10.15GD -> LE Code Transition   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   130
10.16GDesc -> LE Code Transition    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   130
10.17IE -> LE Code Transition With Lea    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   130
10.18IE -> LE Code Transition With Add   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   130
10.19IE -> LE Code Transition, II   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   131
10.20LD -> LE Code Transition With Lea  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   131
10.21LD -> LE Code Transition With Add  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   131
10.22LD -> LE Code Transition, II  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   131
10.23Indirect Branch   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   132
11.1  General Dynamic Model Code Sequence (LP64)    .  .  .  .  .  .  .  .  .  .  .  .  .   135
11.2  General Dynamic Model Code Sequence (ILP32)   .  .  .  .  .  .  .  .  .  .  .  .  .   135
11.3  Local Dynamic Model Code Sequence (LP64) .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   136
11.4  Local Dynamic Model Code Sequence (ILP32)   .  .  .  .  .  .  .  .  .  .  .  .  .  .   136
11.5  GD -> IE Code Transition (LP64)    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   136
11.6  GD -> IE Code Transition (ILP32)  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   137
11.7  GD -> LE Code Transition (LP64)  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   137
11.8  GD -> LE Code Transition (ILP32) .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   137
11.9  LD -> LE Code Transition (LP64)   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   138
11.10LD -> LE Code Transition (ILP32)  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   138
B.1    Call and Jmp Conversion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   155
## 6
AMD64 ABI 1.0 – May 20, 2021 – 11:08

B.2    Mov Conversion .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   156
B.3    Test and Binop Conversion  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   156
## 7
AMD64 ABI 1.0 – May 20, 2021 – 11:08

List of Figures
3.1Scalar Types .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .17
3.2Bit-Field Ranges    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .19
3.3Stack Frame with Base Pointer  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .21
3.4Register Usage    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .26
3.5Parameter Passing Example .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .28
3.6Register Allocation Example  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .28
3.7Virtual Address Configuration   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .31
3.8Conventional Segment Arrangements    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .31
3.9Initial Process Stack .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .34
3.10auxv_tType Definition   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .35
3.11  Auxiliary Vector Types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .36
3.12  Position-Independent Function Prolog Code  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .42
3.13  Absolute Load and Store (Small Model)  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .43
3.14  Position-Independent Load and Store (Small PIC Model)  .  .  .  .  .  .  .  .  .44
3.15  Absolute Load and Store (Medium Model) .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .45
3.16  Position-Independent Load and Store (Medium PIC Model) .  .  .  .  .  .  .  .46
3.17  Position-Independent Load and Store (Medium PIC Model), continued .  .46
3.18  Absolute Global Data Load and Store   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .47
3.19  Faster Absolute Global Data Load and Store  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .47
3.20  Position-Independent Global Data Load and Store  .  .  .  .  .  .  .  .  .  .  .  .  .48
3.21  Faster Position-Independent Global Data Load and Store   .  .  .  .  .  .  .  .  .48
3.22  Position-Independent Direct Function Call (Small and Medium Model)    .48
3.23  Position-Independent Indirect Function Call  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .49
3.24  Absolute Direct and Indirect Function Call .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .49
3.25  Position-Independent Direct and Indirect Function Call  .  .  .  .  .  .  .  .  .  .50
3.26  Absolute Branching Code .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .51
3.27  Implicit Calculation of Target Address  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .52
3.28  Position-Independent Branching Code  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .52
3.29  Absolute Switch Code    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .53
## 8
AMD64 ABI 1.0 – May 20, 2021 – 11:08

3.30  Position-Independent Switch Code  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .54
3.31  Parameter Passing Example with Variable-Argument List  .  .  .  .  .  .  .  .  .55
3.32  Register Allocation Example for Variable-Argument List   .  .  .  .  .  .  .  .  .55
3.33  Register Save Area   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .56
3.34va_listType Declaration  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .56
3.35  Sample Implementation ofva_arg(l, int).  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .58
3.36  DWARF Register Number Mapping   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .60
3.37  Pointer Encoding Specification Byte  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .61
4.1Relocatable Fields .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .70
5.1Global Offset Table  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .78
5.2Procedure Linkage Table (small and medium models)  .  .  .  .  .  .  .  .  .  .  .79
5.3Final Large Code Model PLT  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .82
5.4AMD64 Program Interpreter   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .83
6.1Examples for Unwinding in Assembler .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   105
9.1Example mapping of names .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   111
9.2Mapping of Fortran to C types   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   111
11.1  Function Call without PLT (Small and Medium Models)    .  .  .  .  .  .  .  .  .   133
11.2  Function Address without PLT (Small and Medium Models)   .  .  .  .  .  .  .   134
11.3__tls_get_addrCall    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   135
12.1  Bound Register Usage    .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   140
12.2  Bounds Passing Example  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   142
12.3  Bounds Allocation Example   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   142
12.4  BND Procedure Linkage Table (small and medium models) .  .  .  .  .  .  .  .   144
12.5  Additional Procedure Linkage Table (small and medium models)  .  .  .  .  .   144
12.6  Indirect branch   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   145
13.1  The IBT-enabled PLT (small and medium models) .  .  .  .  .  .  .  .  .  .  .  .  .   148
13.2  The Second IBT-enabled PLT (small and medium models)   .  .  .  .  .  .  .  .   148
13.3  The IBT-enabled PLT For ILP32  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   149
13.4  The Second IBT-enabled PLT For ILP32  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   149
B.1    Procedure Linkage Table Entry Via GOTPLT Slot  .  .  .  .  .  .  .  .  .  .  .  .  .   154
B.2    Procedure Linkage Table Entry Via GOT Slot   .  .  .  .  .  .  .  .  .  .  .  .  .  .  .   155
## 9
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Revision History
IntelChanges  to  this  document  made  by  Intel  Corporation  are  copyright  Intel  Corpo-
ration  and  licensed  under  CC  BY  4.0:https://creativecommons.org/
licenses/by/4.0/legalcode.
1.0Document _Unwind_GetIPInfo. Update to match C++ ABI. Add chapter for alternate
code sequences For security.
0.99Add description of TLS relocations (thanks to Alexandre Oliva) and mention the
decimal floating point and AVX types (thanks to H.J. Lu).
0.98Various clarifications and fixes according to feedback from Sun, thanks to Terrence
Miller. DWARF register numbers for some system registers, thanks to Jan Beulich.
AddR_X86_64_SIZE32andR_X86_64_SIZE64relocations; extend meaning ofe_phnum
to handle more than 0xffff program headers, thanks to Rod Evans.  Add footnote
about passing ofdecimaldatatypes. Specify that_Boolis booleanized at the caller.
0.97Integrate Fortran ABI.
0.96UseSHF_X86_64_LARGEinsteadSHF_AMD64_LARGE(thanks to Evandro Menezes). Cor-
rect various grammatical errors noted by Mark F. Haigh, who also noted that there
are no global VLAs in C99. Thanks also to Robert R. Henry.
0.95Include description of the medium PIC memory model (thanks to Jan Hubi
## ˇ
cka) and
large model (thanks to Evandro Menezes).
0.94Add  sections  in  Development  Environment,  Program  Loading,  a  description  of
EH_FRAME sections and general cleanups to make text in this ABI self-contained.
Thanks to Michael Walker and Terrence Miller.
0.93Add sections about program headers, new section types and special sections for un-
winding information. Thanks to Michael Walker.
0.92Fix some typos (thanks to Bryan Ford), add section about stack layout in the Linux
kernel. Fix example in figure  3.5 (thanks to Tom Horsley). Add section on unwind-
ing through assembler (written by Michal Ludvig). Removemmxextfeature (thanks
to Evandro Menezes).  Add section on Fortran (by Steven Bosscher) and stack un-
winding (by Jan Hubi
## ˇ
cka).
0.91Clarify that x87 is default mode, not MMX (by Hans Peter Anvin).
## 10
AMD64 ABI 1.0 – May 20, 2021 – 11:08

0.90Change DWARF register numbers again; mention that__m128needs alignment; fix
typo in figure 3.3;  add some comments on kernel expectations;  mention TLS ex-
tensions; add example for passing of variable-argument lists; change semantics of
%raxin variable-argument lists; improve formatting; mention that X87 class is not
used for passing; make/lib64a Linux specific section; rename x86-64 to AMD64;
describe passing of complex types.  Special thanks to Andi Kleen, Michal Ludvig,
Michael Matz, David O’Brien and Eric Young for their comments.
0.21Define__int128as class INTEGER in register passing. Mention that%alis used for
variadic argument lists.  Fix some textual problems.  Thanks to H. Peter Anvin, Bo
Thorsen, and Michael Matz.
0.20 — 2002-07-11Change DWARF register number values of%rbx,%rsi,%rsi(thanks
to Michal Ludvig). Fix footnotes for fundamental types (thanks to H. Peter Anvin).
Specifysize_t(thanks to Bo Thorsen and Andreas Schwab).  Add new section on
floating point environment functions.
0.19 — 2002-03-27Set name of Linux dynamic linker, mention%fs. Incorporate changes
from H. Peter Anvin <hpa@zytor.com> for booleans and define handling of sub-64-
bit integer types in registers.
## 11
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 1
## Introduction
The AMD64
## 1
architecture
## 2
is an extension of the x86 architecture.  Any processor imple-
menting the AMD64 architecture specification will also provide compatibility modes for
previous descendants of the Intel 8086 architecture, including 32-bit processors such as
the Intel 386, Intel Pentium, and AMD K6-2 processor. Operating systems conforming to
the AMD64 ABI may provide support for executing programs that are designed to execute
in these compatibility modes.  The AMD64 ABI does not apply to such programs;  this
document applies only to programs running in the “long” mode provided by the AMD64
architecture.
Binaries using the AMD64 instruction set may program to either a 32-bit model, in
which the C data typesint,longand all pointer types are 32-bit objects (ILP32); or to a
64-bit model, in which the Cinttype is 32-bits but the Clongtype and all pointer types
are 64-bit objects (LP64).  This specification covers both LP64 and ILP32 programming
models.
Except where otherwise noted, the AMD64 architecture ABI follows the conventions
described in the Intel386 ABI. Rather than replicate the entire contents of the Intel386
ABI, the AMD64 ABI indicates only those places where changes have been made to the
Intel386 ABI.
No attempt has been made to specify an ABI for languages other than C. However, it
is assumed that many programming languages will wish to link with code written in C, so
that the ABI specifications documented here apply there too.
## 3
## 1
AMD64  has  been  previously  called  x86-64.   The  latter  name  is  used  in  a  number  of  places  out  of
historical reasons instead of AMD64.
## 2
The   architecture   specification   is   available   on   the   web   athttp://www.x86-64.org/
documentation.
## 3
See section 9.1 for details on C++ ABI.
## 12
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 2
## Software Installation
This document does not specify how software must be installed on an AMD64 architecture
machine.
## 13
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 3
## Low Level System Information
## 3.1    Machine Interface
## 3.1.1    Processor Architecture
Any program can expect that an AMD64 processor implements the baseline features men-
tioned in table 3.1.  Most feature names correspond to CPUID bits, as described in the
processor manual.  Exceptions are OSFXSR and SCE, which are controlled by bits in the
%cr4register and theIA32_EFERMSR.
In addition to the AMD64 baseline architecture, several micro-architecture levels im-
plemented by later CPU modules have been defined, starting at levelx86-64-v2. These
levels are intended to support loading of optimized implementations on those systems that
are compatible with them (see below). The levels are cumulative in the sense that features
from previous levels are implicitly included in later levels.
Levelsx86-64-v3andx86-64-v4are only available if the corresponding features
have been fully enabled. This means that the system must pass the full sequence of checks
in the processor manual for these features, including verification of the XCR0 feature flags
obtained usingxgetbv.
Recommended Uses of Micro-Architecture Levels
The names for the micro-architecture levels of table 3.1 are expected to be used as directory
names (to be searched by the dynamic linker, based on the levels supported by the current
CPU), and by compilers, to select groups of CPU features. Distributions may also specify
that they require CPU support for a certain level.
For example, to select the second level,x86-64-v3, a programmer would build a
## 14
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 3.1: Micro-Architecture Levels
Level NameCPU FeatureExample instruction
(baseline)CMOVcmov
CX8cmpxchg8b
FPUfld
FXSRfxsave
MMXemms
OSFXSRfxsave
SCEsyscall
SSEcvtss2si
SSE2cvtpi2pd
x86-64-v2CMPXCHG16Bcmpxchg16b
LAHF-SAHFlahf
POPCNTpopcnt
SSE3addsubpd
SSE4_1blendpd
SSE4_2pcmpestri
SSSE3phaddd
x86-64-v3AVXvzeroall
AVX2vpermd
BMI1andn
BMI2bzhi
F16Cvcvtph2ps
FMAvfmadd132pd
LZCNTlzcnt
MOVBEmovbe
OSXSAVExgetbv
x86-64-v4AVX512Fkmovw
AVX512BWvdbpsadbw
AVX512CDvplzcntd
AVX512DQvpmullq
AVX512VLn/a
## 15
AMD64 ABI 1.0 – May 20, 2021 – 11:08

shared  object  with  the-march=x86-64-v3GCC  flag.   The  resulting  shared  object
needs to be installed into the directory/usr/lib64/glibc-hwcaps/x86-64-v3
or/usr/lib/x86_64-linux-gnu/glibc-hwcaps/x86-64-v3(in case of dis-
tributions  with  a  multi-arch  file  system  layout).   In  order  to  support  systems  that  only
implement the K8 baseline, a fallback implementation must be installed into the default
locations,/usr/lib64or/usr/lib/x66_64-linux/gnu.  It has to be built with
-march=x86-64(the upstream GCC default). If this guideline is not followed, loading
the library will fail on systems that do not support the level for which the optimized shared
object was built.
Shared objects that are installed under the matchingglibc-hwcapssubdirectory
can use the CPU features for this level and earlier levels without further detection logic.
Run-time detection for other CPU features not listed in this section, or listed only under
later  levels,  is  still  required  (even  if  all  current  CPUs  implement  certain  CPU  features
together).
If  a  distribution  requires  support  for  a  certain  level,  they  build  everything  with  the
appropriate-march=option and install the built binaries in the default file system loca-
tions.  When targeting such distributions, programmers can build their binaries with the
same-march=option and install them into the default locations.  Optimized shared ob-
jects for later levels can still be installed into subdirectories with the appropriate name.
## 3.1.2    Data Representation
Within this specification, the termbyterefers to a 8-bit object, the termtwobyterefers to
a 16-bit object, the termfourbyterefers to a 32-bit object, the termeightbyterefers to a
64-bit object, and the termsixteenbyterefers to a 128-bit object.
## 1
## Fundamental Types
Figure 3.1 shows the correspondence between ISO C’s scalar types and the processor’s.
__int128,_Float16,__float80,__float128,__m64,__m128,__m256and__m512types are
optional.
The__float128type uses a 15-bit exponent, a 113-bit mantissa (the high order signif-
icant bit is implicit) and an exponent bias of 16383.
## 2
## 1
The Intel386 ABI uses the termhalfwordfor a 16-bit object, the termwordfor a 32-bit object, the term
doublewordfor a 64-bit object. But most IA-32 processor specific documentation define awordas a 16-bit
object, adoublewordas a 32-bit object, aquadwordas a 64-bit object and adouble quadwordas a 128-bit
object.
## 2
Initial  implementations  of  the  AMD64  architecture  are  expected  to  support  operations  on  the
__float128type only via software emulation.
## 16
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.1: Scalar Types
AlignmentAMD64
## Type
Csizeof(bytes)Architecture
_Bool
## †
## 11boolean
char11signed byte
signed char
unsigned char11unsigned byte
signed short22signed twobyte
unsigned short22unsigned twobyte
Integralsigned int44signed fourbyte
enum
## †††
unsigned int44unsigned fourbyte
signed long (LP64)88signed eightbyte
unsigned long (LP64)88unsigned eightbyte
signed long (ILP32)44signed fourbyte
unsigned long (ILP32)44unsigned fourbyte
signed long long88
## ††††
signed eightbyte
unsigned long long88
## ††††
unsigned eightbyte
## __int128
## ††
1616signed sixteenbyte
signed __int128
## ††
1616signed sixteenbyte
unsigned __int128
## ††
1616unsigned sixteenbyte
## Pointerany-type
## *
(LP64)88unsigned eightbyte
any-type(
## *
## )() (LP64)
any-type
## *
(ILP32)44unsigned fourbyte
any-type(
## *
## )() (ILP32)
_Float16
## ††††††
2216-bit (IEEE-754)
float44single (IEEE-754)
## Floating-double88
## ††††
double (IEEE-754)
point__float80
## ††
161680-bit extended (IEEE-754)
long double
## †††††
161680-bit extended (IEEE-754)
## __float128
## ††
1616128-bit extended (IEEE-754)
long double
## †††††
1616128-bit extended (IEEE-754)
Decimal-_Decimal324432bit BID (IEEE-754R)
floating-_Decimal648864bit BID (IEEE-754R)
point_Decimal1281616128bit BID (IEEE-754R)
## Packed__m64
## ††
88MMXand 3DNow!
## __m128
## ††
1616SSE and SSE-2
## __m256
## ††
## 3232AVX
## __m512
## ††
## 6464AVX-512
## †
This type is calledboolin C++.
## ††
These types are optional.
## †††
C++ and some implementations of C permit enums larger than an int. The underlying type is bumped
to an unsigned int, long int or unsigned long int, in that order.
## ††††
Thelong long,signed long long,unsigned long longanddoubletypes have
4-byte alignment in the Intel386 ABI.
## †††††
Thelong doubletype  is  128-bit,  the  same  as  the__float128type,  on  the  Android™
platform.  More information on the Android™ platform is available fromhttp://www.android.
com/.
## ††††††
The_Float16type, from ISO/IEC TS 18661-3:2015, is optional.
## 17
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Thelong doubletype uses a 15 bit exponent, a 64-bit mantissa with an explicit high
order significant bit and an exponent bias of 16383.
## 3
Although along doublerequires 16
bytes of storage, only the first 10 bytes are significant.  The remaining six bytes are tail
padding, and the contents of these bytes are undefined.
The__int128type is stored in little-endian order in memory, i.e., the 64 low-order bits
are stored at a a lower address than the 64 high-order bits.
The value of_Alignof(max_align_t)is 16.
A null pointer (for all types) has the value zero.
The typesize_tis defined asunsigned longfor LP64 andunsigned intfor ILP32.
Booleans, when stored in a memory object, are stored as single byte objects the value
of which is always 0 (false) or 1 (true).   When stored in integer registers (except for
passing  as  arguments),  all  8  bytes  of  the  register  are  significant;  any  nonzero  value  is
consideredtrue.
Like the Intel386 architecture, the AMD64 architecture in general does not require all
data accesses to be properly aligned.  Misaligned data accesses are slower than aligned
accesses but otherwise behave identically.  The only exceptions are that__m128,__m256
and__m512must always be aligned properly.
Aggregates and Unions
Structures and unions assume the alignment of their most strictly aligned component. Each
member is assigned to the lowest available offset with the appropriate alignment. The size
of any object is always a multiple of the object‘s alignment.
An array uses the same alignment as its elements, except that a local or global array
variable  of  length  at  least  16  bytes  or  a  C99  variable-length  array  variable  always  has
alignment of at least 16 bytes.
## 4
Structure and union objects can require padding to meet size and alignment constraints.
The contents of any padding is undefined.
Bit-Fields
C struct and union definitions may include bit-fields that define integral values of a speci-
fied size.
The  ABI  does  not  permit  bit-fields  having  the  type__m64,__m128,__m256or
__m512. Programs using bit-fields of these types are not portable.
## 3
This type is the x87 double extended precision data type.
## 4
The alignment requirement allows the use of SSE instructions when operating on the array.  The com-
piler cannot in general calculate the size of a variable-length array (VLA), but it is expected that most VLAs
will require at least 16 bytes, so it is logical to mandate that VLAs have at least a 16-byte alignment.
## 18
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.2: Bit-Field Ranges
Bit-field TypeWidthwRange
signed char−2
w−1
to2
w−1
## −1
char1 to 80 to2
w
## −1
unsigned char
0 to2
w
## −1
signed short−2
w−1
to2
w−1
## −1
short1 to 160 to2
w
## −1
unsigned short
0 to2
w
## −1
signed int−2
w−1
to2
w−1
## −1
int1 to 320 to2
w
## −1
unsigned int
0 to2
w
## −1
signed long (LP64)−2
w−1
to2
w−1
## −1
long (LP64)1 to 640 to2
w
## −1
unsigned long (LP64)
0 to2
w
## −1
long (ILP32)1 to 320 to2
w
## −1
unsigned long (ILP32)0 to2
w
## −1
signed long long−2
w−1
to2
w−1
## −1
long long
1 to 640 to2
w
## −1
unsigned long long0 to2
w
## −1
Bit-fields that are neither signed nor unsigned always have non-negative values.  Al-
though they may have type char, short, int, or long (which can have negative values), these
bit-fields have the same range as a bit-field of the same size with the corresponding un-
signed type. Bit-fields obey the same size and alignment rules as other structure and union
members.
## Also:
-  bit-fields are allocated from right to left
-  bit-fields must be contained in a storage unit appropriate for its declared type
-  bit-fields may share a storage unit with other struct / union members
Unnamed bit-fields’ types do not affect the alignment of a structure or union.
## 19
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 3.2    Function Calling Sequence
This section describes the standard function calling sequence, including stack frame lay-
out, register usage, parameter passing and so on.
The  standard  calling  sequence  requirements  apply  only  to  global  functions.   Local
functions that are not reachable from other compilation units may use different conven-
tions. Nevertheless, it is recommended that all functions use the standard calling sequence
when possible.
## 3.2.1    Registers
The AMD64 architecture provides 16 general purpose 64-bit registers.   In addition the
architecture provides 16 SSE registers, each 128 bits wide and 8 x87 floating point reg-
isters, each 80 bits wide.  Each of the x87 floating point registers may be referred to in
MMX/3DNow! mode as a 64-bit register. All of these registers are global to all procedures
active for a given thread.
Intel  AVX  (Advanced  Vector  Extensions)  provides  16  256-bit  wide  AVX  registers
(%ymm0-%ymm15). The lower 128-bits of%ymm0-%ymm15are aliased to the respective 128b-bit
SSE registers (%xmm0-%xmm15).  Intel AVX-512 provides 32 512-bit wide SIMD registers
(%zmm0-%zmm31). The lower 128-bits of%zmm0-%zmm31are aliased to the respective 128b-
bit SSE registers (%xmm0-%xmm31
## 5
). The lower 256-bits of%zmm0-%zmm31are aliased to the
respective 256-bit AVX registers (%ymm0-%ymm31
## 6
). For purposes of parameter passing and
function return,%xmmN,%ymmNand%zmmNrefer to the same register.  Only one of them can
be used at the same time. We use vector register to refer to either SSE, AVX or AVX-512
register. In addition, Intel AVX-512 also provides 8 vector mask registers (%k0-%k7), each
64-bit wide.
Intel Advanced Matrix Extensions (Intel AMX) is a programming paradigm consisting
of two components: a set of 2-dimensional registers (tiles) representing sub-arrays from a
larger 2-dimensional memory image, and accelerators able to operate on tiles. Capability
of  Intel  AMX  implementation  is  enumerated  by  palettes.   Two  palettes  are  supported:
palette 0 represents the initialized state and palette 1 consists of 8 tile registers (%tmm0-
%tmm7) of up to 1 KB size, which is controlled by a tile control register.
This subsection discusses usage of each register. Registers%rbp,%rbxand%r12through
%r15“belong” to the calling function and the called function is required to preserve their
values.   In  other  words,  a  called  function  must  preserve  these  registers’  values  for  its
## 5
%xmm15-%xmm31are only available with Intel AVX-512.
## 6
%ymm15-%ymm31are only available with Intel AVX-512.
## 20
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.3: Stack Frame with Base Pointer
PositionContentsFrame
8n+16(%rbp)memory argument eightbyten
. . .Previous
16(%rbp)memory argument eightbyte0
8(%rbp)return address
## 0(%rbp)previous%rbpvalue
-8(%rbp)unspecifiedCurrent
## . . .
0(%rsp)variable size
-128(%rsp)red zone
caller.  Remaining registers “belong” to the called function.
## 7
If a calling function wants
to preserve such a register value across a function call, it must save the value in its local
stack frame.
The CPU shall be in x87 mode upon entry to a function.  Therefore, every function
that uses theMMXregisters is required to issue anemmsorfemmsinstruction after using
MMXregisters, before returning or calling another function.
## 8
The direction flagDFin
the%rFLAGSregister must be clear (set to “forward” direction) on function entry and re-
turn.  Other user flags have no specified role in the standard calling sequence and arenot
preserved across calls.
The control bits of theMXCSRregister are callee-saved (preserved across calls), while
the status bits are caller-saved (not preserved). The x87 status word register is caller-saved,
whereas the x87 control word is callee-saved.
## 3.2.2    The Stack Frame
In addition to registers, each function has a frame on the run-time stack. This stack grows
downwards from high addresses. Figure  3.3 shows the stack organization.
The end of the input argument area shall be aligned on a 16 (32 or 64, if__m256or
__m512is passed on stack) byte boundary.
## 9
In other words, the stack needs to be 16 (32
## 7
Note that in contrast to the Intel386 ABI,%rdi, and%rsibelong to the called function, not the caller.
## 8
All x87 registers are caller-saved, so callees that make use of theMMXregisters may use the faster
femmsinstruction.
## 9
The maximum aligned boundary is the maximum alignment of all variables passed on stack.  In C11,
## 21
AMD64 ABI 1.0 – May 20, 2021 – 11:08

or 64) byte aligned immediately before the call instruction is executed.  Once control has
been transferred to the function entry point, i.e. immediately after the return address has
been pushed,%rsppoints to the return address, and the value of(%rsp+ 8)is a multiple of
## 16(32or64).
## 10
The 128-byte area beyond the location pointed to by%rspis considered to be reserved
and shall not be modified by signal or interrupt handlers.
## 11
Therefore, functions may use
this area for temporary data that is not needed across function calls.  In particular,  leaf
functions may use this area for their entire stack frame,  rather than adjusting the stack
pointer in the prologue and epilogue. This area is known as the red zone.
## 3.2.3    Parameter Passing
After  the  argument  values  have  been  computed,  they  are  placed  either  in  registers  or
pushed on the stack.  The way how values are passed is described in the following sec-
tions.
DefinitionsWe first define a number of classes to classify arguments.  The classes are
corresponding to AMD64 register classes and defined as:
INTEGERThis class consists of integral types that fit into one of the general purpose
registers.
SSEThe class consists of types that fit into a vector register.
SSEUPThe class consists of types that fit into a vector register and can be passed and
returned in the upper bytes of it.
X87, X87UPThese classes consists of types that will be returned via the x87 FPU.
COMPLEX_X87This class consists of types that will be returned via the x87 FPU.
NO_CLASSThis class is used as initializer in the algorithms. It will be used for padding
and empty structures and unions.
MEMORYThis class consists of types that will be passed and returned in memory via
the stack.
variable of typetypedef struct { _Alignas (512) int i; } var_t;is aligned to 512 bytes.
## 10
The conventional use of%rbpas a frame pointer for the stack frame may be avoided by using%rsp
(the stack pointer) to index into the stack frame.  This technique saves two instructions in the prologue and
epilogue and makes one additional general-purpose register (%rbp) available.
## 11
Locations within 128 bytes can be addressed using one-byte displacements.
## 22
AMD64 ABI 1.0 – May 20, 2021 – 11:08

ClassificationThe size of each argument gets rounded up to eightbytes.
## 12
The basic types are assigned their natural classes:
-  Arguments of types (signed and unsigned)_Bool,char,short,int,long,long long,
and pointers are in the INTEGER class.
-  Arguments of types_Float16,float,double,_Decimal32,_Decimal64and__m64are
in class SSE.
-  Arguments of types__float128,_Decimal128and__m128are split into two halves.
The  least  significant  ones  belong  to  class  SSE,  the  most  significant  one  to  class
## SSEUP.
-  Arguments of type__m256are split into four eightbyte chunks. The least significant
one belongs to class SSE and all the others to class SSEUP.
-  Arguments of type__m512are split into eight eightbyte chunks. The least significant
one belongs to class SSE and all the others to class SSEUP.
-  The 64-bit mantissa of arguments of typelong doublebelongs to class X87,  the
16-bit exponent plus 6 bytes of padding belongs to class X87UP.
-  Arguments of type__int128offer the same operations as INTEGERs, yet they do
not fit into one general purpose register but require two registers.  For classification
purposes__int128is treated as if it were implemented as:
typedef struct {
long low, high;
## } __int128;
with the exception that arguments of type__int128that are stored in memory must
be aligned on a 16-byte boundary.
-  Arguments ofcomplex TwhereTis one of the types_Float16, float or doubleare
treated as if they are implemented as:
struct complexT {
T real;
T imag;
## };
## 12
Therefore the stack will always be eightbyte aligned.
## 23
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  A variable of typecomplex long doubleis classified as type COMPLEX_X87.
The classification of aggregate (structures and arrays) and union types works as fol-
lows:
-  If the size of an object is larger than eight eightbytes, or it contains unaligned fields,
it has class MEMORY
## 13
## .
-  If a C++ object is non-trivial for the purpose of calls, as specified in the C++ ABI
## 14
, it is passed by invisible reference (the object is replaced in the parameter list by
a pointer that has class INTEGER)
## 15
## .
-  If the size of the aggregate exceeds a single eightbyte, each is classified separately.
Each eightbyte gets initialized to class NO_CLASS.
-  Each field of an object is classified recursively so that always two fields are consid-
ered.  The resulting class is calculated according to the classes of the fields in the
eightbyte:
(a)  If both classes are equal, this is the resulting class.
(b)  If one of the classes is NO_CLASS, the resulting class is the other class.
(c)  If one of the classes is MEMORY, the result is the MEMORY class.
(d)  If one of the classes is INTEGER, the result is the INTEGER.
(e)  If  one  of  the  classes  is  X87,  X87UP,  COMPLEX_X87  class,  MEMORY  is
used as class.
(f)  Otherwise class SSE is used.
-  Then a post merger cleanup is done:
## 13
The post merger clean up described later ensures that, for the processors that do not support the__m256
type, if the size of an object is larger than two eightbytes and the first eightbyte is not SSE or any other
eightbyte is not SSEUP, it still has class MEMORY. This in turn ensures that for processors that do support
the__m256type,  if  the  size  of  an  object  is  four  eightbytes  and  the  first  eightbyte  is  SSE  and  all  other
eightbytes are SSEUP, it can be passed in a register.   This also applies to the__m512type.   That is for
processors that support the__m512type, if the size of an object is eight eightbytes and the first eightbyte
is SSE and all other eightbytes are SSEUP, it can be passed in a register, otherwise, it will be passed in
memory.
## 14
See section 9.1 for details on C++ ABI.
## 15
An object whose type is non-trivial for the purpose of calls cannot be passed by value because such
objects must have the same address in the caller and the callee.  Similar issues apply when returning an
object from a function. See C++17 [class.temporary] paragraph 3.
## 24
AMD64 ABI 1.0 – May 20, 2021 – 11:08

(a)  If one of the classes is MEMORY, the whole argument is passed in memory.
(b)  If X87UP is not preceded by X87, the whole argument is passed in memory.
(c)  If the size of the aggregate exceeds two eightbytes and the first eightbyte isn’t
SSE or any other eightbyte isn’t SSEUP, the whole argument is passed in mem-
ory.
(d)  If SSEUP is not preceded by SSE or SSEUP, it is converted to SSE.
PassingOnce arguments are classified, the registers get assigned (in left-to-right order)
for passing as follows:
-  If the class is MEMORY, pass the argument on the stack at an address respecting the
arguments alignment (which might be more than its natural alignement).
-  If the class is INTEGER, the next available register of the sequence%rdi,%rsi,%rdx,
%rcx,%r8and%r9is used
## 16
## .
-  If the class is SSE, the next available vector register is used, the registers are taken
in the order from%xmm0to%xmm7.
-  If the class is SSEUP, the eightbyte is passed in the next available eightbyte chunk
of the last used vector register.
-  If the class is X87, X87UP or COMPLEX_X87, it is passed in memory.
When a value of type_Boolis returned or passed in a register or on the stack, bit 0
contains the truth value and bits 1 to 7 shall be zero
## 17
## .
If there are no registers available for any eightbyte of an argument, the whole argument
is passed on the stack. If registers have already been assigned for some eightbytes of such
an argument, the assignments get reverted.
## 16
Note that%r11is neither required to be preserved, nor is it used to pass arguments. Making this register
available as scratch register means that code in the PLT need not spill any registers when computing the
address to which control needs to be transferred.%alis used to indicate the number of vector arguments
passed to a function requiring a variable number of arguments.%r10is used for passing a function’s static
chain pointer.
## 17
Other bits are left unspecified, hence the consumer side of those values can rely on it being 0 or 1 when
truncated to 8 bit.
## 25
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.4: Register Usage
callee
RegisterUsagesaved
%raxtemporary register; with variable arguments passes information
about the number of vector registers used; 1
st
return register
## No
%rbxcallee-saved registerYes
%rcxused to pass 4
th
integer argument to functionsNo
%rdxused to pass 3
rd
argument to functions; 2
nd
return registerNo
## %rsp
stack pointerYes
%rbpcallee-saved register; optionally used as frame pointerYes
%rsiused to pass 2
nd
argument to functionsNo
%rdiused to pass 1
st
argument to functionsNo
%r8used to pass 5
th
argument to functionsNo
## %r9
used to pass 6
th
argument to functionsNo
%r10temporary  register,  used  for  passing  a  function’s  static  chain
pointer
## No
%r11temporary registerNo
## %r12-r14
callee-saved registersYes
## %r15
callee-saved register; optionally used as GOT base pointerYes
## %xmm0–%xmm1
used to pass and return floating point argumentsNo
%xmm2–%xmm7used to pass floating point argumentsNo
%xmm8–%xmm15temporary registersNo
%tmm0–%tmm7temporary registersNo
%mm0–%mm7temporary registersNo
## %k0–%k7
temporary registersNo
%st0,%st1temporary registers, used to returnlong doubleargumentsNo
%st2–%st7temporary registersNo
%fsthread pointerYes
mxcsr
SSE2 control and status wordpartial
x87 SW
x87 status wordNo
x87 CW
x87 control wordYes
tilecfig
Tile control registerNo
## 26
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Once registers are assigned, the arguments passed in memory are pushed on the stack
in reversed (right-to-left
## 18
) order.
For calls that may call functions that use varargs or stdargs (prototype-less calls or calls
to functions containing ellipsis (. . . )  in the declaration)%al
## 19
is used as hidden argument
to specify the number of vector registers used.  The contents of%aldo not need to match
exactly  the  number  of  registers,  but  must  be  an  upper  bound  on  the  number  of  vector
registers used and is in the range 0–8 inclusive.
When passing__m256or__m512arguments to functions that use varargs or stdarg,
function prototypes must be provided. Otherwise, the run-time behavior is undefined.
Returning of ValuesThe returning of values is done according to the following algo-
rithm:
-  Classify the return type with the classification algorithm.
-  If the type has class MEMORY, then the caller provides space for the return value
and passes the address of this storage in%rdias if it were the first argument to the
function.  In effect, this address becomes a “hidden” first argument.  This storage
must not overlap any data visible to the callee through other names than this argu-
ment.
On return%raxwill contain the address that has been passed in by the caller in%rdi.
-  If the class is INTEGER, the next available register of the sequence%rax,%rdxis
used.
-  If the class is SSE, the next available vector register of the sequence%xmm0,%xmm1is
used.
-  If the class is SSEUP, the eightbyte is returned in the next available eightbyte chunk
of the last used vector register.
-  If the class is X87,  the value is returned on the X87 stack in%st0as 80-bit x87
number.
-  If the class is X87UP, the value is returned together with the previous X87 value in
## %st0.
## 18
Right-to-left order on the stack makes the handling of functions that take a variable number of arguments
simpler.  The location of the first argument can always be computed statically,  based on the type of that
argument. It would be difficult to compute the address of the first argument if the arguments were pushed in
left-to-right order.
## 19
Note that the rest of%raxis undefined, only the contents of%alis defined.
## 27
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  If the class is COMPLEX_X87, the real part of the value is returned in%st0and the
imaginary part in%st1.
As an example of the register passing conventions, consider the declarations and the
function call shown in Figure 3.5.  The corresponding register allocation is given in Fig-
ure 3.6, the stack frame offset given shows the frame before calling the function.
## Figure 3.5: Parameter Passing Example
typedef struct {
int a, b;
double d;
} structparm;
structparm s;
int e, f, g, h, i, j, k;
long double ld;
double m, n;
__m256 y;
__m512 z;
extern void func (int e, int f,
structparm s, int g, int h,
long double ld, double m,
__m256 y,
__m512 z,
double n, int i, int j, int k);
func (e, f, s, g, h, ld, m, y, z, n, i, j, k);
## Figure 3.6: Register Allocation Example
## General Purpose Registers    Floating Point Registers    Stack Frame Offset
%rdi:e%xmm0:s.d0:   ld
%rsi:f%xmm1:m16:  j
%rdx:s.a,s.b%ymm2:y24:  k
## %rcx:g%zmm3:z
## %r8:h%xmm4:n
## %r9:i
## 28
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 3.3    Operating System Interface
## 3.3.1    Exception Interface
As  the  AMD64  manuals  describe,  the  processor  changes  mode  to  handleexceptions,
which  may  be  synchronous,  floating-point/coprocessor  or  asynchronous.   Synchronous
and floating-point/coprocessor exceptions, being caused by instruction execution, can be
explicitly generated by a process.  This section, therefore, specifies those exception types
with defined behavior. The AMD64 architecture classifies exceptions asfaults,traps, and
aborts. See the Intel386 ABI for more information about their differences.
## Hardware Exception Types
The operating system defines the correspondence between hardware exceptions and the
signals specified bysignal(BA_OS) as shown in table 3.2.  Contrary to the i386 archi-
tecture, the AMD64 does not define any instructions that generate a bounds check fault in
long mode.
## 3.3.2    Virtual Address Space
Although the AMD64 architecture uses 64-bit pointers, implementations are only required
to handle 48-bit addresses. Therefore, conforming processes may only use addresses from
## 0x0000000000000000to0x00007fffffffffff
## 20
## .
Processes begin with three logical segments, commonly called text, data, and stack.
Use of shared libraries add other segments and a process may dynamically create seg-
ments.
## 3.3.3    Page Size
Systems are permitted to use any power-of-two page size between 4KB and 64KB, inclu-
sive.
## 3.3.4    Virtual Address Assignments
Conceptually processes have the full address space available. In practice, however, several
factors limit the size of a process.
-  The system reserves a configuration dependent amount of virtual space.
## 20
0x0000ffff ffffffff is not a canonical address and cannot be used.
## 29
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 3.2: Hardware Exceptions and Signals
NumberException nameSignal
0divide error faultSIGFPE
## 1
single step trap/faultSIGTRAP
2non-maskable interruptnone
3breakpoint trapSIGTRAP
4overflow trapSIGSEGV
## 5
## (reserved)
6invalid opcode faultSIGILL
## 7
no coprocessor faultSIGFPE
8double fault abortnone
## 9
coprocessor overrun abortSIGSEGV
10invalid TSS faultnone
11segment no present faultnone
12stack exception faultSIGSEGV
## 13
general protection fault/abortSIGSEGV
14page faultSIGSEGV
## 15
## (reserved)
16coprocessor error faultSIGFPE
other(unspecified)SIGILL
Table 3.3: Floating-Point Exceptions
CodeReason
FPE_FLTDIVfloating-point divide by zero
FPE_FLTOVFfloating-point overflow
FPE_FLTUNDfloating-point underflow
FPE_FLTRESfloating-point inexact result
FPE_FLTINVinvalid floating-point operation
## 30
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  The system reserves a configuration dependent amount of space per process.
-  A process whose size exceeds the system’s available combined physical memory
and secondary storage cannot run. Although some physical memory must be present
to run any process, the system can execute processes that are bigger than physical
memory, paging them to and from secondary storage.  Nonetheless, both physical
memory and secondary storage are shared resources.  System load, which can vary
from one program execution to the next, affects the available amount.
Programs that dereference null pointers are erroneous and a process should not expect
0x0 to be a valid address.
## Figure 3.7: Virtual Address Configuration
0xffffffffffffffffReserved system areaEnd of memory
## . . .
## . . .
0x80000000000Dynamic segments
## . . .
0Process segmentsBeginning of memory
Although applications may control their memory assignments, the typical arrangement
appears in figure 3.8.
## Figure 3.8: Conventional Segment Arrangements
## . . .
0x80000000000Dynamic segments
Stack segment
## . . .
## . . .
Data segments
## . . .
0x400000Text segments
0Unmapped
## 31
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 3.4    Process Initialization
3.4.1    Initial Stack and Register State
## Special Registers
The AMD64 architecture defines floating point instructions.  At process startup the two
floating  point  units,  SSE2  and  x87,  both  have  all  floating-point  exception  status  flags
cleared. The status of the control words is as defined in tables 3.4 and 3.5.
Table 3.4: x87 Floating-Point Control Word
Field    ValueNote
RC0Round to nearest
PC11Double extended precision
## PM
1Precision masked
UM1Underflow masked
OM1Overflow masked
## ZM
1Zero divide masked
DM1De-normal operand masked
IM1Invalid operation masked
Table 3.5: MXCSR Status Bits
Field    ValueNote
FZ0Do not flush to zero
## RC
0Round to nearest
PM1Precision masked
UM1Underflow masked
## OM
1Overflow masked
ZM1Zero divide masked
DM1De-normal operand masked
IM1Invalid operation masked
DAZ0De-normals are not zero
## 32
AMD64 ABI 1.0 – May 20, 2021 – 11:08

TherFLAGSregister contains the system flags, such as the direction flag and the carry
flag.  The low 16 bits (FLAGS portion) ofrFLAGSare accessible by application software.
The state of them at process initialization is shown in table 3.6.
Table 3.6:rFLAGSBits
Field    ValueNote
DF0Direction forward
CF0No carry
PF0Even parity
AF0No auxiliary carry
## ZF
0No zero result
## SF
0Unsigned result
OF0No overflow occurred
## Stack State
This section describes the machine state thatexec(BA_OS) creates for new processes.
Various language implementations transform this initial program state to the state required
by the language standard.
For example, a C program begins executing at a function namedmaindeclared as:
extern int main ( int argc , char
## *
argv[ ] , char
## *
envp[ ] );
where
argcis a non-negative argument count
argvis an array of argument strings, withargv[argc] == 0
envpis an array of environment strings, terminated by a null pointer.
Whenmain()returns its value is passed toexit()and if that has been over-ridden and
returns,_exit()(which must be immune to user interposition).
The initial state of the process stack, i.e. when_startis called is shown in figure 3.9.
## 33
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.9: Initial Process Stack
PurposeStart AddressLength
UnspecifiedHigh Addresses
Information block, including argu-
ment strings,  environment strings,
auxiliary information ...
varies
## Unspecified
Null auxiliary vector entry1 eightbyte
Auxiliary vector entries ...2 eightbytes each
## 0eightbyte
Environment pointers ...1 eightbyte each
## 08+8
## *
argc+%rspeightbyte
Argument pointers8+%rspargc eightbytes
Argument count%rspeightbyte
UndefinedLow Addresses
Argument strings, environment strings, and the auxiliary information appear in no spe-
cific order within the information block and they need not be compactly allocated.
Only the registers listed below have specified values at process entry:
%rbpThe content of this register is unspecified at process initialization time, but the user
code should mark the deepest stack frame by setting the frame pointer to zero.
%rspThe stack pointer holds the address of the byte with lowest address which is part of
the stack. It is guaranteed to be 16-byte aligned at process entry.
%rdxa function pointer that the application should register withatexit(BA_OS).
It is unspecified whether the data and stack segments are initially mapped with execute
permissions or not. Applications which need to execute code on the stack or data segments
should take proper precautions, e.g., by callingmprotect().
## 3.4.2    Thread State
New threads inherit the floating-point state of the parent thread and the state is private to
the thread thereafter.
## 34
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 3.4.3    Auxiliary Vector
The auxiliary vector is an array of the following structures (ref.  figure 3.10), interpreted
according to thea_typemember.
Figure 3.10:auxv_tType Definition
typedef struct
## {
int a_type;
union {
long a_val;
void
## *
a_ptr;
void (
## *
a_fnc)();
} a_un;
} auxv_t;
The AMD64 ABI uses the auxiliary vector types defined in figure 3.11.
## 35
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.11: Auxiliary Vector Types
NameValuea_un
AT_NULL0ignored
AT_IGNORE1ignored
## AT_EXECFD
## 2a_val
AT_PHDR3a_ptr
AT_PHENT4a_val
## AT_PHNUM
## 5a_val
AT_PAGESZ6a_val
AT_BASE7a_ptr
AT_FLAGS8a_val
## AT_ENTRY
## 9a_ptr
AT_NOTELF10a_val
AT_UID11a_val
## AT_EUID
## 12a_val
AT_GID13a_val
AT_EGID14a_val
## AT_PLATFORM
## 15a_ptr
AT_HWCAP16a_val
## AT_CLKTCK
## 17a_val
AT_SECURE23a_val
## AT_BASE_PLATFORM
## 24a_ptr
## AT_RANDOM
## 25a_ptr
## AT_HWCAP2
## 26a_val
AT_EXECFN31a_ptr
AT_NULLThe auxiliary vector has no fixed length; instead its last entry’sa_typemem-
ber has this value.
AT_IGNOREThis type indicates the entry has no meaning. The corresponding value of
a_unis undefined.
AT_EXECFDAt process creation the system may pass control to an interpreter program.
When this happens, the system places either an entry of typeAT_EXECFDor one of
typeAT_PHDRin the auxiliary vector.  The entry for typeAT_EXECFDuses thea_val
## 36
AMD64 ABI 1.0 – May 20, 2021 – 11:08

member to contain a file descriptor open to read the application program’s object
file.
AT_PHDRThe system may create the memory image of the application program before
passing control to the interpreter program. When this happens, thea_ptrmember of
theAT_PHDRentry tells the interpreter where to find the program header table in the
memory image.
AT_PHENTThea_valmember of this entry holds the size, in bytes, of one entry in the
program header table to which theAT_PHDRentry points.
AT_PHNUMThea_valmember of this entry holds the number of entries in the program
header table to which theAT_PHDRentry points.
AT_PAGESZIf present, this entry’sa_valmember gives the system page size, in bytes.
AT_BASEThea_ptrmember of this entry holds the base address at which the interpreter
program was loaded into memory. See “Program Header” in the System V ABI for
more information about the base address.
AT_FLAGSIf  present,  thea_valmember  of  this  entry  holds  one-bit  flags.   Bits  with
undefined semantics are set to zero.
AT_ENTRYThea_ptrmember  of  this  entry  holds  the  entry  point  of  the  application
program to which the interpreter program should transfer control.
AT_NOTELFThea_valmember of this entry is non-zero if the program is in another
format than ELF.
AT_UIDThea_valmember of this entry holds the real user id of the process.
AT_EUIDThea_valmember of this entry holds the effective user id of the process.
AT_GIDThea_valmember of this entry holds the real group id of the process.
AT_EGIDThea_valmember of this entry holds the effective group id of the process.
AT_PLATFORMThea_ptrmember of this entry points to a string containing the plat-
form name.
AT_HWCAPThea_valmember of this entry contains an bitmask of CPU features.  It
mask to the value returned by CPUID 1.EDX.
## 37
AMD64 ABI 1.0 – May 20, 2021 – 11:08

AT_CLKTCKThea_valmember of this entry contains the frequency at which times()
increments.
AT_SECUREThea_valmember of this entry contains one if the program is in secure
mode (for example started with suid). Otherwise zero.
AT_BASE_PLATFORMThea_ptrmember of this entry points to a string identifying
the base architecture platform (which may be different from the platform).
AT_RANDOMThea_ptrmember of this entry points to 16 securely generated random
bytes.
AT_HWCAP2Thea_valmember of this entry contains the extended hardware feature
mask. Currently it is 0, but may contain additional feature bits in the future.
AT_EXECFNThea_ptrmember of this entry is a pointer to the file name of the executed
program.
## 3.5    Coding Examples
This section discusses example code sequences for fundamental operations such as calling
functions, accessing static objects, and transferring control from one part of a program to
another. Unlike previous material, this material is not normative.
## 3.5.1    Architectural Constraints
The AMD64 architecture usually does not allow an instruction to encode arbitrary 64-bit
constants as immediate operand. Most instructions accept 32-bit immediates that are sign
extended to the 64-bit ones.  Additionally the 32-bit operations with register destinations
implicitly perform zero extension making loads of 64-bit immediates with upper half set
to 0 even cheaper.
Additionally the branch instructions accept 32-bit immediate operands that are sign
extended and used to adjust the instruction pointer. Similarly an instruction pointer relative
addressing mode exists for data accesses with equivalent limitations.
In order to improve performance and reduce code size, it is desirable to use different
code models depending on the requirements.
Code models define constraints for symbolic values that allow the compiler to generate
better code. Basically code models differ in addressing (absolute versus position indepen-
dent),  code size,  data size and address range.   We define only a small number of code
models that are of general interest:
## 38
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Small code modelThe virtual address of code executed is known at link time. Addition-
ally all symbols are known to be located in the virtual addresses in the range from0
to2
## 31
## −2
## 24
−1or from0x00000000to0x7ef f f f f f
## 21
## .
This allows the compiler to encode symbolic references with offsets in the range
from−(2
## 31
## )to2
## 24
or  from0x80000000to0x01000000directly  in  the  sign  ex-
tended immediate operands, with offsets in the range from0to2
## 31
## −2
## 24
or from
0x00000000to0x7f000000in the zero extended immediate operands and use in-
struction pointer relative addressing for the symbols with offsets in the range−(2
## 24
## )
to2
## 24
or0xf f000000to0x01000000.
This is the fastest code model and we expect it to be suitable for the vast majority of
programs.
Kernel code modelThe  kernel  of  an  operating  system  is  usually  rather  small  but
runs  in  the  negative  half  of  the  address  space.   So  we  define  all  symbols  to  be
in  the  range  from2
## 64
## −2
## 31
to2
## 64
## −2
## 24
or  from0xf f f f f f f f80000000to
0xf f f f f f f f f f000000.
This  code  model  has  advantages  similar  to  those  of  the  small  model,  but  allows
encoding of zero extended symbolic references only for offsets from2
## 31
to2
## 31
## +2
## 24
or from0x80000000to0x81000000.  The range offsets for sign extended reference
changes to0to2
## 31
## + 2
## 24
or0x00000000to0x81000000.
Medium code modelIn the medium model, the data section is split into two parts — the
data section still limited in the same way as in the small code model and the large
data section having no limits except for available addressing space.  The program
layout must be set in a way so that large data sections (.ldata,.lrodata,.lbss)
come after the text and data sections.
This model requires the compiler to usemovabsinstructions to access large static
data and to load addresses into registers, but keeps the advantages of the small code
model for manipulation of addresses in the small data and text sections (specially
needed for branches).
By default only data larger than 65535 bytes will be placed in the large data section.
Large code modelThe  large  code  model  makes  no  assumptions  about  addresses  and
sizes of sections.
## 21
The number24is chosen arbitrarily. It allows for all memory of objects of size up to2
## 24
or 16M bytes
to be addressed directly because the base address of such objects is constrained to be less than2
## 31
## −2
## 24
or0x7f000000.  Without such constraint only the base address would be accessible directly, but not any
offsetted variant of it.
## 39
AMD64 ABI 1.0 – May 20, 2021 – 11:08

The  compiler  is  required  to  use  themovabsinstruction,  as  in  the  medium  code
model, even for dealing with addresses inside the text section.  Additionally, indi-
rect branches are needed when branching to addresses whose offset from the current
instruction pointer is unknown.
It is possible to avoid the limitation on the text section in the small and medium
models by breaking up the program into multiple shared libraries, so this model is
strictly only required if the text of a single function becomes larger than what the
medium model allows.
Small position independent code model (PIC)Unlike the previous models, the virtual
addresses of instructions and data are not known until dynamic link time.  So all
addresses have to be relative to the instruction pointer.
Additionally the maximum distance between a symbol and the end of an instruction
is limited to2
## 31
## −2
## 24
−1or0x7ef f f f f f, allowing the compiler to use instruction
pointer relative branches and addressing modes supported by the hardware for every
symbol with an offset in the range−(2
## 24
## )to2
## 24
or0xf f000000to0x01000000.
Medium position independent code model (PIC)This   model   is   like   the   previous
model, but similarly to the medium static model adds large data sections at the end
of object files.
In the medium PIC model, the instruction pointer relative addressing can not be used
directly for accessing large static data, since the offset can exceed the limitations on
the size of the displacement field in the instruction.  Instead an unwind sequence
consisting ofmovabs,leaandaddneeds to be used.
Large position independent code model (PIC)This model is like the previous model,
but makes no assumptions about the distance of symbols.
The large PIC model implies the same limitation as the medium PIC model regarding
addressing of static data.  Additionally, references to the global offset table and to
the procedure linkage table and branch destinations need to be calculated in a similar
way. Further the size of the text segment is allowed to be up to 16EB in size, hence
similar restrictions apply to all address references into the text segments, including
branches.
Only small code model and small position independent code model (PIC) are used in
ILP32 binaries.
## 40
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 3.5.2    Conventions
In this document some special assembler symbols are used in the coding examples and
discussion. They are:
•name@GOT: specifies the offset to the GOT entry for the symbolnamefrom the base of
the GOT.
•name@GOTOFF: specifies the offset to the location of the symbolnamefrom the base of
the GOT.
•name@GOTPCREL: specifies the offset to the GOT entry for the symbolnamefrom the
current code location.
•name@PLT: specifies the offset to the PLT entry of symbolnamefrom the current code
location.
•name@PLTOFF: specifies the offset to the PLT entry of symbolnamefrom the base of
the GOT.
•_GLOBAL_OFFSET_TABLE_: specifies the offset to the base of the GOT from the current
code location.
3.5.3    Position-Independent Function Prologue
In the small code model all addresses (including GOT entries) are accessible via the IP-
relative addressing provided by the AMD64 architecture.  Hence there is no need for an
explicit GOT pointer and therefore no function prologue for setting it up is necessary.
In the medium and large code models a register has to be allocated to hold the address
of the GOT in position-independent objects, because the AMD64 ISA does not support an
immediate displacement larger than 32 bits.
As%r15is preserved across function calls,  it is initialized in the function prolog to
hold the GOT address
## 22
for non-leaf functions which call other functions through the PLT.
Other functions are free to use any other register. Throughout this document,%r15will be
used in examples.
## 22
If, at code generation-time, it is determined that either no other functions are called (leaf functions), the
called functions addresses can be resolved and are within 2GB, or no global data objects are referred to, it is
not necessary to store the GOT address in%r15and the prolog code that initializes it may be omitted.
## 41
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.12: Position-Independent Function Prolog Code
medium model:
leaq     _GLOBAL_OFFSET_TABLE_(%rip),%r15 # GOTPC32 reloc
large model:
pushq    %r15                          # save %r15
leaq     1f(%rip),%r11                 # absolute %rip
1: movabs   $_GLOBAL_OFFSET_TABLE_,%r15   # offset to the GOT (R_X86_64_GOTPC64)
leaq     (%r11,%r15),%r15              # absolute address of the GOT
For the medium model the GOT pointer is directly loaded,  for the large model the
absolute value of%ripis added to the relative offset to the base of the GOT in order to
obtain its absolute address (see figure  3.12).
## 3.5.4    Data Objects
This section describes only objects with static storage. Stack-resident objects are excluded
since programs always compute their virtual address relative to the stack or frame pointers.
Because only themovabsinstruction uses 64-bit addresses directly, depending on the
code model either%rip-relative addressing or building addresses in registers and accessing
the memory through the register has to be used.
For absolute addresses%rip-relative encoding can be used in the small model.  In the
medium model themovabsinstruction has to be used for accessing addresses.
Position-independent code cannot contain absolute address. To access a global symbol
the address of the symbol has to be loaded from the Global Offset Table.  The address of
the entry in the GOT can be obtained with a%rip-relative instruction in the small model.
## 42
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Small models
Figure 3.13: Absolute Load and Store (Small Model)
extern int src[65536];.extern  src
extern int dst[65536];.extern  dst
extern int
## *
ptr;.extern  ptr
static int lsrc[65536];.local   lsrc
.comm    lsrc,262144,4
static int ldst[65536];.local   ldst
.comm    ldst,262144,4
static int
## *
lptr;.local   lptr
.comm    lptr,8,8
## .text
dst[0] = src[0];movl     src(%rip), %eax
movl     %eax, dst(%rip)
ptr = dst[0];movq     $dst, ptr(%rip)
## *
ptr = src[0];movq     ptr(%rip),%rax
movl     src(%rip),%edx
movl     %edx, (%rax)
ldst[0] = lsrc[0];movl     lsrc(%rip), %eax
movl     %eax, ldst(%rip)
lptr = ldst;movq     $dst, lptr(%rip)
## *
lptr = lsrc[0];movq     lptr(%rip),%rax
movl     lsrc(%rip),%edx
movl     %edx, (%rax)
## 43
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.14: Position-Independent Load and Store (Small PIC Model)
extern int src[65536];.extern  src
extern int dst[65536];.extern  dst
extern int
## *
ptr;.extern  ptr
static int lsrc[65536];.local   lsrc
.comm    lsrc,262144,4
static int ldst[65536];.local   ldst
.comm    ldst,262144,4
static int
## *
lptr;.local   lptr
.comm    lptr,8,8
## .text
dst[0] = src[0];movq     src@GOTPCREL(%rip), %rax
movl     (%rax), %edx
movq     dst@GOTPCREL(%rip), %rax
movl     %edx, (%rax)
ptr = dst;movq     ptr@GOTPCREL(%rip), %rax
movq     dst@GOTPCREL(%rip), %rdx
movq     %rdx, (%rax)
## *
ptr = src[0];
movq     ptr@GOTPCREL(%rip),%rax
movq     (%rax), %rdx
movq     src@GOTPCREL(%rip), %rax
movl     (%rax), %eax
movl     %eax, (%rdx)
ldst[0] = lsrc[0];movl     lsrc(%rip), %eax
movl     %eax, ldst(%rip)
lptr = ldst;lea      ldst(%rip),%rdx
movq     %rdx, lptr(%rip)
## *
lptr = lsrc[0];movq     lptr(%rip),%rax
movl     lsrc(%rip),%edx
movl     %edx, (%rax)
## 44
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Medium models
Figure 3.15: Absolute Load and Store (Medium Model)
extern int src[65536];.extern  src
extern int dst[65536];.extern  dst
extern int
## *
ptr;.extern  ptr
static int lsrc[65536];.local   lsrc
.comm    lsrc,262144,4
## 23
static int ldst[65536];.local   ldst
.comm    ldst,262144,4
static int
## *
lptr;.local   lptr
.comm    lptr,8,8
## .text
dst[0] = src[0];movabsl  src, %eax
movabsl  %eax, dst
ptr = dst;movabsq  $dst,%rdx
movq     %rdx, ptr
## *
ptr = src[0];movq     ptr(%rip),%rdx
movabsl  src,%eax
movl     %eax, (%rdx)
ldst[0] = lsrc[0];movabsl  lsrc, %eax
movabsl  %eax, ldst
lptr = ldst;movabsq  $ldst,%rdx
movabsq  %rdx, lptr
## *
lptr = lsrc[0];
movq     lptr(%rip),%rdx
movabsl  lsrc,%eax
movl     %eax, (%rdx)
## 45
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.16: Position-Independent Load and Store (Medium PIC Model)
extern int src[65536];.extern  src
extern int dst[65536];.extern  dst
extern int
## *
ptr;.extern  ptr
static int lsrc[65536];.local   lsrc
.comm    lsrc,262144,4
static int ldst[65536];.local   ldst
.comm    ldst,262144,4
static int
## *
lptr;.local   lptr
.comm    lptr,8,8
## .text
dst[0] = src[0];movq     src@GOTPCREL(%rip), %rax
movl     (%rax), %edx
movq     dst@GOTPCREL(%rip), %rax
movl     %edx, (%rax)
ptr = dst;movq     ptr@GOTPCREL(%rip), %rax
movq     dst@GOTPCREL(%rip), %rdx
movq     %rdx, (%rax)
## *
ptr = src[0];
movq     ptr@GOTPCREL(%rip),%rax
movq     (%rax), %rdx
movq     src@GOTPCREL(%rip), %rax
movl     (%rax), %eax
movl     %eax, (%rdx)
Figure 3.17: Position-Independent Load and Store (Medium PIC Model), continued
ldst[0] = lsrc[0];movabsq  lsrc@GOTOFF, %rax
movl     (%rax,%r15), %eax
movabsq  ldst@GOTOFF, %rdx
movl     %eax, (%rdx,%r15)
lptr = ldst;movabsq  ldst@GOTOFF, %rax
addq     %r15, %rax
movq     %rax, lptr(%rip)
## *
lptr = lsrc[0];movabsq  lsrc@GOTOFF, %rax
movl     (%rax,%r15),%eax
movq     lptr(%rip),%rdx
movl     %eax, (%rdx)
## 46
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Large Models
Again, in order to access data at any position in the 64-bit addressing space, it is necessary
to calculate the address explicitly
## 24
, not unlike the medium code model.
Figure 3.18: Absolute Global Data Load and Store
static int src;Lsrc: .long
static int dst;Ldst: .long
extern int
## *
ptr;.extern  ptr
dst = src;movabs  $Lsrc,%rax   ;R_X86_64_64
movabs  $Ldst,%rdx   ;R_X86_64_64
movl    (%rax),%ecx
movl    %ecx,(%rdx)
ptr = &dst;movabs  $ptr,%rax    ;R_X86_64_64
movabs  $Ldst,%rdx   ;R_X86_64_64
movq    %rdx,(%rax)
## *
ptr = src;movabs  $Lsrc,%rax   ;R_X86_64_64
movabs  $ptr,%rdx    ;R_X86_64_64
movl    (%rax),%ecx
movq    (%rdx),%rdx
movl    %ecx,(%rdx)
Figure 3.19: Faster Absolute Global Data Load and Store
## *
ptr = src;
movabs    $ptr,%rdx        ;R_X86_64_64
movl      Lsrc(%rip),%ecx
movq      (%rdx),%rdx
movl      %ecx,(%rdx)
For position-independent code access to both static and external global data assumes
that the GOT address is stored in a dedicated register.  In these examples we assume it is
in%r15
## 25
## (see Function Prologue):
## 24
If,  at code generation-time,  it is determined that a referred to global data object address is resolved
within 2GB, the%rip-relativeaddressing mode can be used instead. See example in figure  3.19.
## 25
If,  at code generation-time,  it is determined that a referred to global data object address is resolved
within 2GB, the%rip-relativeaddressing mode can be used instead. See example in figure  3.21.
## 47
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.20: Position-Independent Global Data Load and Store
static int src;Lsrc: .long
static int dst;Ldst: .long
extern int
## *
ptr;.extern  ptr
dst = src;movabs $Lsrc@GOTOFF,%rax ;R_X86_64_GOTOFF64
movabs $Ldst@GOTOFF,%rdx ;R_X86_64_GOTOFF64
movl   (%rax,%r15),%ecx
movl   %ecx,(%rdx,%r15)
ptr = &dst;movabs $ptr@GOT,%rax     ;R_X86_64_GOT64
movabs $Ldst@GOTOFF,%rdx ;R_X86_64_GOTOFF64
movq   (%rax,%r15),%rax
leaq   (%rdx,%r15),%rcx
movq   %rcx,(%rax)
## *
ptr = src;movabs $Lsrc@GOTOFF,%rax ;R_X86_64_GOTOFF64
movabs $ptr@GOT,%rdx     ;R_X86_64_GOT64
movl   (%rax,%r15),%ecx
movq   (%rdx,%r15),%rdx
movl   %ecx,(%rdx)
Figure 3.21: Faster Position-Independent Global Data Load and Store
## *
ptr = src;movabs    $ptr@GOT,%rdx    ;R_X86_64_GOT64
movl      Lsrc(%rip),%ecx
movq      (%rdx,%r15),%rdx
movl      %ecx,(%rdx)
## 3.5.5    Function Calls
Small and Medium Models
Figure 3.22: Position-Independent Direct Function Call (Small and Medium Model)
extern void function ();.globl function
function ();call function@PLT
## 48
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.23: Position-Independent Indirect Function Call
extern void (
## *
ptr) ();.globl ptr, name
extern void name ();
ptr = name;movq ptr@GOTPCREL(%rip), %rax
movq name@GOTPCREL(%rip), %rdx
movq %rdx, (%rax)
## (
## *
ptr)();movq ptr@GOTPCREL(%rip), %rax
call
## *
## (%rax)
Large models
It cannot be assumed that a function is within 2GB in general.  Therefore, it is necessary
to explicitly calculate the desired address reaching the whole 64-bit address space.
Figure 3.24: Absolute Direct and Indirect Function Call
static (
## *
ptr) (void);Lptr: .quad
extern foo (void);.globl foo
static bar (void);Lbar: ...
foo ();movabs    $foo,%r11   ;R_X86_64_64
call
## *
## %r11
bar ();movabs    $Lbar,%r11  ;R_X86_64_64
call
## *
## %r11
ptr = foo;movabs    $Lptr,%rax  ;R_X86_64_64
movabs    $foo,%r11   ;R_X86_64_64
movq      %r11,(%rax)
ptr = bar;movabs    $Lbar,%r11  ;R_X86_64_64
movq      %r11,(%rax)
## (
## *
ptr) ();movabs    $Lptr,%r11  ;R_X86_64_64
call
## *
## (%r11)
And in the case of position-independent objects
## 26
## :
## 26
See subsection “Implementation advice” for some optimizations.
## 49
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.25: Position-Independent Direct and Indirect Function Call
static (
## *
ptr) (void);Lptr: .quad
extern foo (void);.globl foo
static bar (void);Lbar: ...
foo ();movabs $foo@PLTOFF,%r11  ;R_X86_64_PLTOFF64
call
## *
## (%r11,%r15)
bar ();movabs $Lbar@GOTOFF,%r11 ;R_X86_64_GOTOFF64
leaq   (%r11,%r15),%r11
call
## *
## %r11
ptr = foo;movabs $Lptr@GOTOFF,%rax ;R_X86_64_GOTOFF64
movabs $foo@PLTOFF,%r11  ;R_X86_64_PLTOFF64
leaq   (%r11,%r15),%r11
movq   %r11,(%rax,%r15)
ptr = bar;movabs $Lbar@GOTOFF,%r11 ;R_X86_64_GOTOFF64
leaq   (%r11,%r15),%r11
movq   %r11,(%rax,%r15)
## (
## *
ptr) ();movabs $Lptr@GOTOFF,%r11 ;R_X86_64_GOTOFF64
call
## *
## (%r11,%r15)
Implementation advice
If,  at code generation-time,  certain conditions are determined,  it’s possible to generate
faster or smaller code sequences as the large model normally requires. When:
(absolute) target of function call is within 2GB, a direct call or%rip-relative address-
ing might be used:
bar ();call   Lbar
ptr = bar;movabs $Lptr,%rax         ;R_X86_64_64
leaq   $Lbar(%rip),%r11
movq   %r11,(%rax)
(PIC) the base of GOT is within 2GBan indirect call to the GOT entry might be imple-
mented like so:
foo ();call
## *
(foo@GOT)  ;R_X86_64_GOTPCREL
(PIC) the base of PLT is within 2GB,  the  PLT  entry  may  be  referred  to  relatively  to
## %rip:
ptr = foo;movabs $Lptr@GOTOFF,%rax   ;R_X86_64_GOTOFF64
leaq   $foo@PLT(%rip),%r11 ;R_X86_64_PLT32
movq   %r11,(%rax,%r15)
(PIC) target of function call is within 2GBand is either not global or bound locally, a
direct call to the symbol may be used or it may be referred to relatively to%rip:
## 50
AMD64 ABI 1.0 – May 20, 2021 – 11:08

bar ();call      Lbar
ptr = bar;movabs    $Lptr@GOTOFF,%rax  ;R_X86_64_GOTOFF64
leaq      $Lbar(%rip),%r11
movq      %r11,(%rax,%r15)
## 3.5.6    Branching
Small and Medium Models
As all labels are within 2GB no special care has to be taken when implementing branches.
The full AMD64 ISA is usable.
## Large Models
Because functions can be theoretically up to 16EB long, the maximum 32-bit displace-
ment of conditional and unconditional branches in the AMD64 ISA are not enough to
address the branch target.  Therefore, a branch target address is calculated explicitly
## 27
## .
For absolute objects:
## Figure 3.26: Absolute Branching Code
if (!a)testl     %eax,%eax
## {jnz       1f
movabs    $2f,%r11    ;R_X86_64_64
jmpq
## *
## %r11
## ...1:      ...
## }2:
goto Label;movabs    $Label,%r11 ;R_X86_64_64
jmpq
## *
## %r11
## ......
Label:Label:
## 27
If,  at  code  generation-time,  it  is  determined  that  the  target  addresses  are  within  2GB,  alternatively,
branch target addresses may be calculated implicitly (see figure 3.27)
## 51
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.27: Implicit Calculation of Target Address
if (!a)testl   %eax,%eax
## {jz      2f
## ...1:     ...
## }2:
goto Label;jmp     Label
## ......
Label:Label:
For position-independent objects:
Figure 3.28: Position-Independent Branching Code
if (!a)testl  %eax,%eax
## {jnz    1f
movabs $2f@GOTOFF,%r11    ;R_X86_64_GOTOFF64
leaq   (%r11,%r15),%r11
jmpq
## *
## %r11
## 1:  ...
## ...2:
## }
goto Label;movabs $Label@GOTOFF,%r11 ;R_X86_64_GOTOFF64
leaq   (%r11,%r15),%r11
jmpq
## *
## %r11
## ...
...Label:
## Label:
For absolute objects, the implementation of theswitchstatement is:
## 52
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.29: Absolute Switch Code
switch (a)cmpl   $0,%eax
{jl     .Ldefault
cmpl   $2,%eax
jg     .Ldefault
movabs $.Ltable,%r11  ;R_X86_64_64
jmpq
## *
## (%r11,%eax,8)
.section .lrodata,"aLM",@progbits,8
## .align 8
.Ltable:
.quad .Lcase0          ;R_X86_64_64
.quad .Ldefault        ;R_X86_64_64
.quad .Lcase2          ;R_X86_64_64
## .previous
default:.Ldefault:
## ......
case 0:.Lcase0:
## ......
case 2:.Lcase2:
## ......
## }
When  building  position-independent  objects,  theswitchstatement  implementation
changes to:
## 53
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.30: Position-Independent Switch Code
switch (a)cmpl   $0,%eax
{jl     .Ldefault
cmpl   $2,%eax
jg     .Ldefault
movabs $.Ltable@GOTOFF,%r11 ;R_X86_64_GOTOFF64
leaq   (%r11,%r15),%r11
movq
## *
## (%r11,%eax,8),%r11
leaq   (%r11,%r15),%r11
jmpq
## *
## %r11
.section .lrodata,"aLM",@progbits,8
## .align 8
.Ltable:
.quad .Lcase0@GOTOFF       ;R_X86_64_GOTOFF64
.quad .Ldefault@GOTOFF     ;R_X86_64_GOTOFF64
.quad .Lcase2@GOTOFF       ;R_X86_64_GOTOFF64
## .previous
default:.Ldefault:
## ......
case 0:.Lcase0:
## ......
case 2:.Lcase2:
## ......
## }
## 28
## 3.5.7    Variable Argument Lists
Some otherwise portable C programs depend on the argument passing scheme, implicitly
assuming that all arguments are passed on the stack, and arguments appear in increasing
order on the stack.  Programs that make these assumptions never have been portable, but
they have worked on many implementations.  However, they do not work on the AMD64
architecture because some arguments are passed in registers.  Portable C programs must
use the header file<stdarg.h>in order to handle variable argument lists.
When a function taking variable-arguments is called,%almust be set to the total num-
ber of floating point parameters passed to the function in vector registers.
## 29
## 28
The jump-table is emitted in a different section so as to occupy cache lines without instruction bytes,
thus avoiding exclusive cache subsystems to thrash.
## 29
This implies that the only legal values for%alwhen calling a function with variable-argument lists are
0 to 8 (inclusive).
## 54
AMD64 ABI 1.0 – May 20, 2021 – 11:08

When__m256or__m512is passed as variable-argument, it should always be passed
on stack.   Only named__m256and__m512arguments may be passed in register as
specified in section 3.2.3.
Figure 3.31: Parameter Passing Example with Variable-Argument List
int a, b;
long double ld;
double m, n;
__m256 u, y;
__m512 v, z;
extern void func (int a, double m, __m256 u, __m512 v, ...);
func (a, m, u, v, b, ld, y, z, n);
Figure 3.32: Register Allocation Example for Variable-Argument List
## General Purpose Registers    Floating Point Registers    Stack Frame Offset
%rdi:a%xmm0:m0:   ld
%rsi:b%ymm1:u32:  y
## %rax:    3
## %zmm2:v
## %xmm3:n
## The Register Save Area
The prologue of a function taking a variable argument list and known to call the macro
va_startis expected to save the argument registers to theregister save area.  Each argu-
ment register has a fixed offset in the register save area as defined in the figure  3.33.
Only registers that might be used to pass arguments need to be saved.  Other registers
are not accessed and can be used for other purposes. If a function is known to never accept
arguments passed in registers
## 30
, the register save area may be omitted entirely.
The prologue should use%alto avoid unnecessarily saving XMM registers.  This is
especially important for integer only programs to prevent the initialization of the XMM
unit.
## 30
This fact may be determined either by exploring types used by theva_argmacro, or by the fact that
the named arguments already are exhausted the argument registers entirely.
## 55
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.33: Register Save Area
## Register    Offset
## %rdi0
## %rsi8
## %rdx
## 16
## %rcx24
## %r832
## %r9
## 40
## %xmm048
## %xmm164
## . . .
## %xmm15288
Theva_listType
Theva_listtype is an array containing a single element of one structure containing the
necessary information to implement theva_argmacro. The C definition ofva_listtype is
given in figure 3.34.
Figure 3.34:va_listType Declaration
typedef struct {
unsigned int gp_offset;
unsigned int fp_offset;
void
## *
overflow_arg_area;
void
## *
reg_save_area;
} va_list[1];
Theva_startMacro
Theva_startmacro initializes the structure as follows:
reg_save_areaThe element points to the start of the register save area.
overflow_arg_areaThis  pointer  is  used  to  fetch  arguments  passed  on  the  stack.   It  is
initialized with the address of the first argument passed on the stack, if any, and then
always updated to point to the start of the next argument on the stack.
## 56
AMD64 ABI 1.0 – May 20, 2021 – 11:08

gp_offsetThe element holds the offset in bytes fromreg_save_areato the place where
the next available general purpose argument register is saved.  In case all argument
registers have been exhausted, it is set to the value 48 (6∗8).
fp_offsetThe element holds the offset in bytes fromreg_save_areato the place where
the next available floating point argument register is saved.   In case all argument
registers have been exhausted, it is set to the value 304 (6∗8 + 16∗16).
Theva_argMacro
The algorithm for the genericva_arg(l, type)implementation is defined as follows:
-  Determine whethertypemay be passed in the registers. If not go to step 7.
-  Computenum_gpto hold the number of general purpose registers needed to passtype
andnum_fpto hold the number of floating point registers needed.
-  Verify whether arguments fit into registers. In the case:
l->gp_offset>48−num_gp∗8
or
l->fp_offset>304−num_fp∗16
go to step 7.
-  Fetchtypefroml->reg_save_areawith   an   offset   ofl->gp_offsetand/or
l->fp_offset.This  may  require  copying  to  a  temporary  location  in  case  the
parameter  is  passed  in  different  register  classes  or  requires  an  alignment  greater
than 8 for general purpose registers and 16 for XMM registers.
## 5.  Set:
l->gp_offset=l->gp_offset+num_gp∗8
l->fp_offset=l->fp_offset+num_fp∗16.
-  Return the fetchedtype.
-  Alignl->overflow_arg_areaupwards to a 16 byte boundary if alignment needed by
typeexceeds 8 byte boundary.
## 8.  Fetchtypefroml->overflow_arg_area.
## 57
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 9.  Setl->overflow_arg_areato:
l->overflow_arg_area+sizeof(type)
-  Alignl->overflow_arg_areaupwards to an 8 byte boundary.
-  Return the fetchedtype.
Theva_argmacro is usually implemented as a compiler builtin and expanded in simpli-
fied forms for each particular type. Figure 3.35 is a sample implementation of theva_arg
macro.
Figure 3.35: Sample Implementation ofva_arg(l, int)
movll->gp_offset,%eax
cmpl$48,%eaxIs register available?
jaestackIf not, use stack
leal$8(%rax),%edxNext available register
addql->reg_save_area,%raxAddress of saved register
movl%edx,l->gp_offsetUpdategp_offset
jmpfetch
stack:    movql->overflow_arg_area,%raxAddress of stack slot
leaq8(%rax),%rdxNext available stack slot
movq%rdx,l->overflow_arg_areaUpdate
fetch:movl(%rax),%eaxLoad argument
3.6    DWARF Definition
This section
## 31
defines the Debug With Arbitrary Record Format (DWARF) debugging for-
mat for the AMD64 processor family.  The AMD64 ABI does not define a debug format.
However, all systems that do implement DWARF on AMD64 shall use the following defi-
nitions.
DWARF is a specification developed for symbolic, source-level debugging.  The de-
bugging information format does not favor the design of any compiler or debugger.  For
more  information  on  DWARF,  seeDWARF Debugging Format Standard,  available  at:
http://www.dwarfstd.org/.
## 31
This section is structured in a way similar to the PowerPC psABI
## 58
AMD64 ABI 1.0 – May 20, 2021 – 11:08

3.6.1    DWARF Release Number
The DWARF definition requires some machine-specific definitions.  The register number
mapping needs to be specified for the AMD64 registers. In addition, starting with version
3 the DWARF specification requires processor-specific address class codes to be defined.
3.6.2    DWARF Register Number Mapping
## Table 3.36
## 32
outlines the register number mapping for the AMD64 processor family.
## 33
## 3.7    Stack Unwind Algorithm
The stack frames are not self descriptive and where stack unwinding is desirable (such
as  for  exception  handling)  additional  unwind  information  needs  to  be  generated.   The
information  is  stored  in  an  allocatable  section.eh_framewhose  format  is  identical  to
.debug_framedefined by the DWARF debug information standard, seeDWARF Debug-
ging Information Format, with the following extensions:
Position independenceIn order to avoid load time relocations for position independent
code, the FDE CIE offset pointer should be stored relative to the start of CIE ta-
ble entry.  Frames using this extension of the DWARF standard must set the CIE
identifier tag to 1.
Outgoing arguments area deltaTo maintain the size of the temporarily allocated outgo-
ing arguments area present on the end of the stack (when usingpushinstructions),
operationGNU_ARGS_SIZE(0x2e) can be used.  This operation takes a singleuleb128
argument specifying the current size.  This information is used to adjust the stack
frame when jumping into the exception handler of the function after unwinding the
stack frame. Additionally the CIE Augmentation shall contain an exact specification
of the encoding used.  It is recommended to use a PC relative encoding whenever
possible and adjust the size according to the code model used.
CIE Augmentations:The augmentation field is formated according to the augmentation
field formating string stored in the CIE header.
The string may contain the following characters:
## 32
The table defines Return Address to have a register number, even though the address is stored in 0(%rsp)
and not in a physical register.
## 33
This document does not define mappings for privileged registers.
## 59
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 3.36: DWARF Register Number Mapping
Register NameNumberAbbreviation
General Purpose Register RAX0%rax
General Purpose Register RDX1%rdx
General Purpose Register RCX
## 2%rcx
General Purpose Register RBX3%rbx
General Purpose Register RSI4%rsi
General Purpose Register RDI
## 5%rdi
Frame Pointer Register RBP6%rbp
Stack Pointer Register RSP7%rsp
## Extended Integer Registers 8-158-15%r8–%r15
Return Address RA
## 16
## Vector Registers 0–717-24%xmm0–%xmm7
## Extended Vector Registers 8–1525-32%xmm8–%xmm15
## Floating Point Registers 0–7
## 33-40%st0–%st7
MMX Registers 0–741-48%mm0–%mm7
Flag Register49%rFLAGS
Segment Register ES
## 50%es
Segment Register CS51%cs
Segment Register SS
## 52%ss
Segment Register DS53%ds
Segment Register FS
## 54%fs
Segment Register GS
## 55%gs
## Reserved
## 56-57
FS Base address58%fs.base
GS Base address
## 59%gs.base
## Reserved60-61
## Task Register62%tr
LDT Register63%ldtr
128-bit Media Control and Status64%mxcsr
x87 Control Word65%fcw
x87 Status Word66%fsw
## Upper Vector Registers 16–3167-82%xmm16–%xmm31
## Reserved83-117
## Vector Mask Registers 0–7118-125%k0–%k7
## Reserved126-129
## 60
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 3.37: Pointer Encoding Specification Byte
MaskMeaning
0x1Values are stored asuleb128orsleb128type (according to flag 0x8)
## 0x2
Values are stored as 2 bytes wide integers (udata2orsdata2)
0x3Values are stored as 4 bytes wide integers (udata4orsdata4)
0x4Values are stored as 8 bytes wide integers (udata8orsdata8)
0x8Values are signed
## 0x10
Values are PC relative
0x20Values are text section relative
0x30Values are data section relative
## 0x40
Values are relative to the start of function
zIndicates that auleb128is present determining the size of the augmentation sec-
tion.
LIndicates the encoding (and thus presence) of an LSDA pointer in the FDE aug-
mentation.
The data filed consist of single byte specifying the way pointers are encoded.
It is a mask of the values specified by the table  3.37.
The default DWARF pointer encoding (direct 4-byte absolute pointers) is rep-
resented by value 0.
RIndicates a non-default pointer encoding for FDE code pointers.  The formating
is represented by a single byte in the same way as in the ‘L’ command.
PIndicates the presence and an encoding of a language personality routine in the
CIE augmentation.  The encoding is represented by a single byte in the same
way as in the ’L’ command followed by a pointer to the personality function
encoded by the specified encoding.
When the augmentation is present, the first command must always be ‘z’ to allow
easy skipping of the information.
In order to simplify manipulation of the unwind tables,  the runtime library provide
higher level API to stack unwinding mechanism, for details see section 6.2.
## 61
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 4
## Object Files
4.1    ELF Header
## 4.1.1    Machine Information
## Programming Model
As  described  in  Section  1,  binaries  using  the  AMD64  instruction  set  may  program  to
either a 32-bit model (ILP32) or to a 64-bit model (LP64).  This specification describes
both binaries that use the ILP32 and the LP64 model.
## File Class
For  AMD64  ILP32  objects,  the  file  class  value  ine_ident[EI_CLASS]must  be
ELFCLASS32. For AMD64 LP64 objects, the file class value must beELFCLASS64.
## Data Encoding
For the data encoding ine_ident[EI_DATA], AMD64 objects useELFDATA2LSB.
Processor identification
Processor identification resides in the ELF headerse_machinemember and must have
the valueEM_X86_64.
## 1
## 1
The value of this identifier is 62.
## 62
AMD64 ABI 1.0 – May 20, 2021 – 11:08

4.1.2    Number of Program Headers
Thee_phnummember contains the number of entries in the program header table.  The
product ofe_phentsizeande_phnumgives the table’s size in bytes.  If a file has no
program header table,e_phnumholds the value zero.
If the number of program headers is greater than or equal toPN_XNUM(0xffff), this mem-
ber has the valuePN_XNUM(0xffff).  The actual number of program header table entries is
contained in thesh_infofield of the section header at index 0. Otherwise, thesh_info
member of the initial entry contains the value zero.
## 4.2    Sections
## 4.2.1    Section Flags
In order to allow linking object files of different code models, it is necessary to provide
for a way to differentiate those sections which may hold more than 2GB from those which
may not.  This is accomplished by defining a processor-specific section attribute flag for
sh_flag(see table 4.1).
Table 4.1: AMD64 Specific Section Header Flag,sh_flags
NameValue
SHF_X86_64_LARGE0x10000000
SHF_X86_64_LARGEIf an object file section doesnothave this flag set, then it may not hold
more than 2GB and can be freely referred to in objects using smaller code models.
Otherwise, only objects using larger code models can refer to them.  For example,
a medium code model object can refer to data in a section that sets this flag besides
being able to refer to data in a section that does not set it;  likewise, a small code
model object can refer only to code in a section that does not set this flag.
## 63
AMD64 ABI 1.0 – May 20, 2021 – 11:08

4.2.2    Section types
## Table 4.2: Section Header Types
sh_type nameValue
SHT_X86_64_UNWIND0x70000001
SHT_X86_64_UNWINDThis section contains unwind function table entries for stack
unwinding. The contents are described in Section 4.2.4 of this document.
## 4.2.3    Special Sections
Table 4.3: Special sections
NameTypeAttributes
.gotSHT_PROGBITSSHF_ALLOC+SHF_WRITE
## .plt
## SHT_PROGBITSSHF_ALLOC+SHF_EXECINSTR
.eh_frameSHT_X86_64_UNWINDSHF_ALLOC
.gotThis section holds the global offset table.
.pltThis section holds the procedure linkage table.
.eh_frameThis section holds the unwind function table.  The contents are described in
Section 4.2.4 of this document.
The additional sections defined in table 4.4 are used by a system supporting the large
code model.
## 64
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 4.4: Additional Special Sections for the Large Code Model
NameTypeAttributes
.lbssSHT_NOBITSSHF_ALLOC+SHF_WRITE+SHF_X86_64_LARGE
.ldataSHT_PROGBITSSHF_ALLOC+SHF_WRITE+SHF_X86_64_LARGE
.ldata1SHT_PROGBITSSHF_ALLOC+SHF_WRITE+SHF_X86_64_LARGE
.lgotSHT_PROGBITSSHF_ALLOC+SHF_WRITE+SHF_X86_64_LARGE
## .lplt
## SHT_PROGBITSSHF_ALLOC+SHF_EXECINSTR+SHF_X86_64_LARGE
.lrodataSHT_PROGBITSSHF_ALLOC+SHF_X86_64_LARGE
.lrodata1SHT_PROGBITSSHF_ALLOC+SHF_X86_64_LARGE
.ltextSHT_PROGBITSSHF_ALLOC+SHF_EXECINSTR+SHF_X86_64_LARGE
In order to enable static linking of objects using different code models, the following
section ordering is suggested:
.plt .init .fini .text .got .rodata .rodata1 .data .data1 .bssThesesections
can have a combined size of up to 2GB.
.lplt .ltext .lgot .lrodata .lrodata1 .ldata .ldata1 .lbssThese   sections   plus
the above can have a combined size of up to 16EB.
4.2.4    EH_FRAME sections
The call frame information needed for unwinding the stack is output into one or more ELF
sections of typeSHT_X86_64_UNWIND. In the simplest case there will be one such section per
object file and it will be named.eh_frame.  An.eh_framesection consists of one or more
subsections.  Each subsection contains a CIE (Common Information Entry) followed by
varying number of FDEs (Frame Descriptor Entry).  A FDE corresponds to an explicit or
compiler generated function in a compilation unit, all FDEs can access the CIE that begins
their subsection for data. If the code for a function is not one contiguous block, there will
be a separate FDE for each contiguous sub-piece.
If  an  object  file  contains  C++  template  instantiations  there  shall  be  a  separate  CIE
immediately preceding each FDE corresponding to an instantiation.
Using the preferred encoding specified below, the.eh_framesection can be entirely
resolved at link time and thus can become part of the text segment.
EH_PEencoding below refers to the pointer encoding as specified in the enhanced LSB
Chapter 7 forEh_Frame_Hdr.
## 65
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 4.5: Common Information Entry (CIE)
FieldLength (byte)Description
Length4Length  of  the  CIE  (not  including  this  4-
byte field)
CIE id
4Value 0 for.eh_frame(used to distinguish
CIEs and FDEs when scanning the section)
Version1Value One (1)
CIE    Augmenta-
tion String
stringNull-terminated  string  with  legal  values
being "" or ’z’ optionally followed by sin-
gle occurrances of ’P’, ’L’, or ’R’ in any
order.  The presence of character(s) in the
string  dictates  the  content  of  field  8,  the
Augmentation Section. Each character has
one or two associated operands in the AS
(see  table  4.6  for  which  ones).   Operand
order depends on position in the string (’z’
must be first).
## Code  Align  Fac-
tor
uleb128To  be  multiplied  with  the  "Advance  Lo-
cation" instructions in the Call Frame In-
structions
## Data  Align  Fac-
tor
sleb128To be multiplied with all offsets in the Call
## Frame Instructions
## Ret Address Reg
1/uleb128A  "virtual"  register  representation  of  the
return address. In Dwarf V2, this is a byte,
otherwise it is uleb128.  It is a byte in gcc
## 3.3.x
OptionalCIE
## Augmentation
## Section
varyingPresent  if  Augmentation  String  in  Aug-
mentation Section field 4 is not 0.  See ta-
ble 4.6 for the content.
OptionalCall
FrameInstruc-
tions
varying
## 66
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 4.6: CIE Augmentation Section Content
CharOperandsLength (byte)Description
zsizeuleb128Length  of  the  remainder  of  the
## Augmentation Section
## P
personality_enc1Encoding    specifier    -    preferred
value   is   a   pc-relative,signed
## 4-byte
personality    rou-
tine
(encoded)Encoded pointer to personality rou-
tine (actually to the PLT entry for
the personality routine)
Rcode_enc1Non-defaultencodingforthe
code-pointers(FDEmem-
bersinitial_locationand
address_rangeand the operand for
DW_CFA_set_loc)  -  preferred  value
is pc-relative, signed 4-byte
## L
lsda_enc1FDE   augmentation   bodies   may
contain LSDA pointers.  If so they
are encoded as specified here - pre-
ferred  value  is  pc-relative,  signed
4-byte possibly indirect thru a GOT
entry
## 67
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 4.7: Frame Descriptor Entry (FDE)
FieldLength (byte)Description
Length4Length of the FDE (not including this 4-
byte field)
CIE pointer
4Distance from this field to the nearest pre-
ceding  CIE  (the  value  is  subtracted  from
the current address).  This value can never
be  zero  and  thus  can  be  used  to  distin-
guish CIE’s and FDE’s when scanning the
## .eh_framesection
## Initial Location
varReference to the function code correspond-
ing  to  this  FDE.  If  ’R’  is  missing  from
the CIE Augmentation String, the field is
an 8-byte absolute pointer.  Otherwise, the
correspondingEH_PEencoding in the CIE
Augmentation Section is used to interpret
the reference
## Address Range
varSize of the function code corresponding to
this FDE. If ’R’ is missing from the CIE
Augmentation String, the field is an 8-byte
unsigned  number.   Otherwise,  the  size  is
determined by the correspondingEH_PEen-
coding  in  the  CIE  Augmentation  Section
(the value is always absolute)
OptionalFDE
## Augmentation
## Section
varPresent if CIE Augmentation String is non-
empty. See table 4.8 for the content.
OptionalCall
FrameInstruc-
tions
var
## 68
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 4.8: FDE Augmentation Section Content
CharOperandsLength (byte)Description
zlengthuleb128Length of the remainder of the Aug-
mentation Section
## L
LSDAvarLSDA  pointer,  encoded  in  the  for-
mat  specified  by  the  corresponding
operand  in  the  CIE’s  augmentation
body. (only present if length > 0).
The existence and size of the optional call frame instruction area must be computed
based on the overall size and the offset reached while scanning the preceding fields of the
CIE or FDE.
The overall size of a.eh_framesection is given in the ELF section header.  The only
way to determine the number of entries is to scan the section until the end, counting entries
as they are encountered.
## 4.3    Symbol Table
The discussion of "Function Addresses" in Section 5.2 defines some special values for
symbol table fields.
TheSTT_GNU_IFUNC
## 2
symbol type is optional. It is the same asSTT_FUNCexcept
that it always points to a function or piece of executable code which takes no arguments
and returns a function pointer.   If anSTT_GNU_IFUNCsymbol is referred to by a re-
location, then evaluation of that relocation is delayed until load-time.  The value used in
the relocation is the function pointer returned by an invocation of theSTT_GNU_IFUNC
symbol.
The purpose of theSTT_GNU_IFUNCsymbol type is to allow the run-time to select
between  multiple  versions  of  the  implementation  of  a  specific  function.   The  selection
made  in  general  will  take  the  currently  available  hardware  into  account  and  select  the
most appropriate version.
## 2
It   is   specified   inLinux   Extensions   to   gABIathttps://gitlab.com/x86-psABIs/
Linux-ABI
## 69
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 4.4    Relocation
## 4.4.1    Relocation Types
Figure 4.4.1 shows the allowed relocatable fields.
## Figure 4.1: Relocatable Fields
## 7word80
## 15word160
## 31word320
## 63word640
word8This specifies a 8-bit field occupying 1 byte.
word16This specifies a 16-bit field occupying 2 bytes with arbitrary
byte  alignment.   These  values  use  the  same  byte  order  as
other word values in the AMD64 architecture.
word32This specifies a 32-bit field occupying 4 bytes with arbitrary
byte  alignment.   These  values  use  the  same  byte  order  as
other word values in the AMD64 architecture.
word64This specifies a 64-bit field occupying 8 bytes with arbitrary
byte  alignment.   These  values  use  the  same  byte  order  as
other word values in the AMD64 architecture.
wordclassThis  specifiesword64for  LP64  and  specifiesword32for
## ILP32.
The following notations are used for specifying relocations in table  4.9:
ARepresents the addend used to compute the value of the relocatable field.
## 70
AMD64 ABI 1.0 – May 20, 2021 – 11:08

BRepresents the base address at which a shared object has been loaded into memory
during execution.  Generally, a shared object is built with a 0 base virtual address,
but the execution address will be different.
GRepresents the offset into the global offset table at which the relocation entry’s symbol
will reside during execution.
GOTRepresents the address of the global offset table.
LRepresents the place (section offset or address) of the Procedure Linkage Table entry
for a symbol.
PRepresents the place (section offset or address) of the storage unit being relocated (com-
puted usingr_offset).
SRepresents the value of the symbol whose index resides in the relocation entry.
ZRepresents the size of the symbol whose index resides in the relocation entry.
The AMD64 LP64 ABI architecture uses onlyElf64_Relarelocation entries with
explicit addends. Ther_addendmember serves as the relocation addend.
The AMD64 ILP32 ABI architecture uses onlyElf32_Relarelocation entries in
relocatable  files.   Executable  files  or  shared  objects  may  use  eitherElf32_Relaor
Elf32_Relrelocation entries.
## 71
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Table 4.9: Relocation Types
NameValueFieldCalculation
R_X86_64_NONE0nonenone
R_X86_64_641word64S + A
R_X86_64_PC322word32S + A - P
R_X86_64_GOT323word32G + A
## R_X86_64_PLT32
4word32L + A - P
R_X86_64_COPY5nonenone
R_X86_64_GLOB_DAT6wordclassS
R_X86_64_JUMP_SLOT7wordclassS
R_X86_64_RELATIVE8wordclassB + A
## R_X86_64_GOTPCREL
9word32G + GOT + A - P
R_X86_64_3210word32S + A
R_X86_64_32S11word32S + A
R_X86_64_1612word16S + A
## R_X86_64_PC16
13word16S + A - P
R_X86_64_814word8S + A
## R_X86_64_PC8
15word8S + A - P
R_X86_64_DTPMOD6416word64
R_X86_64_DTPOFF6417word64
R_X86_64_TPOFF6418word64
R_X86_64_TLSGD19word32
R_X86_64_TLSLD20word32
R_X86_64_DTPOFF3221word32
R_X86_64_GOTTPOFF22word32
R_X86_64_TPOFF3223word32
## R_X86_64_PC64
## †
24word64S + A - P
## R_X86_64_GOTOFF64
## †
25word64S + A - GOT
## R_X86_64_GOTPC32
26word32GOT + A - P
## R_X86_64_SIZE32
32word32Z + A
## R_X86_64_SIZE64
## †
33word64Z + A
R_X86_64_GOTPC32_TLSDESC34word32
R_X86_64_TLSDESC_CALL35none
R_X86_64_TLSDESC36word64×2
R_X86_64_IRELATIVE37wordclassindirect (B + A)
## R_X86_64_RELATIVE64
## ††
38word64B + A
## Deprecated39
## Deprecated40
R_X86_64_GOTPCRELX41word32G + GOT + A - P
## R_X86_64_REX_GOTPCRELX
42word32G + GOT + A - P
## †
This relocation is used only for LP64.
## ††
This relocation only appears in ILP32 executable files or shared objects.
The special semantics for most of these relocation types are identical to those used for
## 72
AMD64 ABI 1.0 – May 20, 2021 – 11:08

the Intel386 ABI.
## 3  4
TheR_X86_64_GOTPCRELrelocation    has    different    semantics    from    the
R_X86_64_GOT32or  equivalent  i386R_386_GOTPCrelocation.In  particular,
because  the  AMD64  architecture  has  an  addressing  mode  relative  to  the  instruction
pointer,  it is possible to load an address from the GOT using a single instruction.  The
calculation done by theR_X86_64_GOTPCRELrelocation gives the difference between
the location in the GOT where the symbol’s address is given and the location where the
relocation is applied.
For the occurrence ofname@GOTPCRELin the following assembler instructions:
call
## *
name@GOTPCREL(%rip)
jmp
## *
name@GOTPCREL(%rip)
mov        name@GOTPCREL(%rip), %reg
test       %reg, name@GOTPCREL(%rip)
binop      name@GOTPCREL(%rip), %reg
wherebinopis  one  ofadc,add,and,cmp,or,sbb,sub,xorinstructions,   the
R_X86_64_GOTPCRELXrelocation, or theR_X86_64_REX_GOTPCRELXrelocation
if theREXprefix is present, should be generated, instead of theR_X86_64_GOTPCREL
relocation. See also section B.2.
TheR_X86_64_32andR_X86_64_32Srelocations truncate the computed value
to  32-bits.    The  linker  must  verify  that  the  generated  value  for  theR_X86_64_32
(R_X86_64_32S) relocation zero-extends (sign-extends) to the original 64-bit value.
A program or object file usingR_X86_64_8,R_X86_64_16,R_X86_64_PC16
orR_X86_64_PC8relocations is not conformant to this ABI, these relocations are only
added for documentation purposes.  TheR_X86_64_16, andR_X86_64_8relocations
truncate the computed value to 16-bits resp. 8-bits.
TherelocationsR_X86_64_DTPMOD64,R_X86_64_DTPOFF64,
## R_X86_64_TPOFF64,R_X86_64_TLSGD,R_X86_64_TLSLD,
R_X86_64_DTPOFF32,R_X86_64_GOTTPOFFandR_X86_64_TPOFF32are
listed for completeness.  They are part of the Thread-Local Storage ABI extensions and
are  documented  in  the  document  called  “ELF  Handling  for  Thread-Local  Storage”
## 5
## .
## 3
Even though the AMD64 architecture supports IP-relative addressing modes, a GOT is still required
since the offset from a particular instruction to a particular data item cannot be known by the static linker.
## 4
Note that the AMD64 architecture assumes that offsets into GOT are 32-bit values, not 64-bit values.
This choice means that a maximum of2
## 32
## /8 = 2
## 29
entries can be placed in the GOT. However, that should
be more than enough for most programs.  In the event that it is not enough, the linker could create mul-
tiple GOTs.  Because 32-bit offsets are used, loads of global data do not require loading the offset into a
displacement register; the base plus immediate displacement addressing form can be used.
## 5
This document is currently available viahttp://www.akkadia.org/drepper/tls.pdf
## 73
AMD64 ABI 1.0 – May 20, 2021 – 11:08

The relocationsR_X86_64_GOTPC32_TLSDESC,R_X86_64_TLSDESC_CALLand
R_X86_64_TLSDESCare also used for Thread-Local Storage, but are not documented
there  as  of  this  writing.   A  description  can  be  found  in  the  document  “Thread-Local
Storage Descriptors for IA32 and AMD64/EM64T”
## 6
## .
In order to make this document self-contained, a description of the TLS relocations
follows.
The%fssegment register is used to implement the thread pointer. The linear address of
the thread pointer is stored at offset 0 relative to the%fssegment register.  The following
code loads the thread pointer in the%raxregister:
movq        %fs:0, %rax
R_X86_64_DTPMOD64resolves to the index of the dynamic thread vector entry that
points  to  the  base  address  of  the  TLS  block  corresponding  to  the  module  that  defines
the referenced symbol.R_X86_64_DTPOFF64andR_X86_64_DTPOFF32compute
the offset from the pointer in that entry to the referenced symbol.  The linker generates
such relocations in adjacent entries in the GOT, in response toR_X86_64_TLSGDand
R_X86_64_TLSLDrelocations.  If the linker can compute the offset itself, because the
referenced  symbol  binds  locally,  the  relocationsR_X86_64_64andR_X86_64_32
may  be  used  instead.   Otherwise,  such  relocations  are  always  in  pairs,  such  that  the
R_X86_64_DTPOFF64relocation applies to the word64 right past the corresponding
R_X86_64_DTPMOD64relocation.
R_X86_64_TPOFF64andR_X86_64_TPOFF32resolve   to   the   offset   from
the  thread  pointer  to  a  thread-local  variable.The  former  is  generated  in  response
toR_X86_64_GOTTPOFF,  that  resolves  to  a  PC-relative  address  of  a  GOT  entry
containing such a 64-bit offset.
R_X86_64_TLSGDandR_X86_64_TLSLDboth resolve to PC-relative offsets to
aDTPMODGOT entry.   The difference between them is that,  forR_X86_64_TLSGD,
the following GOT entry will contain the offset of the referenced symbol into its TLS
block, whereas, forR_X86_64_TLSLD, the following GOT entry will contain the off-
set for the base address of the TLS block.  The idea is that adding this offset to the re-
sult  ofR_X86_64_DTPMOD32for  a  symbol  ought  to  yield  the  same  as  the  result  of
R_X86_64_DTPMOD64for the same symbol.
R_X86_64_TLSDESCresolves to a pair of word64s, called TLS Descriptor, the first
of  which  is  a  pointer  to  a  function,  followed  by  an  argument.   The  function  is  passed
a pointer to the this pair of entries in %rax and,  using the argument in the second en-
try, it must compute and return in %rax the offset from the thread pointer to the symbol
## 6
This document is currently available via
http://www.fsfla.org/~lxoliva/writeups/TLS/RFC-TLSDESC-x86.txt
## 74
AMD64 ABI 1.0 – May 20, 2021 – 11:08

referenced in the relocation, without modifying any registers other than processor flags.
R_X86_64_GOTPC32_TLSDESCresolves to the PC-relative address of a TLS descrip-
tor corresponding to the named symbol.R_X86_64_TLSDESC_CALLmust annotate the
instruction used to call the TLS Descriptor resolver function, so as to enable relaxation of
that instruction.
R_X86_64_IRELATIVEis similar toR_X86_64_RELATIVEexcept that the value
used in this relocation is the program address returned by the function, which takes no
arguments, at the address of the result of the correspondingR_X86_64_RELATIVEre-
location.
One use of theR_X86_64_IRELATIVErelocation is to avoid name lookup for the
locally definedSTT_GNU_IFUNCsymbols at load-time.  Support for this relocation is
optional, but is required for theSTT_GNU_IFUNCsymbols.
## 4.4.2    Large Models
In order to extend both the PLT and the GOT beyond 2GB, it is necessary to add appropri-
ate relocation types to handle full 64-bit addressing. See figure 4.10.
## Table 4.10: Large Model Relocation Types
NameValueFieldCalculation
R_X86_64_GOT6427word64G + A
R_X86_64_GOTPCREL6428word64G + GOT - P + A
R_X86_64_GOTPC6429word64GOT - P + A
## Deprecated30
R_X86_64_PLTOFF6431word64L - GOT + A
## 75
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 5
Program Loading and Dynamic Linking
## 5.1    Program Loading
Program loading is a process of mapping file segments to virtual memory segments.  For
efficient mapping executable and shared object files must have segments whose file offsets
and virtual addresses are congruent modulo the page size.
To save space the file page holding the last page of the text segment may also contain
the first page of the data segment.  The last data page may contain file information not
relevant to the running process. Logically, the system enforces the memory permissions as
if each segment were complete and separate; segments’ addresses are adjusted to ensure
each logical page in the address space has a single set of permissions.   In the example
above,  the  region  of  the  file  holding  the  end  of  text  and  the  beginning  of  data  will  be
mapped twice: at one virtual address for text and at a different virtual address for data.
The end of the data segment requires special handling for uninitialized data, which the
system defines to begin with zero values. Thus if a file’s last data page includes information
not in the logical memory page, the extraneous data must be set to zero, not the unknown
contents of the executable file. “Impurities” in the other three pages are not logically part
of the process image; whether the system expunges them is unspecified.
One aspect of segment loading differs between executable files and shared objects.
Executable  file  segments  typically  contain  absolute  code  (see  section  3.5  “Coding  Ex-
amples”).   For the process to execute correctly,  the segments must reside at the virtual
addresses used to build the executable file.  Thus the system uses thep_vaddrvalues
unchanged as virtual addresses.
On  the  other  hand,  shared  object  segments  typically  contain  position-independent
code.  This lets a segments virtual address change from one process to another, without
invalidating execution behavior. Though the system chooses virtual addresses for individ-
## 76
AMD64 ABI 1.0 – May 20, 2021 – 11:08

ual processes, it maintains the segments’ relative positions. Because position-independent
code uses relative addressing between segments, the difference between virtual addresses
in memory must match the difference between virtual addresses in the file.
5.1.1    Program header
The following AMD64 program header types are defined:
## Table 5.1: Program Header Types
NameValue
PT_GNU_EH_FRAME0x6474e550
PT_SUNW_EH_FRAME0x6474e550
PT_SUNW_UNWIND0x6464e550
PT_GNU_EH_FRAME, PT_SUNW_EH_FRAME and PT_SUNW_UNWINDThe
segment contains the stack unwind tables. See Section 4.2.4 of this document.
## 1
## 5.2    Dynamic Linking
## Dynamic Section
Dynamic section entries give information to the dynamic linker. Some of this information
is processor-specific, including the interpretation of some entries in the dynamic structure.
Global Offset Table (GOT)
Position-independent code cannot, in general, contain absolute virtual addresses.  Global
offset tables hold absolute addresses in private data, thus making the addresses available
without  compromising  the  position-independence  and  shareability  of  a  program’s  text.
A program references its global offset table using position-independent addressing and
extracts absolute values, thus redirecting position-independent references to absolute lo-
cations.
## 1
The value for these program headers have been placed in thePT_LOOSandPT_HIOS(os specific
range) in order to adapt to the existing GNU implementation. New OS’s wanting to agree on these program
header should also add it into their OS specific range.
## 77
AMD64 ABI 1.0 – May 20, 2021 – 11:08

If a program requires direct access to the absolute address of a symbol, that symbol
will have a global offset table entry.  Because the executable file and shared objects have
separate global offset tables, a symbol’s address may appear in several tables. The dynamic
linker processes all the global offset table relocations before giving control to any code in
the process image, thus ensuring the absolute addresses are available during execution.
The tables first entry (number zero) is reserved to hold the address of the dynamic
structure, referenced with the symbol_DYNAMIC. This allows a program, such as the dy-
namic linker, to find its own dynamic structure without having yet processed its relocation
entries.  This is especially important for the dynamic linker, because it must initialize it-
self without relying on other programs to relocate its memory image.   On the AMD64
architecture, entries one and two in the global offset table also are reserved.
The global offset table contains 64-bit addresses.
For the large models the GOT is allowed to be up to 16EB in size.
## Figure 5.1: Global Offset Table
extern Elf64_Addr _GLOBAL_OFFSET_TABLE_ [];
The symbol_GLOBAL_OFFSET_TABLE_may reside in the middle of the.gotsection,
allowing both negative and non-negative offsets into the array of addresses.
## Function Addresses
References to the address of a function from an executable file and the shared objects asso-
ciated with it might not resolve to the same value. References from within shared objects
will normally be resolved by the dynamic linker to the virtual address of the function it-
self.  References from within the executable file to a function defined in a shared object
will normally be resolved by the link editor to the address of the procedure linkage table
entry for that function within the executable file.
To allow comparisons of function addresses to work as expected, if an executable file
references a function defined in a shared object, the link editor will place the address of
the procedure linkage table entry for that function in its associated symbol table entry.
This will result in symbol table entries with section index ofSHN_UNDEFbut a type of
STT_FUNCand a non-zerost_value.  A reference to the address of a function from
within a shared library will be satisfied by such a definition in the executable.
## 78
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Some relocations are associated with procedure linkage table entries.  These entries
are used for direct function calls rather than for references to function addresses.  These
relocations do not use the special symbol value described above.  Otherwise a very tight
endless loop would be created.
## Procedure Linkage Table
Much as the global offset table redirects position-independent address calculations to ab-
solute locations, the procedure linkage table redirects position-independent function calls
to absolute locations. The link editor cannot resolve execution transfers (such as function
calls) from one executable or shared object to another.  Consequently, the link editor ar-
ranges to have the program transfer control to entries in the procedure linkage table. On the
AMD64 architecture, procedure linkage tables reside in shared text, but they use addresses
in the private global offset table. The dynamic linker determines the destinations’ absolute
addresses and modifies the global offset table’s memory image accordingly. The dynamic
linker thus can redirect the entries without compromising the position-independence and
shareability of the program’s text.  Executable files and shared object files have separate
procedure linkage tables. Unlike Intel386 ABI, this ABI uses the same procedure linkage
table for both programs and shared objects (see figure 5.2).
Figure 5.2: Procedure Linkage Table (small and medium models)
.PLT0: pushq    GOT+8(%rip)              # GOT[1]
jmp
## *
GOT+16(%rip)            # GOT[2]
nopl     0x0(%rax)
.PLT1: jmp
## *
name1@GOTPCREL(%rip)    # 16 bytes from .PLT0
pushq    $index1
jmp      .PLT0
.PLT2: jmp
## *
name2@GOTPCREL(%rip)    # 16 bytes from .PLT1
pushq    $index2
jmp      .PLT0
## .PLT3: ...
Following the steps below, the dynamic linker and the program “cooperate” to resolve
symbolic references through the procedure linkage table and the global offset table.
-  When first creating the memory image of the program, the dynamic linker sets the
second and the third entries in the global offset table to special values. Steps below
explain more about these values.
## 79
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  Each shared object file in the process image has its own procedure linkage table, and
control transfers to a procedure linkage table entry only from within the same object
file.
-  For illustration, assume the program callsname1, which transfers control to the label
## .PLT1.
-  The first instruction jumps to the address in the global offset table entry forname1.
Initially the global offset table holds the address of the followingpushqinstruction,
not the real address ofname1.
-  Now the program pushes a relocation index (index) on the stack.   The relocation
index  is  a  32-bit,  non-negative  index  into  the  relocation  table  addressed  by  the
DT_JMPRELdynamic section entry.  The designated relocation entry will have type
R_X86_64_JUMP_SLOT, and its offset will specify the global offset table entry
used in the previousjmpinstruction.  The relocation entry contains a symbol table
index that will reference the appropriate symbol,name1in the example.
-  After pushing the relocation index, the program then jumps to.PLT0, the first entry
in the procedure linkage table. Thepushqinstruction places the value of the second
global offset table entry (GOT+8) on the stack, thus giving the dynamic linker one
word of identifying information. The program then jumps to the address in the third
global offset table entry (GOT+16), which transfers control to the dynamic linker.
-  When the dynamic linker receives control, it unwinds the stack, looks at the desig-
nated relocation entry, finds the symbol’s value, stores the “real” address forname1
in its global offset table entry, and transfers control to the desired destination.
-  Subsequent executions of the procedure linkage table entry will transfer directly to
name1, without calling the dynamic linker a second time. That is, thejmpinstruction
at.PLT1will transfer toname1, instead of “falling through” to thepushqinstruction.
TheLD_BIND_NOWenvironment variable can change the dynamic linking behavior.  If
its value is non-null, the dynamic linker evaluates procedure linkage table entries before
transferring control to the program. That is, the dynamic linker processes relocation entries
of typeR_X86_64_JUMP_SLOTduring process initialization.  Otherwise, the dynamic linker
evaluates procedure linkage table entries lazily, delaying symbol resolution and relocation
until the first execution of a table entry.
## 80
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Relocation entries of typeR_X86_64_TLSDESCmay also be subject to lazy relocation,
using a single entry in the procedure linkage table and in the global offset table, at loca-
tions given byDT_TLSDESC_PLTandDT_TLSDESC_GOT, respectively, as described
in “Thread-Local Storage Descriptors for IA32 and AMD64/EM64T”
## 2
## .
For  self-containment,DT_TLSDESC_GOTspecifies  a  GOT  entry  in  which  the  dy-
namic loader should store the address of its internal TLS Descriptor resolver function,
whereasDT_TLSDESC_PLTspecifies the address of a PLT entry to be used as the TLS
descriptor resolver function for lazy resolution from within this module.  The PLT entry
must push the linkmap of the module onto the stack and tail-call the internal TLS Descrip-
tor resolver function.
## Large Models
In the small and medium code models the size of both the PLT and the GOT is limited by
the maximum 32-bit displacement size. Consequently, the base of the PLT and the top of
the GOT can be at most 2GB apart.
Therefore, in order to support the available addressing space of 16EB, it is necessary
to extend both the PLT and the GOT. Moreover, the PLT needs to support the GOT being
over 2GB away and the GOT can be over 2GB in size.
## 3
The PLT is extended as shown in figure 5.3 with the assumption that the GOT address
is in%r15
## 4
## .
## 2
This document is currently available via
http://www.fsfla.org/~lxoliva/writeups/TLS/RFC-TLSDESC-x86.txt
## 3
If it is determined that the base of the PLT is within 2GB of the top of the GOT, it is also allowed to use
the same PLT layout for a large code model object as that of the small and medium code models.
## 4
## See Function Prologue.
## 81
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 5.3: Final Large Code Model PLT
.PLT0:  pushq   8(%r15)          # GOT[1]
jmpq
## *
16(%r15)        # GOT[2]
nopl    0x0(%rax,%rax,1)
.PLT1:  movabs  $name1@GOT,%r11  # 16 bytes from .PLT0
jmp
## *
## (%r11,%r15)
.PLT1a: pushq   $index1          # "call" dynamic linker
jmp     .PLT0
.PLT2:  ...                      # 21 bytes from .PLT1
.PLTx:  movabs  $namex@GOT,%r11  # 102261125th entry
jmp
## *
## (%r11,%r15)
.PLTxa: pushq   $indexx
pushq   8(%r15)          # repeat .PLT0 code
jmpq
## *
## 16(%r15)
.PLTy: ...                       # 27 bytes from .PLTx
This way, for the first 102261125 entries, each PLT entry besides.PLT0uses only 21
bytes. Afterwards, the PLT entry code changes by repeating that of .PLT0, when each PLT
entry is 27 bytes long. Notice that any alignment consideration is dropped in order to keep
the PLT size down.
Each extended PLT entry is thus 5 to 11 bytes larger than the small and medium code
model PLT entries.
The functionality of entry .PLT0 remains unchanged from the small and medium code
models.
Note that the symbol index is still limited to 32 bits, which would allow for up to 4G
global and external functions.
Typically, UNIX compilers support two types of PLT, generally through the options
-fpicand-fPIC. When building position-independent objects using the large code model,
only-fPICis allowed. Using the option-fpicwith the large code model remains reserved
for future use.
## 5.2.1    Program Interpreter
The valid program interpreter for programs conforming to the AMD64 ABI is listed in
Table 5.4, which also contains the program interpreter used by Linux.
## 82
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 5.4: AMD64 Program Interpreter
Data ModelPathLinux Path
LP64/lib/ld64.so.1/lib64/ld-linux-x86-64.so.2
ILP32/lib/ldx32.so.1/libx32/ld-linux-x32.so.2
5.2.2    Initialization and Termination Functions
The implementation is responsible for executing the initialization functions specified by
DT_INIT,DT_INIT_ARRAY, andDT_PREINIT_ARRAYentries in the executable file and
shared object files for a process, and the termination (or finalization) functions specified
byDT_FINIandDT_FINI_ARRAY, as specified by theSystem V ABI. The user program
plays no further part in executing the initialization and termination functions specified by
these dynamic tags.
## 5.3    Program Property
The following processor-specific program property type ranges
## 5
are defined:
## Table 5.2: Program Property Type Ranges
NameValue
GNU_PROPERTY_X86_UINT32_AND_LO0xc0000002
GNU_PROPERTY_X86_UINT32_AND_HI0xc0007fff
## GNU_PROPERTY_X86_UINT32_OR_LO
## 0xc0008000
GNU_PROPERTY_X86_UINT32_OR_HI0xc000ffff
## GNU_PROPERTY_X86_UINT32_OR_AND_LO
## 0xc0010000
## GNU_PROPERTY_X86_UINT32_OR_AND_HI
## 0xc0017fff
Thepr_datafield of each property contains a 4-byte unsigned integer.
## GNU_PROPERTY_X86_UINT32_AND_LO..GNU_PROPERTY_X86_UINT32_AND_HI
A bit in the outputpr_datafield is set only if it is set in all relocatable inputpr_data
## 5
SeeLinux Extensions to gABIathttps://gitlab.com/x86-psABIs/Linux-ABI
## 83
AMD64 ABI 1.0 – May 20, 2021 – 11:08

fields.  If all bits in the the outputpr_datafield are zero, this property should be
removed from output. If the bit is 1, all input relocatables have the feature. If the bit
is 0 or the property is missing, it is unknown whether all input relocatables have the
feature.
## GNU_PROPERTY_X86_UINT32_OR_LO..GNU_PROPERTY_X86_UINT32_OR_HI
A bit in the outputpr_datafield is set if it is set in any relocatable inputpr_data
fields.  If all bits in the the outputpr_datafield are zero, this property should be
removed from output.  If the bit is 1, some input relocatables have the feature.  If
the bit is 0 or the property is missing, it is unknown whether any input relocatables
have the feature.
## GNU_PROPERTY_X86_UINT32_OR_AND_LO..GNU_PROPERTY_X86_UINT32_OR_AND_HI
A bit in the outputpr_datafield is set if it is set in any relocatable inputpr_data
fields and this property is present in all relocatable input files.  A missing property
implies that its bits have unknown values.  When all bits in the the outputpr_data
field are zero, this property should not be removed from output to indicate it has
zero in all bits.  If the property is in output, all input relocatables have the property.
If the bit is 1, some input relocatables have the feature. If the bit is 0, none of input
relocatables have the feature.
The following property types are defined:
## Table 5.3: Program Property Types
NameValue
## GNU_PROPERTY_X86_FEATURE_1_ANDGNU_PROPERTY_X86_UINT32_AND_LO + 0
## GNU_PROPERTY_X86_FEATURE_2_USEDGNU_PROPERTY_X86_UINT32_OR_AND_LO + 1
## GNU_PROPERTY_X86_FEATURE_2_NEEDED
## GNU_PROPERTY_X86_UINT32_OR_LO + 1
## GNU_PROPERTY_X86_ISA_1_USED
## GNU_PROPERTY_X86_UINT32_OR_AND_LO + 2
## GNU_PROPERTY_X86_ISA_1_NEEDED
## GNU_PROPERTY_X86_UINT32_OR_LO + 2
GNU_PROPERTY_X86_FEATURE_1_ANDThe x86 processor features indicated by
the corresponding bits are used in program.
GNU_PROPERTY_X86_FEATURE_2_USEDThe  x86  processor  features  indicated
by the corresponding bits are used in program.   Their support in the hardware is
optional.  Its absence in an x86 ELF binary implies that any x86 processor features
may be used.GNU_PROPERTY_X86_FEATURE_2_USEDcan be used to check
for features used in the program.
## 84
AMD64 ABI 1.0 – May 20, 2021 – 11:08

GNU_PROPERTY_X86_FEATURE_2_NEEDEDThe   x86   processor   features   in-
dicated  by  the  corresponding  bits  are  needed  in  program  and  they  must  be
supported  by  the  hardware.Loader  may  refuse  to  load  the  program  whose
GNU_PROPERTY_X86_FEATURE_2_NEEDEDfeatures  aren’t  supported  by  the
hardware.
GNU_PROPERTY_X86_ISA_1_USEDThe x86 instruction sets indicated by the cor-
responding  bits  are  used  in  program.   Their  support  in  the  hardware  is  optional.
GNU_PROPERTY_X86_ISA_1_USEDcan be used to check for ISAs used in the
program.
GNU_PROPERTY_X86_ISA_1_NEEDEDThex86instructionsetsindicated
by   the   corresponding   bits   are   needed   in   program   and   they   must   be   sup-
ported   by   the   hardware.Loader   may   refuse   to   load   the   program   whose
GNU_PROPERTY_X86_ISA_1_NEEDEDISAs aren’t supported by the hardware.
The following bits are defined forGNU_PROPERTY_X86_FEATURE_1_AND:
Table 5.4: GNU_PROPERTY_X86_FEATURE_1_AND Bit Flags
NameValue
## GNU_PROPERTY_X86_FEATURE_1_IBT1U « 0
## GNU_PROPERTY_X86_FEATURE_1_SHSTK1U « 1
## GNU_PROPERTY_X86_FEATURE_1_LAM_U48
## 1U « 2
## GNU_PROPERTY_X86_FEATURE_1_LAM_U571U « 3
GNU_PROPERTY_X86_FEATURE_1_IBTThis indicates that all executable sections
are compatible with IBT (see Section 13.1.1) whenendbr64instruction starts each
valid target where an indirect branch instruction can land.
GNU_PROPERTY_X86_FEATURE_1_SHSTKThis indicates that all executable sec-
tions are compatible with SHSTK (see Section 13.1.2) where return address popped
from shadow stack always matches return address popped from normal stack.
GNU_PROPERTY_X86_FEATURE_1_LAM_U48This indicates that all sections are
compatible with LAM_U48 (see Section 14.2.1).
GNU_PROPERTY_X86_FEATURE_1_LAM_U57This indicates that all sections are
compatible with LAM_U57 (see Section 14.2.2).
## 85
AMD64 ABI 1.0 – May 20, 2021 – 11:08

The following bits are defined forGNU_PROPERTY_X86_ISA_1_USEDandGNU_PROPERTY_X86_ISA_1_NEEDED
based on micro-architecture levels in table 3.1:
## Table 5.5: Bit Flags For X86 Instruction Sets
NameValue
## GNU_PROPERTY_X86_ISA_1_BASELINE1U « 0
## GNU_PROPERTY_X86_ISA_1_V21U « 1
## GNU_PROPERTY_X86_ISA_1_V31U « 2
## GNU_PROPERTY_X86_ISA_1_V4
## 1U « 3
GNU_PROPERTY_X86_ISA_1_BASELINEThis   indicates   thatbaselinemicro-
architecture level must be supported by the hardware (see table 3.1).
GNU_PROPERTY_X86_ISA_1_V2This    indicates    that    micro-architecture    level
x86-64-v2must be supported by the hardware (see table 3.1).
GNU_PROPERTY_X86_ISA_1_V3This    indicates    that    micro-architecture    level
x86-64-v3must be supported by the hardware (see table 3.1).
GNU_PROPERTY_X86_ISA_1_V4This    indicates    that    micro-architecture    level
x86-64-v4must be supported by the hardware (see table 3.1).
The    following    bits    are    defined    forGNU_PROPERTY_X86_FEATURE_2_USEDand
## GNU_PROPERTY_X86_FEATURE_2_NEEDED:
## 86
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Table 5.6: Bit Flags For X86 Processor Features
NameValue
## GNU_PROPERTY_X86_FEATURE_2_X86
## †
## 1U « 0
## GNU_PROPERTY_X86_FEATURE_2_X87
## 1U « 1
## GNU_PROPERTY_X86_FEATURE_2_MMX1U « 2
## GNU_PROPERTY_X86_FEATURE_2_XMM1U « 3
## GNU_PROPERTY_X86_FEATURE_2_YMM1U « 4
## GNU_PROPERTY_X86_FEATURE_2_ZMM
## 1U « 5
## GNU_PROPERTY_X86_FEATURE_2_FXSR1U « 6
## GNU_PROPERTY_X86_FEATURE_2_XSAVE1U « 7
## GNU_PROPERTY_X86_FEATURE_2_XSAVEOPT
## 1U « 8
## GNU_PROPERTY_X86_FEATURE_2_XSAVEC1U « 9
## GNU_PROPERTY_X86_FEATURE_2_TMM1U « 10
## GNU_PROPERTY_X86_FEATURE_2_MASK1U « 11
## †
This bit should always be set whenGNU_PROPERTY_X86_FEATURE_2_USED
is used.
GNU_PROPERTY_X86_FEATURE_2_X86This  indicates  that  basic  x86  processor
features are used.
GNU_PROPERTY_X86_FEATURE_2_X87This indicates that x87 FPU is used.
GNU_PROPERTY_X86_FEATURE_2_MMXThis indicates that MMX register fea-
ture is used.
GNU_PROPERTY_X86_FEATURE_2_XMMThis indicates that XMM register fea-
ture is used.
GNU_PROPERTY_X86_FEATURE_2_YMMThis indicates that YMM register fea-
ture is used.
GNU_PROPERTY_X86_FEATURE_2_ZMMThis  indicates  that  ZMM  register  fea-
ture is used.
GNU_PROPERTY_X86_FEATURE_2_FXSRThis  indicates  that  FXSR  feature  is
used.
## 87
AMD64 ABI 1.0 – May 20, 2021 – 11:08

GNU_PROPERTY_X86_FEATURE_2_XSAVEOPTThis indicates that XSAVEOPT
feature is used.
GNU_PROPERTY_X86_FEATURE_2_XSAVECThis indicates that XSAVEC feature
is used.
GNU_PROPERTY_X86_FEATURE_2_TMMThis  indicates  that  TMM  register  fea-
ture is used.
GNU_PROPERTY_X86_FEATURE_2_MASKThis indicates that mask register fea-
ture is used.
## 88
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 6
## Libraries
A further review of the Intel386 ABI is needed.
## 6.1    C Library
## 6.1.1    Global Data Symbols
The symbols_fp_hw,__flt_roundsand__huge_valare not provided by the AMD64 ABI.
## 6.1.2    Floating Point Environment Functions
ISO C 99 defines the floating point environment functions from<fenv.h>. Since AMD64
has two floating point units with separate control words, the programming environment has
to keep the control values in sync.  On the other hand this means that routines accessing
the control words only need to access one unit, and the SSE unit is the unit that should
be accessed in these cases.  The functionfegetroundtherefore only needs to report the
rounding value of the SSE unit and can ignore the x87 unit.
## 6.2    Unwind Library Interface
This  section  defines  the  Unwind  Library  interface
## 1
,  expected  to  be  provided  by  any
AMD64 psABI-compliant system. This is the interface on which the C++ ABI exception-
handling  facilities  are  built.   We  assume  as  a  basis  the  Call  Frame  Information  tables
described in the DWARF Debugging Information Format document.
## 1
The overall structure and the external interface is derived from the IA-64 UNIX System V ABI
## 89
AMD64 ABI 1.0 – May 20, 2021 – 11:08

This section is meant to specify a language-independent interface that can be used to
provide higher level exception-handling facilities such as those defined by C++.
The unwind library interface consists of at least the following routines:
_Unwind_RaiseException,
_Unwind_Resume,
_Unwind_DeleteException,
_Unwind_GetGR,
_Unwind_SetGR,
_Unwind_GetIP,
_Unwind_GetIPInfo,
_Unwind_SetIP,
_Unwind_GetRegionStart,
_Unwind_GetLanguageSpecificData,
_Unwind_ForcedUnwind,
_Unwind_GetCFA
In addition,  two data types are defined (_Unwind_Contextand_Unwind_Exception)
to interface a calling runtime (such as the C++ runtime) and the above routine.  All rou-
tines and interfaces behave as if definedextern "C".  In particular, the names are not
mangled. All names defined as part of this interface have a"_Unwind_"prefix.
Lastly, a language and vendor specific personality routine will be stored by the com-
piler in the unwind descriptor for the stack frames requiring exception processing.  The
personality routine is called by the unwinder to handle language-specific tasks such as
identifying the frame handling a particular exception.
## 6.2.1    Exception Handler Framework
Reasons for Unwinding
There are two major reasons for unwinding the stack:
-  exceptions, as defined by languages that support them (such as C++)
-  “forced” unwinding (such as caused bylongjmpor thread termination)
The interface described here tries to keep both similar.  There is a major difference,
however.
-  In the case where an exception is thrown, the stack is unwound while the exception
propagates, but it is expected that the personality routine for each stack frame knows
## 90
AMD64 ABI 1.0 – May 20, 2021 – 11:08

whether it wants to catch the exception or pass it through.  This choice is thus del-
egated to the personality routine, which is expected to act properly for any type of
exception, whether “native” or “foreign”. Some guidelines for “acting properly” are
given below.
-  During “forced unwinding”, on the other hand, an external agent is driving the un-
winding.  For instance, this can be thelongjmproutine.  This external agent, not
each personality routine, knows when to stop unwinding. The fact that a personality
routine is not given a choice about whether unwinding will proceed is indicated by
the_UA_FORCE_UNWINDflag.
To    accommodate    these    differences,two    different    routines    are    proposed.
_Unwind_RaiseExceptionperforms  exception-style  unwinding,   under  control  of  the
personality routines._Unwind_ForcedUnwind, on the other hand, performs unwinding, but
gives an external agent the opportunity to intercept calls to the personality routine.  This
is done using a proxy personality routine, that intercepts calls to the personality routine,
letting the external agent override the defaults of the stack frame’s personality routine.
As a consequence, it is not necessary for each personality routine to know about any
of the possible external agents that may cause an unwind.  For instance, the C++ person-
ality routine need deal only with C++ exceptions (and possibly disguising foreign excep-
tions), but it does not need to know anything specific about unwinding done on behalf of
longjmpor pthreads cancellation.
## The Unwind Process
The standard ABI exception handling/unwind process begins with the raising of an excep-
tion, in one of the forms mentioned above.  This call specifies an exception object and an
exception class.
The runtime framework then starts a two-phase process:
-  In thesearchphase, the framework repeatedly calls the personality routine, with the
_UA_SEARCH_PHASEflag as described below,  first for the current%ripand register
state, and then unwinding a frame to a new%ripat each step, until the personal-
ity routine reports either success (a handler found in the queried frame) or failure
(no handler) in all frames.  It does not actually restore the unwound state, and the
personality routine must access the state through the API.
-  If the search phase reports a failure, e.g.  because no handler was found, it will call
terminate()rather than commence phase 2.
## 91
AMD64 ABI 1.0 – May 20, 2021 – 11:08

If the search phase reports success,  the framework restarts in thecleanupphase.
Again, it repeatedly calls the personality routine, with the_UA_CLEANUP_PHASEflag
as described below, first for the current%ripand register state, and then unwinding
a  frame  to  a  new%ripat  each  step,  until  it  gets  to  the  frame  with  an  identified
handler.  At that point, it restores the register state, and control is transferred to the
user landing pad code.
Each of these two phases uses both the unwind library and the personality routines,
since the validity of a given handler and the mechanism for transferring control to it are
language-dependent,  but the method of locating and restoring previous stack frames is
language-independent.
A two-phase exception-handling model is not strictly necessary to implement C++ lan-
guage semantics, but it does provide some benefits.  For example, the first phase allows
an exception-handling mechanism todismissan exception before stack unwinding begins,
which allowspresumptiveexception handling (correcting the exceptional condition and
resuming execution at the point where it was raised).  While C++ does not support pre-
sumptive exception handling, other languages do, and the two-phase model allows C++ to
coexist with those languages on the stack.
Note that even with a two-phase model, we may execute each of the two phases more
than once for a single exception, as if the exception was being thrown more than once. For
instance, since it is not possible to determine if a given catch clause will re-throw or not
without executing it, the exception propagation effectively stops at each catch clause, and
if it needs to restart, restarts at phase 1. This process is not needed for destructors (cleanup
code), so the phase 1 can safely process all destructor-only frames at once and stop at the
next enclosing catch clause.
For example, if the first two frames unwound contain only cleanup code, and the third
frame contains a C++ catch clause, the personality routine in phase 1, does not indicate
that it found a handler for the first two frames. It must do so for the third frame, because it
is unknown how the exception will propagate out of this third frame, e.g.  by re-throwing
the exception or throwing a new one in C++.
The API specified by the AMD64 psABI for implementing this framework is described
in the following sections.
## 6.2.2    Data Structures
## Reason Codes
The  unwind  interface  uses  reason  codes  in  several  contexts  to  identify  the  reasons  for
failures or other actions, defined as follows:
## 92
AMD64 ABI 1.0 – May 20, 2021 – 11:08

typedef enum {
## _URC_NO_REASON = 0,
## _URC_FOREIGN_EXCEPTION_CAUGHT = 1,
## _URC_FATAL_PHASE2_ERROR = 2,
## _URC_FATAL_PHASE1_ERROR = 3,
## _URC_NORMAL_STOP = 4,
## _URC_END_OF_STACK = 5,
## _URC_HANDLER_FOUND = 6,
## _URC_INSTALL_CONTEXT = 7,
## _URC_CONTINUE_UNWIND = 8
} _Unwind_Reason_Code;
The interpretations of these codes are described below.
## Exception Header
The unwind interface uses a pointer to an exception header object as its representation
of an exception being thrown.  In general, the full representation of an exception object
is language- and implementation-specific, but is prefixed by a header understood by the
unwind interface, defined as follows:
typedef void (
## *
_Unwind_Exception_Cleanup_Fn)
(_Unwind_Reason_Code reason,
struct _Unwind_Exception
## *
exc);
struct _Unwind_Exception {
uint64                        exception_class;
_Unwind_Exception_Cleanup_Fn  exception_cleanup;
uint64                        private_1;
uint64                        private_2;
## };
An_Unwind_Exceptionobject must be eightbyte aligned. The first two fields are set by
user code prior to raising the exception, and the latter two should never be touched except
by the runtime.
Theexception_classfield is a language- and implementation-specific identifier of the
kind of exception. It allows a personality routine to distinguish between native and foreign
exceptions, for example. By convention, the high 4 bytes indicate the vendor (for instance
AMD\0), and the low 4 bytes indicate the language.  For the C++ ABI described in this
document, the low four bytes are C++\0.
Theexception_cleanuproutine  is  called  whenever  an  exception  object  needs  to  be
destroyed by a different runtime than the runtime which created the exception object, for
instance if a Java exception is caught by a C++ catch handler. In such a case, a reason code
(see above) indicates why the exception object needs to be deleted:
_URC_FOREIGN_EXCEPTION_CAUGHT = 1This indicates that a different runtime caught this
exception.  Nested foreign exceptions, or re-throwing a foreign exception, result in
undefined behavior.
## 93
AMD64 ABI 1.0 – May 20, 2021 – 11:08

_URC_FATAL_PHASE1_ERROR = 3The personality routine encountered an error during phase
1, other than the specific error codes defined.
_URC_FATAL_PHASE2_ERROR = 2The personality routine encountered an error during phase
2, for instance a stack corruption.
Normally,   all   errors   should   be   reported   during   phase   1   by   returning   from
_Unwind_RaiseException.However,  landing  pad  code  could  cause  stack  corruption
between phase 1 and phase 2.  For a C++ exception, the runtime should callterminate()
in that case.
The private unwinder state (private_1andprivate_2) in an exception object should
be neither read by nor written to by personality routines or other parts of the language-
specific runtime.  It is used by the specific implementation of the unwinder on the host
to store internal information, for instance to remember the final handler frame between
unwinding phases.
In addition to the above information, a typical runtime such as the C++ runtime will
add  language-specific  information  used  to  process  the  exception.   This  is  expected  to
be a contiguous area of memory after the_Unwind_Exceptionobject,  but this is not re-
quired as long as the matching personality routines know how to deal with it,  and the
exception_cleanuproutine de-allocates it properly.
## Unwind Context
The_Unwind_Contexttype is an opaque type used to refer to a system-specific data struc-
ture used by the system unwinder.  This context is created and destroyed by the system,
and passed to the personality routine during unwinding.
struct _Unwind_Context
6.2.3    Throwing an Exception
_Unwind_RaiseException
_Unwind_Reason_Code _Unwind_RaiseException
( struct _Unwind_Exception
## *
exception_object );
Raise  an  exception,  passing  along  the  given  exception  object,  which  should  have
itsexception_classandexception_cleanupfields  set.   The  exception  object  has  been
allocated  by  the  language-specific  runtime,  and  has  a  language-specific  format,  ex-
cept  that  it  must  contain  an_Unwind_Exceptionstruct  (see  Exception  Header  above).
_Unwind_RaiseExceptiondoes not return, unless an error condition is found (such as no
## 94
AMD64 ABI 1.0 – May 20, 2021 – 11:08

handler for the exception, bad stack format, etc.). In such a case, an_Unwind_Reason_Code
value is returned.
Possibilities are:
_URC_END_OF_STACKThe unwinder encountered the end of the stack during phase 1, with-
out finding a handler.  The unwind runtime will not have modified the stack.  The
C++ runtime will normally calluncaught_exception()in this case.
_URC_FATAL_PHASE1_ERRORThe unwinder encountered an unexpected error during phase
1, e.g. stack corruption. The unwind runtime will not have modified the stack. The
C++ runtime will normally callterminate()in this case.
If  the  unwinder  encounters  an  unexpected  error  during  phase  2,  it  should  return
_URC_FATAL_PHASE2_ERRORto its caller.  In C++, this will usually be__cxa_throw, which
will callterminate().
The unwind runtime will likely have modified the stack (e.g.  popped frames from it)
or register context, or landing pad code may have corrupted them.  As a result, the the
caller of_Unwind_RaiseExceptioncan make no assumptions about the state of its stack or
registers.
_Unwind_ForcedUnwind
typedef _Unwind_Reason_Code (
## *
_Unwind_Stop_Fn)
(int version,
_Unwind_Action actions,
uint64 exceptionClass,
struct _Unwind_Exception
## *
exceptionObject,
struct _Unwind_Context
## *
context,
void
## *
stop_parameter );
_Unwind_Reason_Code_Unwind_ForcedUnwind
( struct _Unwind_Exception
## *
exception_object,
_Unwind_Stop_Fn stop,
void
## *
stop_parameter );
Raise an exception for forced unwinding,  passing along the given exception object,
which should have itsexception_classandexception_cleanupfields set.  The exception
object has been allocated by the language-specific runtime, and has a language-specific
format,  except that it must contain an_Unwind_Exceptionstruct (see Exception Header
above).
Forced unwinding is a single-phase process (phase 2 of the normal exception-handling
process). Thestopandstop_parameterparameters control the termination of the unwind
process,  instead of the usual personality routine query.  Thestopfunction parameter is
called  for  each  unwind  frame,  with  the  parameters  described  for  the  usual  personality
routine below, plus an additionalstop_parameter.
## 95
AMD64 ABI 1.0 – May 20, 2021 – 11:08

When  thestopfunction  identifies  the  destination  frame,  it  transfers  control  (ac-
cording  to  its  own,  unspecified,  conventions)  to  the  user  code  as  appropriate  without
returning,  normally  after  calling_Unwind_DeleteException.   If  not,  it  should  return  an
_Unwind_Reason_Codevalue as follows:
_URC_NO_REASONThis  is  not  the  destination  frame.   The  unwind  runtime  will  call  the
frame’s personality routine with the_UA_FORCE_UNWINDand_UA_CLEANUP_PHASEflags
set in actions, and then unwind to the next frame and call the stop function again.
_URC_END_OF_STACKIn order to allow_Unwind_ForcedUnwindto perform special process-
ing when it reaches the end of the stack, the unwind runtime will call it after the
last frame is rejected, with aNULLstack pointer in the context, and the stop function
must catch this condition (i.e. by noticing theNULLstack pointer). It may return this
reason code if it cannot handle end-of-stack.
_URC_FATAL_PHASE2_ERRORThe stop function may return this code for other fatal condi-
tions, e.g. stack corruption.
If the stop function returns any reason code other than_URC_NO_REASON, the stack state
is indeterminate from the point of view of the caller of_Unwind_ForcedUnwind. Rather than
attempt to return, therefore, the unwind library should return_URC_FATAL_PHASE2_ERRORto
its caller.
## Example:longjmp_unwind()
The expected implementation oflongjmp_unwind()is as follows. Thesetjmp()routine
will have saved the state to be restored in its customary place, including the frame pointer.
Thelongjmp_unwind()routine will call_Unwind_ForcedUnwindwith a stop function that
compares the frame pointer in the context record with the saved frame pointer. If equal, it
will restore thesetjmp()state as customary, and otherwise it will return_URC_NO_REASON
or_URC_END_OF_STACK.
If a future requirement for two-phase forced unwinding were identified, an alternate
routine could be defined to request it, and an actions parameter flag defined to support it.
_Unwind_Resume
void _Unwind_Resume
(struct _Unwind_Exception
## *
exception_object);
Resume propagation of an existing exception e.g.  after executing cleanup code in a
partially unwound stack.  A call to this routine is inserted at the end of a landing pad that
## 96
AMD64 ABI 1.0 – May 20, 2021 – 11:08

performed cleanup, but did not resume normal execution. It causes unwinding to proceed
further.
_Unwind_Resumeshould  not  be  used  to  implement  re-throwing.   To  the  unwinding
runtime,  the  catch  code  that  re-throws  was  a  handler,  and  the  previous  unwinding
session  was  terminated  before  entering  it.Re-throwing  is  implemented  by  calling
_Unwind_RaiseExceptionagain with the same exception object.
This is the only routine in the unwind library which is expected to be called directly
by generated code: it will be called at the end of a landing pad in a "landing-pad" model.
## 6.2.4    Exception Object Management
_Unwind_DeleteException
void _Unwind_DeleteException
(struct _Unwind_Exception
## *
exception_object);
Deletes the given exception object. If a given runtime resumes normal execution after
catching a foreign exception, it will not know how to delete that exception. Such an excep-
tion will be deleted by calling_Unwind_DeleteException.  This is a convenience function
that calls the function pointed to by theexception_cleanupfield of the exception header.
## 6.2.5    Context Management
These functions are used for communicating information about the unwind context (i.e.
the unwind descriptors and the user register state) between the unwind library and the
personality routine and landing pad. They include routines to read or set the context record
images of registers in the stack frame corresponding to a given unwind context, and to
identify the location of the current unwind descriptors and unwind frame.
_Unwind_GetGR
uint64 _Unwind_GetGR
(struct _Unwind_Context
## *
context, int index);
This function returns the 64-bit value of the given general register.   The register is
identified by its index as given in figure 3.36.
During the two phases of unwinding, no registers have a guaranteed value.
_Unwind_SetGR
void _Unwind_SetGR
(struct _Unwind_Context
## *
context,
int index,
uint64 new_value);
## 97
AMD64 ABI 1.0 – May 20, 2021 – 11:08

This function sets the 64-bit value of the given register, identified by its index as for
_Unwind_GetGR.
The behavior is guaranteed only if the function is called during phase 2 of unwinding,
and applied to an unwind context representing a handler frame, for which the personality
routine will return_URC_INSTALL_CONTEXT. In that case,  only registers%rdi,%rsi,%rdx,
%rcxshould be used.  These scratch registers are reserved for passing arguments between
the personality routine and the landing pads.
_Unwind_GetIP
uint64 _Unwind_GetIP
(struct _Unwind_Context
## *
context);
This function returns the 64-bit value of the instruction pointer (IP).
During unwinding, the value is guaranteed to be the address of the instruction imme-
diately following the call site in the function identified by the unwind context. This value
may be outside of the procedure fragment for a function call that is known to not return
(such as_Unwind_Resume).
Applications which unwind through asynchronous signals and other non-call locations
should use_Unwind_GetIPInfobelow, and the additional flag that function provides.
_Unwind_GetIPInfo
uint64 _Unwind_GetIPInfo
(struct _Unwind_Context
## *
context, int
## *
ip_before_insn);
This  function  returns  the  same  value  as_Unwind_GetIP.  In  addition,  the  argument
ip_before_insnmust not be not null, and
## *
ip_before_insnis updated with a flag which
indicates whether the returned pointer is at or after the first not yet fully executed instruc-
tion.
## If
## *
ip_before_insnis false, the application calling_Unwind_GetIPInfoshould assume
that the instruction pointer provided points after a call instruction which has not yet re-
turned. In general, this means that the application should use the preceding call instruction
as the instruction pointer location of the unwind context.  Typically, this can be approxi-
mated by subtracting one from the returned instruction pointer.
## If
## *
ip_before_insnis true, then the instruction pointer does not refer to an active call
site.  Usually, this means that the instruction pointer refers to the point at which an asyn-
chronous signal arrived.  In this case,  the application should use the instruction pointer
returned from_Unwind_GetIPInfoas the instruction pointer location of the unwind con-
text, without adjustment.
## 98
AMD64 ABI 1.0 – May 20, 2021 – 11:08

_Unwind_SetIP
void _Unwind_SetIP
(struct _Unwind_Context
## *
context,
uint64 new_value);
This function sets the value of the instruction pointer (IP) for the routine identified by
the unwind context.
The  behavior  is  guaranteed  only  when  this  function  is  called  for  an  unwind
context  representing  a  handler  frame,  for  which  the  personality  routine  will  return
_URC_INSTALL_CONTEXT. In this case, control will be transferred to the given address, which
should be the address of a landing pad.
_Unwind_GetLanguageSpecificData
uint64 _Unwind_GetLanguageSpecificData
(struct _Unwind_Context
## *
context);
This routine returns the address of the language-specific data area for the current stack
frame.
This routine is not strictly required: it could be accessed through_Unwind_GetIPusing
the documented format of the DWARF Call Frame Information Tables, but since this work
has been done for finding the personality routine in the first place, it makes sense to cache
the result in the context. We could also pass it as an argument to the personality routine.
_Unwind_GetRegionStart
uint64 _Unwind_GetRegionStart
(struct _Unwind_Context
## *
context);
This routine returns the address of the beginning of the procedure or code fragment
described by the current unwind descriptor block.
This information is required to access any data stored relative to the beginning of the
procedure fragment. For instance, a call site table might be stored relative to the beginning
of the procedure fragment that contains the calls. During unwinding, the function returns
the start of the procedure fragment containing the call site in the current stack frame.
_Unwind_GetCFA
uint64 _Unwind_GetCFA
(struct _Unwind_Context
## *
context);
This function returns the 64-bit Canonical Frame Address which is defined as the value
of%rspat the call site in the previous frame.  This value is guaranteed to be correct any
time the context has been passed to a personality routine or a stop function.
## 99
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 6.2.6    Personality Routine
_Unwind_Reason_Code (
## *
## __personality_routine)
(int version,
_Unwind_Action actions,
uint64 exceptionClass,
struct _Unwind_Exception
## *
exceptionObject,
struct _Unwind_Context
## *
context);
The personality routine is the function in the C++ (or other language) runtime library
which  serves  as  an  interface  between  the  system  unwind  library  and  language-specific
exception handling semantics.  It is specific to the code fragment described by an unwind
info block, and it is always referenced via the pointer in the unwind info block, and hence
it has no psABI-specified name.
## Parameters
The personality routine parameters are as follows:
versionVersion number of the unwinding runtime, used to detect a mis-match between
the unwinder conventions and the personality routine, or to provide backward com-
patibility. For the conventions described in this document, version will be 1.
actionsIndicates what processing the personality routine is expected to perform, as a bit
mask. The possible actions are described below.
exceptionClassAn  8-byte  identifier  specifying  the  type  of  the  thrown  exception.   By
convention, the high 4 bytes indicate the vendor (for instance AMD\0), and the low
4 bytes indicate the language. For the C++ ABI described in this document, the low
four bytes are C++\0.  This is not a null-terminated string.  Some implementations
may use no null bytes.
exceptionObjectThe pointer to a memory location recording the necessary information
for processing the exception according to the semantics of a given language (see the
Exception Header section above).
contextUnwinder state information for use by the personality routine. This is an opaque
handle used by the personality routine in particular to access the frame’s registers
(see the Unwind Context section above).
return valueThe return value from the personality routine indicates how further unwind
should happen, as well as possible error conditions. See the following section.
## 100
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Personality Routine Actions
The actions argument to the personality routine is a bitwise OR of one or more of the
following constants:
typedef int _Unwind_Action;
const _Unwind_Action _UA_SEARCH_PHASE = 1;
const _Unwind_Action _UA_CLEANUP_PHASE = 2;
const _Unwind_Action _UA_HANDLER_FRAME = 4;
const _Unwind_Action _UA_FORCE_UNWIND = 8;
_UA_SEARCH_PHASEIndicates  that  the  personality  routine  should  check  if  the  current
frame  contains  a  handler,   and  if  so  return_URC_HANDLER_FOUND,  or  otherwise
return_URC_CONTINUE_UNWIND._UA_SEARCH_PHASEcannot be set at the same time as
## _UA_CLEANUP_PHASE.
_UA_CLEANUP_PHASEIndicates that the personality routine should perform cleanup for the
current frame.  The personality routine can perform this cleanup itself, by calling
nested procedures, and return_URC_CONTINUE_UNWIND. Alternatively, it can setup the
registers (including the IP) for transferring control to a "landing pad",  and return
## _URC_INSTALL_CONTEXT.
_UA_HANDLER_FRAMEDuring phase 2, indicates to the personality routine that the current
frame is the one which was flagged as the handler frame during phase 1.  The per-
sonality routine is not allowed to change its mind between phase 1 and phase 2, i.e.
it must handle the exception in this frame in phase 2.
_UA_FORCE_UNWINDDuring phase 2, indicates that no language is allowed to "catch" the
exception.  This flag is set while unwinding the stack forlongjmpor during thread
cancellation. User-defined code in a catch clause may still be executed, but the catch
clause must resume unwinding with a call to_Unwind_Resumewhen finished.
Transferring Control to a Landing Pad
If the personality routine determines that it should transfer control to a landing pad (in
phase 2), it may set up registers (including IP) with suitable values for entering the landing
pad (e.g. with landing pad parameters), by calling the context management routines above.
It then returns_URC_INSTALL_CONTEXT.
Prior to executing code in the landing pad, the unwind library restores registers not
altered by the personality routine,  using the context record,  to their state in that frame
## 101
AMD64 ABI 1.0 – May 20, 2021 – 11:08

before the call that threw the exception, as follows. All registers specified as callee-saved
by the base ABI are restored, as well as scratch registers%rdi,%rsi,%rdx,%rcx(see below).
Except for those exceptions, scratch (or caller-saved) registers are not preserved, and their
contents are undefined on transfer.
The  landing  pad  can  either  resume  normal  execution  (as,  for  instance,  at  the  end
of  a  C++  catch),  or  resume  unwinding  by  calling_Unwind_Resumeand  passing  it  the
exceptionObjectargument received by the personality routine._Unwind_Resumewill never
return.
_Unwind_Resumeshould be called if and only if the personality routine did not return
_Unwind_HANDLER_FOUNDduring phase 1.  As a result, the unwinder can allocate resources
(for instance memory) and keep track of them in the exception object reserved words.  It
should then free these resources before transferring control to the last (handler) landing
pad. It does not need to free the resources before entering non-handler landing-pads, since
_Unwind_Resumewill ultimately be called.
The landing pad may receive arguments from the runtime, typically passed in regis-
ters set using_Unwind_SetGRby the personality routine.  For a landing pad that can call
to_Unwind_Resume,  one argument must be theexceptionObjectpointer,  which must be
preserved to be passed to_Unwind_Resume.
The landing pad may receive other arguments, for instance a switch value indicating
the type of the exception. Four scratch registers are reserved for this use (%rdi,%rsi,%rdx,
## %rcx).
Rules for Correct Inter-Language Operation
The following rules must be observed for correct operation between languages and/or run
times from different vendors:
An exception which has an unknown class must not be altered by the personality rou-
tine.  The semantics of foreign exception processing depend on the language of the stack
frame being unwound.  This covers in particular how exceptions from a foreign language
are mapped to the native language in that frame.
If a runtime resumes normal execution, and the caught exception was created by an-
other runtime, it should call_Unwind_DeleteException. This is true even if it understands
the exception object format (such as would be the case between different C++ run times).
A runtime is not allowed to catch an exception if the_UA_FORCE_UNWINDflag was passed
to the personality routine.
Example: Foreign Exceptions in C++.In C++, foreign exceptions can be caught by a
catch(...)statement. They can also be caught as if they were of a__foreign_exception
## 102
AMD64 ABI 1.0 – May 20, 2021 – 11:08

class,  defined in<exception>.   The__foreign_exceptionmay have subclasses,  such as
__java_exceptionand__ada_exception, if the runtime is capable of identifying some of
the foreign languages.
The behavior is undefined in the following cases:
-  A__foreign_exceptioncatch argument is accessed in any way (including taking its
address).
-  A__foreign_exceptionis active at the same time as another exception (either there
is a nested exception while catching the foreign exception, or the foreign exception
was itself nested).
•uncaught_exception(),set_terminate(),set_unexpected(),terminate(),    or
unexpected()is  called  at  a  time  a  foreign  exception  exists  (for  example,  calling
set_terminate()during unwinding of a foreign exception).
All these cases might involve accessing C++ specific content of the thrown exception,
for instance to chain active exceptions.
Otherwise, a catch block catching a foreign exception is allowed:
-  to resume normal execution, thereby stopping propagation of the foreign exception
and deleting it, or
-  to re-throw the foreign exception. In that case, the original exception object must be
unaltered by the C++ runtime.
A catch-all block may be executed during forced unwinding.  For instance, a longjmp
may execute code in acatch(...)during stack unwinding.   However,  if this happens,
unwinding will proceed at the end of the catch-all block, whether or not there is an explicit
re-throw.
Setting the low 4 bytes of exception class to C++\0 is reserved for use by C++ run-
times compatible with the common C++ ABI.
## 6.3    Unwinding Through Assembler Code
For successful unwinding on AMD64 every function must provide a valid debug informa-
tion in the DWARF Debugging Information Format. In high level languages (e.g. C/C++,
Fortran, Ada, ...)  this information is generated by the compiler itself.  However for hand-
written assembly routines the debug info must be provided by the author of the code.  To
ease this task some new assembler directives are added:
## 103
AMD64 ABI 1.0 – May 20, 2021 – 11:08

.cfi_startprocis used at the beginning of each function that should have an entry in
.eh_frame.   It initializes some internal data structures and emits architecture de-
pendent initial CFI instructions. Each.cfi_startprocdirective has to be closed by
## .cfi_endproc.
.cfi_endprocis used at the end of a function where it closes its unwind entry previously
opened by.cfi_startprocand emits it to.eh_frame.
.cfi_def_cfa REGISTER, OFFSETdefines a rule for computing CFA as: take address from
REGISTER and add OFFSET to it.
.cfi_def_cfa_register REGISTERmodifies  a  rule  for  computing  CFA.  From  now  on
REGISTER will be used instead of the old one. The offset remains the same.
.cfi_def_cfa_offset OFFSETmodifies a rule for computing CFA. The register remains
the same, but OFFSET is new. Note that this is the absolute offset that will be added
to a defined register to compute the CFA address.
.cfi_adjust_cfa_offset OFFSETis similar to.cfi_def_cfa_offsetbut OFFSET is a rel-
ative value that is added or subtracted from the previous offset.
.cfi_offset REGISTER, OFFSETsaves the previous value of REGISTER at offset OFF-
SET from CFA.
.cfi_rel_offset REGISTER, OFFSETsaves  the  previous  value  of  REGISTER  at  offset
OFFSET from the current CFA register.  This is transformed to.cfi_offsetusing
the known displacement of the CFA register from the CFA. This is often easier to
use, because the number will match the code it is annotating.
.cfi_escape EXPRESSION[, ...]allows  the  user  to  add  arbitrary  bytes  to  the  unwind
info.  One might use this to add OS-specific CFI opcodes, or generic CFI opcodes
that the assembler does not support.
## 104
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 6.1: Examples for Unwinding in Assembler
# - function with local variable allocated on the stack
.type   func_locvars,@function
func_locvars:
## .cfi_startproc
# allocate space for local vars
sub     $0x1234, %rsp
## .cfi_adjust_cfa_offset  0x1234
# body
## ...
# release space of local vars and return
add     $0x1234, %rsp
## .cfi_adjust_cfa_offset  -0x1234
ret
## .cfi_endproc
# - function that moves frame pointer to another register
#   and then allocates space for local variables
.type   func_otherreg,@function
func_otherreg:
## .cfi_startproc
# save frame pointer to r12
movq    %rsp, %r12
.cfi_def_cfa_register   r12
# allocate space for local vars
# (no .cfi_{def,adjust}_cfa_offset needed here,
# because CFA is computed from r12!)
sub     $100,%rsp
# body
## ...
# restore frame pointer from r12
movq    %r12, %rsp
.cfi_def_cfa_register   rsp
ret
## .cfi_endproc
## 105
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 7
## Development Environment
During compilation of C or C++ code at least the symbols in table 7.1 are defined by the
pre-processor
## 1
## .
Table 7.1: Predefined Pre-Processor Symbols
__amd64     Defined for both LP64 and ILP32 programming models.
__amd64__   Defined for both LP64 and ILP32 programming models.
__x86_64    Defined for both LP64 and ILP32 programming models.
__x86_64__  Defined for both LP64 and ILP32 programming models.
_LP64       Defined for LP64 programming model.
__LP64__    Defined for LP64 programming model.
_ILP32      Defined for ILP32 programming model.
__ILP32__   Defined for ILP32 programming model.
## 1
__LP64and__LP64__were added to GCC 3.3 in March, 2003.
## 106
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 8
## Execution Environment
Not done yet.
## 107
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 9
## Conventions
## 1
## 1
This chapter is used to document some features special to the AMD64 ABI. The different sections might
be moved to another place or removed completely.
## 108
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 9.1    C++
For the C++ ABI we will use the IA-64 C++ ABI and instantiate it appropriately.  The
current draft of that ABI is available at:
http://mentorembedded.github.io/cxx-abi/
## 109
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 9.2    Fortran
A formal Fortran ABI does not exist.  Most Fortran compilers are designed for very spe-
cific high performance computing applications, so Fortran compilers use different pass-
ing conventions and memory layouts optimized for their specific purpose.  For example,
Fortran applications that must run on distributed memory machines need a different data
representation for array descriptors (also known as dope vectors, or fat pointers) than ap-
plications running on symmetric multiprocessor shared memory machines.  A normative
ABI for Fortran is therefore not desirable.  However, for interoperability of different For-
tran compilers, as well as for interoperability with other languages, this section provides
some some guidelines for data types representation, and argument passing. The guidelines
in this section are derived from the GNU Fortran 77 (G77) compiler, and are also followed
by the GNU Fortran 95 (gfortran) compiler (restricted to Fortran 77 features). Other For-
tran compilers already available for AMD64 at the time of this writing may use different
conventions, so compatibility is not guaranteed.
When  this  text  uses  the  termFortran procedure,  the  text  applies  to  both  Fortran
FUNCTIONandSUBROUTINEsubprograms  as  well  as  for  alternateENTRYpoints,  unless
specifically stated otherwise.
Everything not explicitly defined in this ABI is left to the implementation.
## 9.2.1    Names
External names in Fortran are names of entities visible to all subprograms at link time.
This includes names ofCOMMONblocks and Fortran procedures.  To avoid name space con-
flicts with linked-in libraries, all external names have to be mangled.  And to avoid name
space conflicts of mangled external names with local names, all local names must also be
mangled. The mangling scheme is straightforward as follows:
-  all names that do not have any underscores in it should haveoneunderscore ap-
pended
-  all external names containing one or more underscores in it (wherever) should have
twounderscores appended
## 2
## .
-  all external names should be mapped to lower case, following the traditional UNIX
model for Fortran compilers
For examples see figure 9.1:
## 2
Historically, this is to be compatible with f2c.
## 110
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 9.1: Example mapping of names
Fortran external nameLinker name
FOOfoo_
foofoo_
## Foo
foo_
foo_foo___
f_oof_oo__
The entry point of the main program unit is calledMAIN__.  The symbol name for the
blank common block is__BLNK__.  the external name of the unnamedBLOCK DATAroutine
is__BLOCK_DATA__.
9.2.2    Representation of Fortran Types
For historical reasons, GNU Fortran 77 maps Fortran programs to the C ABI, so the data
representation can be explained best by providing the mapping of Fortran types to C types
used by G77 on AMD64
## 3
as in figure  9.2.  The “TYPE
## *
N” notation specifies that variables
or aggregate members of typeTYPEshall occupyNbytes of storage.
Figure 9.2: Mapping of Fortran to C types
FortranData kindEquivalent C type
## INTEGER
## *
4Default integersigned int
## INTEGER
## *
8Double precision integersigned long
## REAL
## *
## 4
Single precision FP numberfloat
## REAL
## *
8Double precision FP numberdouble
## COMPLEX
## *
4Single precision complex FP numbercomplex float
## COMPLEX
## *
8Double precision complex FP numbercomplex double
LOGICALBoolean logical typesigned int
CHARACTERText stringchar[] + length
The values for typeLOGICALare.TRUE.implemented as 1 and.FALSE.implemented as
## 0.
## 3
G77 provides a headerg2c.hwith the equivalent C type definitions for all supported Fortran scalar
types.
## 111
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Data objects with aCHARACTERtype
## 4
are represented as an array of characters of the
C char type (not guaranteed to be “\0” terminated) with a separate length counter to dis-
tinguish betweenCHARACTERdata objects with a length parameter, and aggregate types of
CHARACTERdata objects, possibly also with a length parameter.
Layout  of  other  aggregate  types  is  implementation  defined.   GNU  Fortran  puts  all
arrays in contiguous memory in column-major order. GNU Fortran 95 builds an equivalent
C struct for derived types without reordering the type fields. Other compilers may use other
representations as needed.  The representation and use of Fortran 90/95 array descriptors
is implementation defined. Note that array indices start at 1 by default.
Fortran 90/95 allow different kinds of each basic type using thekindtype parameter of
a type. Kind type parameter values are implementation defined.
Layout of he commonly used Cray pointers is implementation defined.
## 9.2.3    Argument Passing
For each given Fortran 77 procedure, an equivalent C prototype can be derived. Once this
equivalent C prototype is known, the C ABI conventions should be applied to determine
how arguments are passed to the Fortran procedure.
G77 passes all (user defined) formal arguments of a procedure by reference.  Specifi-
cally, pointers to the location in memory of a variable, array, array element, a temporary
location that holds the result of evaluating an expression or a temporary or permanent lo-
cation that holds the value of a constant (xf. g77 manual) are passed as actual arguments.
Artificial compiler generated arguments may be passed by value or by reference as they
are inherently compiler and hence implementation specific.
Data objects with aCHARACTERtype are passed as a pointer to the character string and
its length, so that eachCHARACTERformal argument in a Fortran procedure results in two
actual arguments in the equivalent C prototype.  The first argument occupies the position
in the formal argument list of the Fortran procedure.  This argument is a pointer to the
array of characters that make up the string, passed by the caller.  The second argument
is appended to the end of the user-specified formal argument list.   This argument is of
the default integer type and its value is the length of the array of characters, that is the
length, passed as the first argument. This length is passed by value. When more than one
CHARACTERargument is present in an argument list, the length arguments are appended in
the order the original arguments appear. The above discussion also applies to sub-strings.
This ABI does not define the passing of optional arguments. They are allowed only in
Fortran 90/95 and their passing is implementation defined.
## 4
This includes sub-strings.
## 112
AMD64 ABI 1.0 – May 20, 2021 – 11:08

This ABI does not define array functions (function returning arrays). They are allowed
only in Fortran 90/95 and requires the definition of array descriptors.
Note that Fortran 90/95 procedure arguments with theINTENT(IN)attribute should also
passed by reference if the procedure is to be linked with code written in Fortran 77. Fortran
77 does not and can not support theINTENTattribute because it has no concept of explicit
interfaces.  It is therefore not possible to declare the callee’s arguments asINTENT(IN). A
Fortran 77 compiler must assume that all procedure arguments areINTENT(INOUT)in the
Fortran 90/95 sense.
## 9.2.4    Functions
The calling of statement functions is implementation defined (as they are defined only
locally, the compiler has the freedom to apply any calling convention it likes).
Subroutines  with  alternate  returns  (e.g.   "SUBROUTINE  X(*,*)"  called  as  "CALL
X(*10,*20)") are implemented as functions returning anINTEGERof the default kind. The
value of this returned integer is whatever integer is specified in the "RETURN" statement
for the subroutine
## 5
, or 0 for aRETURNstatement without an argument. It is up to the caller
to jump to the corresponding alternate return label. The actual alternate-return arguments
are omitted from the calling sequence.
An example:
## SUBROUTINE SHOW_ALTERNATE_RETURN (N)
## INTEGER N
## CALL ALTERNATE_RETURN_EXAMPLE (N,
## *
## 10,
## *
## 20,
## *
## 30)
## WRITE (
## *
## ,
## *
) ’OK - Normal Return’
## RETURN
## 10        WRITE (
## *
## ,
## *
) ’1st alternate return’
## RETURN
## 20        WRITE (
## *
## ,
## *
) ’2nd alternate return’
## RETURN
## 30        WRITE (
## *
## ,
## *
) ’2nd alternate return’
## RETURN
## END
## SUBROUTINE ALTERNATE_RETURN_EXAMPLE (N,
## *
## ,
## *
## ,
## *
## )
## INTEGER N
IF (N .EQ. 0 ) RETURN     ! Implicit "RETURN 0"
## IF ( N .EQ. 1 ) RETURN 1
## IF ( N .EQ. 2 ) RETURN 2
## RETURN 3
## END
## 5
This integer indicates the position of an alternate return from the subroutine in the formal argument list
## 113
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Here theSUBROUTINE ALTERNATE_RETURN_EXAMPLEis implemented as a function return-
ing anINTEGER
## *
4with value 0 if N is 0, 1 if N is 1, 2 if N is 2 and 3 for all other values of N.
This return value is used by the caller as if the actual call were replaced by this sequence:
## INTEGER X
## X = CALL ALTERNATE_RETURN_EXAMPLE (N)
## GOTO (10, 20, 30), X
All in all the effect is that the index of the returned to label (starting from 1) will be
contained in%raxafter the call.
AlternateENTRYpoints of aSUBROUTINEorFUNCTIONshould be treated as separate sub-
programs,  as mandated by the Fortran standard.   I.e.   arguments passed to an alternate
ENTRYshould be passed as if the alternateENTRYis a separateSUBROUTINEorFUNCTION. If a
FUNCTIONhas alternateENTRYpoints, the result of each of the alternateENTRYpoints must be
returned as if the alternateENTRYis a separateFUNCTIONwith the result type of the alternate
ENTRY. The external naming of alternateENTRYpoints follows section 9.2.1.
9.2.5    COMMON blocks
In absence of anyEQUIVALENCEdeclaration involving variables inCOMMONblocks the layout
of aCOMMONblock is exactly the same as the layout of the equivalent C structure (with types
of variables substituted according to section  9.2.2), including the alignment requirements.
This ABI defines the layout under presence ofEQUIVALENCEstatements only in some
cases:
-  the layout of theCOMMONblock must not change if one ignores theEQUIVALENCE, which
amongst other things means:
-  If two arrays are equivalenced, the larger array must be named in theCOMMONblock,
and there must be complete inclusion, in particular the other array may not extend
the size of the equivalenced segment. It may also not change the alignment require-
ment.
-  If an array element and a scalar are equivalenced, the array must be named in the
COMMONblock and it must not be smaller than the scalar. The type of the scalar must
not require bigger alignment than the array.
-  if two scalars are equivalenced they must have the same size and alignment require-
ments.
## 114
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Other cases are implementation defined.
Because the Fortran standard allows the blankCOMMONblock to have different sizes in
different subprograms, it may be impossible to determine if it is small enough to fit in the
.bsssection.  When compiling for the medium or large code models the blankCOMMON
block should therefore always be put in the.lbsssection.
## 9.2.6    Intrinsics
This sections lists the set of intrinsics which has to be supported at minimum by a con-
forming compiler.  They are separated by origin.  They follow regular calling and naming
conventions.
The  signature  of  intrinsics  uses  the  syntaxreturn−type(argtype1, argtype2, ...),
where  the  individual  types  can  be  the  following  characters:V(as  in  void)  designates
aSUBROUTINE,LaLOGICAL,IanINTEGER,RaREAL,  andCaCHARACTER.  HenceI(R,L)
designates aFUNCTIONreturning anINTEGERand taking aREALand aLOGICAL. If an argument
is an array, this is indicated using a trailing number, e.g.I13is anINTEGERarray with 13
elements.   If aCHARACTERargument or return value has a fixed length,  this is indicated
using an asterisk and a trailing number, for exampleC*16is aCHARACTER(len=16).  If a
CHARACTERargument of arbitrary length must be passed,  the trailing number is replaced
withN, for exampleC*N.
Table 9.1: Mil intrinsics
NameSignatureMeaning
BTestL(I,I)Test bit
IAndI(I,I)Boolean AND
IOr
I(I,I)Boolean OR
IEOr
I(I,I)Boolean XOR
NotI(I)Boolean NOT
IBClr
I(I,I)Clear a bit
IBits
I(I,I,I)Extract a bit subfield of a variable
IBSetI(I,I)Set a bit
IShftI(I,I)Logical bit shift
IShftCI(I,I,I)Circular bit shift
MvBitsV(I,I,I,I,I)Move a bit field
## 115
AMD64 ABI 1.0 – May 20, 2021 – 11:08

BTest (I, Pos)Returns.TRUE.if bitPosinIis set, returns.FALSE.otherwise.
IAnd (I, J)Returns value resulting from a boolean AND on each pair of bits inIand
## J.
IOr (I, J)Returns value resulting from a boolean OR on each pair of bits inIandJ.
IEOr (I, J)Returns value resulting from a boolean XOR on each pair of bits inIand
## J.
Not (I)Returns value resulting from a boolean NOT on each bit inI.
IBClr (I, Pos)Returns the value ofIwith bitPoscleared (set to zero).
IBits (I, Pos, Len)Extracts  a  subfield  starting  from  bit  positionPosand  with  a
length  (towards  the  most  significant  bit)  ofLenbits  fromI.  The  result  is  right-
justified and the remaining bits are zeroed.
IBSet (I, Pos)Returns the value ofIwith the bit in positionPosset to one.
IShft (I, Shift)All bits ofIare shiftedShiftplaces.Shift.GT.0indicates a left shift,
Shift.EQ.0indicates no shift, andShift.LT.0indicates a right shift. Bits shifted out
from the least (when shifting right) or most (when shifting left) significant position
are lost. Bits shifted in at the opposite end are not set (i.e. zero).
IShftC (I, Shift, Size)The rightmostSizebits of the argumentIare shifted circu-
larlyShiftplaces. The unshifted bits of the result are the same as the unshifted bits
of I.
MvBits (From, FromPos, Len, To, ToPos)MoveLenbits ofFromfrom bit po-
sitionsFromPosthroughFromPos+Len-1to bit positionsToPosthroughToPos+Len-1
ofTo.  The bit portions ofTothat are not affected by the movement of bits are un-
changed.
## 116
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 9.2: F77 intrinsics
NameMeaning
AbsAbsolute value
ACos
Arc cosine
AIntTruncate to whole number
ANIntRound to nearest whole number
ASinArc sine
ATan
## Arc Tangent
ATan2Arc Tangent
CharCharacter from code
## Cmplx
ConstructCOMPLEX(KIND=1)value
ConjgComplex conjugate
CosCosine
CosHHyperbolic cosine
DbleConvert to double precision
DiM
Difference magnitude (non-negative subtract)
DProdDouble-precision product
## Exp
## Exponential
ICharCode for character
IndexLocate aCHARACTERsubstring
## Int
Convert toINTEGERvalue truncated to whole number
LenLength of character entity
LGeLexically greater than or equal
LGtLexically greater than
LLe
Lexically less than or equal
LLtLexically less than
## Log
Natural logarithm
Log10Common logarithm
MaxMaximum value
MinMinimum value
ModRemainder
NIntConvert toINTEGERvalue rounded to nearest whole number
RealConvert value to typeREAL(KIND=1)
SinSine
SinHHyperbolic sine
SqRtSquare root
## Tan
## Tangent
TanH
Hyperbolic tangent
## 117
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Refer to the Fortran 77 language standard for signature and definition of the F77 in-
trinsics listed in table 9.2. These intrinsics can have a prefix as per the standard hence the
table is not exhaustive.
Table 9.3: F90 intrinsics
NameMeaning
ACharASCII character from code
Bit_SizeNumber of bits in arguments type
CPU_Time
Get current CPU time
IACharASCII code for character
Len_TrimGet last non-blank character in string
System_Clock
Get current system clock value
Refer to the Fortran 90 language standard for signature and definition of the F90 in-
trinsics listed in table 9.3.
Table 9.4: Math intrinsics
Name    SignatureMeaning
BesJ0R(R)Bessel function
BesJ1R(R)Bessel function
BesJN
R(I,R)Bessel function
BesY0R(R)Bessel function
BesY1
R(R)Bessel function
BesYN
R(I,R)Bessel function
ErF
R(R)Error function
ErFCR(R)Complementary error function
IRandI(I)Random number
## Rand
R(I)Random number
SRandV(I)Random seed
BesJ0 (X)Calculates the Bessel function of the first kind of order 0 of X. Returns aREAL
of the same kind asX.
## 118
AMD64 ABI 1.0 – May 20, 2021 – 11:08

BesJ1 (X)Calculates the Bessel function of the first kind of order 1 of X. Returns aREAL
of the same kind asX.
BesJN (N, X)Calculates the Bessel function of the first kind of order N of X. Returns a
REALof the same kind asX.
BesY0 (X)Calculates the Bessel function of the second kind of order 0 of X. Returns a
REALof the same kind asX.
BesY1 (X)Calculates the Bessel function of the second kind of order 1 of X. Returns a
REALof the same kind asX.
BesYN (N, X)Calculates the Bessel function of the second kind of order N of X. Returns
aREALof the same kind asX.
ErF (X)Calculates the error function of X. Returns aREALof the same kind asX.
ErFC (X)Calculates the complementary error function of X, i.e.1 - ERF(X). Returns a
REALof the same kind asX.
IRand (Flag)Flag is optional. Returns a uniform quasi-random number up to a system-
dependent limit. IfFlag .EQ. 0orFlagis not passed, the next number in sequence
is returned.  IfFlag .EQ. 1, the generator is restarted.  IfFlaghas any other value,
the generator is restarted with the value ofFlagas the new seed.
Rand (Flag)Flag is optional.  Returns a uniform quasi-random number between 0 and
- IfFlag .EQ. 0orFlagis not passed, the next number in sequence is returned. If
Flag .EQ. 1, the generator is restarted. IfFlaghas any other value, the generator is
restarted with the value ofFlagas the new seed.
SRand (Seed)Reinitializes the random number generator forIRandandRandwith the
seed inSeed.
## 119
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 9.5: Unix intrinsics
NameSignatureMeaning
AbortV()Abort the program
## Access
I(C,C)Check file accessibility
DTimeV(R2,R)Get elapsed time since last call
ETimeV(R2,R)Get elapsed time for process
FlushV(I)Flush buffered output
FNum
I(I)Get file descriptor from Fortran unit number
FStatV(I,I13,I)Get file information
GErrorV(C*N)Get error message for last error
GetArg
V(I,C*N)Obtain command-line argument
GetCWDV(C*N,I)Get current working directory
GetEnvV(C*N,C*N)Get environment variable
GetGIdI()Get process group ID
GetPIdI()Get process ID
GetUId
I()Get process user ID
GetLogV(C*N)Get login name
HostNm
V(C*N,I)Get host name
IArgCI()Obtain count of command-line arguments
IDateV(I3)Get local date info
IErrNo
I()Get error number for last error
ITimeV(I3)Get local time of day
LStatV(C*N,I13,I)Get file information
PErrorV(C*N)Print error message for last error
## Rename
V(C*N,C*N,I)Rename file
SleepV(I)Sleep for a specified time
## System
V(C*N,I)Invoke shell (system) command
Abort ()Prints a message and potentially causes a core dump.
Access (Name, Mode)Checks fileNamefor accessibility in the mode specified byMode.
Returns0if the file is accessible in that mode, otherwise an error code.Namemust
be aNULL-terminated string ofCHARACTER(i.e.  a C-style string).  Trailing blanks in
Nameare ignored.Modemust be a concatenation of any of the following characters:
rmeaning test for read permission,wmeaning test for write permission,xmeaning
test for execute/search permission, or a space meaning test for existence of the file.
## 120
AMD64 ABI 1.0 – May 20, 2021 – 11:08

DTime (TArray, Result)When called for the first time, returns the number of sec-
onds of runtime since the start of the program inResult, the user component of this
runtime inTArray(1), and the system time inTArray(2).  Subsequent invocations
values based on accumulations since the previous invocation.
ETime (TArray, Result)Returns the number of seconds of runtime since the start
of the program inResult, the user component of this runtime inTArray(1), and the
system time inTArray(2).  Subsequent invocations values based on accumulations
since the previous invocation.
Flush (Unit)Flushes  the  Fortran  I/O  unit  with  IDUnit.   The  unit  must  be  open  for
output. If the optionalUnitargument is omitted, all open units are flushed.
FNum (Unit)Returns the UNIX(tm) file descriptor number corresponding to the Fortran
I/O unitUnit. The unit must be open.
FStat (Unit, SArray, Status)Obtains  data  about  the  file  open  on  Fortran  I/O
unitUnitand places it in the arraySArray. The values in this array are as follows:
-  Device ID
-  Inode number
-  File mode
-  Number of links
-  Owner’s UID
-  Owner’s GID
-  ID of device containing directory entry for file
-  File size (bytes)
-  Last access time
-  Last modification time
-  Last file status change time
-  Preferred I/O block size (-1 if not available)
-  Number of blocks allocated (-1 if not available)
If an element is not available, or not relevant on the host system, it is returned as 0
except when indicated otherwise in the above list.  If the optionalStatusargument
is supplied, it contains 0 on success or a nonzero error code upon return.
## 121
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Gerror (Message)Returns the system error message corresponding to the last system
error (errno in C). The message is returned inMessage. IfMessageis longer than the
error message, it is padded with blanks after the message.  IfMessageis not long
enough to hold the error message, the error message is truncated to the length of
## Message.
GetArg (Pos, Value)Returns inValuethe command-line argument in positionPos.  If
there are fever thanPoscommand-line arguments,Valueis filled with blanks. IfPos
is 0, the name of the program is returned. IfValueis longer than the command-line
argument, it is padded with blanks after the argument. IfValueis not long enough to
hold the command-line argument, the argument is truncated to the length ofValue.
GetCWD (Name, Status)Returns inNamethe current working directory.  If the optional
Statusargument is supplied, it contains 0 on success or a nonzero error code upon
return.
GetEnv (Name, Value)Returns inValuethe environment variable identified withName.
IfNamehas not been set,Valueis filled with blanks. Anullcharacter marks the end
of the name inName. Trailing blanks inNameare ignored. IfValueis longer than the
environment variable, it is padded with blanks after the variable. IfValueis not long
enough to hold the environment variable, the variable is truncated to the length of
## Value.
GetGId ()Returns the group ID for the current process.
GetPId ()Returns the process ID for the current process.
GetUId ()Returns the user ID for the current process.
GetLog (Login)Returns the login name for the process inLogin, or a blank string if the
host system does not supportgetlogin(3). IfLoginis longer than the login name, it
is padded with blanks after the login name.  IfLoginis not long enough to hold the
login name, the login name is truncated to the length of ofLogin.
HotNm (Name, Status)Returns inNamesystem’s host name.   If the optionalStatus
argument is supplied, it contains 0 on success or a nonzero error code upon return.
IfNameis longer than the host name, it is padded with blanks after the host name.
IfNameis not long enough to hold the host name, the host name is truncated to the
length of ofName.
IArgC ()Returns the number of command-line arguments. The program name itself is not
included in this number.
## 122
AMD64 ABI 1.0 – May 20, 2021 – 11:08

IDate (TArray)Returns the current local date day, month, year in elements 1, 2, and 3
ofTarray, respectively. The year has four significant digits.
IErrno ()Returns the last system error number (errnoin C).
ITime (TArray)Returns the current local time hour, minutes, and seconds in elements
1, 2, and 3 ofTArray, respectively.
LStat (File, SArray, Status)Obtains data about a file namedFileand places
places it in the arraySArray. The values in this array are as follows:
-  Device ID
-  Inode number
-  File mode
-  Number of links
-  Owner’s UID
-  Owner’s GID
-  ID of device containing directory entry for file
-  File size (bytes)
-  Last access time
-  Last modification time
-  Last file status change time
-  Preferred I/O block size (-1 if not available)
-  Number of blocks allocated (-1 if not available)
If an element is not available, or not relevant on the host system, it is returned as 0
except when indicated otherwise in the above list.  If the optionalStatusargument
is supplied, it contains 0 on success or a nonzero error code upon return.
PError (MsgPrefix)Prints a newline-terminated error message corresponding to the last
system error. This is prefixed by the stringMsgPrefix, a colon and a space. The error
message is printed on the Cstderrstream.
Rename (Path1, Path2, Status)Renames the file namedPath1toPath2.  Anull
character marks the end of the names.  Trailing blanks are ignored.  If the optional
Statusargument is supplied, it contains 0 on success or a nonzero error code upon
return.
## 123
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Sleep (Seconds)Causes the program to pause forSecondsseconds.
System (Command, Status)Passes   the   string   inCommandto   a   shell   though
system(3).If   the   optional   argumentStatusis   present,   it   contains   the
value returned bysystem(3).
## 124
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 10
ILP32 Programming Model
"x32" is commonly used to refer to AMD64 ILP32 programming model.
## 10.1    Parameter Passing
When a value of pointer type is returned or passed in a register, bits 32 to 63 shall be zero.
## 10.2    Address Space
ILP32 binaries reside in the lower 32 bits of the 64-bit virtual address space and all ad-
dresses are 32 bits in size.  They should conform to small code model or small position
independent code model (PIC) described in Section 3.5.1.
10.3    Thread-Local Storage Support
ILP32 Thread-Local Storage (TLS) support is based on LP64 TLS implementation with
some modifications.
10.3.1    Global Thread-Local Variable
For a global thread-local variable x:
extern __thread int x;
## 125
AMD64 ABI 1.0 – May 20, 2021 – 11:08

General Dynamic ModelLoad address ofxinto%rax
## Table 10.1: General Dynamic Model Code Sequence
## LP64                             ILP32
0x00  .byte  0x660x00  leaq   x@tlsgd(%rip),%rdi
0x01  leaq   x@tlsgd(%rip),%rdi
## 0x07  .word  0x6666
0x08  .word  0x66660x09  rex64
0x0a  rex640x0a  call   __tls_get_addr@plt
0x0b  call   __tls_get_addr@plt
In TLSDESC code sequence,lealinstruction must be encoded withrexprefix even
if it isn’t required by destination register. If thelealencoding has a variable length,
linker can’t tell where it starts and can’t safely perform GDesc -> IE/LE optimiza-
tion.
Table 10.2: General Dynamic Model Code Sequence with TLSDESC
## LP64                                ILP32
0x00  leaq  x@tlsdesc(%rip),%rax0x00  rex leal  x@tlsdesc(%rip),%eax
0x07  call
## *
x@tlsdesc((%rax)0x07  call
## *
x@tlsdesc(%eax)
0x08  add   %fs:0x0,%eax0x09  add       %fs:0x0,%eax
Initial Exec ModelLoad address ofxinto%rax.  Instructionaddlmust be encoded with
rexprefix even if it isn’t required by destination register.   Otherwise linker can’t
safely perform IE -> LE optimization.
## Table 10.3: Initial Exec Model Code Sequence
## LP64                                 ILP32
0x00  movq  %fs:0,%rax0x00  movl      %fs:0,%eax
0x09  addq  x@gottpoff(%rip),%rax0x08  rex addl  x@gottpoff(%rip),%eax
Initial Exec Model, IILoad value ofxinto%edi.%fs:(%eax)memory operand can’t be
used for ILP32 since its effective address is the base address of%fs+ value of%eax
zero-extended to a 64-bit result, which is incorrect with negative value in%eax.
## 126
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 10.4: Initial Exec Model Code Sequence, II
## LP64                               ILP32
0x00  movq  x@gottpoff(%rip),%rax0x00  movq  x@gottpoff(%rip),%rax
0x07  movl  %fs:(%rax),%edi0x07  movl  %fs:(%rax),%edi
10.3.2    Static Thread-Local Variable
For a static thread-local variable x:
static __thread int x;
Local Dynamic ModelLoad address ofxinto%rax
## Table 10.5: Local Dynamic Model Code Sequence With Lea
## LP64                             ILP32
0x00  leaq  x@tlsld(%rip),%rdi0x00  leaq  x@tlsld(%rip),%rdi
0x07  call  __tls_get_addr@plt
0x07  call  __tls_get_addr@plt
0x0c  leaq  x@dtpoff(%rax),%rax0x0c  leal  x@dtpoff(%rax),%eax
or
## Table 10.6: Local Dynamic Model Code Sequence With Add
## LP64                            ILP32
0x00  leaq  x@tlsld(%rip),%rdi0x00  leaq  x@tlsld(%rip),%rdi
0x07  call  __tls_get_addr@plt0x07  call  __tls_get_addr@plt
0x0c  addq  $x@dtpoff,%rax0x0c  addl  $x@dtpoff,%eax
For code sequence with TLSDESC, local dynamic model is similar to general dy-
namic model. The same encoding requirement forlealinstruction also applies.
## 127
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 10.7: General Dynamic Model Code Sequence with TLSDESC
## LP64                                ILP32
0x00  leaq  x@tlsdesc(%rip),%rax0x00  rex leal  x@tlsdesc(%rip),%eax
0x07  call
## *
x@tlsdesc((%rax)0x07  call
## *
x@tlsdesc(%eax)
0x08  add   %fs:0x0,%eax0x09  add       %fs:0x0,%eax
Local Dynamic Model, IILoad value ofxinto%edi
Table 10.8: Local Dynamic Model Code Sequence, II
## LP64                             ILP32
0x00  movq  %fs:0,%rax0x00  movl  %fs:0,%eax
0x09  movl  x@dtpoff(%rax),%edi0x08  movl  x@dtpoff(%rax),%edi
Local Exec ModelLoad address ofxinto%rax
## Table 10.9: Local Exec Model Code Sequence With Lea
## LP64                            ILP32
0x00  movq  %fs:0,%rax0x00  movl  %fs:0,%eax
0x09  leaq  x@tpoff(%rax),%rax
0x08  leal  x@tpoff(%rax),%eax
or
## Table 10.10: Local Exec Model Code Sequence With Add
## LP64                       ILP32
0x00  movq  %fs:0,%rax0x00  movl  %fs:0,%eax
0x09  addq  $x@tpoff,%rax0x08  addl  $x@tpoff,%eax
Local Exec Model, IILoad value ofxinto%edi
## 128
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 10.11: Local Exec Model Code Sequence, II
## LP64                            ILP32
0x00  movq  %fs:0,%rax0x00  movl  %fs:0,%eax
0x09  movl  x@tpoff(%rax),%edi0x08  movl  x@tpoff(%rax),%edi
Local Exec Model, IIILoad value ofxinto%edi
Table 10.12: Local Exec Model Code Sequence, III
## LP64                          ILP32
0x00  movl  %fs:x@tpoff,%edi0x00  movl  %fs:x@tpoff,%edi
10.3.3    TLS Linker Optimization
General Dynamic To Initial ExecLoad address ofxinto%rax
Table 10.13: GD -> IE Code Transition
## GD                                 IE
0x00  leaq   x@tlsgd(%rip),%rdi0x00  movl  %fs:0, %eax
## 0x07  .word  0x6666
0x08  addq  x@gottpoff(%rip),%rax
0x09  rex64
0x0a  call   __tls_get_addr@plt
Table 10.14: GDesc -> IE Code Transition
GDesc                                    LE
0x00  rex leal  x@tlsdes(%rip),%eax0x00  rex movl  x@gottpoff(%rip), %eax
0x07  call
## *
foo@TLSCALL(%eax)
0x07  nopl      (%rax)
## 129
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 10.15: GD -> LE Code Transition
## GD                               LE
0x00  leaq   x@tlsgd(%rip),%rdi0x00  movl  %fs:0, %eax
0x07  .word  0x66660x08  leal  x@tpoff(%rax),%eax
0x09  rex64
0x0a  call   __tls_get_addr@plt
Table 10.16: GDesc -> LE Code Transition
GDesc                                LE
0x00  rex leal  x@tlsdes(%rip),%eax0x00  rex movl  $x@tpoff, %eax
0x07  call
## *
foo@TLSCALL(%eax)0x07  nopl      (%rax)
Initial Exec To Local ExecLoad address ofxinto%rax
Table 10.17: IE -> LE Code Transition With Lea
## IE                                    LE
0x00  movl      %fs:0,%eax0x00  movl      %fs:0,%eax
0x08  rex addl  x@gottpoff(%rip),%eax0x08  rex leal  x@tpoff(%rax),%eax
or
Table 10.18: IE -> LE Code Transition With Add
## IE                               LE
0x00  movl  %fs:0,%r12d0x00  movl  %fs:0,%r12d
0x09  addl  x@gottpoff(%rip),%r12d0x09  addl  $x@tpoff,%r12d
Initial Exec To Local Exec, IILoad value ofxinto%edi.
## 130
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 10.19: IE -> LE Code Transition, II
## IE                               LE
0x00  movq  x@gottpoff(%rip),%rax0x00  movq  x@tpoff,%rax
0x07  movl  %fs:(%rax),%edi
0x07  movl  %fs:(%rax),%edi
Local Dynamic to Local ExecLoad address ofxinto%rax
Table 10.20: LD -> LE Code Transition With Lea
## LD                               LE
0x00  leaq  x@tlsld(%rip),%rdi0x00  nopl  0x0(%rax)
0x07  call  __tls_get_addr@plt0x04  movl  %fs:0,%eax
0x0c  leal  x@dtpoff(%rax),%eax0x0c  leal  x@tpoff(%rax),%eax
or
Table 10.21: LD -> LE Code Transition With Add
## LD                            LE
0x00  leaq  x@tlsld(%rip),%rdi0x00  nopl  0x0(%rax)
0x07  call  __tls_get_addr@plt
0x04  movl  %fs:0,%eax
0x0c  addq  $x@dtpoff,%rax0x0c  addl  $x@tpoff,%eax
Local Dynamic To Local Exec, IILoad value ofxinto%edi.
Table 10.22: LD -> LE Code Transition, II
## LD                               LE
0x00  leaq  x@tlsld(%rip),%rdi0x00  nopl  0x0(%rax)
0x07  call  __tls_get_addr@plt0x04  movl  %fs:0,%eax
0x0c  movl  x@dtpoff(%rax),%eax
0x0c  movl  x@tpoff(%rax),%eax
## 131
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 10.4    Kernel Support
Kernel should limit stack and addresses returned from system calls bewteen0x00000000
to0xf f f f f f f f.
## 10.5    Coding Examples
Although ILP32 binaries run in the 64-bit mode, not all 64-bit instructions are supported.
This section discusses example code sequences for fundamental operations which are dif-
ferent from the 64-bit mode.
## 10.5.1    Indirect Branch
Since indirect branch via memory loads a 64-bit address at the memory location, it is not
supported in ILP32. Indirect branch via register should be used instead. The 32-bit address
from memory is loaded into the lower 32 bits of a register, which will automatically zero-
extend the upper 32 bits of the register.  Then the indirect call can be performed via the
64-bit register.
## Table 10.23: Indirect Branch
## LP64                   ILP32
call
## *
## %raxcall
## *
## %rax
call
## *
func_p(%rip)movl  func_p(%rip), %eax
call
## *
## %rax
call
## *
func_pmovl  func_p, %eax
call
## *
## %rax
jmp
## *
## %raxjmp
## *
## %rax
jmp
## *
func_p(%rip)movl  func_p(%rip), %eax
jmp
## *
## %rax
jmp
## *
func_pmovl  func_p, %eax
jmp
## *
## %rax
## 132
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 11
## Alternate Code Sequences For Security
11.1    Code Sequences without PLT
Procedure Linkage Table (PLT), see Section 5.2 for detail, is used to access external func-
tions defined in shared object and support
Lazy symbol resolutionThe function address is resolved only when it is called the first
time at run-time.
Canonical function addressThe PLT entry of the external function is used as its address,
aka function pointer.
The first instruction in the PLT entry is an indirect branch via the Global Offset Table
(GOT), see Section 5.2 for detail, entry of the external function, which is set up in such
a way that it will be updated to the address of the function body the first time when the
function is called.  Since the GOT entry is writable, any address may be written to it at
run-time, which is a potential security risk.
11.1.1    Indirect Call via the GOT Slot
For small and medium models, different code sequence is used to avoid PLT:
Figure 11.1: Function Call without PLT (Small and Medium Models)
extern void func (void);.globl func
func (void);call
## *
func@GOTPCREL(%rip)
## 133
AMD64 ABI 1.0 – May 20, 2021 – 11:08

The direct branch is replaced by an indirect branch via the GOT slot, which is similar
to the first instruction in the PLT slot.
Figure 11.2: Function Address without PLT (Small and Medium Models)
extern void func (void);.globl func
void
## *
ptr (void)func:
{movq func@GOTPCREL(%rip), %rax
return func;ret
## }
Instead using the PLT slot as function address, the function address is retrieved from
the GOT slot.
If linker determines the function is defined locally, it converts indirect branch via the
GOT slot to direct branch with anopprefix and converts load via the GOT slot to load
immediate orlea, see Section B.2 for details.
After dynamic linker resolved all symbols by updating GOT entries with symbol ad-
dresses, GOT can be made read-only and overwriting GOT becomes a hard error imme-
diately.  Since PLT is no longer used to call external function, lazy symbol resolution is
disabled and a function can only be interposed during symbol resolution at startup. Tools
and features which depend on lazy symbol resolution will not work properly.  However,
there are also a few side benefits:
No extra direct branch to PLT entrySince  indirect  branch  is  6  byte  long  and  direct
branch is 5 byte long, when indirect branch via the GOT slot is used to call a local
function, code size will be increased by one byte for each call.  Since one PLT slot
has 16 bytes, there will be code size increase when indirect branch via the GOT slot
is used to call an external function more than 16 times.
Custom calling conventionSince external function is called directly via the GOT slot,
instead of invoking dynamic linker to lookup function symbol when called the first
time, parameters can be passed differently from what is specified in this document.
11.1.2    Thread-Local Storage without PLT
TLS  code  sequences  for  general  and  local  dynamic  models  can  be  updated  to  replace
direct call to__tls_get_addrvia the PLT entry, with indirect call to__tls_get_addrvia
the GOT slot, see Figure 11.3.  Since directcallinstruction is 4-byte long and indirect
callinstruction is 5-byte long, the extra one byte must be handled properly.
## 134
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 11.3:__tls_get_addrCall
Direct via PLT                 Indirect via GOT
call  __tls_get_addr@PLTcall
## *
__tls_get_addr@GOTPCREL(%rip)
General Dynamic Model for Global Variable
For general dynamic model, one0x66prefix beforecallinstruction is removed to make
room for indirect call:
extern __thread int x;
the following alternate code sequence loads address ofxinto%raxwithout PLT:
Table 11.1: General Dynamic Model Code Sequence (LP64)
With PLT                              Without PLT
## 0x00  .byte  0x660x00  .byte  0x66
0x01  leaq   x@tlsgd(%rip),%rdi0x01  leaq   x@tlsgd(%rip),%rdi
## 0x08  .word  0x66660x08  .byte  0x66
0x0a  rex640x09  rex64
0x0b  call   __tls_get_addr@PLT0x0a  call
## *
__tls_get_addr@GOTPCREL(%rip)
Table 11.2: General Dynamic Model Code Sequence (ILP32)
With PLT                              Without PLT
0x00  leaq   x@tlsgd(%rip),%rdi0x00  leaq   x@tlsgd(%rip),%rdi
## 0x07  .word  0x66660x07  .byte  0x66
0x09  rex64
0x08  rex64
0x0a  call   __tls_get_addr@PLT0x09  call
## *
__tls_get_addr@GOTPCREL(%rip)
Static Thread-Local Variable
For local dynamic model, indirect call is used instead of direct call:
## 135
AMD64 ABI 1.0 – May 20, 2021 – 11:08

static __thread int x;
the following alternate code sequence loads the address of the TLS block of the module,
which contains variablex, into%raxwithout PLT:
Table 11.3: Local Dynamic Model Code Sequence (LP64)
With PLT                             Without PLT
0x00  leaq  x@tlsld(%rip),%rdi0x00  leaq  x@tlsld(%rip),%rdi
0x07  call  __tls_get_addr@PLT0x07  call
## *
__tls_get_addr@GOTPCREL(%rip)
Table 11.4: Local Dynamic Model Code Sequence (ILP32)
With PLT                             Without PLT
0x00  leaq  x@tlsld(%rip),%rdi0x00  leaq  x@tlsld(%rip),%rdi
0x07  call  __tls_get_addr@PLT
0x07  call
## *
__tls_get_addr@GOTPCREL(%rip)
TLS Linker Optimization
Since the code sequence with indirect call for general dynamic model has the same length
as the one with direct call, linker just needs to recognize new instruction pattern to convert
general dynamic access to initial exec or local exec accesses.
General Dynamic to Initial ExecTo load address ofxinto%rax:
Table 11.5: GD -> IE Code Transition (LP64)
## GD                                       IE
0x00  .byte  0x660x00  movq  %fs:0, %rax
0x01  leaq   x@tlsgd(%rip),%rdi0x09  addq  x@gottpoff(%rip),%rax
## 0x09  .byte  0x66
0x0a  rex64
0x0b  call
## *
__tls_get_addr@GOTPCREL(%rip)
## 136
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 11.6: GD -> IE Code Transition (ILP32)
## GD                                       IE
0x00  leaq   x@tlsgd(%rip),%rdi0x00  movl  %fs:0, %eax
## 0x07  .byte  0x66
0x08  addq  x@gottpoff(%rip),%rax
0x08  rex64
0x0a  call
## *
__tls_get_addr@GOTPCREL(%rip)
General Dynamic to Local ExecTo load address ofxinto%rax:
Table 11.7: GD -> LE Code Transition (LP64)
## GD                                     LE
0x00  .byte  0x660x00  movq  %fs:0, %rax
0x01  leaq   x@tlsgd(%rip),%rdi0x09  leaq  x@tpoff(%rax),%rax
## 0x08  .byte  0x66
0x09  rex64
0x0a  call
## *
__tls_get_addr@GOTPCREL(%rip)
Table 11.8: GD -> LE Code Transition (ILP32)
## GD                                     LE
0x00  leaq   x@tlsgd(%rip),%rdi0x00  movl  %fs:0, %eax
0x07  .byte  0x660x08  leaq  x@tpoff(%rax),%rax
0x08  rex64
0x09  call
## *
__tls_get_addr@GOTPCREL(%rip)
Local Dynamic to Local ExecFor local dynamic model to local exec model transition,
linker generates 40x66prefixes, instead of 3, beforemovinstruction for LP64 and
generate a 5-bytenop, instead of 4-byte, beforemovinstruction for ILP32.  To load
the address of the TLS block of the module, which contains variablex, into%rax
without PLT:
## 137
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Table 11.9: LD -> LE Code Transition (LP64)
## LD                                 LE
0x00  leaq  x@tlsld(%rip),%rdi0x00  .long  0x66666666
0x07  call
## *
__tls_get_addr@GOTPCREL(%rip)0x04  movq   %fs:0,%rax
Table 11.10: LD -> LE Code Transition (ILP32)
## LD                                 LE
0x00  leaq  x@tlsld(%rip),%rdi0x00  nopw  0x0(%rax)
0x07  call
## *
__tls_get_addr@GOTPCREL(%rip)0x05  movl  %fs:0,%eax
## 138
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 12
Intel MPX Extension
Intel  MPX  (Memory  Protection  Extensions)  provides  4  128-bit  wide  bound  registers
(%bnd0-%bnd3).   For  purposes  of  parameter  passing  and  function  return,  the  lower  64
bits  of%bndNspecify  lower  bound  of  the  corresponding  parameter,  and  the  upper  64
bits  specify  upper  bound  of  the  parameter.   The  upper  bound  is  represented  in  one’s
complement form.
## 1
12.1    Parameter Passing and Returning of Values
APOINTERclass  is  added  for  passing  and  returning  pointer  types  andINTEGERclass  is
updated as follows:
POINTERThis class consists of pointer types.
INTEGERThis class consists of integral types (except pointer types) that fit into one of
the general purpose registers.
## 12.1.1    Classification
Pointers and integers are classified as:
-  Pointers are in the POINTER class.
-  Arguments of types (signed and unsigned)_Bool,char,short,int,longandlong
longare in the INTEGER class.
## 1
MPX is not supported under ILP32 since MPX requires 64-bit pointers.
## 139
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Figure 12.1: Bound Register Usage
Preserved across
RegisterUsagefunction calls
%bnd0–%bnd3used to pass/return bounds of pointer argu-
ments/return values
## No
The classification of aggregate (structures and arrays) and union types is updated as
follows:
-  When a C++ object is passed by invisible reference, the object is replaced in the
parameter list by a pointer that has class POINTER.
-  If one of the classes is POINTER, the result is the POINTER.
## 12.1.2    Passing
For parameter passing, if the class is INTEGER or POINTER, the next available register
of the sequence%rdi,%rsi,%rdx,%rcx,%r8and%r9is used.
Bounds passing
Intel MPX provides ISA extensions that allow passing bounds for a pointer argument that
specify  memory  area  that  may  be  legally  accessed  by  dereferencing  the  pointer.   This
paragraph desribes how the bounds are passed to the callee.
Several functions used in the description below are defined as follows:
BOUND_MAP_STORE(bnd, addr, ptr)This function executes Intel MPXbndstxin-
struction.ptrargument is used to initialize index field of the memory operand of
thebndstxinstruction,addris  encoded in  base  and/or  displacement fields  of  the
memory operand,bndis encoded in the register operand.
BOUND_MAP_LOAD(addr, ptr)This function executes Intel MPXbndldxinstruction.
ptrargument is used to initialize index field of the memory operand of thebndldxin-
struction,addris encoded in base and/or displacement fields of the memory operand.
The following algorithm is used to decide how bounds are passed for each eightbyte:
## 140
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  If the class is INTEGER, the eightbyte is passed in a general purpose register and the
function being called uses varargs or stdarg, then the class is converted to POINTER.
Artificial bounds that allow accessing all memory are created for pointers contained
in the eightbyte.
-  If the class is POINTER and the eightbyte is passed in a general purpose register,
then the bounds associated with the pointers contained in the eightbyte are passed in
the next available registers from the sequence%bnd0,%bnd1,%bnd2and%bnd3. If there
are no bound registers available for the bounds of an argument, then the bounds are
passed in a CPU defined manner by executingBOUND_MAP_STORE(bnd, addr, ptr)
function, wherebndis the current bounds of the argument,addris the address of
stack location beyond the location of the callee’s return address (that will be put on
stack by the corresponding call instruction),ptris the actual value of the pointer
argument.  For each call, there may be up to two such pointer arguments, the first
one has its bounds associated with (<return address stack location>–8) address, and
the second one - with (<return address stack location>–16).
-  If the class is POINTER and the eightbyte is passed on the stack,  or the class is
MEMORY and the argument contains pointer members, then the bounds associated
with each pointer contained in the eightbyte are passed in a CPU defined manner
by executingBOUND_MAP_STORE(bnd, addr, ptr)function, wherebndis the current
bounds of the pointer argument,addris the address of the pointer argument’s stack
location,ptris the actual value of the pointer argument. If the eightbyte may contain
parts of partially overlapping pointers, then bounds associated with the pointers are
ignored and special bounds that allow accessing all memory are passed for such
pointers.
The callee uses the same algorithm to classify the incoming parameters. If a parameter is
passed to the callee usingBOUND_MAP_STORE, then the callee fetches the passed bounds using
BOUND_MAP_LOAD(addr, ptr), whereaddris the same address passed to the corresponding
BOUND_MAP_STOREin the caller, andptris the actual value of the pointer parameter fetched
by the callee either from a general purpose register or from a stack location.
When passing arguments with bounds to functions, function prototypes must be pro-
vided. Otherwise, the run-time behavior is undefined.
12.1.3    Returning of Values
The returning of values is updated:
## 141
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  If the class is INTEGER or POINTER, the next available register of the sequence
%rax,%rdxis used.
Returning of Bounds
The returning of bounds is done according to the following algorithm:
-  Classify the return type with the classification algorithm.
-  If the type has class MEMORY, on return%bnd0must contain bounds of the “hidden”
first argument that has been passed in by the caller in%rdi.
-  If the class is POINTER, the next available register of the sequence%bnd0,%bnd1,
%bnd2,%bnd3is used to return bounds of the pointers contained in the eightbyte.
As an example of bound passing conventions, consider the declaration and the function
call is show in Figure 12.2. The corresponding bound registers allocation is given in Figure
12.3, the stack frame offset given shows the frame before calling the function.
## Figure 12.2: Bounds Passing Example
extern void func (int
## *
p1, int
## *
p2, int
## *
p3,
int
## *
p4, int
## *
p5, int x,
int
## *
p6);
func(p1, p2, p3, p4, p5, x, p6);
## Figure 12.3: Bounds Allocation Example
Bound Registers    Stack Frame Offset forGeneral Purpose    Stack Frame
BOUND_MAP_STORERegistersOffset
%bnd0:p1-16:             p5
## 2
%rdi:p10:      p6
%bnd1:p2-24:             p6%rsi:p2
## %bnd2:p3%rdx:p3
## %bnd3:p4%rcx:p4
## %r8:p5
## %r9:x
## 142
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## 12.1.4    Variable Argument Lists
Passing variable arguments is updated as follows:
## The Register Save Area
If a function taking a variable argument list is compiled for Intel MPX, then the bounds
passed for the argument registers saved to the register save area are saved for each ar-
gument  register  in  the  prolog  by  executingBOUND_MAP_STORE(bnd, addr, ptr)function
(12.1.2), wherebndis the current bounds of the pointer argument,addris the address of
the argument register’s location in register save area andptris the actual value of the
argument register.
Theva_argMacro
Theva_arg(l, type)implementation is updated as follows:
-  Fetchtypefroml->reg_save_areawith   an   offset   ofl->gp_offsetand/or
l->fp_offset.This  may  require  copying  to  a  temporary  location  in  case  the
parameter  is  passed  in  different  register  classes  or  requires  an  alignment  greater
than 8 for general purpose registers and 16 for XMM registers.  If type specifies a
pointer, then the bounds of the argument being fetched are loaded by executing
BOUND_MAP_LOAD(l->reg_save_area + l->gp_offset, ptr)(12.1.2),  whereptris
the actual value fetched froml->reg_save_areawith an offset ofl->gp_offset.
12.2    Program Loading and Dynamic Linking
To preserve bound registers for symbol lookup in branch instructions with theBND(0xf2)
prefix, linker should generate the BND procedure linkage table (see figure 12.4) together
with an additional procedure linkage table (see figure 12.5).
## 2
Before the call tofunc()the return address of the call is not yet put on the stack,  thus offset-16
accounts for the push of the return address that will be made by the call instruction.
## 143
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 12.4: BND Procedure Linkage Table (small and medium models)
.PLT0: pushq    GOT+8(%rip)              # GOT[1]
bnd jmp
## *
GOT+16(%rip)            # GOT[2]
nopl     (%rax)
.PLT1: pushq    $index1                  # 16 bytes from .PLT0
bnd jmp  PLT0
nopl     0(%rax,%rax,1)
.PLT2: pushq    $index2                  # 16 bytes from .PLT1
bnd jmp  .PLT0
nopl     0(%rax,%rax,1)
## .PLT3: ...
Figure 12.5: Additional Procedure Linkage Table (small and medium models)
.APLT1: bnd jmp  name1@GOTPCREL(%rip)
nop
.APLT2: bnd jmp  name2@GOTPCREL(%rip)    # 8 bytes from .APLT1
nop
.APLT3: ...                              # 8 bytes from .APLT2
To support indirect branches with theBND(0xf2) prefix (see figure 12.6), branches in
all BND procedure linkage table entries must have theBND(0xf2) prefix.
## 144
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 12.6: Indirect branch
movq     fp@GOTPCREL(%rip), %rax
bnd jmp
## *
## (%rax)
## ...
.globl   fp
## .section .data.rel,"aw",@progbits
## .align   8
.type    fp, @object
.size    fp, 8
fp:
.quad    memcpy
When  the  BND  procedure  linkage  table  is  used,  the  initial  value  of  the  global  off-
set table entry for the external function is the address of the corresponding entry of the
additional procedure linkage table.
## 145
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 13
Intel CET Extension
Intel CET (Control-flow Enforcement Technology) Extension includes:
IBT (Indirect Branch Tracking)Branch   protection   to   defend   against   Jump/Call
## Oriented Programming.
SHSTK (Shadow Stack)Return  address  protection  to  defend  against  Return  Oriented
## Programming,
## 13.1    Program Loading
13.1.1    ProcessGNU_PROPERTY_X86_FEATURE_1_IBT
On an IBT capable processor, the following steps should be taken:
-  Whenloadinganexecutablewithoutinterpreter,enableIBTif
GNU_PROPERTY_X86_FEATURE_1_IBTis set on the executable.
-  Whenloadinganexecutablewithaninterpreter,enableIBTif
GNU_PROPERTY_X86_FEATURE_1_IBTis   set   on   the   interpreter.The   interpreter
should disable IBT ifGNU_PROPERTY_X86_FEATURE_1_IBTisn’t set on the executable
or any shared objects loaded via theDT_NEEDEDtag.
-  AfterIBTisenabled,whenloadingasharedobject
## 1
without
## GNU_PROPERTY_X86_FEATURE_1_IBT:
## 1
The legacy code page bitmap isn’t used since it can’t cover the jitted code generated by legecy JIT
engines.
## 146
AMD64 ABI 1.0 – May 20, 2021 – 11:08

(a)  If legacy interwork is allowed, IBT should be disabled.
(b)  If legacy interwork isn’t allowed, it causes an error.
13.1.2    ProcessGNU_PROPERTY_X86_FEATURE_1_SHSTK
On a SHSTK capable processor, the following steps should be taken:
-  Whenloadinganexecutablewithoutinterpreter,enableSHSTKif
GNU_PROPERTY_X86_FEATURE_1_SHSTKis set on the executable.
-  Whenloadinganexecutablewithaninterpreter,enableSHSTKif
GNU_PROPERTY_X86_FEATURE_1_SHSTKis  set  on  the  interpreter.The  interpreter
should  disable  SHSTK  ifGNU_PROPERTY_X86_FEATURE_1_SHSTKisn’t  set  on  the
executable or any shared objects loaded via theDT_NEEDEDtag.
-  AfterSHSTKisenabled,whenloadingasharedobjectwithout
## GNU_PROPERTY_X86_FEATURE_1_SHSTK:
(a)  If legacy interwork is allowed, SHSTK should be disabled.
(b)  If legacy interwork isn’t allowed, it causes an error.
## 13.2    Dynamic Linking
To support IBT, linker should generate a IBT-enabled PLT (Procedure Linkage Table) (see
figure 13.1) together with a second PLT (see figure 13.2).
## 147
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 13.1: The IBT-enabled PLT (small and medium models)
.PLT0: pushq    GOT+8(%rip)             # GOT[1]
bnd jmp
## *
GOT+16(%rip)           # GOT[2]
nopl     (%rax)
.PLT1: endbr64                          # 16 bytes from .PLT0
pushq    $index1
bnd jmp  PLT0
nop
.PLT2: endbr64                          # 16 bytes from .PLT1
pushq    $index2
bnd jmp  .PLT0
nop
## .PLT3: ...
Figure 13.2: The Second IBT-enabled PLT (small and medium models)
.SPLT1: endbr64
bnd jmp
## *
name1@GOTPCREL(%rip)
nopl     0x0(%rax,%rax,1)
.SPLT2: endbr64
bnd jmp
## *
name2@GOTPCREL(%rip)  # 16 bytes from .SPLT1
nopl     0x0(%rax,%rax,1)
.SPLT3: ...                             # 16 bytes from .SPLT2
The BND prefix is added so that the IBT-enabled procedure linkage table is also com-
patible with MPX.
When the IBT-enabled procedure linkage table is used, the initial value of the global
offset table entry for the external function is the address of the corresponding entry of the
second procedure linkage table.
13.2.1    IBT-enabled PLTs For ILP32
Since MPX isn’t supported in ILP32 programming model, there is no need to add the BND
prefix.
## 148
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure 13.3: The IBT-enabled PLT For ILP32
.PLT0: pushq    GOT+8(%rip)             # GOT[1]
jmp
## *
GOT+16(%rip)           # GOT[2]
nopl     0x0(%rax)
.PLT1: endbr64                          # 16 bytes from .PLT0
pushq    $index1
jmp  PLT0
xchg %ax,%ax
.PLT2: endbr64                          # 16 bytes from .PLT1
pushq    $index2
jmp  .PLT0
xchg %ax,%ax
## .PLT3: ...
Figure 13.4: The Second IBT-enabled PLT For ILP32
.SPLT1: endbr64
jmp  name1@GOTPCREL(%rip)
nopw 0x0(%rax,%rax,1)
.SPLT2: endbr64
jmp  name2@GOTPCREL(%rip)       # 16 bytes from .SPLT1
nopw 0x0(%rax,%rax,1)
.SPLT3: ...                             # 16 bytes from .SPLT2
## 149
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Chapter 14
Intel LAM Extension
14.1    Intel LAM
Intel LAM (Linear Address Masking) Extension allows software to locate metadata in data
pointers and dereference them without needing to mask the metadata bits. It supports:
LAM_U48Activate LAM for user data pointers and use of bits 62:48 as masked meta-
data.
LAM_U57Activate LAM for user data pointers and use of bits 62:57 as masked meta-
data.
## 14.2    Program Loading
14.2.1    ProcessGNU_PROPERTY_X86_FEATURE_1_LAM_U48
On an LAM capable processor, the following steps should be taken:
-  When    loading    an    executable    without    interpreter,enable    LAM_U48    if
GNU_PROPERTY_X86_FEATURE_1_LAM_U48is set on the executable.
-  When    loading    an    executable    with    an    interpreter,enable    LAM_U48    if
GNU_PROPERTY_X86_FEATURE_1_LAM_U48is  set  on  the  interpreter.The  interpreter
should disable LAM_U48 ifGNU_PROPERTY_X86_FEATURE_1_LAM_U48isn’t set on the
executable or any shared objects loaded via theDT_NEEDEDtag.
-  After    LAM_U48    is    enabled,when    loading    a    shared    object    without
## GNU_PROPERTY_X86_FEATURE_1_LAM_U48:
## 150
AMD64 ABI 1.0 – May 20, 2021 – 11:08

(a)  If legacy interwork is allowed, LAM_U48 should be disabled.
(b)  If legacy interwork isn’t allowed, it causes an error.
14.2.2    ProcessGNU_PROPERTY_X86_FEATURE_1_LAM_U57
On an LAM capable processor, the following steps should be taken:
-  When    loading    an    executable    without    interpreter,enable    LAM_U57    if
GNU_PROPERTY_X86_FEATURE_1_LAM_U57is set on the executable.
-  When    loading    an    executable    with    an    interpreter,enable    LAM_U57    if
GNU_PROPERTY_X86_FEATURE_1_LAM_U57is  set  on  the  interpreter.The  interpreter
should disable LAM_U57 ifGNU_PROPERTY_X86_FEATURE_1_LAM_U57isn’t set on the
executable or any shared objects loaded via theDT_NEEDEDtag.
-  After    LAM_U57    is    enabled,when    loading    a    shared    object    without
## GNU_PROPERTY_X86_FEATURE_1_LAM_U57:
(a)  If legacy interwork is allowed, LAM_U57 should be disabled.
(b)  If legacy interwork isn’t allowed, it causes an error.
GNU_PROPERTY_X86_FEATURE_1_LAM_U48impliesGNU_PROPERTY_X86_FEATURE_1_LAM_U57.
## 151
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Appendix A
## Linux Conventions
This chapter describes some details that are only relevant to GNU/Linux systems and the
Linux kernel.
A.1    Execution of 32-bit Programs
The  AMD64  processors  are  able  to  execute  64-bit  AMD64  and  also  32-bit  ia32  pro-
grams. Libraries conforming to the Intel386 ABI will live in the normal places like/lib,
/usr/liband/usr/bin. Libraries following the AMD64, will uselib64subdirec-
tories for the libraries, e.g/lib64and/usr/lib64. Programs conforming to Intel386
ABI and to the AMD64 ABI will share directories like/usr/bin.  In particular, there
will be no/bin64directory.
A.2    AMD64 Linux Kernel Conventions
The section is informative only.
## A.2.1    Calling Conventions
The Linux AMD64 kernel uses internally the same calling conventions as user-level appli-
cations (see section 3.2.3 for details). User-level applications that like to call system calls
should use the functions from the C library.  The interface between the C library and the
Linux kernel is the same as for the user-level applications with the following differences:
## 152
AMD64 ABI 1.0 – May 20, 2021 – 11:08

-  User-level applications use as integer registers for passing the sequence%rdi,%rsi,
%rdx,%rcx,%r8and%r9.  The kernel interface uses%rdi,%rsi,%rdx,%r10,%r8and
## %r9.
-  A system-call is done via thesyscallinstruction. The kernel destroys registers%rcx
and%r11.
-  The number of the syscall has to be passed in register%rax.
-  System-calls are limited to six arguments,  no argument is passed directly on the
stack.
-  Returning from thesyscall, register%raxcontains the result of the system-call.  A
value in the range between -4095 and -1 indicates an error, it is-errno.
-  Only values of class INTEGER or class MEMORY are passed to the kernel.
## A.2.2    Stack Layout
The Linux kernel may align the end of the input argument area to a 8, instead of 16, byte
boundary.  It does not honor the red zone (see section   3.2.2) and therefore this area is
not allowed to be used by kernel code. Kernel code should be compiled by GCC with the
option-mno-red-zone.
## A.2.3    Miscellaneous Remarks
Linux Kernel code is not allowed to change the x87 and SSE units.  If those are changed
by kernel code, they have to be restored properly before sleeping or leaving the kernel. On
preemptive kernels also more precautions may be needed.
## 153
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Appendix B
## Linker Optimization
This chapter describes optimizations which may be performed by linker.
B.1    Combine GOTPLT and GOT Slots
In the small and medium models,  when there are both PLT and GOT references to the
same function symbol, normally linker creates a GOTPLT slot for PLT entry and a GOT
slot for GOT reference. A run-time JUMP_SLOT relocation is created to update the GOT-
PLT slot and a run-time GLOB_DAT relocation is created to update the GOT slot.  Both
JUMP_SLOT and GLOB_DAT relocations apply the same symbol value to GOTPLT and
GOT slots, respectively, at run-time.
As an optimization, linker may combine GOTPLT and GOT slots into a single GOT
slot and remove the run-time JUMP_SLOT relocation. It replaces the regular PLT entry:
Figure B.1: Procedure Linkage Table Entry Via GOTPLT Slot
.PLT: jmp      [GOTPLT slot]
pushq    relocation index
jmp      .PLT0
with an GOT PLT entry with an indirect jump via the GOT slot:
## 154
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Figure B.2: Procedure Linkage Table Entry Via GOT Slot
.PLT: jmp      [GOT slot]
nop
and resolves the PLT reference to the GOT PLT entry. Indirectjmpis an 5-byte instruction.
nopcan be encoded as a 3-byte instruction or a 11-byte instruction for 8-byte or 16-byte
PLT slot. A separate PLT with 8-byte slots may be used for this optimization.
This optimization isn’t applicable to theSTT_GNU_IFUNCsymbols since their GOT-
PLT slots are resolved to the selected implementation and their GOT slots are resolved to
their PLT entries.
This optimization must be avoided if pointer equality is needed since the symbol value
won’t be cleared in this case and the dynamic linker won’t update the GOT slot. Otherwise,
the resulting binary will get into an infinite loop at run-time.
B.2    Optimize GOTPCRELX Relocations
The  AMD64  instruction  encoding  supports  converting  certain  instructions  on  memory
operand withR_X86_64_GOTPCRELXorR_X86_64_REX_GOTPCRELXrelocations
against symbol,foo, into a different form on immediate operand iffoois defined locally.
Convert call and jmpConvert   memory   operand   ofcallandjmpinto   immediate
operand.
Table B.1: Call and Jmp Conversion
## Memory Operand       Immediate Operand
call
## *
foo@GOTPCREL(%rip)nop call foo
call
## *
foo@GOTPCREL(%rip)
call foo nop
jmp
## *
foo@GOTPCREL(%rip)jmp foo nop
Convert movConvert memory operand ofmovinto immediate operand. When position-
independent code is disabled andfoois defined locally in the lower 32-bit address
space, memory operand inmovcan be converted into immediate operand. Otherwise,
movmust be changed tolea.
## 155
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Table B.2: Mov Conversion
## Memory Operand          Immediate Operand
mov foo@GOTPCREL(%rip), %regmov $foo, %reg
mov foo@GOTPCREL(%rip), %reglea foo(%rip), %reg
Convert Test and BinopConvert  memory  operand  oftestandbinopinto  immediate
operand,  wherebinopis one ofadc,add,and,cmp,or,sbb,sub,xorinstructions,
when position-independent code is disabled.
Table B.3: Test and Binop Conversion
## Memory Operand          Immediate Operand
test %reg, foo@GOTPCREL(%rip)test $foo, %reg
binop foo@GOTPCREL(%rip), %regbinop $foo, %reg
## 156
AMD64 ABI 1.0 – May 20, 2021 – 11:08

## Index
## .cfi_adjust_cfa_offset, 104
## .cfi_def_cfa, 104
## .cfi_def_cfa_offset, 104
## .cfi_def_cfa_register, 104
## .cfi_endproc, 104
## .cfi_escape, 104
## .cfi_offset, 104
## .cfi_rel_offset, 104
## .cfi_startproc, 104
## .eh_frame, 104
## %rax, 54
## _UA_CLEANUP_PHASE, 92
## _UA_FORCE_UNWIND, 91
## _UA_SEARCH_PHASE, 91
_Unwind_Context, 90
_Unwind_DeleteException, 90
_Unwind_Exception, 90
_Unwind_ForcedUnwind, 90, 91
_Unwind_GetCFA, 90
_Unwind_GetGR, 90
_Unwind_GetIP, 90
_Unwind_GetIPInfo, 90
_Unwind_GetLanguageSpecificData, 90
_Unwind_GetRegionStart, 90
_Unwind_RaiseException, 90, 91
_Unwind_Resume, 90
_Unwind_SetGR, 90
_Unwind_SetIP, 90
## __float128, 16
## 157
AMD64 ABI 1.0 – May 20, 2021 – 11:08

auxiliary vector, 35
boolean, 18
byte, 16
## C++, 109
Call Frame Information tables, 89
code models, 38
Convert call and jmp, 155
Convert mov, 155
Convert Test and Binop, 156
double quadword, 16
doubleword, 16
## DT_FINI, 83
## DT_FINI_ARRAY, 83
## DT_INIT, 83
## DT_INIT_ARRAY, 83
## DT_JMPREL, 80
## DT_PREINIT_ARRAY, 83
DWARF Debugging Information Format, 89, 103
eightbyte, 16
exceptions, 29
exec, 33
fegetround, 89
fourbyte, 16
## General Dynamic Model, 126
## General Dynamic To Initial Exec, 129
global offset table, 78
halfword, 16
## Initial Exec Model, 126
Initial Exec Model, II, 126
## Initial Exec To Local Exec, 130
Initial Exec To Local Exec, II, 130
Kernel code model, 39
## 158
AMD64 ABI 1.0 – May 20, 2021 – 11:08

Large code model, 39
Large position independent code model, 40
## Local Dynamic Model, 127
Local Dynamic Model, II, 128
Local Dynamic to Local Exec, 131
Local Dynamic To Local Exec, II, 131
## Local Exec Model, 128
Local Exec Model, II, 128
Local Exec Model, III, 129
longjmp, 90
Medium code model, 39
Medium position independent code model, 40
micro-architecture levels, 14
## PIC, 40, 125
## Procedure Linkage Table, 71
procedure linkage table, 79–80
program interpreter, 82
quadword, 16
## R_X86_64_JUMP_SLOT, 80
## R_X86_64_TLSDESC, 81
red zone, 22, 153
register save area, 55
signal, 29
sixteenbyte, 16
size_t, 18
Small code model, 39
small code model, 125
Small position independent code model, 40
small position independent code model, 125
terminate(), 91
Thread-Local Storage, 73
twobyte, 16
Unwind Library interface, 89
## 159
AMD64 ABI 1.0 – May 20, 2021 – 11:08

va_arg, 57, 143
va_list, 56
va_start, 56
word, 16
## 160
AMD64 ABI 1.0 – May 20, 2021 – 11:08