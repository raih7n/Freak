#include "pch.h"

#include "Assembler.h"

namespace freak
{
	Assembler::Assembler(Environment::Arch arch)
	{
		m_environment.setArch(arch);
		m_code.init(m_environment);
		m_assembler = std::make_unique<asmjit::x86::Assembler>(&m_code);
	}
}