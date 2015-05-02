#pragma once

#include <stdlib.h>
#include <vector>
#include <wchar.h>
#include <string>


namespace vcard
{
	class parameter
	{
		std::string	m_name;
		std::string	m_value;
	public:
		parameter();
		parameter(const parameter& val);
		~parameter();

		bool parse(const char* text);
		bool isParam(const char* name);
		const char* getValue()	{ return m_value.c_str(); }
		const char* getName()	{ return m_name.c_str(); }
	};

	class field
	{
		typedef std::vector<parameter> paramsArray;

		std::string	m_type;
		std::string	m_rawValue;
		paramsArray	m_params;
	public:
		field();
		field(const field& val);
		~field();

		bool			parse(const char* text);
		bool			haveParam(const char* paramName);
		bool			haveParam(const char* paramName, const char* paramvalue);
		std::string		getType()		{ return m_type; }
		std::string		getRawValue()	{ return m_rawValue; }
		std::string		getParamValue(const char* paramName);
		std::wstring	getConvertedStringValue(std::string& version);
		LPBYTE			getBinaryValue(UINT& size);
		std::vector<std::wstring> getStructuredText(std::string& version);
	
	private:
		void			unfold(std::string& str);
		void			unescape(std::string& str);
		void			unfoldQP(std::string& str);
		std::wstring	toWString(std::string& src);
		std::wstring	convertToWString(std::string& src, std::wstring& charset);
	};

	class card
	{
		typedef std::vector<field> fieldsArray;

		fieldsArray	m_fields;
	public:
		card(void);
		card(const card& val);
		~card(void);

		bool	parse(const char* vcardText);
		field	getField(int idx)	{ return m_fields[idx];			}
		int		count()				{ return (int) m_fields.size(); }
		std::string version();
	};

	class reader
	{
		char*	m_text;
		int		m_curPos;
	public:
		reader();
		~reader();

		bool	open(const wchar_t* fileName);
		card*	next();
		void	close();
	};
};
