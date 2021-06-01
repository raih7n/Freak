#pragma once

#include "config.h"

#include "ldasm.h"
#include "Assembler.h"

namespace freak
{
	constexpr auto JMP_SIZE = 0x5;

	/// <summary>
	/// Register access for function hooks
	/// </summary>
	struct regs_t
	{
		uint32_t eflags;
		uint32_t edi;
		uint32_t esi;
		uint32_t ebp;
		uint32_t esp;
		uint32_t ebx;
		uint32_t edx;
		uint32_t ecx;
		uint32_t eax;	
	};

	/// <summary>
	/// result enums
	/// </summary>
	enum eStatus
	{
		SUCCESS = 0,
		INVALID_ADDRESS,
	};

	// function signature for all hooks
	using hookFn = void(__cdecl*)(regs_t);

	/// <summary>
	/// Main hooking class
	/// </summary>
	class Hook
	{
	public:
		Hook() = default;

		/// <summary>
		/// Places a hook at the given address
		/// </summary>
		/// <param name="address">target memory address</param>
		/// <param name="func">function pointer to hook</param>
		/// <returns></returns>
		FREAK_API eStatus patchHook(uintptr_t address, hookFn func);

	private:
		/// <summary>
		/// Generates thunk to and from hook
		/// </summary>
		/// <param name="assembler">assembler object</param>
		void genThunk(Assembler& assembler);

		/// <summary>
		/// Copies old code bytes before patching
		/// </summary>
		/// <param name="ptr">pointer to function memory</param>
		void CopyOldCode(uint8_t* ptr);

	private:
		Hook(const Hook&) = delete;
		Hook& operator=(Hook const&) = delete;

	private:
		hookFn m_callback = nullptr; // hook function pointer
		uintptr_t m_address = 0; // target address

		size_t m_origCodeSize = 0; // Bytes of original code
		uint8_t* m_buffer = nullptr; // Buffer to hold old code & thunk
		uint8_t* m_origCode = nullptr; // pointer in m_buffer to original code
		uint8_t* m_newCode = nullptr; // pointer in m_buffere to thunk
	};
}