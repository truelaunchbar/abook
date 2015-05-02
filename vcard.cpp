#include "globals.h"
#include "VCard.h"
#include "base64.h"
#include <Mlang.h>
#include <comutil.h>

vcard::card::card( void )
{

}

vcard::card::card( const card& val )
{
	m_fields = val.m_fields;
}

vcard::card::~card( void )
{

}

bool vcard::card::parse( const char* vcardText )
{
	char* start = (char*) vcardText;
	char* end	= strstr(start, "\r\n");
	while(end)
	{
		while(end && (isspace(end[2]) || end[-1] == '='))
		{
			end += 2;
			end	= strstr(end, "\r\n");
		}
		if(!end) break;

		char* fldText = new char[end - start + 1];
		strncpy(fldText, start, end - start);
		fldText[end - start] = 0;
		field fld;
		if(fld.parse(fldText))
		{
			m_fields.push_back(fld);
		}
		delete fldText;
		start = end + 2;
		end	= strstr(start, "\r\n");
	}
	return true;
}

std::string vcard::card::version()
{
	for(size_t i=0; i < m_fields.size(); i++)
	{
		if(m_fields[i].getType() == "VERSION")
		{
			return m_fields[i].getRawValue();
		}
	}
	return std::string();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vcard::parameter::parameter()
{
}

vcard::parameter::parameter( const parameter& val )
{
	m_name	= val.m_name;
	m_value	= val.m_value;
}
vcard::parameter::~parameter()
{
}

bool vcard::parameter::parse( const char* text )
{
	char* start = (char*) text;
	char* end	= strstr(start, "=");
	if(end)
	{
		char* name = new char[end - start + 1];
		strncpy(name, start, end - start);
		name[end - start] = 0;
		m_name = name;
		delete name;
		m_value = end + 1;
	} else
	{
		m_name = start;
	}
	return true;
}

bool vcard::parameter::isParam( const char* name )
{
	if(name == m_name)
	{
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vcard::field::field()
{
}

vcard::field::field( const field& val )
{
	m_type		= val.m_type;
	m_rawValue	= val.m_rawValue;
	m_params	= val.m_params;
}

vcard::field::~field()
{
}

bool vcard::field::parse( const char* text )
{
	char* start = (char*) text;
	char* valueStart = strstr(start, ":");
	if(!valueStart)
	{
		return false;
	}
	m_rawValue = valueStart + 1;

	char* pStart = start;
	char* pEnd	 = strstr(pStart, ";");
	if(!pEnd || pEnd > valueStart) pEnd = valueStart;
	char* typeName = new char[pEnd - pStart + 1];
	strncpy(typeName, pStart, pEnd - pStart);
	typeName[pEnd - pStart] = 0;
	m_type = typeName;
	delete typeName;

	pStart = pEnd + 1;
	while(pStart < valueStart)
	{
		pEnd = strstr(pStart, ";");
		if(!pEnd) pEnd = valueStart;

		char* paramText = new char[pEnd - pStart + 1];
		strncpy(paramText, pStart, pEnd - pStart);
		paramText[pEnd - pStart] = 0;
		parameter param;
		if(param.parse(paramText))
		{
			m_params.push_back(param);
		}
		delete paramText;
		
		pStart = pEnd + 1;
	}
	return true;
}

bool vcard::field::haveParam( const char* paramName )
{
	for(paramsArray::iterator iter = m_params.begin(); iter != m_params.end(); iter++)
	{
		if(iter->isParam(paramName))
		{
			return true;
		}
	}
	return false;
}

bool vcard::field::haveParam( const char* paramName, const char* paramvalue )
{
	for(paramsArray::iterator iter = m_params.begin(); iter != m_params.end(); iter++)
	{
		if(iter->isParam(paramName) && StrStrIA(iter->getValue(), paramvalue))
		{
			return true;
		}
	}
	return false;
}

std::string vcard::field::getParamValue( const char* paramName )
{
	for(paramsArray::iterator iter = m_params.begin(); iter != m_params.end(); iter++)
	{
		if(iter->isParam(paramName))
		{
			return std::string(iter->getValue());
		}
	}
	return std::string();
}

std::wstring vcard::field::getConvertedStringValue(std::string& version)
{
	std::string val		= m_rawValue;
	std::string charset = getParamValue("CHARSET");
	std::string enc		= getParamValue("ENCODING");

	if(charset.empty() && version[0] >= '3')
	{
		charset = "utf-8";
	}

	if(haveParam("QUOTED-PRINTABLE") || enc == "QUOTED-PRINTABLE")
	{
		unfoldQP(val);
		std::string decStr;
		int idx = 0;
		while(idx < val.length())
		{
			if(val[idx] == '=' && idx + 2 >= val.length())
			{
				int iii = 0;
				iii++;
			}
			if(val[idx] == '=' && idx + 2 < val.length())
			{
				char charCode[5];
				charCode[0] = '0';
				charCode[1] = 'x';
				charCode[2] = val[idx + 1];
				charCode[3] = val[idx + 2];
				charCode[4] = 0;
				char* stop = NULL;
				char chr = (char) strtoul(charCode, &stop, 16);
				decStr += chr;
				idx += 3;
			}  else
			{
				decStr += val[idx];
				idx++;
			}
		}
		val = decStr;
	} else if(haveParam("BASE64") || enc == "BASE64" || enc == "b" || enc == "B")
	{
		val = base64::DecodeToStr(val);
	} else
	{
		unfold(val);
	}
	unescape(val);

	return convertToWString(val, toWString(charset));
}

void vcard::field::unfold( std::string& str )
{
	size_t begin = str.find("\r\n");
	size_t end;
	
	while(begin != std::string::npos)
	{
		end = begin + 2;

		while(isspace(str[end]))
		{
			end++;
		}
		str.erase(begin, end - begin);
		begin = str.find("\r\n", begin);
	}
}

std::wstring vcard::field::toWString( std::string& src )
{
	int outLen = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
	LPWSTR mbStr = (LPWSTR) malloc(outLen * sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, mbStr, outLen);
	std::wstring ret = mbStr;
	delete mbStr;
	return ret;
}

std::wstring vcard::field::convertToWString( std::string& src, std::wstring& charset )
{
	std::wstring ret;

	if(!charset.empty())
	{
		IMultiLanguage* ml = NULL;
		HRESULT hr = CoCreateInstance(CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (LPVOID*) &ml);	

		if(hr == S_OK)
		{
			MIMECSETINFO csInfo;
			_bstr_t chs(charset.c_str());
			ml->GetCharsetInfo(chs, &csInfo);

			DWORD pdwMode = 0;
			UINT dstSize = (UINT) ((src.length() + 1) * 2);
			WCHAR* dst = new WCHAR[dstSize];

			if(ml->ConvertStringToUnicode(&pdwMode, csInfo.uiInternetEncoding, (CHAR*) src.c_str(), NULL, dst, &dstSize) == S_OK)
			{
				dst[dstSize] = 0;
				ret = dst;
			}
			delete dst;

			ml->Release();
		}
	}
	if(ret.empty())
	{
		ret = toWString(src);
	}
	return ret;
}

void vcard::field::unfoldQP( std::string& str )
{
	size_t begin = str.find("=\r\n");
	size_t end;

	while(begin != std::string::npos)
	{
		end = begin + 3;

		while(isspace(str[end]))
		{
			end++;
		}
		str.erase(begin, end - begin);
		begin = str.find("=\r\n", begin);
	}
}

LPBYTE vcard::field::getBinaryValue( UINT& size )
{
	LPBYTE ret = NULL;
	std::string val		= m_rawValue;
	std::string enc		= getParamValue("ENCODING");

	if(haveParam("BASE64") || enc == "BASE64" || enc == "b" || enc == "B")
	{
		size_t outSize = 0;
		ret = (LPBYTE) base64::DecodeToBin(val, &outSize);
		size = (UINT) outSize;
	}

	return ret;
}

std::vector<std::wstring> vcard::field::getStructuredText(std::string& version)
{
	std::vector<std::wstring> ret;
	std::wstring val = getConvertedStringValue(version);
	std::wstring curStr;
	for(size_t i = 0; i < val.length(); i++)
	{
		if(val[i] == L';')
		{
			ret.push_back(curStr);
			curStr.clear();
		} else
		{
			curStr += val[i];
		}
	}
	ret.push_back(curStr);
	
	return ret;
}

void vcard::field::unescape( std::string& str )
{
	size_t pos = str.find('\\');
	while(pos != std::string::npos)
	{
		if(str[pos + 1] == 'N' || str[pos + 1] == 'n')
		{
			str.erase(pos, 2);
			str.insert(pos, 1, '\n');
		} else
		{
			str.erase(pos, 1);
		}
		pos = str.find('\\', pos + 1);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

vcard::reader::reader()
{
	m_text = NULL;
}

vcard::reader::~reader()
{
	close();
}

void vcard::reader::close()
{
	if(m_text)
	{
		delete m_text;
		m_text = NULL;
	}
}

bool vcard::reader::open( const wchar_t* fileName )
{
	close();
	HANDLE	hFfile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFfile == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	DWORD sz = GetFileSize(hFfile, NULL);
	m_text = new char[sz + 1];
	ReadFile(hFfile, m_text, sz, &sz, NULL);
	m_text[sz] = 0;
	m_curPos = 0;
	CloseHandle(hFfile);
	return true;
}

vcard::card* vcard::reader::next()
{
	card* ret = NULL;
	if(m_text)
	{
		char* start = strstr(m_text + m_curPos, "BEGIN:VCARD\r\n");
		char* end	= strstr(m_text + m_curPos, "END:VCARD\r\n");
		if(start && end)
		{
			start += strlen("BEGIN:VCARD\r\n");
			char* vcardText = new char[end - start + 1];
			strncpy(vcardText, start, end - start);
			vcardText[end - start] = 0;
			ret = new card;
			if(!ret->parse(vcardText))
			{
				delete ret;
				ret = NULL;
			}
			m_curPos = (int) ((end - m_text) + strlen("END:VCARD\r\n"));
		}
	}
	return ret;
}
