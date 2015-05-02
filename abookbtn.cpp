#include "globals.h"
#include "abookbtn.h"
#include "resource.h"
#include "RecordBtn.h"
#include "ContactDlg.h"
#include "AlphaGroupBtn.h"
#include "FieldGroupBtn.h"
#include "PropertiesDlg.h"
#include "xuistrings.h"
#include <WinInet.h>
#include "ImportDlg.h"
#include "XMLProps.h"
#include <Shlobj.h>

CAddressBook::CAddressBook() : m_xui(g_hInst)
{
	m_menuImgImport.load(FindResource(g_hInst, MAKEINTRESOURCE(IDR_IMPORT), TEXT("PNG")), g_hInst);
	m_menuImgNew.load(FindResource(g_hInst, MAKEINTRESOURCE(IDR_NEW), TEXT("PNG")), g_hInst);

	m_interactiveTips = NULL;
	m_photoSize.cx = 100;
	m_photoSize.cy = 100;
	m_Description = NULL;
	m_StartMenuID = 0;
	lstrcpy(m_groupField, L"category");
	m_groupType = GROUP_TYPE_NONE;
	m_defFont[0] = 0;
	m_defFontSize = 0;
	m_hkNew = 0;
	m_hkMenu = 0;
	m_tipIcon = NULL;
	m_version = 0;
	m_nexID = 1;
	m_changesHandle = NULL;

	m_template.text = load_text_from_resource(L"default.html", L"TEMPLATE");

	InitializeCriticalSection(&m_sync);
	load_master_css();
}

CAddressBook::~CAddressBook()
{
	PostThreadMessage(CTxThread::getID(), WM_QUIT, 0, 0);

	CTxThread::Stop();
	
	if(m_tipIcon)
	{
		DestroyIcon(m_tipIcon);
		m_tipIcon = NULL;
	}
	if(m_tipIcon) DestroyIcon(m_tipIcon);
	if(m_hIcon) DestroyIcon(m_hIcon);
	if(m_pngIcon)
	{
		delete m_pngIcon;
		m_pngIcon = NULL;
	}
}

void CAddressBook::ActivateWindow(HWND wndActivate)
{
	if(!IsWindow(wndActivate)) return;

	HWND wnd = GetForegroundWindow();

	int iMyTID;
	int iCurrTID;

	iMyTID   = GetCurrentThreadId();
	iCurrTID = GetWindowThreadProcessId(wndActivate,0);

	AttachThreadInput(iCurrTID, iMyTID, TRUE);

	::SetForegroundWindow(wndActivate);

	AttachThreadInput(iCurrTID, iMyTID, FALSE);
}


void CAddressBook::load_master_css()
{
	LPWSTR css = NULL;

	HRSRC hResource = ::FindResource(g_hInst, L"master.css", L"CSS");
	if (hResource)
	{
		DWORD imageSize = ::SizeofResource(g_hInst, hResource);
		if (imageSize)
		{
			LPCSTR pResourceData = (LPCSTR) ::LockResource(::LoadResource(g_hInst, hResource));
			if (pResourceData)
			{
				css = new WCHAR[imageSize * 3];
				int ret = MultiByteToWideChar(CP_UTF8, 0, pResourceData, imageSize, css, imageSize * 3);
				css[ret] = 0;
			}
		}
	}
	if (css)
	{
		m_web_context.load_master_stylesheet(css);
		delete css;
	}
}

void CAddressBook::init_font()
{
	HDC hdc = GetDC(NULL);
	m_defFontSizePx = MulDiv(m_defFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
}

BOOL CAddressBook::Save(IStream *data)
{ 
	CPluginStream stream;
	stream.BeginNode(TEXT("settings"));
		stream.SaveDWORD(TEXT("version"),		m_version);
		stream.SaveDWORD(TEXT("nexID"),			m_nexID);
		stream.SaveWORD( TEXT("hkNew"),			m_hkNew);
		stream.SaveWORD( TEXT("hkMenu"),		m_hkMenu);
		stream.SaveDWORD(TEXT("defFontSize"),	m_defFontSize);
		stream.SaveString(TEXT("defFont"),		m_defFont);
		stream.SaveDWORD(TEXT("groupType"),		m_groupType);
		stream.SaveString(TEXT("groupField"),	m_groupField);
		stream.SaveString(TEXT("iconLocation"),	m_iconLocation);
		stream.SaveString(TEXT("template"),		m_template.text.c_str());
		stream.SaveDWORD(TEXT("tpl_id"),		m_template.id);
		stream.SaveDWORD(TEXT("tpl_type"),		(DWORD) m_template.type);
		stream.SaveString(TEXT("tpl_path"),		m_template.path.c_str());
		stream.SaveString(TEXT("tpl_name"),		m_template.name.c_str());
		stream.SaveString(TEXT("description"), m_Description);

		stream.SaveDWORD( TEXT("photoCX"),		m_photoSize.cx);
		stream.SaveDWORD( TEXT("photoCY"),		m_photoSize.cy);

		stream.SaveDWORD(TEXT("interactive"), m_interactiveTips);
	stream.EndNode();

	stream.BeginNode(TEXT("templates"));
		stream.SaveDWORD(TEXT("count"), (DWORD) m_templates.size());
		for(int i=0; i < (int) m_templates.size(); i++)
		{
			TCHAR valName[255];
			wsprintf(valName, TEXT("tpl#%d"), i);
			stream.SaveString(valName,	m_templates[i].text.c_str());
			wsprintf(valName, TEXT("name#%d"), i);
			stream.SaveString(valName, m_templates[i].name.c_str());
			wsprintf(valName, TEXT("path#%d"), i);
			stream.SaveString(valName, m_templates[i].path.c_str());
		}
	stream.EndNode();

	stream.BeginNode(TEXT("records"));
		stream.SaveDWORD(TEXT("count"),	(DWORD) m_records.size());

		for(int i=0; i < (int) m_records.size(); i++)
		{
			TCHAR nodeName[100];
			wsprintf(nodeName, TEXT("##%d"), i);
			stream.BeginNode(nodeName);
				m_records[i]->Save(&stream);
			stream.EndNode();
		}
	stream.EndNode();

	stream.BeginNode(TEXT("cmdline"));
		stream.SaveDWORD(TEXT("count"),	(DWORD) m_progs.size());

		for(size_t i=0; i < m_progs.size(); i++)
		{
			TCHAR nodeName[100];
			wsprintf(nodeName, TEXT("##%d"), (UINT) i);
			stream.BeginNode(nodeName);
				stream.SaveString(L"field",	m_progs[i].getField().c_str());
				stream.SaveString(L"prog",	m_progs[i].getCmdLine().c_str());
			stream.EndNode();
		}
	stream.EndNode();

	stream.Save(data);

	return TRUE; 
}

BOOL CAddressBook::Load(IStream *data)
{ 
	CPluginStream stream;
	if(stream.Load(data))
	{
		if(stream.OpenNode(TEXT("settings")))
		{
			m_version = stream.GetDWORD(TEXT("version"));
			m_nexID = stream.GetDWORD(TEXT("nexID"));
			m_hkNew = stream.GetWORD( TEXT("hkNew"));
			m_hkMenu = stream.GetWORD( TEXT("hkMenu"));
			m_defFontSize = stream.GetDWORD(TEXT("defFontSize"));
			StringCchCopy(m_defFont, 32, stream.GetString(TEXT("defFont"), TEXT("")));
			m_groupType = stream.GetDWORD(TEXT("groupType"));
			StringCchCopy(m_groupField, 255, stream.GetString(TEXT("groupField"), TEXT("")));
			MAKE_STR(m_iconLocation, stream.GetString(TEXT("iconLocation")));
			m_template.id = stream.GetDWORD(L"tpl_id", 0);
			m_template.type = (tpl_type) stream.GetDWORD(L"tpl_type", tpl_type_custom);
			m_template.text = stream.GetString(TEXT("template"), L"");
			m_template.name = stream.GetString(TEXT("tpl_name"), L"");
			m_template.path = stream.GetString(TEXT("tpl_path"), L"");
			MAKE_STR(m_Description, stream.GetString(TEXT("description"), m_xui.getStringDef(XUI_DEF_DESCRIPTION)));

			m_photoSize.cx = stream.GetDWORD(TEXT("photoCX"), 100);
			m_photoSize.cy = stream.GetDWORD(TEXT("photoCY"), 100);
			m_interactiveTips = stream.GetDWORD(TEXT("interactive"), FALSE);

			stream.CloseNode();
		}
	}

	if(stream.OpenNode(TEXT("templates"))) 
	{
		int cnt = stream.GetDWORD(TEXT("count"));
		for(int i=0; i < cnt; i++)
		{
			TCHAR valName[255];
			TCHAR tplName[255];
			TCHAR tplBase[255];
			wsprintf(tplName, TEXT("tpl#%d"), i);
			wsprintf(valName, TEXT("name#%d"), i);
			wsprintf(tplBase, TEXT("path#%d"), i);
			m_templates.emplace_back(stream.GetString(valName, L""), stream.GetString(tplName, L""), stream.GetString(tplBase, L""), tpl_type_custom, 0);
		}

		stream.CloseNode();
	}

	if(stream.OpenNode(TEXT("records"))) 
	{
		int count = stream.GetDWORD(TEXT("count"));
		for(int i=0; i < count; i++)
		{
			TCHAR nodeName[100];
			wsprintf(nodeName, TEXT("##%d"), i);
			stream.OpenNode(nodeName);
				CRecord::ptr rec(new CRecord);
				rec->Load(&stream);
				m_records.push_back(rec);
			stream.CloseNode();
		}
		stream.CloseNode();
	}

	if(stream.OpenNode(TEXT("cmdline"))) 
	{
		int count = stream.GetDWORD(TEXT("count"));
		for(int i=0; i < count; i++)
		{
			TCHAR nodeName[100];
			wsprintf(nodeName, TEXT("##%d"), i);
			stream.OpenNode(nodeName);

			CProgram prg;
			prg.setField(stream.GetString(L"field", TEXT("")));
			prg.setCmdLine(stream.GetString(L"prog", TEXT("")));

			m_progs.push_back(prg);

			stream.CloseNode();
		}
		stream.CloseNode();
	}

	m_template.load_text(m_container);
	m_images.init_by_template(m_template, m_container);
	init_font();
	m_container->SetHotKey(m_hkNew, HOTKEY_NEWCONTACT);

	CTxThread::Run();
	
	return TRUE; 
}

UINT CAddressBook::GetModeFlags(void)
{
	return BTN_FLAG_SUPPORTSAVEDICON | BTN_FLAG_SUPPORTPNGICONS;
}

BOOL CAddressBook::OnProperties() 
{ 
	if(m_container)
	{
		m_container->SetHotKey(0, HOTKEY_NEWCONTACT);
		m_container->SetHotKey(0, HOTKEY_MENU);
		if(openProperties())
		{
			init_font();
			m_container->SetHotKey(m_hkNew, HOTKEY_NEWCONTACT);
			m_container->SetHotKey(m_hkMenu, HOTKEY_MENU);
			m_container->UpdateTips();
			m_container->SaveButton();
			return TRUE;
		}
		m_container->SetHotKey(m_hkNew, HOTKEY_NEWCONTACT);
		m_container->SetHotKey(m_hkMenu, HOTKEY_MENU);
	}
	return FALSE;
}

UINT CAddressBook::GetSupportedActions(void)
{
	return CTMS_PROPERTIES | CTMS_CHANGEICON | CTMS_RESETICON;
}

void CAddressBook::GetSize(SIZE* sz, BOOL actual)
{
	// TODO: Set the size of plugin button here
}

BOOL CAddressBook::OnCreate()
{
	if(m_container)
	{
		MAKE_STR(m_Description, m_xui.getStringDef(XUI_DEF_DESCRIPTION));

		NONCLIENTMETRICS ncm;
		ZeroMemory(&ncm, sizeof(ncm));
		ncm.cbSize = sizeof(ncm);
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
		lstrcpy(m_defFont, ncm.lfMenuFont.lfFaceName);
		//m_defFontSize = ncm.lfMenuFont.lfHeight;
		if(ncm.lfMenuFont.lfHeight < 0)
		{
			HDC hDC = GetDC(NULL);
			m_defFontSize = MulDiv(-ncm.lfMenuFont.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY));
			ReleaseDC(NULL, hDC);
		} else
		{
			m_defFontSize = 9;
		}

		return openProperties();
	}
	return FALSE;
}

HICON CAddressBook::GetTipIcon(int tipID)
{
	if(!m_tipIcon)
	{
		m_tipIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);
	}
	return m_tipIcon;
}

LPWSTR CAddressBook::GetTipCaption(int tipID)
{
	LPWSTR name = (LPWSTR) CoTaskMemAlloc((lstrlen(m_Name) + 1) * sizeof(TCHAR));
	lstrcpy(name, m_Name);
	return name;
}

LPWSTR CAddressBook::GetTipText(int tipID)
{
	LPWSTR name = (LPWSTR) CoTaskMemAlloc((lstrlen(m_Description) + 1) * sizeof(TCHAR));
	lstrcpy(name, m_Description);
	return name;
}

BOOL CAddressBook::isMenu(void)
{
	return TRUE;
}

UINT CAddressBook::GetMenuFlags(void)
{
	return MENU_F_NORENAME | MENU_F_VARHEIGHT;
}

BOOL CAddressBook::LoadIcon(int size)
{
	if(!CTlbButton::LoadIcon(size))
	{
		m_hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
		m_loadedIconSize = size;
	}
	return m_hIcon ? TRUE : FALSE;
}

BOOL CAddressBook::SetIconLocation(LPCWSTR szLocation)
{
	if(m_tipIcon) DestroyIcon(m_tipIcon);
	m_tipIcon = NULL;
	if(m_hIcon) DestroyIcon(m_hIcon);
	m_hIcon = NULL;
	if(m_pngIcon)
	{
		delete m_pngIcon;
		m_pngIcon = NULL;
	}
	return CTlbButton::SetIconLocation(szLocation);
}

HANDLE CAddressBook::GetChangesHandle() 
{ 
	if(!m_changesHandle)
	{
		m_changesHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	return m_changesHandle; 
}

void CAddressBook::PrepareChanges(HANDLE hChanges)
{
	ResetEvent(hChanges);
}

void CAddressBook::CloseChangesHandle(HANDLE hChanges)
{
	CloseHandle(hChanges);
	m_changesHandle = NULL;
}

struct AB_ENUM_DATA
{
	DWORD						groupType;
	std::wstring				groupField;
	std::wstring				chars;
	std::vector<std::wstring>	values;
	std::vector<UINT>			nonGroup;
	int							lastID;

	AB_ENUM_DATA() : groupType(0), lastID(0)
	{

	}

	~AB_ENUM_DATA()
	{
	}
};

struct AB_CHILD_DATA
{
	DWORD			groupType;
	std::wstring	groupField;
	WCHAR			chr;
	std::wstring	value;
	UINT			id;

	AB_CHILD_DATA() : groupType(0), chr(0), id(0)
	{

	}

	~AB_CHILD_DATA()
	{
	}
};

BOOL CAddressBook::FirstChildData(CHILDS_DATA* data)
{
	//m_maxWidth = 0;
	if(m_records.empty()) return FALSE;

	AB_ENUM_DATA* enumData = new AB_ENUM_DATA;
	enumData->groupType = m_groupType;
	switch(m_groupType)
	{
	case GROUP_TYPE_NONE:
		{
			LockItems();
			for(auto& rec : m_records)
			{
				enumData->nonGroup.push_back(rec->m_ID);
			}	
			UnlockItems();
			if (!enumData->nonGroup.empty())
			{
				UINT id = enumData->nonGroup[0];
				if (data->Name)
				{
					wsprintf(data->Name, TEXT("rec:%d"), id);
				}
				enumData->lastID = 0;
				AB_CHILD_DATA* childData = new AB_CHILD_DATA;
				childData->groupType = enumData->groupType;
				childData->id = id;
				data->childData = (LPVOID)childData;
			}
			else
			{
				delete enumData;
				return FALSE;
			}
		}
		break;
	case GROUP_TYPE_ALPHA:
		{
			enumData->chars = GetFirstLetters();
			enumData->lastID = 0;
			AB_CHILD_DATA* childData = new AB_CHILD_DATA;
			childData->groupType = enumData->groupType;
			childData->chr = enumData->chars[enumData->lastID];
			data->childData = (LPVOID) childData;
			if(data->Name)
			{
				lstrcpy(data->Name, TEXT("letter: "));
				data->Name[7] = enumData->chars[enumData->lastID];
			}
		}
		break;
	case GROUP_TYPE_FIELD:
		{
			enumData->values = GetValuesOfField(m_groupField);
			enumData->nonGroup = GetFieldEmptyRecords(m_groupField);
			enumData->groupField = m_groupField;
			if(!enumData->values.empty())
			{
				enumData->lastID = 0;
				AB_CHILD_DATA* childData = new AB_CHILD_DATA;
				childData->groupType = enumData->groupType;
				childData->groupField = enumData->groupField;
				childData->value = enumData->values[enumData->lastID];
				data->childData = (LPVOID) childData;
				if(data->Name)
				{
					StringCchPrintf(data->Name, 255, L"grp:%s:%s", childData->groupField.c_str(), childData->value.c_str());
					CharLower(data->Name);
				}
			} else
			{
				enumData->lastID = -1;
				UINT id = enumData->nonGroup[-enumData->lastID - 1];
				AB_CHILD_DATA* childData = new AB_CHILD_DATA;
				childData->groupType = GROUP_TYPE_NONE;
				childData->id = id;
				data->childData = (LPVOID) childData;
				if(data->Name)
				{
					wsprintf(data->Name, TEXT("rec:%d"), id);
				}
			}
		}
		break;
	default:
		delete enumData;
		return FALSE;
	}
	data->enumData = (LPVOID) enumData;
	return TRUE;
}

BOOL CAddressBook::NextChildData(CHILDS_DATA* data)
{
	AB_ENUM_DATA* enumData = (AB_ENUM_DATA*) data->enumData;
	switch(enumData->groupType)
	{
	case GROUP_TYPE_NONE:
		{
			enumData->lastID++;
			if (enumData->lastID >= (int)enumData->nonGroup.size())
			{
				delete enumData;
				data->enumData = NULL;
				return FALSE;
			}
			UINT id = enumData->nonGroup[enumData->lastID];
			if (data->Name)
			{
				wsprintf(data->Name, TEXT("rec:%d"), id);
			}
			AB_CHILD_DATA* childData = new AB_CHILD_DATA;
			childData->groupType = enumData->groupType;
			childData->id = id;
			data->childData = (LPVOID) childData;
		}
		break;
	case GROUP_TYPE_ALPHA:
		{
			enumData->lastID++;
			if (enumData->lastID >= (int)enumData->chars.length())
			{
				delete enumData;
				data->enumData = NULL;
				return FALSE;
			}
			AB_CHILD_DATA* childData = new AB_CHILD_DATA;
			childData->groupType = enumData->groupType;
			childData->chr = enumData->chars[enumData->lastID];
			data->childData = (LPVOID) childData;
			if(data->Name)
			{
				lstrcpy(data->Name, TEXT("letter: "));
				data->Name[7] = enumData->chars[enumData->lastID];
			}
		}
		break;
	case GROUP_TYPE_FIELD:
		{
			BOOL isFirstNonGroup = FALSE;
			if (enumData->lastID >= 0)
			{
				enumData->lastID++;
				if (enumData->lastID < (int)enumData->values.size())
				{
					AB_CHILD_DATA* childData = new AB_CHILD_DATA;
					childData->groupType = enumData->groupType;
					childData->groupField = enumData->groupField;
					childData->value = enumData->values[enumData->lastID];
					data->childData = (LPVOID) childData;
					if(data->Name)
					{
						StringCchPrintf(data->Name, 255, L"grp:%s:%s", childData->groupField.c_str(), childData->value.c_str());
						CharLower(data->Name);
					}
				} else
				{
					enumData->lastID = -1;
					isFirstNonGroup = TRUE;
				}
			}
			if(enumData->lastID < 0)
			{
				if(!isFirstNonGroup) enumData->lastID--;
				if ((-enumData->lastID - 1) >= enumData->nonGroup.size())
				{
					delete enumData;
					data->enumData = NULL;
					return FALSE;
				}
				UINT id = enumData->nonGroup[-enumData->lastID - 1];
				AB_CHILD_DATA* childData = new AB_CHILD_DATA;
				childData->groupType = GROUP_TYPE_NONE;
				childData->id = id;
				data->childData = (LPVOID) childData;
				if(data->Name)
				{
					wsprintf(data->Name, TEXT("rec:%d"), id);
				}
			}
		}
		break;
	default:
		delete enumData;
		data->enumData = NULL;
		return FALSE;
	}
	return TRUE;
}

BOOL CAddressBook::FreeChildData(CHILDS_DATA* data)
{
	AB_CHILD_DATA* childData = (AB_CHILD_DATA*) data->childData;
	delete childData;
	return TRUE;
}

BOOL CAddressBook::CreateChild(CREATE_CHILD_DATA* childData)
{
	AB_CHILD_DATA* data = (AB_CHILD_DATA*) childData->childData;
	switch(data->groupType)
	{
	case GROUP_TYPE_NONE:
		{
			CRecordBtn* btn = new CRecordBtn(this, data->id, this);
			childData->child = btn;
		}
		return TRUE;
	case GROUP_TYPE_ALPHA:
		{
			CAlphaGroupBtn* btn = new CAlphaGroupBtn(this, data->chr);
			childData->child = btn;
		}
		return TRUE;
	case GROUP_TYPE_FIELD:
		{
			CFieldGroupBtn* btn = new CFieldGroupBtn(this, data->groupField.c_str(), data->value.c_str());
			childData->child = btn;
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CAddressBook::FirstChildDataF(CHILDS_DATA* data, LPWSTR filter, HANDLE hStop)
{
	if (m_records.empty()) return FALSE;

	AB_ENUM_DATA* enumData = new AB_ENUM_DATA;
	enumData->groupType = GROUP_TYPE_NONE;

	LockItems();
	for (auto& rec : m_records)
	{
		if (rec->filter(filter))
		{
			enumData->nonGroup.push_back(rec->m_ID);
		}
	}
	UnlockItems();
	if (!enumData->nonGroup.empty())
	{
		UINT id = enumData->nonGroup[0];
		if (data->Name)
		{
			wsprintf(data->Name, TEXT("rec:%d"), id);
		}
		enumData->lastID = 0;
		AB_CHILD_DATA* childData = new AB_CHILD_DATA;
		childData->groupType = enumData->groupType;
		childData->id = id;
		data->childData = (LPVOID)childData;

		data->enumData = (LPVOID)enumData;

		return TRUE;
	}
	else
	{
		delete enumData;
	}
	return FALSE;
}

BOOL CAddressBook::NextChildDataF(CHILDS_DATA* data, LPWSTR filter, HANDLE hStop)
{
	AB_ENUM_DATA* enumData = (AB_ENUM_DATA*)data->enumData;

	enumData->lastID++;
	if (enumData->lastID >= (int)enumData->nonGroup.size())
	{
		delete enumData;
		data->enumData = NULL;
		return FALSE;
	}
	UINT id = enumData->nonGroup[enumData->lastID];
	if (data->Name)
	{
		wsprintf(data->Name, TEXT("rec:%d"), id);
	}
	AB_CHILD_DATA* childData = new AB_CHILD_DATA;
	childData->groupType = enumData->groupType;
	childData->id = id;
	data->childData = (LPVOID)childData;

	return TRUE;
}

BOOL CAddressBook::SupportSearchMenu()
{
	return TRUE;
}

int CAddressBook::QueryContextMenu(HMENU hMenu, int index, int cmdFirst, int cmdLast)
{
	m_StartMenuID = cmdFirst;
	if(isVista())
	{
		tlbInsertMenuStringItem(hMenu, cmdFirst + MID_ADDCONTACT, index, m_xui.getStringDef(XUI_MENU_NEWCONTACT), FALSE, 0, 0, NULL, m_menuImgNew.createBitmap());
		tlbInsertMenuStringItem(hMenu, cmdFirst + MID_IMPORT, index + 1, m_xui.getStringDef(XUI_MENU_IMPORT), FALSE, 0, 0, NULL, m_menuImgImport.createBitmap());
	} else
	{
		tlbInsertMenuStringItem(hMenu, cmdFirst + MID_ADDCONTACT, index, m_xui.getStringDef(XUI_MENU_NEWCONTACT), FALSE, 0, 0, NULL, NULL);
		tlbInsertMenuStringItem(hMenu, cmdFirst + MID_IMPORT, index + 1, m_xui.getStringDef(XUI_MENU_IMPORT), FALSE, 0, 0, NULL, NULL);
	}

	return 2;
}

BOOL CAddressBook::OnContextMenuCommand(int ID)
{
	switch(ID)
	{
	case MID_ADDCONTACT:
		onNewContact();
		break;
	case MID_IMPORT:
		onImport();
		break;
	}
	return TRUE;
}

std::wstring CAddressBook::GetText(UINT recID, LPCWSTR tpl)
{
	std::wstring out;
	if (is_empty(tpl))
	{
		out = m_template.text;
	}
	else
	{
		out = tpl;
	}
	LockItems();
	auto iter = find_record(recID);
	if (iter != m_records.end())
	{
		(*iter)->ApplyTemplate(out, &m_xui, m_progs);
	}
	UnlockItems();

	return out;
}

std::wstring CAddressBook::GetBasePath()
{
	return m_template.get_base_path(m_container);
}

void CAddressBook::onNewContact(void)
{
	PostThreadMessage(CTxThread::getID(), WM_EDITRECORD, 0, (LPARAM) ((IRecordParent*) this));
}

CRecord::vector::iterator CAddressBook::find_record(UINT recID)
{
	auto iter = std::find_if(m_records.begin(), m_records.end(), [&recID](CRecord::ptr& rec)
	{
		if (rec->m_ID == recID)
		{
			return true;
		}
		return false;
	});
	return iter;
}

BOOL CAddressBook::OnSetLCID(DWORD dwLCID, HMODULE hInstance)
{
	TCHAR szLCID[20];
	wsprintf(szLCID, TEXT("%d"), dwLCID);
	BOOL ret = CTlbButton::OnSetLCID(dwLCID, hInstance);

	TCHAR langPath[MAX_PATH];
	m_container->GetFolder(langPath, FOLDER_TYPE_TLBDLL);
	PathAddBackslash(langPath);
	lstrcat(langPath, L"langs\\");
	lstrcat(langPath, szLCID);
	lstrcat(langPath, L"\\abook.xml");
	m_xui.clearStrings();
	m_xui.loadStrings(langPath);

	return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

AB_FIELDS g_defText[] = 
{
	{XUI_FLD_PHOTO,				L"photo",				FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},

	{XUI_FLD_ADR_POBOX,			L"home_pobox",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_ADDRESS1,		L"home_address",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_ADDRESS2,		L"home_address2",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_CITY,			L"home_city",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_ZIP,			L"home_zip",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_STATE,			L"home_state",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},
	{XUI_FLD_ADR_COUNTRY,		L"home_country",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_HOMEADDR},

	{XUI_FLD_ADR_B_POBOX,		L"work_pobox",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_ADDRESS1,	L"work_address",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_ADDRESS2,	L"work_address2",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_CITY,		L"work_city",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_ZIP,			L"work_zip",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_STATE,		L"work_state",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	{XUI_FLD_ADR_B_COUNTRY,		L"work_country",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_BUSINESSADDR},
	
	{XUI_FLD_ADR_TITLE,			L"title",				TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_ORG},
	{XUI_FLD_ADR_ROLE,			L"role",				TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_ORG},
	{XUI_FLD_ADR_DEPARTMENT,	L"department",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_ORG},
	{XUI_FLD_ADR_ORGANIZATION,	L"organization",		TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_ORG},

	{XUI_FLD_FIRSTNAME,			L"firstname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_LASTNAME,			L"lastname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_MIDDLENAME,		L"middlename",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_PREFIXNAME,		L"prefixname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_SUFFIXNAME,		L"suffixname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_DISPLAYNAME,		L"displayname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},
	{XUI_FLD_NICKNAME,			L"nickname",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NAME},

	{XUI_FLD_PHONEWORK,			L"workphone",			FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONEHOME,			L"homephone",			FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONEFAX,			L"fax",					FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONEMOBILE,		L"mobile",				FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONEPAGER,		L"pager",				FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE,				L"primaryphone",		FALSE,	FALSE,	NULL,	TRUE,	XUI_GROUP_NONE},

	{XUI_FLD_NOTES,				L"notes",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_CATEGORY,			L"category",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_NONE},

	{XUI_FLD_URL,				L"url",					TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_EMAIL,				L"email",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_EMAILADD,			L"emailadd",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLHOME,			L"urlhome",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLWORK,			L"urlwork",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLBLOG,			L"urlblog",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLPROFILE,		L"urlprofile",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},

	{XUI_FLD_URLFACEBOOK,		L"urlfacebook",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLTWITTER,		L"urltwitter",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLVK,				L"urlvk",				TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLGPLUS,			L"urlgplus",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLYOUTUBE,		L"urlyoutube",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLINSTAGRAM,		L"urlinstagram",		TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLFLICKR,			L"urlflickr",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLLINKEDIN,		L"urllinkedin",			TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},
	{XUI_FLD_URLLIVEJOURNAL,	L"urllivejournal",		TRUE,	TRUE,	NULL,	FALSE,	XUI_GROUP_INTERNET},

	{XUI_FLD_IM,				L"im",					TRUE,	FALSE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_ICQ,				L"icq",					TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_GOOGLE_TALK,		L"gtalk",				TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_AIM,				L"aim",					TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_YAHOO,				L"yahoo",				TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_SKYPE,				L"skype",				TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_QQ,				L"qq",					TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_MSN,				L"msn",					TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},
	{XUI_FLD_JABBER,			L"jabber",				TRUE,	TRUE,	NULL,	TRUE,	XUI_GROUP_IM},

	{XUI_FLD_PHONE1,			L"phonetype1",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE2,			L"phonetype2",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE3,			L"phonetype3",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE4,			L"phonetype4",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE5,			L"phonetype5",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE6,			L"phonetype6",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},
	{XUI_FLD_PHONE7,			L"phonetype7",			FALSE,	TRUE,	NULL,	FALSE,	XUI_GROUP_NONE},

	{XUI_FLD_PHONENOTE1,		L"phonenote1",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE2,		L"phonenote2",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE3,		L"phonenote3",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE4,		L"phonenote4",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE5,		L"phonenote5",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE6,		L"phonenote6",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONENOTE7,		L"phonenote7",			TRUE,	FALSE,	NULL,	FALSE,	XUI_GROUP_PHONE},

	{XUI_FLD_PHONE1,			L"phone1",				TRUE,	FALSE,	L"phonetype1",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE2,			L"phone2",				TRUE,	FALSE,	L"phonetype2",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE3,			L"phone3",				TRUE,	FALSE,	L"phonetype3",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE4,			L"phone4",				TRUE,	FALSE,	L"phonetype4",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE5,			L"phone5",				TRUE,	FALSE,	L"phonetype5",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE6,			L"phone6",				TRUE,	FALSE,	L"phonetype6",	FALSE,	XUI_GROUP_PHONE},
	{XUI_FLD_PHONE7,			L"phone7",				TRUE,	FALSE,	L"phonetype7",	FALSE,	XUI_GROUP_PHONE},

	{NULL, NULL, NULL,			NULL,					FALSE,	FALSE,	NULL,			FALSE,	NULL, NULL, NULL}
};



int CAddressBook::GetHotkeysCount(void)
{
	return 2;
}

BOOL CAddressBook::GetHotKeyData(int idx, HOTKEY_DATA* hkData)
{
	switch(idx)
	{
	case 0:
		hkData->hotKey = m_hkNew;
		hkData->hotKeyID = HOTKEY_NEWCONTACT;
		hkData->id = HOTKEY_NEWCONTACT;
		StringCchCopy(hkData->name, 260, m_xui.getStringDef(XUI_HK_NEWCONTACT));
		break;
	case 1:
		hkData->hotKey = m_hkMenu;
		hkData->hotKeyID = HOTKEY_MENU;
		hkData->id = HOTKEY_MENU;
		StringCchCopy(hkData->name, 260, m_xui.getStringDef(XUI_HK_OPEN));
		break;
	}
	return TRUE;
}

void CAddressBook::OnHotkey(UINT hkID)
{
	switch(hkID)
	{
	case HOTKEY_NEWCONTACT:
		onNewContact();
		break;
	case HOTKEY_MENU:
		m_container->ShowMenu();
		break;
	}
}

void CAddressBook::onImport(void)
{
	LPWSTR fileName = new WCHAR[10240];
	fileName[0] = 0;
	OPENFILENAMEW ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = g_hInst;
	ofn.lpstrFilter = TEXT("vCard files (*.vcf)\0*.vcf\0");
	ofn.lpstrCustomFilter = NULL;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = 10240;
	ofn.lpstrTitle = m_xui.getStringDef(XUI_TITLE_IMPORT);
	ofn.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
	if(GetOpenFileNameW(&ofn))
	{
		CImportDlg dlg(&m_xui, this);
		if(ofn.nFileExtension)
		{
			dlg.addFile(fileName);
		} else
		{
			WCHAR path[MAX_PATH];
			LPWSTR flName = fileName + ofn.nFileOffset;
			while(flName[0])
			{
				lstrcpy(path, fileName);
				PathAddBackslash(path);
				lstrcat(path, flName);
				dlg.addFile(flName);
				flName += lstrlen(flName) + 1;
			}
		}
		if(dlg.DoModal(m_container->GetToolbarWindow()) == IDOK)
		{
			for (size_t i = 0; i < dlg.m_records.size(); i++)
			{
				if(dlg.m_actions[i] == IMPORT_ACT_UPDATE)
				{
					UpdateRecord(dlg.m_records[i]->m_ID, *dlg.m_records[i], FALSE);
				} else if(dlg.m_actions[i] == IMPORT_ACT_IMPORT)
				{
					UpdateRecord(0, *dlg.m_records[i], FALSE);
				}
			}
			m_container->SaveButton();
			SetEvent(m_changesHandle);
		}
	}
	delete fileName;
}


std::wstring CAddressBook::GetFirstLetters(void)
{
	std::wstring out;

	LockItems();

	std::for_each(m_records.begin(), m_records.end(), [&out](CRecord::ptr& rec)
	{
		std::wstring displayName = rec->GetFieldValue(L"displayname");
		while (!displayName.empty() && !IsCharAlphaNumeric(displayName[0]))
		{
			displayName.erase(0, 1);
		}
		if (displayName.empty())
		{
			displayName = L"?";
		}
		if (IsCharNumeric(displayName[0]))
		{
			displayName = L"1";
		}
		auto iter = std::find_if(out.begin(), out.end(), [&displayName](const wchar_t& chr)
		{
			wchar_t str[2];
			str[0] = chr;
			str[1] = 0;
			if (!StrCmpNI(str, displayName.c_str(), 1))
			{
				return true;
			}
			return false;
		});
		if (iter == out.end())
		{
			out += displayName[0];
		}
	});

	UnlockItems();

	return out;
}

std::vector<UINT> CAddressBook::GetRecordsByFirstLetter(TCHAR alpha)
{
	std::vector<UINT> out;

	LockItems();

	TCHAR str[2];
	str[0] = alpha;
	str[1] = 0;

	std::for_each(m_records.begin(), m_records.end(), [&out, &str](CRecord::ptr& rec)
	{
		std::wstring displayName = rec->GetFieldValue(L"displayname");
		while (!displayName.empty() && !IsCharAlphaNumeric(displayName[0]))
		{
			displayName.erase(0, 1);
		}
		if (displayName.empty())
		{
			displayName = L"?";
		}
		if (IsCharNumeric(displayName[0]))
		{
			displayName = L"1";
		}
		if (!StrCmpNI(str, displayName.c_str(), 1))
		{
			out.push_back(rec->m_ID);
		}
	});

	UnlockItems();

	return out;
}

void CAddressBook::onEditContact(UINT id, IRecordParent* parent)
{
	PostThreadMessage(CTxThread::getID(), WM_EDITRECORD, (WPARAM) id, (LPARAM) parent);
}

BOOL CAddressBook::isDeletedRecord(UINT recID)
{
	switch(m_groupType)
	{
	case GROUP_TYPE_ALPHA:
		return TRUE;
	case GROUP_TYPE_NONE:
		{
			BOOL res = TRUE;
			LockItems();
			auto iter = find_record(recID);
			if (iter != m_records.end())
			{
				res = FALSE;
			}
			else
			{
				res = TRUE;
			}
			UnlockItems();
			return res;
		}
		break;
	case GROUP_TYPE_FIELD:
		{
			BOOL ret = TRUE;
			LockItems();
			auto rec = find_record(recID);
			if (rec != m_records.end())
			{
				std::wstring val = (*rec)->GetFieldValue(m_groupField);
				if (val.empty())
				{
					ret = FALSE;
				}
			}
			UnlockItems();
			return ret;
		}
		break;
	}
	return TRUE;
}

BOOL CAddressBook::GetRecord(UINT recID, CRecord& rec)
{
	BOOL ret = FALSE;
	LockItems();
	auto iter = find_record(recID);
	if (iter != m_records.end())
	{
		ret = TRUE;
		rec = *(*iter);
	}
	UnlockItems();
	return ret;
}

void CAddressBook::DeleteRecord(UINT recID)
{
	LockItems();
	auto iter = find_record(recID);
	if (iter != m_records.end())
	{
		m_records.erase(iter);
	}
	UnlockItems();
	m_container->SaveButton();
	PostThreadMessage(CTxThread::getID(), WM_DELRECORD, (WPARAM) recID, NULL);
}

void CAddressBook::NotfyDelete(void)
{
	if(m_changesHandle)
	{
		SetEvent(m_changesHandle);
	}
}

void CAddressBook::GetAlphaGroupText(WCHAR alpha, LPWSTR txt, int cbTXT)
{
	std::vector<UINT> recs = GetRecordsByFirstLetter(alpha);
	std::wstring minVal;
	std::wstring maxVal;
	BOOL isFirst = TRUE;
	std::for_each(recs.begin(), recs.end(), [&isFirst, &minVal, &maxVal, this](const UINT& recID)
	{
		LockItems();
		auto rec = find_record(recID);
		if (rec != m_records.end())
		{
			std::wstring dn = (*rec)->GetFieldValue(L"displayname");
			if (isFirst)
			{
				isFirst = FALSE;
				minVal = dn;
				maxVal = dn;
			}
			else
			{
				if (StrCmpI(minVal.c_str(), dn.c_str()) > 0)
				{
					minVal = dn;
				}
				if (StrCmpI(maxVal.c_str(), dn.c_str()) < 0)
				{
					maxVal = dn;
				}
			}
		}
		UnlockItems();
	});
	if(!StrCmpI(minVal.c_str(), maxVal.c_str()))
	{
		int len = lstrlen(minVal.c_str()) + 1;
		if(cbTXT >= len)
		{
			lstrcpy(txt, minVal.c_str());
		} else
		{
			lstrcpy(txt + cbTXT - 4, L"...");
		}
	} else
	{
		int lenMin = lstrlen(minVal.c_str());
		int lenMax = lstrlen(maxVal.c_str());
		if(lenMin + lenMax + 4 <= cbTXT)
		{
			lstrcpy(txt, minVal.c_str());
			lstrcat(txt, L"...");
			lstrcat(txt, maxVal.c_str());
		} else
		{
			int dif = cbTXT - (lenMin + lenMax + 4) / 2 + 1;
			lstrcpyn(txt, minVal.c_str(), lenMin - dif);
			lstrcat(txt, L"...");
			lstrcpyn(txt + lstrlen(txt), maxVal.c_str(), lenMax - dif);
		}
	}
}

void CAddressBook::NotifyChange(void)
{
}

std::vector<UINT> CAddressBook::GetRecordsByField(LPCWSTR field, LPCWSTR value)
{
	std::vector<UINT> out;

	LockItems();

	if (StrCmpI(field, L"category"))
	{
		std::for_each(m_records.begin(), m_records.end(), [&field, &value, &out](CRecord::ptr& rec)
		{
			std::wstring val = rec->GetFieldValue(field);
			if (!StrCmpI(value, val.c_str()))
			{
				out.push_back(rec->m_ID);
			}
		});
	}
	else
	{
		std::for_each(m_records.begin(), m_records.end(), [&field, &value, &out](CRecord::ptr& rec)
		{
			std::wstring val = rec->GetFieldValue(field);
			if (!StrCmpI(value, val.c_str()))
			{
				if (!val.empty())
				{
					LPWSTR valStr = NULL;
					MAKE_STR(valStr, val.c_str());
					LPWSTR tok = wcstok(valStr, L",");
					while (tok)
					{
						LPWSTR catValue = NULL;
						MAKE_STR(catValue, tok);
						StrTrim(catValue, L" \t\r\n");
						if (!StrCmpI(value, catValue))
						{
							out.push_back(rec->m_ID);
							break;
						}
						tok = wcstok(NULL, L",");
						FREE_CLEAR_STR(catValue);
					}
					FREE_CLEAR_STR(valStr);
				}
			}
		});
	}
	UnlockItems();

	return out;
}

std::vector<std::wstring> CAddressBook::GetValuesOfField(LPCWSTR field)
{
	std::vector<std::wstring> out;

	LockItems();

	if (StrCmpI(field, L"category"))
	{
		std::for_each(m_records.begin(), m_records.end(), [&field, &out](CRecord::ptr& rec)
		{
			std::wstring newVal = rec->GetFieldValue(field);
			if (!newVal.empty())
			{
				auto iter = std::find_if(out.begin(), out.end(), [&newVal](std::wstring& str)
				{
					if (!StrCmpI(str.c_str(), newVal.c_str()))
					{
						return true;
					}
					return false;
				});
				if (iter != out.end())
				{
					out.push_back(newVal);
				}
			}
		});
	}
	else
	{
		std::for_each(m_records.begin(), m_records.end(), [&field, &out](CRecord::ptr& rec)
		{
			std::wstring newVal = rec->GetFieldValue(field);
			if (!newVal.empty())
			{
				LPWSTR valStr = NULL;
				MAKE_STR(valStr, newVal.c_str());
				LPWSTR tok = wcstok(valStr, L",");
				while (tok)
				{
					LPWSTR catValue = NULL;
					MAKE_STR(catValue, tok);
					StrTrim(catValue, L" \t\r\n");

					auto iter = std::find_if(out.begin(), out.end(), [&catValue](std::wstring& str)
					{
						if (!StrCmpI(str.c_str(), catValue))
						{
							return true;
						}
						return false;
					});
					if (iter == out.end())
					{
						out.push_back(std::wstring(catValue));
					}

					FREE_CLEAR_STR(catValue);
					tok = wcstok(NULL, L",");
				}
				FREE_CLEAR_STR(valStr);
			}
		});
	}
	UnlockItems();
	return out;
}

std::vector<UINT> CAddressBook::GetFieldEmptyRecords(LPCWSTR field)
{
	std::vector<UINT> out;

	LockItems();
	std::for_each(m_records.begin(), m_records.end(), [&field, &out](CRecord::ptr& rec)
	{
		std::wstring newVal = rec->GetFieldValue(field);
		if (newVal.empty())
		{
			out.push_back(rec->m_ID);
		}
	});
	UnlockItems();
	return out;
}

std::vector<std::wstring> CAddressBook::GetCategories(void)
{
	return GetValuesOfField(L"category");
}

cairo_container::image_ptr CAddressBook::GetIcon(UINT recID, LPCTSTR iconID, int width, int height)
{
	cairo_container::image_ptr ret;
	if(!StrCmpI(iconID, TEXT("photo"))) 
	{
		LockItems();
		auto rec = find_record(recID);
		if (rec != m_records.end())
		{
			ret = (*rec)->GetPhoto(width >= 0 ? width : m_photoSize.cx, height >= 0 ? height : m_photoSize.cy);
		}
		UnlockItems();
	}
	if(!ret)
	{
		if(!StrCmpNI(iconID, L"phonetype", 9))
		{
			LockItems();
			auto rec = find_record(recID);
			if (rec != m_records.end())
			{
				std::wstring imgID = (*rec)->GetFieldValue(iconID);
				if (!imgID.empty())
				{
					imgID += L".png";
					ret = m_images.getImage(imgID.c_str());
				}
			}
			UnlockItems();
		}
		if (!ret)
		{
			std::wstring imgID = iconID;
			if (imgID.find(L'.', 0) == std::wstring::npos)
			{
				imgID += L".png";
			}
			ret = m_images.getImage(imgID.c_str());
		}
	}
	return ret;
}

BOOL CAddressBook::SupportSortMenu(void)
{
	return TRUE;
}

void CAddressBook::CompareButtons(COMPARE_BUTTONS_DATA* cpBtns)
{
	if(cpBtns->btn1->isMenu() && !cpBtns->btn2->isMenu())
	{
		cpBtns->result = -1;
	} else if(!cpBtns->btn1->isMenu() && cpBtns->btn2->isMenu())
	{
		cpBtns->result = 1;
	} else	if(cpBtns->fnc == AMID_NAME)
	{
		TCHAR txt1[500];
		TCHAR txt2[500];
		cpBtns->btn1->GetButtonText(txt1);
		cpBtns->btn2->GetButtonText(txt2);
		cpBtns->result = StrCmpI(txt1, txt2);
	} else
	{
		cpBtns->result = 0;
	}
}

BOOL CAddressBook::QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData)
{
	switch(idx)
	{
	case AMID_NAME-1:
		qamData->sortID = AMID_NAME;
		StringCchCopy(qamData->name, 100, m_xui.getStringDef(XUI_SORT_NAME));
		return TRUE;
	}
	return FALSE;
}

void CAddressBook::UpdateRecord(UINT recID, CRecord& rec, BOOL save)
{
	LockItems();
	if(recID)
	{
		auto rec_iter = find_record(recID);
		if (rec_iter != m_records.end())
		{
			*(*rec_iter) = rec;
			(*rec_iter)->GetPhoto(m_photoSize.cx, m_photoSize.cy);
		}
	}
	else
	{
		auto rec_ptr = std::shared_ptr<CRecord>(new CRecord(rec));
		rec_ptr->m_ID = m_nexID++;
		m_records.push_back(rec_ptr);
	}
	UnlockItems();
	if(save)
	{
		m_container->SaveButton();
	}
	SetEvent(m_changesHandle);
}

DWORD CAddressBook::AcceptDropObject(IDataObject* lpObj, DWORD dwKeys)
{
	if(m_groupType == GROUP_TYPE_FIELD) 
	{
		FORMATETC fmt;
		fmt.cfFormat = (WORD) g_clipRecord;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.ptd = NULL;
		fmt.tymed = TYMED_HGLOBAL;

		if(lpObj->QueryGetData(&fmt) == S_OK)
		{
			return DROPEFFECT_MOVE;
		}	
	}
	return DROPEFFECT_NONE;
}

DWORD CAddressBook::DropObject(IDataObject* lpObj, DWORD dwEffect)
{
	FORMATETC fmt;
	fmt.cfFormat = (WORD) g_clipRecord;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.ptd = NULL;
	fmt.tymed = TYMED_HGLOBAL;
	if(lpObj->QueryGetData(&fmt) == S_OK)
	{
		STGMEDIUM med;
		ZeroMemory(&med, sizeof(med));
		if(lpObj->GetData(&fmt, &med) == S_OK)
		{
			DWORD* id = (DWORD*) GlobalLock(med.hGlobal);

			CRecord rec;
			if (GetRecord(id[0], rec))
			{
				rec.DeleteField(m_groupField);
				UpdateRecord(id[0], rec);
				if(m_changesHandle)
				{
					SetEvent(m_changesHandle);
				}
				FORMATETC fmt;
				fmt.cfFormat = (WORD) g_clipParent;
				fmt.dwAspect = DVASPECT_CONTENT;
				fmt.lindex = -1;
				fmt.ptd = NULL;
				fmt.tymed = TYMED_HGLOBAL;
				if(lpObj->QueryGetData(&fmt) == S_OK)
				{
					STGMEDIUM med;
					ZeroMemory(&med, sizeof(med));
					if(lpObj->GetData(&fmt, &med) == S_OK)
					{
						IRecordParent** parent = (IRecordParent**) GlobalLock(med.hGlobal);
						if(parent[0]) parent[0]->NotfyDelete();

						GlobalUnlock(med.hGlobal);
						ReleaseStgMedium(&med);
					}
				}
			}

			GlobalUnlock(med.hGlobal);
			ReleaseStgMedium(&med);
		}
	}

	return DROPEFFECT_NONE;
}

BOOL CAddressBook::SupportGlassMenu()
{
	return TRUE;
}

BOOL CAddressBook::openProperties()
{
	HWND hWndBand = m_container->GetToolbarWindow();
	CPropertiesDlg dlg(&m_xui, &m_web_context, m_container);
	dlg.SetTemplates(m_templates);
	dlg.m_hkNew			= m_hkNew;
	dlg.m_hkMenu		= m_hkMenu;
	dlg.m_defFontSize	= m_defFontSize;
	dlg.m_groupType		= m_groupType;
	dlg.m_photoSize		= m_photoSize;
	dlg.m_progs			= m_progs;
	dlg.m_template		= m_template;
	dlg.m_interactiveTips = m_interactiveTips;
	MAKE_STR	(dlg.m_Description, m_Description);
	lstrcpy		(dlg.m_defFont,		m_defFont);
	lstrcpy		(dlg.m_groupField,	m_groupField);

	if(dlg.DoModal(hWndBand) == IDOK)
	{
		dlg.SaveTemplates(m_templates);
		m_progs			= dlg.m_progs;
		m_photoSize		= dlg.m_photoSize;
		m_groupType		= dlg.m_groupType;
		m_defFontSize	= dlg.m_defFontSize;
		m_hkNew			= dlg.m_hkNew;
		m_hkMenu		= dlg.m_hkMenu;
		m_interactiveTips = dlg.m_interactiveTips;
		lstrcpy(m_groupField, dlg.m_groupField);
		lstrcpy		(m_defFont,		dlg.m_defFont);
		MAKE_STR	(m_Description, dlg.m_Description);
		m_template		= dlg.m_template;
		m_template.load_text(m_container);
		m_images.init_by_template(m_template, m_container);
		return TRUE;
	}
	return FALSE;
}

DWORD CAddressBook::ThreadProc()
{
	CoInitialize(NULL);

	std::vector<CContactDlg*>	dialogs;
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		if(msg.hwnd == NULL)
		{
			switch(msg.message)
			{
			case WM_DELRECORD:
				{
					UINT id = (UINT) msg.wParam;
					for(std::vector<CContactDlg*>::iterator i = dialogs.begin(); i != dialogs.end(); i++)
					{
						if((*i)->m_rec.m_ID == id)
						{
							DestroyWindow((*i)->get_wnd());
							delete (*i);
							dialogs.erase(i);
							break;
						}
					}
				}
				break;
			case WM_EDITRECORD:
				{
					UINT id = (UINT) msg.wParam;
					IRecordParent*	recParent = (IRecordParent*) msg.lParam;

					BOOL openNew = TRUE;

					if(id != 0)
					{
						for(std::vector<CContactDlg*>::iterator i = dialogs.begin(); i != dialogs.end(); i++)
						{
							if((*i)->m_rec.m_ID == id)
							{
								ShowWindow((*i)->get_wnd(), SW_NORMAL);
								ActivateWindow((*i)->get_wnd());
								openNew = FALSE;
								break;
							}
						}
					}

					if(openNew)
					{
						CContactDlg* dlg = new CContactDlg(&m_xui, this, recParent);
						GetRecord(id, dlg->m_rec);
						dlg->m_categories = GetCategories();
						HWND hwnd = dlg->Create(NULL);
						if(hwnd)
						{
							ShowWindow(hwnd, SW_NORMAL);
							ActivateWindow(hwnd);
							dialogs.push_back(dlg);
						} else
						{
							delete dlg;
						}
					}
				}
				break;
			}
		}

		if(!msg.hwnd)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else
		{
			HWND dlg = msg.hwnd;
			while(TRUE)
			{
				HWND parent = GetParent(dlg);
				if(parent)
				{
					dlg = parent;
				} else
				{
					break;;
				}
			}

			if(!IsDialogMessage(dlg, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	for(std::vector<CContactDlg*>::iterator i = dialogs.begin(); i != dialogs.end(); i++)
	{
		DestroyWindow((*i)->get_wnd());
		delete (*i);
	}
	dialogs.clear();

	return 0;
}

void CAddressBook::runLink( LPCWSTR link )
{
	std::wstring lnk = link;
	size_t nameBegin	= 1;
	size_t nameEnd		= lnk.find(L':', nameBegin);
	std::wstring fldName = lnk.substr(nameBegin, nameEnd - nameBegin);
	std::wstring fldValue = lnk.substr(nameEnd + 1);
	std::wstring cmdLine;
	for(size_t i=0; i < m_progs.size(); i++)
	{
		if(m_progs[i].getField() == fldName)
		{
			cmdLine = m_progs[i].getCmdLine(fldValue);
		}
	}
	if(!cmdLine.empty())
	{
		if(PathIsURL(cmdLine.c_str()))
		{
			DWORD len = (DWORD) cmdLine.length() * 3;
			LPWSTR url = new WCHAR[len];
			UrlCanonicalize(cmdLine.c_str(), url, &len, URL_ESCAPE_SPACES_ONLY | URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE | URL_DONT_SIMPLIFY);
			ShellExecute(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
			delete url;
		} else
		{
			LPWSTR path = NULL;
			MAKE_STR(path, cmdLine.c_str());
			std::wstring args = PathGetArgs(cmdLine.c_str());
			PathRemoveArgs(path);
			ShellExecute(NULL, NULL, path, args.c_str(), NULL, SW_SHOWNORMAL);
			FREE_CLEAR_STR(path);
		}
	}
}

UINT CAddressBook::FindSimilarRecord( CRecord& rec )
{
	UINT ret = 0;
	LockItems();
	for(auto& rec_iter : m_records)
	{
		std::wstring displayName1 = rec.GetFieldValue(L"displayname");
		std::wstring displayName2 = rec_iter->GetFieldValue(L"displayname");
		if(!displayName1.empty() && !displayName2.empty() && !StrCmpI(displayName1.c_str(), displayName2.c_str()))
		{
			ret = rec_iter->m_ID;
			break;
		}
	}
	UnlockItems();
	return ret;
}

