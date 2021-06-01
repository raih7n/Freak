#include "pch.h"

#include "hook.h"

namespace freak
{
	eStatus Hook::patchHook(uintptr_t address, hookFn func)
	{
		if (!address)
			return INVALID_ADDRESS;

		MEMORY_BASIC_INFORMATION meminfo{};

		auto r = VirtualQuery(
			reinterpret_cast<LPCVOID>(address),
			reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&meminfo),
			sizeof(MEMORY_BASIC_INFORMATION)
		);

		if (r == 0)
			return INVALID_ADDRESS;

		m_callback = func;
		m_address = address;
	
		m_buffer = (uint8_t*)VirtualAlloc(nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		m_origCode = m_buffer + 0x100;
		m_newCode = m_buffer + 0x200;

		CopyOldCode((uint8_t*)address);

		Assembler assembler(asmjit::Environment::Arch::kArchX86);

		genThunk(assembler);

		DWORD oldprot;
		VirtualProtect((LPVOID)m_address, m_origCodeSize, PAGE_EXECUTE_READWRITE, &oldprot);

		auto set_jump = [](uintptr_t src, uint8_t* dst) -> void {
			*reinterpret_cast<uint8_t*>(src) = 0xE9;
			*reinterpret_cast<uintptr_t*>(src + 1) = (uintptr_t)dst - (uintptr_t)(src) - JMP_SIZE;
		};

		set_jump(m_address, m_newCode);

		for (size_t i = JMP_SIZE; i < m_origCodeSize; i++)
			*reinterpret_cast<uint8_t*>(m_address + i) = 0x90;

		VirtualProtect((LPVOID)m_address, m_origCodeSize, oldprot, &oldprot);

		return SUCCESS;
	}

	void Hook::genThunk(Assembler& assembler)
	{
		auto& buffer = assembler->code()->textSection()->buffer();

		// push all gp regsiters & eflags onto the stack
		assembler->pushad();
		assembler->pushfd();

		assembler->lea(asmjit::x86::eax, asmjit::x86::dword_ptr((uintptr_t)m_callback));
		assembler->call(asmjit::x86::eax);

		assembler->popfd();
		assembler->popad();

		// place original code bytes at the end of the thunk
		for (size_t i = 0; i < m_origCodeSize; i++)
			assembler->db(m_origCode[i]);

		uintptr_t jmpBack = m_address - (uintptr_t)(m_newCode) - 0x11;

		// jmp jmBack
		assembler->jmp(asmjit::x86::dword_ptr(jmpBack));

		memcpy(m_newCode, buffer.data(), buffer.size());
	}

	void Hook::CopyOldCode(uint8_t* ptr)
	{
		ldasm_data ld{};
		uint8_t* src = ptr;
		uint8_t* original = m_origCode;
		uint32_t diassm = 0;

		while (diassm < JMP_SIZE)
		{
			auto len = ldasm(src, &ld, is_x64);

			// Determine code end
			if (ld.flags & F_INVALID
				|| (len == 1 && (src[ld.opcd_offset] == 0xCC || src[ld.opcd_offset] == 0xC3))
				|| (len == 3 && src[ld.opcd_offset] == 0xC2))
				break;

			memcpy(original, src, len);

			// instruction has relative offset
			if (ld.flags & F_RELATIVE)
			{
				int32_t delta = 0;
				uint32_t ofst = (ld.disp_offset != 0 ? ld.disp_offset : ld.imm_offset);
				uint32_t sz = ld.disp_size != 0 ? ld.disp_size : ld.imm_size;

				memcpy(&delta, src + ofst, sz);
				memcpy(src + ofst, &delta, sz);
			}

			src += len;
			m_origCodeSize += len;
			original += len;
			diassm += len;
		}
	}
}