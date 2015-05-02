#include "globals.h"
#include "record.h"
#include <WinInet.h>
#include "PluginStream.h"
#include "images_cache.h"
#include "resource.h"
#include "xuistrings.h"

CRecord::CRecord()
{
	m_imgSize.cx = 100;
	m_imgSize.cy = 100;
	m_ID = 0;
	m_flags = 0;
	m_image = NULL;
	m_cbImage = 0;
	m_img = NULL;
}

CRecord::CRecord(const CRecord& val) : CRecord()
{
	*this = val;
}

CRecord::~CRecord()
{
}

void CRecord::Load(CPluginStream* stream)
{
	m_ID = stream->GetDWORD(TEXT("ID"));
	m_flags = stream->GetDWORD(TEXT("flags"));
	LPBYTE bin = (LPBYTE)stream->GetBIN(TEXT("image"), &m_cbImage);
	m_image = NULL;
	if (m_cbImage)
	{
		m_image = new BYTE[m_cbImage];
		memcpy(m_image, bin, m_cbImage);
	}

	if (stream->OpenNode(TEXT("fields")))
	{
		int count = stream->GetDWORD(TEXT("count"));
		for (int i = 0; i < count; i++)
		{
			TCHAR nodeName[100];
			wsprintf(nodeName, TEXT("#%d"), i);
			if (stream->OpenNode(nodeName))
			{
				FIELD fld;
				ZeroMemory(&fld, sizeof(fld));
				fld.displayName = stream->GetString(L"displayName", L"");
				fld.name = stream->GetString(L"name", L"");
				fld.value = stream->GetString(L"value", L"");
				fld.flags = stream->GetDWORD(TEXT("flags"));
				m_fields.push_back(fld);
				stream->CloseNode();
			}
		}
		stream->CloseNode();
	}

	InitImage();
}

void CRecord::Save(CPluginStream* stream)
{
	stream->SaveDWORD(TEXT("ID"), m_ID);
	stream->SaveDWORD(TEXT("flags"), m_flags);
	stream->SaveBIN(TEXT("image"), m_image, m_cbImage);

	stream->BeginNode(TEXT("fields"));
	stream->SaveDWORD(TEXT("count"), (UINT) m_fields.size());
	for (int i = 0; i < (int) m_fields.size(); i++)
	{
		TCHAR nodeName[100];
		wsprintf(nodeName, TEXT("#%d"), i);
		stream->BeginNode(nodeName);
			stream->SaveDWORD(TEXT("flags"), m_fields[i].flags);
			stream->SaveString(TEXT("displayName"), m_fields[i].displayName.c_str());
			stream->SaveString(TEXT("name"), m_fields[i].name.c_str());
			stream->SaveString(TEXT("value"), m_fields[i].value.c_str());
		stream->EndNode();
	}
	stream->EndNode();
}

cairo_container::image_ptr CRecord::GetPhoto(int cx, int cy)
{
	//if (!m_image) return nullptr;

	if (cx != m_imgSize.cx || cy != m_imgSize.cy)
	{
		m_img = nullptr;
	}
	if (!m_img)
	{
		m_imgSize.cx = cx;
		m_imgSize.cy = cy;
		m_img = cairo_container::image_ptr(new CTxDIB);
		if (m_image)
		{
			if (!m_img->load(m_image, m_cbImage))
			{
				if (!m_img->load(FindResource(g_hInst, L"defphoto.png", L"DEFIMAGES"), g_hInst))
				{
					m_img = nullptr;
				}
			}
		}
		else
		{
			if (!m_img->load(FindResource(g_hInst, L"defphoto.png", L"DEFIMAGES"), g_hInst))
			{
				m_img = nullptr;
			}
		}
		if (m_img)
		{
			if (cx >= 0 && cy >= 0)
			{
				if (m_img->getWidth() > cx || m_img->getHeight() > cy)
				{
					int newCX;
					int newCY;
					if (m_img->getWidth() - cx > m_img->getHeight() - cy)
					{
						newCX = cx;
						newCY = (int)((double)m_img->getHeight() / ((double)m_img->getWidth() / (double)newCX));
					}
					else
					{
						newCY = cy;
						newCX = (int)((double)m_img->getWidth() / ((double)m_img->getHeight() / (double)newCY));
					}
					m_img->resample(newCX, newCY);
				}
			}
		}
	}
	return m_img;
}

void CRecord::InitImage(LPBYTE imgBin, DWORD cbImage, BOOL copy)
{
	if (imgBin)
	{
		ClearImage();
		if (!copy)
		{
			m_image = imgBin;
			m_cbImage = cbImage;
		}
		else
		{
			m_image = new BYTE[cbImage];
			memcpy(m_image, imgBin, cbImage);
			m_cbImage = cbImage;
		}
	}
}

void CRecord::ClearImage(void)
{
	if (m_image)
	{
		delete m_image;
	}
	m_image = NULL;
	m_cbImage = 0;
	m_img = NULL;
}

size_t find_endif(std::wstring& str, size_t startPos)
{
	size_t pos = str.find(L'{', startPos);
	int cnt = 0;
	while (pos != std::wstring::npos)
	{
		if (str.substr(pos, 3) == L"{if")
		{
			cnt++;
		}
		else if (str.substr(pos, 7) == L"{endif}")
		{
			if (!cnt)
			{
				return pos;
			}
			cnt--;
		}
		pos = str.find(L'{', pos + 1);
	}
	return pos;
}

void CRecord::ApplyTemplate(std::wstring& tpl, CXUIEngine* xui, CProgram::vector& progs)
{
	size_t pos = 0;
	while (true)
	{
		size_t tgStart = tpl.find(L'{', pos);
		if (tgStart == std::wstring::npos) break;
		size_t tgEnd = tpl.find(L'}', tgStart);
		if (tgEnd == std::wstring::npos) break;
		std::wstring fldName = tpl.substr(tgStart + 1, tgEnd - tgStart - 1);
		std::wstring replace;
		if (!StrCmpNI(fldName.c_str(), L"imgsrc:", 7))
		{
			replace = fldName.substr(7);
		}
		else if (!StrCmpNI(fldName.c_str(), L"img:", 4))
		{
			replace = L"<img src=\"";
			replace += fldName.substr(4);
			replace += L"\">";
		}
		else if (!StrCmpNI(fldName.c_str(), L"label:", 6))
		{
			std::wstring fld = fldName.substr(6);
			if (fld == L"homeaddress")
			{
				replace = xui->getStringDef(XUI_GROUP_HOMEADDR);
			}
			else if (fld == L"workaddress")
			{
				replace = xui->getStringDef(XUI_GROUP_BUSINESSADDR);
			}
			else if (fld == L"internet")
			{
				replace = xui->getStringDef(XUI_GROUP_INTERNET);
			}
			else if (fld == L"instmsg")
			{
				replace = xui->getStringDef(XUI_GROUP_IM);
			}
			else if (fld == L"org")
			{
				replace = xui->getStringDef(XUI_GROUP_ORG);
			}
			else if (fld == L"org")
			{
				replace = xui->getStringDef(XUI_GROUP_ORG);
			}
			else if (fld == L"name")
			{
				replace = xui->getStringDef(XUI_GROUP_NAME);
			}
			else
			{
				replace = GetFieldLabel(fldName.substr(6).c_str(), xui);
			}
		}
		else if (!StrCmpNI(fldName.c_str(), L"ifnotempty:", 11))
		{
			std::wstring fld = fldName.substr(11);
			if (!if_not_empty(fld))
			{
				size_t endIf = find_endif(tpl, tgEnd);
				if (endIf != std::wstring::npos)
				{
					tgEnd = endIf + 6;
				}
				else
				{
					tgEnd = endIf;
				}
			}
		}
		else if (!StrCmpNI(fldName.c_str(), L"ifempty:", 8))
		{
			std::wstring fld = fldName.substr(8);
			if (if_not_empty(fld))
			{
				size_t endIf = find_endif(tpl, tgEnd);
				if (endIf != std::wstring::npos)
				{
					tgEnd = endIf + 6;
				}
				else
				{
					tgEnd = endIf;
				}
			}
		}
		else if (!StrCmpNI(fldName.c_str(), L"src:", 4))
		{
			std::wstring fld = fldName.substr(4);
			std::wstring val = GetFieldValue(fld.c_str());
			if (!StrCmpNI(fld.c_str(), L"url", 3))
			{
				if (!val.empty())
				{
					WCHAR outUrl[INTERNET_MAX_URL_LENGTH];
					if (!PathIsURL(val.c_str()))
					{
						DWORD sz = INTERNET_MAX_URL_LENGTH;
						if (UrlApplyScheme(val.c_str(), outUrl, &sz, URL_APPLY_DEFAULT) != S_OK)
						{
							StringCchCopy(outUrl, INTERNET_MAX_URL_LENGTH, val.c_str());
						}
					}
					else
					{
						StringCchCopy(outUrl, INTERNET_MAX_URL_LENGTH, val.c_str());
					}
					replace = outUrl;
				}
			}
			else
			{
				replace = val;
			}

		}
		else if (!StrCmpNI(fldName.c_str(), L"url", 3))
		{
			std::wstring val = GetFieldValue(fldName.c_str());
			if (!val.empty())
			{
				WCHAR outUrl[INTERNET_MAX_URL_LENGTH];
				if (!PathIsURL(val.c_str()))
				{
					DWORD sz = INTERNET_MAX_URL_LENGTH;
					if (UrlApplyScheme(val.c_str(), outUrl, &sz, URL_APPLY_DEFAULT) != S_OK)
					{
						StringCchCopy(outUrl, INTERNET_MAX_URL_LENGTH, val.c_str());
					}
				}
				else
				{
					StringCchCopy(outUrl, INTERNET_MAX_URL_LENGTH, val.c_str());
				}
				replace = L"<a href=\"";
				replace += outUrl;
				replace += L"\">";
				replace += val;
				replace += L"</a>";
			}
		}
		else if (!StrCmpI(fldName.c_str(), L"email") || !StrCmpI(fldName.c_str(), L"emailadd"))
		{
			std::wstring val = GetFieldValue(fldName.c_str());
			if (!val.empty())
			{
				replace = L"<a href=\"mailto:";
				replace += val;
				replace += L"\">";
				replace += val;
				replace += L"</a>";
			}
		}
		else
		{
			BOOL makeURL = FALSE;
			std::wstring fff = GetFieldType(fldName.c_str());
			for (size_t i = 0; i < progs.size(); i++)
			{
				if (progs[i].getField() == fff)
				{
					makeURL = TRUE;
					break;
				}
			}
			std::wstring val = GetFieldValue(fldName.c_str());
			if (!val.empty())
			{
				if (makeURL)
				{
					replace = L"<a href=\":";
					replace += fff;
					replace += L":";
					replace += val;
					replace += L"\">";
					replace += val;
					replace += L"</a>";
				}
				else
				{
					replace = val;
				}
			}
			else if (fldName != L"endif")
			{
				BOOL found = FALSE;
				for (int i = 0; g_defText[i].fldName && !found; i++)
				{
					if (fldName == g_defText[i].fldName)
					{
						found = TRUE;
					}
				}
				if (!found)
				{
					replace = L"{";
					replace += fldName;
					replace += L"}";
				}
			}
		}
		if (tgEnd == std::wstring::npos)
		{
			tpl.erase(tgStart, tgEnd);
		}
		else
		{
			tpl.erase(tgStart, tgEnd - tgStart + 1);
		}
		tpl.insert(tgStart, replace);
		pos += replace.length();
	}
}

bool CRecord::if_not_empty(const std::wstring &fld)
{
	litehtml::string_vector fields;
	litehtml::split_string(fld, fields, L":");
	for (const auto& f : fields)
	{
		if (if_not_empty_one(f))
		{
			return true;
		}
	}
	return false;
}

bool CRecord::if_not_empty_one(const std::wstring &fld)
{
	bool found = false;
	if (!StrCmpI(fld.c_str(), L"photo"))
	{
		if (m_image)
		{
			found = true;
		}
	}
	else if (fld == L"homeaddress")
	{
		std::vector<std::wstring> flds = {
			{ L"home_pobox" },
			{ L"home_address" },
			{ L"home_address2" },
			{ L"home_city" },
			{ L"home_zip" },
			{ L"home_state" },
			{ L"home_country" }
		};
		std::wstring val;
		for (const auto& f : flds)
		{
			val = GetFieldValue(f.c_str());
			if (!val.empty())
			{
				found = true;
				break;
			}
		}
	}
	else if (fld == L"workaddress")
	{
		std::vector<std::wstring> flds = {
			{ L"work_pobox" },
			{ L"work_address" },
			{ L"work_address2" },
			{ L"work_city" },
			{ L"work_zip" },
			{ L"work_state" },
			{ L"work_country" }
		};
		std::wstring val;
		for (const auto& f : flds)
		{
			val = GetFieldValue(f.c_str());
			if (!val.empty())
			{
				found = true;
				break;
			}
		}
	}
	else
	{
		std::wstring val = GetFieldValue(fld.c_str());
		if (!val.empty())
		{
			found = true;
		}
	}
	return found;
}

void CRecord::load_sample()
{
	struct
	{
		LPCTSTR	name;
		LPCTSTR value;
	} defVals[] =
	{
		{ L"displayname",		L"Vito M. Gutman" },
		{ L"firstname",			L"Vito" },
		{ L"lastname",			L"Gutman" },

		{ L"home_pobox",		L"PO2348"},
		{ L"home_address",		L"551 Beeghley Street"},
		{ L"home_city",			L"Temple"},
		{ L"home_zip",			L"76501"},
		{ L"home_state",		L"TX"},
		{ L"home_country",		L"Unided States"},

		{ L"work_pobox",		L"PO7888"},
		{ L"work_address",		L"2869 Norma Lane"},
		{ L"work_city",			L"Haynesville"},
		{ L"work_zip",			L"71038"},
		{ L"work_state",		L"LA"},
		{ L"work_country",		L"Unided States"},


		{ TEXT("phone1"), TEXT("318-624-4130") },
		{ TEXT("phonetype1"), TEXT("primaryphone") },
		{ TEXT("phonenote1"), TEXT("8:00 - 11:00") },

		{ TEXT("phone2"), TEXT("540-776-7077") },
		{ TEXT("phonetype2"), TEXT("homephone") },
		{ TEXT("phonenote2"), TEXT("Father's home") },

		{ TEXT("url"), TEXT("http://www.tordex.com") },
		{ TEXT("email"), TEXT("vmg@email.com") },
		{ TEXT("icq"), TEXT("11171065") },
		{ TEXT("gtalk"), TEXT("vmg@email.com") },
		{ TEXT("msn"), TEXT("vmg@email.com") },

		{ L"urlfacebook", L"http://www.facebook.com" },
		{ L"urltwitter", L"http://www.twitter.com" },
		{ L"urlvk", L"http://www.vk.com" },
		{ L"urlgplus", L"http://www.google.com" },
		{ L"urlyoutube", L"http://www.youtube.com" },
		{ L"urlinstagram", L"http://www.instagram.com" },
		{ L"urlflickr", L"http://www.flickr.com" },
		{ L"urllinkedin", L"http://www.linkedin.com" },
		{ L"urllivejournal", L"http://www.livejournal.com" },

		{ L"title", L"Dr." },
		{ L"role", L"Designer" },
		{ L"department", L"IT" },
		{ L"organization", L"Tordex" },

		{ TEXT("notes"), TEXT("True Launch Bar is the best\nwith Address Book plugin.") },

		{ NULL, NULL },
	};

	for (int i = 0; defVals[i].value; i++)
	{
		FIELD fld;
		fld.name = defVals[i].name;
		fld.value = defVals[i].value;
		m_fields.push_back(fld);
	}

	HRSRC hRes = FindResource(g_hInst, MAKEINTRESOURCE(IDR_PREVIEW_PHOTO), TEXT("PNG"));
	int resSZ = SizeofResource(g_hInst, hRes);
	LPBYTE img = new BYTE[resSZ];
	HGLOBAL hgRes = LoadResource(g_hInst, hRes);
	memcpy(img, LockResource(hgRes), resSZ);

	InitImage(img, resSZ);
}

bool CRecord::filter(LPCWSTR filter_str)
{
	for (const auto& fld : m_fields)
	{
		if (StrStrI(fld.value.c_str(), filter_str))
		{
			return true;
		}
	}
	return false;
}

CRecord& CRecord::operator=(const CRecord& val)
{
	ClearImage();
	m_ID = val.m_ID;
	m_flags = val.m_flags;
	if (val.m_image)
	{
		m_image = new BYTE[val.m_cbImage];
		memcpy(m_image, val.m_image, val.m_cbImage);
		m_cbImage = val.m_cbImage;
	}
	m_fields = val.m_fields;
	InitImage();
	return *this;
}

std::wstring CRecord::GetFieldValue(LPCWSTR fldName)
{
	auto fld = find_field(fldName);

	if (fld != m_fields.end())
	{
		return fld->value;
	}
	return std::wstring();
}

void CRecord::SetFieldValue(LPCWSTR fldName, LPCWSTR value)
{
	auto fld_iter = find_field(fldName);
	if (fld_iter != m_fields.end())
	{
		if (is_empty(value))
		{
			m_fields.erase(fld_iter);
		}
		else
		{
			fld_iter->value = value;
		}
	}
	else if (!is_empty(value))
	{
		FIELD fld;
		fld.value = value;
		fld.name = fldName;
		m_fields.push_back(fld);
	}
}

std::wstring CRecord::GetFieldLabel(LPCWSTR fldName, CXUIEngine* xui)
{
	BOOL found = FALSE;
	for (size_t i = 0; i < m_fields.size(); i++)
	{
		if (!StrCmpI(m_fields[i].name.c_str(), fldName))
		{
			return GetFieldLabel((int) i, xui);
		}
	}
	for (int i = 0; g_defText[i].fldName; i++)
	{
		if (!StrCmpI(g_defText[i].fldName, fldName))
		{
			if (g_defText[i].strID)
			{
				return std::wstring(xui->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
			}
		}
	}
	return std::wstring();
}

std::wstring CRecord::GetFieldLabel(int fldIDX, CXUIEngine* xui)
{
	if (m_fields[fldIDX].name.length() == 6 && !StrCmpN(m_fields[fldIDX].name.c_str(), L"phone", 5))
	{
		TCHAR phoneLabel[100];
		lstrcpy(phoneLabel, TEXT("phonetype"));
		lstrcat(phoneLabel, m_fields[fldIDX].name.c_str() + 5);
		return GetFieldLabel(phoneLabel, xui);
	}

	for (int i = 0; g_defText[i].fldName; i++)
	{
		if (!StrCmpI(g_defText[i].fldName, m_fields[fldIDX].name.c_str()))
		{
			if (g_defText[i].strID)
			{
				return std::wstring(xui->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
			}
			else
			{
				for (int j = 0; g_defText[j].fldName; j++)
				{
					if (!StrCmpI(g_defText[j].fldName, m_fields[fldIDX].value.c_str()))
					{
						return std::wstring(xui->getStringDef(g_defText[j].strID, g_defText[j].attr, g_defText[j].defText));
					}
				}
			}
		}
	}
	return std::wstring();
}

cairo_container::image_ptr CRecord::get_image(LPCWSTR url, images_cache* cache, int width, int height)
{
	cairo_container::image_ptr ret;
	if (!StrCmpI(url, TEXT("photo")))
	{
		ret = GetPhoto(width, height);
	}
	if (!ret)
	{
		if (!StrCmpNI(url, L"phonetype", 9))
		{
			std::wstring imgID = GetFieldValue(url);
			if (!imgID.empty())
			{
				imgID += L".png";
				ret = cache->getImage(imgID.c_str());
			}
		}
		if (!ret)
		{
			std::wstring imgID = url;
			if (imgID.find(L'.', 0) == std::wstring::npos)
			{
				imgID += L".png";
			}
			ret = cache->getImage(imgID.c_str());
		}
	}
	return ret;
}

FIELD::vector::iterator CRecord::find_field(LPCWSTR fldName)
{
	return std::find_if(m_fields.begin(), m_fields.end(),
		[&fldName](const FIELD& val)
		{
			if (!StrCmpI(val.name.c_str(), fldName))
			{
				return true;
			}
			return false;
		});
}

void CRecord::DeleteField(LPCWSTR fldName)
{
	auto fld = find_field(fldName);
	if (fld != m_fields.end())
	{
		m_fields.erase(fld);
	}
}

std::wstring CRecord::GetFieldType(LPCWSTR fldName)
{
	LPCWSTR typeFld = NULL;
	for (int i = 0; g_defText[i].fldName; i++)
	{
		if (!StrCmpI(fldName, g_defText[i].fldName))
		{
			if (g_defText[i].redirNameTo)
			{
				return GetFieldValue(g_defText[i].redirNameTo);
			}
		}
	}
	return std::wstring(fldName);
}
