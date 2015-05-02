#include "globals.h"
#include ".\contactdlg.h"
#include "resource.h"
#include "CropPhotoDlg.h"

CContactDlg::CContactDlg(CXUIEngine* engine, CAddressBook* btn, IRecordParent* recParent) : CXUIDialog(L"res:contact.xml", engine)
{
	m_btn			= btn;
	m_recParent		= recParent;
	m_icon			= LoadIcon(engine->get_hInstance(), MAKEINTRESOURCE(IDI_CONTACT));
}

CContactDlg::~CContactDlg(void)
{
	DestroyIcon(m_icon);
}

void CContactDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();

	SendMessage(m_hWnd, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) m_icon);

	for(int i=0; g_defText[i].fldName; i++)
	{
		if(g_defText[i].inList)
		{
			CXUIElement* el = find((LPTSTR) g_defText[i].fldName);
			if(el)
			{
				el->value_STR(m_rec.GetFieldValue(g_defText[i].fldName).c_str());
			}
		}
	}
	for(int i=1; i <= 7; i++)
	{
		WCHAR elID[50];
		wsprintf(elID, L"phonetype%d", i);
		std::wstring strVal = m_rec.GetFieldValue(elID);
		int idx = phoneTypeToInt(strVal.c_str());
		CXUIElement* el = find(elID);
		if(el)
		{
			el->value_INT(idx);
		}
	}
}

BOOL CContactDlg::OnEndDialog( UINT code )
{
	if(code == IDOK)
	{
		for(int i=0; g_defText[i].fldName; i++)
		{
			if(g_defText[i].inList)
			{
				CXUIElement* el = find((LPTSTR) g_defText[i].fldName);
				if(el)
				{
					m_rec.SetFieldValue(g_defText[i].fldName, el->value_STR());
				}
			}
		}
		for(int i=1; i <= 7; i++)
		{
			WCHAR elID[50];
			wsprintf(elID, L"phonetype%d", i);
			CXUIElement* el = find(elID);
			if(el)
			{
				int idx			= el->value_INT();
				LPCWSTR strVal	= phoneTypeToStr(idx);
				m_rec.SetFieldValue(elID, strVal);
			}
		}
		if(m_recParent)
		{
			m_btn->UpdateRecord(m_rec.m_ID, m_rec);
			m_recParent->NotifyChange();
		}
	}
	return TRUE;
}

LPWSTR g_phoneTypes[] = 
{
	L"primaryphone",
	L"homephone",
	L"workphone",
	L"fax",
	L"pager",
	L"mobile",
	NULL
};


int CContactDlg::phoneTypeToInt( LPCWSTR val )
{
	for(int i=0; g_phoneTypes[i]; i++)
	{
		if(!StrCmpI(val, g_phoneTypes[i]))
		{
			return i;
		}
	}
	return 0;
}

LPCWSTR CContactDlg::phoneTypeToStr( int val )
{
	return g_phoneTypes[val];
}

BOOL CContactDlg::OnSelectCategory()
{
	MENUITEMINFO mi;
	mi.cbSize = sizeof(MENUITEMINFO);
	mi.fMask = MIIM_TYPE | MIIM_ID;
	HMENU hMenu = CreatePopupMenu();

	for(int i=0; i < (int) m_categories.size(); i++)
	{
		mi.fType = MFT_STRING;
		mi.dwTypeData = (LPWSTR) m_categories[i].c_str();
		mi.cch = lstrlen(mi.dwTypeData);
		mi.wID = i + 1;
		InsertMenuItem(hMenu, i, TRUE, &mi);
	}

	RECT rc;
	GetWindowRect(find(L"btnCategory")->get_wnd(), &rc);
	DWORD ret = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, NULL, m_hWnd, NULL);
	DestroyMenu(hMenu);
	if(ret)
	{
		std::wstring str = find(L"category")->value_STRdef(L"");
		if(str.empty())
		{
			find(L"category")->value_STR(m_categories[ret-1].c_str());
		} else
		{
			if(*(str.end() - 1) != L',')
			{
				str += L", ";
			}
			str += m_categories[ret-1];
			find(L"category")->value_STR(str.c_str());
		}
	}
	return TRUE;
}

BOOL CContactDlg::OnDrawPhoto( HDC hdc, LPRECT rcDraw )
{
	cairo_container::image_ptr img = m_rec.GetPhoto(rcDraw->right - rcDraw->left, rcDraw->bottom - rcDraw->top);
	if(img)
	{
		int x = rcDraw->left + (rcDraw->right - rcDraw->left) / 2 - img->getWidth() / 2;
		int y = rcDraw->top + (rcDraw->bottom - rcDraw->top) / 2 - img->getHeight() / 2;
		img->draw(hdc, x, y);
	} else
	{
		int x = rcDraw->left + (rcDraw->right - rcDraw->left) / 2 - 128 / 2;
		int y = rcDraw->top + (rcDraw->bottom - rcDraw->top) / 2 - 128 / 2;
		m_engine->DrawImage(hdc, x, y, 128, 128, L"photo.png");
	}
	return TRUE;
}

BOOL CContactDlg::OnDeletePhoto()
{
	m_rec.ClearImage();
	m_ctlPhoto->redraw(TRUE);
	return TRUE;
}

BOOL CContactDlg::OnPhotoSelected( LPWSTR fileName )
{
	CCropPhotoDlg dlg(m_engine, fileName);
	
	if(dlg.DoModal(m_hWnd))
	{
		m_rec.InitImage(dlg.m_img, dlg.m_imgSize, TRUE);
		m_ctlPhoto->redraw(TRUE);
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*
CContactTab::CContactTab() : CTrueDialog(IDD_TAB_CONTACT)
{
}

CContactTab::~CContactTab()
{
}

CAddressTab::CAddressTab() : CTrueDialog(IDD_TAB_ADDRESS)
{
}

CAddressTab::~CAddressTab()
{
}

struct 
{
	DWORD	ctlID;
	LPCWSTR	fldName;
} g_defAddress[] = 
{
	{IDC_ADDR_HOME_ADDRESS1,		L"home_address"},
	{IDC_ADDR_HOME_ADDRESS2,		L"home_address2"},
	{IDC_ADDR_HOME_CITY,			L"home_city"},
	{IDC_ADDR_HOME_ZIP,				L"home_zip"},
	{IDC_ADDR_HOME_STATE,			L"home_state"},
	{IDC_ADDR_HOME_COUNTRY,			L"home_country"},

	{IDC_ADDR_WORK_ADDRESS1,		L"work_address"},
	{IDC_ADDR_WORK_ADDRESS2,		L"work_address2"},
	{IDC_ADDR_WORK_CITY,			L"work_city"},
	{IDC_ADDR_WORK_ZIP,				L"work_zip"},
	{IDC_ADDR_WORK_STATE,			L"work_state"},
	{IDC_ADDR_WORK_COUNTRY,			L"work_country"},

	{0,		NULL}
};


void CAddressTab::OnInitDialog(void)
{
	for(int i=0; g_defAddress[i].fldName; i++)
	{
		LPWSTR val = m_parent->m_rec.GetFieldValue(g_defAddress[i].fldName);
		if(val)
		{
			SetDlgItemText(m_hWnd, g_defAddress[i].ctlID, val);
		}
	}
}

DWORD CAddressTab::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	switch(pnmh->code)
	{
	case PSN_APPLY:
		{
			for(int i=0; g_defAddress[i].fldName; i++)
			{
				TCHAR val[500];
				GetDlgItemText(m_hWnd, g_defAddress[i].ctlID, val, 500);
				m_parent->m_rec.SetFieldValue(g_defAddress[i].fldName, val);
			}
		}
		break;
	case PSN_HELP:
		break;
	}
	return TRUE;
}


struct 
{
	DWORD	ctlID;
	LPCWSTR	fldName;
} g_defContact[] = 
{
	{IDC_FIRSTNAME,					L"firstname"},
	{IDC_LASTNAME,					L"lastname"},
	{IDC_NICNAME,					L"nickname"},

	{IDC_ADDR_WORK_TITLE,			L"title"},
	{IDC_ADDR_WORK_DEPARTMENT,		L"department"},
	{IDC_ADDR_WORK_ORGANIZATION,	L"organization"},

	{IDC_URL,						L"url"},
	{IDC_EMAIL,						L"email"},
	{IDC_IM,						L"im"},

	{0,		NULL}
};

void CContactTab::OnInitDialog(void)
{
	for(int i=0; g_defContact[i].fldName; i++)
	{
		LPWSTR val = m_parent->m_rec.GetFieldValue(g_defContact[i].fldName);
		if(val)
		{
			SetDlgItemText(m_hWnd, g_defContact[i].ctlID, val);
		}
	}
}

DWORD CContactTab::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	switch(pnmh->code)
	{
	case PSN_APPLY:
		{
			for(int i=0; g_defContact[i].fldName; i++)
			{
				TCHAR val[500];
				GetDlgItemText(m_hWnd, g_defContact[i].ctlID, val, 500);
				m_parent->m_rec.SetFieldValue(g_defContact[i].fldName, val);
			}
		}
		break;
	case PSN_HELP:
		break;
	}
	return TRUE;
}

CGeneralTab::CGeneralTab() : CTrueDialog(IDD_TAB_GENERAL)
{
}

CGeneralTab::~CGeneralTab()
{
}


struct 
{
	DWORD	ctlID;
	LPCWSTR	fldName;
	BOOL	isCombo;
} g_defGeneral[] = 
{
	{IDC_DISPLAYNAME,	L"displayname",	FALSE},
	{IDC_NOTES,			L"notes",		FALSE},
	{IDC_CATEGORY,		L"category",	FALSE},
	{IDC_PHONE1,		L"phone1",		FALSE},
	{IDC_PHONE2,		L"phone2",		FALSE},
	{IDC_PHONE3,		L"phone3",		FALSE},
	{IDC_PHONE4,		L"phone4",		FALSE},
	{IDC_PHONE5,		L"phone5",		FALSE},
	{IDC_PHONE6,		L"phone6",		FALSE},
	{IDC_PHONE7,		L"phone7",		FALSE},

	{IDC_PHONETYPE1,	L"phonetype1",	TRUE},
	{IDC_PHONETYPE2,	L"phonetype2",	TRUE},
	{IDC_PHONETYPE3,	L"phonetype3",	TRUE},
	{IDC_PHONETYPE4,	L"phonetype4",	TRUE},
	{IDC_PHONETYPE5,	L"phonetype5",	TRUE},
	{IDC_PHONETYPE6,	L"phonetype6",	TRUE},
	{IDC_PHONETYPE7,	L"phonetype7",	TRUE},

	{IDC_PHONENOTE1,	L"phonenote1",	FALSE},
	{IDC_PHONENOTE2,	L"phonenote2",	FALSE},
	{IDC_PHONENOTE3,	L"phonenote3",	FALSE},
	{IDC_PHONENOTE4,	L"phonenote4",	FALSE},
	{IDC_PHONENOTE5,	L"phonenote5",	FALSE},
	{IDC_PHONENOTE6,	L"phonenote6",	FALSE},
	{IDC_PHONENOTE7,	L"phonenote7",	FALSE},

	{0,		NULL}
};

struct 
{
	LPCWSTR value;
	UINT	resID;
} g_defPhones[] = 
{
	{TEXT("primaryphone"),	IDS_FLD_PHONEPRIMARY},
	{TEXT("homephone"),		IDS_FLD_PHONEHOME},
	{TEXT("workphone"),		IDS_FLD_PHONEWORK},
	{TEXT("fax"),			IDS_FLD_PHONEFAX},
	{TEXT("pager"),			IDS_FLD_PHONEPAGER},
	{TEXT("mobile"),		IDS_FLD_PHONEMOBILE},
	{NULL,	0},
};

void CGeneralTab::OnInitDialog(void)
{
	for(int i=0; g_defGeneral[i].fldName; i++)
	{
		LPWSTR val = m_parent->m_rec.GetFieldValue(g_defGeneral[i].fldName);
		if(!g_defGeneral[i].isCombo) 
		{
			if(val)
			{
				SetDlgItemText(m_hWnd, g_defGeneral[i].ctlID, val);
			}
		} else
		{
			BOOL isSet = FALSE;
			for(int j=0; g_defPhones[j].value; j++)
			{
				TCHAR str[255];
				LoadString(m_hResInst, g_defPhones[j].resID, str, 255);
				int idx = SendDlgItemMessage(m_hWnd, g_defGeneral[i].ctlID, CB_ADDSTRING, 0, (LPARAM) str);
				if(!StrCmpI(g_defPhones[j].value, val))
				{
					SendDlgItemMessage(m_hWnd, g_defGeneral[i].ctlID, CB_SETCURSEL, idx, NULL);
					isSet = TRUE;
				}
			}
			if(!isSet)
			{
				SendDlgItemMessage(m_hWnd, g_defGeneral[i].ctlID, CB_SETCURSEL, 0, NULL);
			}
		}
	}
	SendDlgItemMessage(m_hWnd, IDC_PHOTO, PHB_SETPICTURE, m_parent->m_rec.m_cbImage, (LPARAM) m_parent->m_rec.m_image);
}

DWORD CGeneralTab::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	switch(pnmh->code)
	{
	case PSN_APPLY:
		{
			for(int i=0; g_defGeneral[i].fldName; i++)
			{
				if(!g_defGeneral[i].isCombo) 
				{
					TCHAR val[500];
					GetDlgItemText(m_hWnd, g_defGeneral[i].ctlID, val, 500);
					m_parent->m_rec.SetFieldValue(g_defGeneral[i].fldName, val);
				} else
				{
					int idx = SendDlgItemMessage(m_hWnd, g_defGeneral[i].ctlID, CB_GETCURSEL, 0, NULL);
					m_parent->m_rec.SetFieldValue(g_defGeneral[i].fldName, g_defPhones[idx].value);
				}
			}
			DWORD sz = SendDlgItemMessage(m_hWnd, IDC_PHOTO, PHB_GETPICTURESIZE, 0, 0);
			LPBYTE pic = NULL;
			if(sz)
			{
				if(m_parent->m_rec.m_image) delete m_parent->m_rec.m_image;
				m_parent->m_rec.m_image = new BYTE[sz];
				SendDlgItemMessage(m_hWnd, IDC_PHOTO, PHB_GETPICTURE, 0, (LPARAM) m_parent->m_rec.m_image);
				m_parent->m_rec.m_cbImage = sz;
			} else
			{
				if(m_parent->m_rec.m_image) delete m_parent->m_rec.m_image;
				m_parent->m_rec.m_image = NULL;
				m_parent->m_rec.m_cbImage = 0;
			}
		}
		break;
	case PSN_HELP:
		break;
	}
	return TRUE;
}

void CGeneralTab::OnCommand(DWORD itemID)
{
	switch(LOWORD(itemID))
	{
	case IDC_SELECTCAT:
		if(HIWORD(itemID) == BN_CLICKED && m_parent->m_categories[0])
		{
			MENUITEMINFO mi;
			mi.cbSize = sizeof(MENUITEMINFO);
			mi.fMask = MIIM_TYPE | MIIM_ID;
			HMENU hMenu = CreatePopupMenu();

			for(int i=0; m_parent->m_categories[i]; i++)
			{
				mi.fType = MFT_STRING;
				mi.dwTypeData = m_parent->m_categories[i];
				mi.cch = lstrlen(mi.dwTypeData);
				mi.wID = i + 1;
				InsertMenuItem(hMenu, i, TRUE, &mi);
			}

			RECT rc;
			GetWindowRect(GetDlgItem(m_hWnd, IDC_SELECTCAT), &rc);
			DWORD ret = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, rc.left, rc.bottom, NULL, m_hWnd, NULL);
			DestroyMenu(hMenu);
			if(ret)
			{
				SetDlgItemText(m_hWnd, IDC_CATEGORY, m_parent->m_categories[ret-1]);
			}
		}
		break;
	}
}
*/
