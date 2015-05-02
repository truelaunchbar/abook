#pragma once

class CProgram
{
public:
	typedef std::vector<CProgram>	vector;
private:
	std::wstring	m_program;
	std::wstring	m_field;
public:
	CProgram();
	CProgram(const CProgram& val);
	~CProgram();

	void operator=(const CProgram& val);

	std::wstring getCmdLine(std::wstring& val);
	std::wstring getCmdLine()	{ return m_program; }
	std::wstring getField();

	void setCmdLine(LPCWSTR val)	{ m_program = val; }
	void setField(LPCWSTR val)		{ m_field = val; }
};
