#include "globals.h"
#include <Shlobj.h>
#include "XMLProps.h"

void get_templates_file(CTlbContainer* container, LPWSTR custom_tpls_file)
{
	container->GetFolder(custom_tpls_file, FOLDER_TYPE_USERPROFILE);
	PathAddBackslash(custom_tpls_file);
	StringCchCat(custom_tpls_file, MAX_PATH, L"Address Book");
	if (!PathFileExists(custom_tpls_file))
	{
		SHCreateDirectoryEx(NULL, custom_tpls_file, NULL);
	}
	StringCchCat(custom_tpls_file, MAX_PATH, L"\\templates.xml");
}


void load_templates_from_folder(CUSTOM_TEMPLATE::vector& tpls, LPCWSTR folder, LPCWSTR prefix)
{
	TCHAR pathBase[MAX_PATH];
	lstrcpy(pathBase, folder);
	PathAddBackslash(pathBase);

	WIN32_FIND_DATA fd;
	TCHAR findMask[MAX_PATH];
	lstrcpy(findMask, pathBase);
	lstrcat(findMask, TEXT("*"));

	HANDLE hFind = FindFirstFile(findMask, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (lstrcmp(fd.cFileName, TEXT(".")) && lstrcmp(fd.cFileName, TEXT("..")) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				std::wstring skin_path = prefix;
				skin_path += fd.cFileName;
				skin_path += L"\\abook.ini";

				std::wstring ini_path = pathBase;
				ini_path += fd.cFileName;
				ini_path += L"\\abook.ini";

				WCHAR name[255];
				GetPrivateProfileString(L"options", L"name", L"", name, 255, ini_path.c_str());

				tpls.emplace_back(name, L"", skin_path.c_str(), tpl_type_skin, 0);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}
}



CUSTOM_TEMPLATE::vector load_templates(CTlbContainer* container, bool custom_only)
{
	CUSTOM_TEMPLATE::vector out;

	WCHAR path[MAX_PATH];

	if (!custom_only)
	{
		container->GetFolder(path, FOLDER_TYPE_USERPROFILE);
		PathAddBackslash(path);
		StringCchCat(path, MAX_PATH, L"skins\\abook\\");
		load_templates_from_folder(out, path, L"{profile}");

		container->GetFolder(path, FOLDER_TYPE_COMMONPROFILE);
		PathAddBackslash(path);
		StringCchCat(path, MAX_PATH, L"skins\\abook\\");
		load_templates_from_folder(out, path, L"{common}");

		GetModuleFileName(g_hInst, path, MAX_PATH);
		PathRemoveFileSpec(path);
		PathAddBackslash(path);
		StringCchCat(path, MAX_PATH, L"skins\\");
		load_templates_from_folder(out, path, L"{app}");
	}

	get_templates_file(container, path);

	if (PathFileExists(path))
	{
		CXMLProps xml;
		if (xml.OpenFile(path, L"abook"))
		{
			if (xml.OpenNode(L"tpl"))
			{
				while (true)
				{
					auto name = std::auto_ptr<WCHAR>(xml.GetSTRValue(L"name", L""));
					auto base = std::auto_ptr<WCHAR>(xml.GetSTRValue(L"base", L""));
					out.emplace_back(name.get(), L"", base.get(), tpl_type_custom, xml.GetINTValue(L"id", 0));
					if (!xml.OpenNextNode()) break;
				}
			}
		}
	}

	return out;
}

std::wstring expand_skin_path(CTlbContainer* container, const std::wstring& path)
{
	std::wstring skin_path = path;
	if (!StrCmpNI(skin_path.c_str(), L"{profile}", 9))
	{
		WCHAR folder_path[MAX_PATH];

		container->GetFolder(folder_path, FOLDER_TYPE_USERPROFILE);
		PathAddBackslash(folder_path);
		StringCchCat(folder_path, MAX_PATH, L"skins\\abook\\");
		skin_path.replace(0, 9, folder_path);
	}
	else if (!StrCmpNI(skin_path.c_str(), L"{common}", 8))
	{
		WCHAR folder_path[MAX_PATH];

		container->GetFolder(folder_path, FOLDER_TYPE_COMMONPROFILE);
		PathAddBackslash(folder_path);
		StringCchCat(folder_path, MAX_PATH, L"skins\\abook\\");
		skin_path.replace(0, 8, folder_path);
	}
	else if (!StrCmpNI(skin_path.c_str(), L"{app}", 5))
	{
		WCHAR folder_path[MAX_PATH];

		GetModuleFileName(g_hInst, folder_path, MAX_PATH);
		PathRemoveFileSpec(folder_path);
		PathAddBackslash(folder_path);
		StringCchCat(folder_path, MAX_PATH, L"skins\\");
		skin_path.replace(0, 5, folder_path);
	}
	return skin_path;
}

std::wstring get_skin_name(CTlbContainer* container, const std::wstring& path)
{
	std::wstring skin_path = expand_skin_path(container, path);
	WCHAR name[255];
	GetPrivateProfileString(L"options", L"name", L"", name, 255, skin_path.c_str());
	return std::wstring(name);
}

void CUSTOM_TEMPLATE::load_text(CTlbContainer* container)
{
	switch (type)
	{
	case tpl_type_custom:
		if (id)
		{
			WCHAR xml_path[MAX_PATH];
			get_templates_file(container, xml_path);
			if (PathFileExists(xml_path))
			{
				CXMLProps xml;
				if (xml.OpenFile(xml_path, L"abook"))
				{
					WCHAR nodeQuery[255];
					wsprintf(nodeQuery, L"tpl[@id=\"%d\"]", id);
					if (xml.OpenNode(nodeQuery))
					{
						auto txt = std::auto_ptr<WCHAR>(xml.GetNodeText());
						if (txt.get())
						{
							text = txt.get();
						}
					}
				}
			}
		}
		break;
	case tpl_type_skin:
		{
			std::wstring skin_path = expand_skin_path(container, path);
			WCHAR tpl_path[MAX_PATH];
			StringCchCopy(tpl_path, MAX_PATH, skin_path.c_str());
			PathRemoveFileSpec(tpl_path);
			PathAddBackslash(tpl_path);
			StringCchCat(tpl_path, MAX_PATH, L"template.html");
			if (PathFileExists(tpl_path))
			{
				auto txt = std::auto_ptr<WCHAR>(load_utf8_file(tpl_path));
				if (txt.get())
				{
					text = txt.get();
				}
			}
		}
		break;
	default:
		break;
	}
	if (text.empty())
	{
		text = load_text_from_resource(L"default.html", L"TEMPLATE");
	}
}

bool CUSTOM_TEMPLATE::save(CTlbContainer* container)
{
	if (type != tpl_type_custom) return false;

	WCHAR xml_path[MAX_PATH];
	get_templates_file(container, xml_path);
	CXMLProps xml;
	bool is_ok = true;
	if (!xml.OpenFile(xml_path, L"abook"))
	{
		is_ok = false;
		if (xml.CreateFile(xml_path, L"abook"))
		{
			is_ok = true;
		}
	}
	if (is_ok)
	{
		is_ok = false;
		if (id != 0)
		{
			WCHAR nodeQuery[255];
			wsprintf(nodeQuery, L"tpl[@id=\"%d\"]", id);
			if (xml.OpenNode(nodeQuery))
			{
				is_ok = true;
			}
		}
		if (!is_ok)
		{
			id = xml.GetINTValue(L"next-id", 0) + 1;
			xml.SetINTValue(L"next-id", id);
			if (xml.CreateNode(L"tpl"))
			{
				is_ok = true;
			}
		}

		if (is_ok)
		{
			is_ok = false;
			xml.SetINTValue(L"id", id);
			xml.SetSTRValue(L"name", name.c_str());
			xml.SetSTRValue(L"base", path.c_str());
			xml.SetNodeText(text.c_str());
			if (xml.Save(NULL))
			{
				is_ok = true;
			}
		}
	}
	return is_ok;
}

bool CUSTOM_TEMPLATE::erase(CTlbContainer* container)
{
	if (type != tpl_type_custom || !id) return false;

	WCHAR xml_path[MAX_PATH];
	get_templates_file(container, xml_path);
	CXMLProps xml;
	if (xml.OpenFile(xml_path, L"abook"))
	{
		WCHAR nodeQuery[255];
		wsprintf(nodeQuery, L"tpl[@id=\"%d\"]", id);
		if (xml.OpenNode(nodeQuery))
		{
			xml.RemoveNode();
			if (xml.Save(NULL))
			{
				return true;
			}
		}
	}
	return false;
}

std::wstring CUSTOM_TEMPLATE::get_base_path(CTlbContainer* container)
{
	std::wstring ret;
	if (!path.empty())
	{
		ret = expand_skin_path(container, path);
		size_t pos = ret.find_last_of(L"\\");
		if (pos != std::wstring::npos)
		{
			ret.erase(pos + 1);
		}
	}
	return ret;
}
