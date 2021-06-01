# Freak

Freak is a MSVC C++ x86 library for making local mid-function hooks. 

# Usage

```cpp
#include <Windows.h>
#include <Freak\hook.h>

void LdrLoadDllHook(regs_t regs)
{
	// get the ModuleFileName argument from the stack & print it
	PUNICODE_STRING ModFileName = (PUNICODE_STRING)regs.ebp + 0x10;
	std::cout << ModFileName.buffer;
	// do more cool things here
}

HMODULE ntdll = GetModuleHandle("ntdll.dll")

using fnLdrLoadDll = NTSTATUS(__stdcall*)(PWCHAR, ULONG, PUNICODE_STRING, PHANDLE);
auto pfnLdrLoadDll = (fnLdrLoadDll)GetProcAddress(ntdll, "LdrLoadDll");

// mov ecx, [ebp + 10h]
uintptr_t target = (uintptr_t)pfnLdrLoadDll + 0x1C;

Hook hook;
hook.placeHook(target, &LdrLoadDllHook);

```

## Credits
The following are used without modification. Credits to their authors. Libraries can be installed via vcpkg or your prefered package manager
- [asmjit][github.asmjit]
AsmJit is a lightweight library for machine code generation written in C++ language.

[//]: # (Hyperlink IDs)
[github.asmjit]: https://github.com/asmjit/asmjit