#include "globals.h"
#include "applications.h"

CProgram::CProgram()
{

}

CProgram::CProgram(const CProgram& val)
{
	m_program = val.m_program;
	m_field = val.m_field;
}

CProgram::~CProgram()
{

}

std::wstring CProgram::getCmdLine(std::wstring& val)
{
	std::wstring ret = m_program;
	replace_str(ret, std::wstring(L"%v%"), val);
	return ret;
}

std::wstring CProgram::getField()
{
	return m_field;
}

void CProgram::operator=(const CProgram& val)
{
	m_program = val.m_program;
	m_field = val.m_field;
}
