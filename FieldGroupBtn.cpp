#include "globals.h"
#include "abookbtn.h"
#include "RecordBtn.h"
#include ".\fieldgroupbtn.h"
#include "resource.h"
#include "xuistrings.h"

CFieldGroupBtn::CFieldGroupBtn(CAddressBook* parent, LPCWSTR field, LPCWSTR value)
{
	m_isChanged = FALSE;
	m_value = NULL;
	MAKE_STR(m_value, value);
	lstrcpy(m_field, field);
	lstrcpy(m_Name, m_value);
	m_parent = parent;
	m_changesHandle = NULL;
}

CFieldGroupBtn::~CFieldGroupBtn(void)
{
	FREE_CLEAR_STR(m_value);
}

void CFieldGroupBtn::GetChildUID(LPWSTR uid)
{
	StringCchPrintf(uid, 255, L"grp:%s:%s", m_field, m_value);
	CharLower(uid);
}

BOOL CFieldGroupBtn::isMenu(void)
{
	return TRUE;
}

HANDLE CFieldGroupBtn::GetChangesHandle() 
{ 
	if(!m_changesHandle)
	{
		m_changesHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
	}
	return m_changesHandle; 
}

void CFieldGroupBtn::PrepareChanges(HANDLE hChanges)
{
	ResetEvent(hChanges);
}

void CFieldGroupBtn::CloseChangesHandle(HANDLE hChanges)
{
	CloseHandle(hChanges);
	m_changesHandle = NULL;
}

UINT CFieldGroupBtn::GetMenuFlags(void)
{
	return MENU_F_NORENAME | MENU_F_VARHEIGHT;
}

struct AB_FIELD_ENUM_DATA
{
	std::vector<UINT>	recs;
	int					lastID;
};

BOOL CFieldGroupBtn::FirstChildData(CHILDS_DATA* data)
{
	AB_FIELD_ENUM_DATA* enumData = new AB_FIELD_ENUM_DATA;
	enumData->lastID = 0;
	enumData->recs = m_parent->GetRecordsByField(m_field, m_value);
	if(enumData->recs.empty())
	{
		delete enumData;
		return FALSE;
	}
	data->childData = (LPVOID) enumData->recs[enumData->lastID];
	data->enumData = (LPVOID) enumData;
	if(data->Name)
	{
		wsprintf(data->Name, TEXT("rec:%d"), enumData->recs[enumData->lastID]);
	}
	return TRUE;
}

BOOL CFieldGroupBtn::NextChildData(CHILDS_DATA* data)
{
	AB_FIELD_ENUM_DATA* enumData = (AB_FIELD_ENUM_DATA*) data->enumData;
	enumData->lastID++;
	if (enumData->lastID >= (int)enumData->recs.size())
	{
		delete enumData;
		return FALSE;
	}
	data->childData = (LPVOID) enumData->recs[enumData->lastID];
	data->enumData = (LPVOID) enumData;
	if(data->Name)
	{
		wsprintf(data->Name, TEXT("rec:%d"), enumData->recs[enumData->lastID]);
	}
	return TRUE;
}

BOOL CFieldGroupBtn::FreeChildData(CHILDS_DATA* data)
{
	return TRUE;
}

BOOL CFieldGroupBtn::CreateChild(CREATE_CHILD_DATA* childData)
{
	UINT id = (UINT) childData->childData;
	CRecordBtn* btn = new CRecordBtn(m_parent, id, this);
	childData->child = btn;
	return TRUE;
}


UINT CFieldGroupBtn::ReadChanges(void)
{
	if(m_parent->GetGroupType() != GROUP_TYPE_FIELD || StrCmpI(m_parent->GetGroupField(), m_field))
	{
		return 2;
	}
	UINT res = 0;
	std::vector<UINT> recs = m_parent->GetRecordsByField(m_field, m_value);
	if(recs.empty())
	{
		res = 2;
	} else if(m_isChanged)
	{
		res = 1;
	}
	return res;
}

BOOL CFieldGroupBtn::isDeletedRecord(UINT recID)
{
	CRecord rec;
	if(!m_parent->GetRecord(recID, rec))
	{
		return TRUE;
	}
	std::wstring val = rec.GetFieldValue(m_field);
	if(StrCmpI(m_value, val.c_str()))
	{
		return TRUE;
	}
	return FALSE;
}

void CFieldGroupBtn::NotfyDelete(void)
{
	if(ReadChanges() == 2)
	{
		m_parent->NotfyDelete();
	} else
	{
		if(m_changesHandle)
		{
			SetEvent(m_changesHandle);
		}
	}
}

void CFieldGroupBtn::NotifyChange(void)
{
}

BOOL CFieldGroupBtn::LoadIcon(int size)
{
	if(!CTlbButton::LoadIcon(size))
	{
		m_hIcon = (HICON) LoadImage(g_hInst, MAKEINTRESOURCE(IDI_GROUPICON), IMAGE_ICON, size, size, LR_DEFAULTCOLOR);
	}
	return m_hIcon ? TRUE : FALSE;
}

DWORD CFieldGroupBtn::AcceptDropObject(IDataObject* lpObj, DWORD dwKeys)
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
	return DROPEFFECT_NONE;
}

DWORD CFieldGroupBtn::DropObject(IDataObject* lpObj, DWORD dwEffect)
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
			if(m_parent->GetRecord(id[0], rec))
			{
				rec.SetFieldValue(m_field, m_value);
				m_parent->UpdateRecord(id[0], rec);
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

BOOL CFieldGroupBtn::SupportSortMenu(void)
{
	return TRUE;
}

void CFieldGroupBtn::CompareButtons(COMPARE_BUTTONS_DATA* cpBtns)
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

BOOL CFieldGroupBtn::QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData)
{
	switch(idx)
	{
	case AMID_NAME-1:
		qamData->sortID = AMID_NAME;
		StringCchCopy(qamData->name, 100, m_parent->m_xui.getStringDef(XUI_SORT_NAME));
		return TRUE;
	}
	return FALSE;
}

BOOL CFieldGroupBtn::SupportGlassMenu()
{
	return TRUE;
}

BOOL CFieldGroupBtn::applyMargins()
{
	return FALSE;
}

