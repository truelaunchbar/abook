#include "globals.h"
#include "ImportDlg.h"
#include "vcard.h"
#include "xuistrings.h"
#include "resource.h"
#include "ContactDlg.h"

CImportDlg::CImportDlg(CXUIEngine* engine, CAddressBook* btn) : CXUIDialog(L"res:import.xml", engine)
{
	m_btn			= btn;
	m_itemHeight	= 0;
	m_textHeight	= 0;
	m_fntBold		= NULL;
	m_fntNormal		= NULL;
	m_fntCaption	= NULL;
	m_fntLinks		= NULL;
}

CImportDlg::~CImportDlg(void)
{
	clear();
	if(m_fntBold)		DeleteFont(m_fntBold);
	if(m_fntNormal)		DeleteFont(m_fntNormal);
	if(m_fntCaption)	DeleteFont(m_fntCaption);
	if(m_fntLinks)		DeleteFont(m_fntLinks);
	if(m_hIcon)			DestroyIcon(m_hIcon);
}

void CImportDlg::addFile(LPCWSTR fileName)
{
	m_files.push_back(std::wstring(fileName));
}

void CImportDlg::clear()
{
	for(size_t i = 0; i < m_records.size(); i++)
	{
		delete m_records[i];
	}
	m_records.clear();
}

void CImportDlg::import()
{
	for(size_t i = 0; i < m_files.size(); i++)
	{
		import(m_files[i].c_str());
	}
}

void CImportDlg::import( LPCWSTR fileName )
{
	vcard::reader vcf;
	if(vcf.open(fileName))
	{
		CRecord* rec = NULL;
		int phoneid = 1;
		vcard::card* crd = vcf.next();
		while(crd)
		{
			rec		= new CRecord;
			phoneid	= 1;
			for(int i=0; i < crd->count(); i++)
			{
				vcard::field fld = crd->getField(i);

				struct
				{
					LPSTR	vcardName;
					LPWSTR	recName;
				} defNames[] =
				{
					{"NICKNAME",	L"nickname"},
					{"FN",			L"displayname"},
					{"TITLE",		L"title"},
					{"ROLE",		L"role"},
					{"CATEGORIES",	L"category"},
					{"NOTE",		L"notes"},
					{"URL",			L"url"},
					{"X-ICQ",		L"icq"},
					{"X-GTALK",		L"gtalk"},
					{"X-AIM",		L"aim"},
					{"X-YAHOO",		L"yahoo"},
					{"X-SKYPE",		L"skype"},
					{"X-MSN",		L"msn"},
					{"X-JABBER",	L"jabber"},
					{"X-QQ",		L"qq"},
					{NULL,			NULL}
				};

				BOOL found = FALSE;
				for(int j=0; defNames[j].vcardName; j++)
				{
					if(defNames[j].vcardName == fld.getType())
					{
						rec->SetFieldValue(defNames[j].recName, fld.getConvertedStringValue(crd->version()).c_str());
						found = TRUE;
						break;
					}
				}

				if(!found)
				{
					if(fld.getType() == "TEL")
					{
						if(phoneid <= 7)
						{
							WCHAR recFldName[100];
							wsprintf(recFldName, L"phone%d", phoneid);
							rec->SetFieldValue(recFldName, fld.getConvertedStringValue(crd->version()).c_str());
							wsprintf(recFldName, L"phonetype%d", phoneid);
							if(fld.haveParam("HOME"))
							{
								rec->SetFieldValue(recFldName, L"homephone");
							} else if(fld.haveParam("WORK"))
							{
								rec->SetFieldValue(recFldName, L"workphone");
							} else if(fld.haveParam("FAX"))
							{
								rec->SetFieldValue(recFldName, L"fax");
							} else if(fld.haveParam("CELL"))
							{
								rec->SetFieldValue(recFldName, L"mobile");
							} else if(fld.haveParam("PAGER"))
							{
								rec->SetFieldValue(recFldName, L"pager");
							} else
							{
								rec->SetFieldValue(recFldName, L"primaryphone");
							}
							phoneid++;
						}
					} else if(fld.getType() == "N")
					{
						LPWSTR names[] =
						{
							L"lastname",
							L"firstname",
							L"middlename",
							L"prefixname",
							L"suffixname",
							NULL
						};
						std::vector<std::wstring> values = fld.getStructuredText(crd->version());
						for(size_t j = 0; j < values.size() && names[j]; j++)
						{
							rec->SetFieldValue(names[j], values[j].c_str());
						}
					} else if(fld.getType() == "ORG")
					{
						LPWSTR names[] =
						{
							L"organization",
							L"department",
							NULL
						};
						std::vector<std::wstring> values = fld.getStructuredText(crd->version());
						for(size_t j = 0; j < values.size() && names[j]; j++)
						{
							rec->SetFieldValue(names[j], values[j].c_str());
						}
					} else if(fld.getType() == "PHOTO")
					{
						UINT photoSize = 0;
						LPBYTE photo = fld.getBinaryValue(photoSize);
						if(photo)
						{
							rec->InitImage(photo, photoSize, TRUE);
							delete photo;
						}
					} else if(fld.getType() == "EMAIL")
					{
						if(fld.haveParam("INTERNET") || fld.haveParam("TYPE", "INTERNET"))
						{
							rec->SetFieldValue(L"email", fld.getConvertedStringValue(crd->version()).c_str());
						}
					} else if(fld.getType() == "ADR")
					{
						LPWSTR adrComponentsW[] = 
						{
							L"work_pobox",
							L"work_address2",
							L"work_address",
							L"work_city",
							L"work_state",
							L"work_zip",
							L"work_country",
							NULL
						};

						LPWSTR adrComponentsH[] = 
						{
							L"home_pobox",
							L"home_address2",
							L"home_address",
							L"home_city",
							L"home_state",
							L"home_zip",
							L"home_country",
							NULL
						};

						LPWSTR* adrComponents = NULL;

						if(fld.haveParam("HOME"))
						{
							adrComponents = adrComponentsH;
						} else
						{
							if(fld.haveParam("WORK"))
							{
								adrComponents = adrComponentsW;
							} else
							{
								std::string type = fld.getParamValue("TYPE");
								if(StrStrIA(type.c_str(), "work"))
								{
									adrComponents = adrComponentsW;
								} else
								{
									adrComponents = adrComponentsH;
								}
							}
						}

						std::vector<std::wstring> values = fld.getStructuredText(crd->version());
						for(size_t j = 0; j < values.size() && adrComponents[j]; j++)
						{
							rec->SetFieldValue(adrComponents[j], values[j].c_str());
						}
					}
				}
			}

			if(rec->GetFieldValue(L"displayname").empty())
			{
				std::wstring fname = rec->GetFieldValue(L"firstname");
				std::wstring lname = rec->GetFieldValue(L"lastname");
				if(fname.empty() && lname.empty())
				{
					std::wstring org = rec->GetFieldValue(L"organization");
					std::wstring dep = rec->GetFieldValue(L"department");
					if(org.empty() && dep.empty())
					{
						std::wstring nickname = rec->GetFieldValue(L"nickname");
						if(!nickname.empty())
						{
							rec->SetFieldValue(L"displayname", nickname.c_str());
						}
					} else
					{
						std::wstring str;
						if(!org.empty())
						{
							str = org;
						}
						if(!dep.empty())
						{
							if(!org.empty())	str += L", ";
							str += dep;
						}
						rec->SetFieldValue(L"displayname", str.c_str());
					}
				} else
				{
					std::wstring str;
					if(!fname.empty())
					{
						str = fname;
					}
					if(!lname.empty())
					{
						if(!fname.empty())	str += L" ";
						str += lname;
					}
					rec->SetFieldValue(L"displayname", str.c_str());
				}
			}

			rec->m_ID = m_btn->FindSimilarRecord(*rec);
			m_records.push_back(rec);
			UINT action = IMPORT_ACT_IMPORT;
			if(rec->m_ID)
			{
				action = IMPORT_ACT_NONE;
			}
			m_actions.push_back(action);

			crd = vcf.next();
		}
		vcf.close();
	}
}

void CImportDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();

	m_hIcon = LoadIcon(m_engine->get_hInstance(), MAKEINTRESOURCE(IDI_IMPORT));
	SendMessage(m_hWnd, WM_SETICON, (WPARAM) ICON_BIG, (LPARAM) m_hIcon);

	calcItemHeight();

	import();
	updateImportBtn();
	fillList();
}

BOOL CImportDlg::OnEndDialog( UINT code )
{
	return TRUE;
}

void CImportDlg::updateImportBtn()
{
	BOOL found = FALSE;
	for(size_t i=0; i < m_actions.size(); i++)
	{
		if(m_actions[i])
		{
			found = TRUE;
			break;
		}
	}
	if(found)
	{
		find(L"import")->set_disabled(FALSE);
	} else
	{
		find(L"import")->set_disabled(TRUE);
	}
}

void CImportDlg::fillList()
{
	m_ctlList->clear();
	for(size_t i = 0; i < m_records.size(); i++)
	{
		m_ctlList->addItem((UINT_PTR) i, m_itemHeight);
	}
	m_ctlList->update();
}

BOOL CImportDlg::OnDrawRecord( HDC hdc, CUSTOM_LIST_DRAWITEM* di )
{

	DrawVGradient(hdc, di->rcItem, RGB(240, 240, 240), RGB(216, 216, 216), 255);

	LPCWSTR flds[] = 
	{
		L"phone1",
		L"phone2",
		L"email",
		L"emailadd",
		L"url",
		L"nickname",
		NULL
	};

	SetBkMode(hdc, TRANSPARENT);
	HFONT oldFnt = SelectFont(hdc, m_fntBold);

	if(m_actions[di->itemID])
	{
		SetTextColor(hdc, RGB(0, 0, 0));
	} else
	{
		SetTextColor(hdc, RGB(150, 150, 150));
	}

	int imgX = di->rcItem.left + 4;
	int imgY = di->rcItem.top  + 4;

/*
	CTxDIB* img = m_records[di->itemID]->GetPhoto(50, 50);
	if(img)
	{
		img->draw(hdc, imgX, imgY);
	} else
*/
	{
		m_engine->DrawImage(hdc, imgX, imgY, 50, 50, L"photomin.png");
	}

	int txtLeft = imgX + 50 + 4;
	int txtTop  = di->rcItem.top + 4;

	oldFnt = SelectFont(hdc, m_fntBold);

	RECT rcText;
	rcText.left		= di->rcItem.left + 58;
	rcText.right	= di->rcItem.right - m_linksWidth;
	rcText.top		= di->rcItem.top + 4;
	rcText.bottom	= rcText.top + m_captionHeight;

	std::wstring val = m_records[di->itemID]->GetFieldValue(L"displayname");
	if(!val.empty())
	{
		SelectFont(hdc, m_fntCaption);
		DrawText(hdc, val.c_str(), -1, &rcText, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);
	}

	SelectFont(hdc, m_fntLinks);

	RECT rcLink = rcText;
	rcLink.left	 = di->rcItem.right - m_linksWidth;
	rcLink.right = di->rcItem.right;
	rcLink.bottom	= rcLink.top + m_linksHeight;

	SetTextColor(hdc, RGB(0, 0, 200));

	switch(m_actions[di->itemID])
	{
	case IMPORT_ACT_IMPORT:
		DrawText(hdc, m_engine->getStringDef(XUI_IMPORT_NEW), -1, &rcLink, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);
		break;
	case IMPORT_ACT_UPDATE:
		DrawText(hdc, m_engine->getStringDef(XUI_IMPORT_UPDATE), -1, &rcLink, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);
		break;
	case IMPORT_ACT_NONE:
		DrawText(hdc, m_engine->getStringDef(XUI_IMPORT_SKIP), -1, &rcLink, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);
		break;
	}

	SelectFont(hdc, m_fntBold);

	if(m_actions[di->itemID])
	{
		SetTextColor(hdc, RGB(0, 0, 0));
	} else
	{
		SetTextColor(hdc, RGB(150, 150, 150));
	}

	rcText.top		= rcText.bottom;
	rcText.bottom += m_textHeight;
	rcText.right	= di->rcItem.right;

	for(int i = 0; flds[i]; i++)
	{
		val = m_records[di->itemID]->GetFieldValue(flds[i]);

		if(!val.empty())
		{
			std::wstring strLabel;
			if(i < 2)
			{
				if(!m_records[di->itemID]->GetFieldType(flds[i]).empty())
				{
					strLabel = m_records[di->itemID]->GetFieldLabel(m_records[di->itemID]->GetFieldType(flds[i]).c_str(), m_engine);
				} else
				{
					strLabel = m_records[di->itemID]->GetFieldLabel(flds[i], m_engine);
				}
			} else
			{
				strLabel = m_records[di->itemID]->GetFieldLabel(flds[i], m_engine);
			}
			strLabel += L":";

			rcText.left		= di->rcItem.left + 58;
			rcText.right	= rcText.left + m_labelsWidth;

			SelectFont(hdc, m_fntNormal);
			DrawText(hdc, strLabel.c_str(), -1, &rcText, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);

			rcText.left		= rcText.right;
			rcText.right	= di->rcItem.right;

			SelectFont(hdc, m_fntBold);
			if(!val.empty())
			{
				DrawText(hdc, val.c_str(), -1, &rcText, DT_LEFT | DT_BOTTOM | DT_NOPREFIX);
			}

			rcText.top		= rcText.bottom;
			rcText.bottom += m_textHeight;
		}
		if(rcText.bottom > di->rcItem.bottom) break;
	}

	SelectFont(hdc, oldFnt);

	return TRUE;
}

void CImportDlg::calcItemHeight()
{
	HFONT fnt = GetStockFont(DEFAULT_GUI_FONT);
	LOGFONT lf;
	GetObject(fnt, sizeof(LOGFONT), &lf);

	lf.lfQuality = CLEARTYPE_QUALITY;

	lstrcpy(lf.lfFaceName, L"Verdana");

	m_fntNormal		= CreateFontIndirect(&lf);
	lf.lfUnderline	= TRUE;
	m_fntLinks		= CreateFontIndirect(&lf);
	lf.lfUnderline	= FALSE;
	lf.lfWeight		= FW_BOLD;
	m_fntBold		= CreateFontIndirect(&lf);
	lf.lfHeight		= (int) ((double) lf.lfHeight * 3.0 / 2.0);
	m_fntCaption	= CreateFontIndirect(&lf);

	HDC hdc = GetDC(m_hWnd);
	SIZE sz;

	HFONT oldFnt = SelectFont(hdc, m_fntCaption);
	GetTextExtentPoint32(hdc, L"QIqpti", 6, &sz);
	m_captionHeight = sz.cy + 4;

	SelectFont(hdc, m_fntBold);
	GetTextExtentPoint32(hdc, L"QIqpti", 6, &sz);
	m_textHeight = sz.cy + 4;

	SelectFont(hdc, m_fntNormal);

	struct
	{
		LPWSTR strID;
		LPWSTR attr;
		LPWSTR defText;
	} flds[] = 
	{
		{ XUI_FLD_PHONEWORK		},
		{ XUI_FLD_PHONEHOME		},
		{ XUI_FLD_PHONEFAX		},
		{ XUI_FLD_PHONEMOBILE	},
		{ XUI_FLD_PHONEPAGER	},
		{ XUI_FLD_PHONE			},
		{ XUI_FLD_EMAIL			},
		{ XUI_FLD_EMAILADD		},
		{ XUI_FLD_NICKNAME		},
		{ XUI_FLD_URL			},
		{ NULL, NULL, NULL		}
	};

	m_labelsWidth = 0;

	for(int i=0; flds[i].strID; i++)
	{
		SIZE sz;
		GetTextExtentPoint32(hdc, m_engine->getStringDef(flds[i].strID, NULL, flds[i].defText), lstrlen(m_engine->getStringDef(flds[i].strID, NULL, flds[i].defText)), &sz);
		if(sz.cx > m_labelsWidth)
		{
			m_labelsWidth = sz.cx;
		}
	}
	m_labelsWidth += 10;

	struct
	{
		LPWSTR strID;
		LPWSTR attr;
		LPWSTR defText;
	} links[] = 
	{
		{ XUI_IMPORT_SKIP		},
		{ XUI_IMPORT_NEW		},
		{ XUI_IMPORT_UPDATE		},
		{ NULL, NULL, NULL		}
	};

	m_linksWidth	= 0;
	m_linksHeight	= 0;

	for(int i=0; links[i].strID; i++)
	{
		SIZE sz;
		GetTextExtentPoint32(hdc, m_engine->getStringDef(links[i].strID, NULL, links[i].defText), lstrlen(m_engine->getStringDef(links[i].strID, NULL, links[i].defText)), &sz);
		if(sz.cx > m_linksWidth)
		{
			m_linksWidth = sz.cx;
		}
		if(sz.cy > m_linksHeight)
		{
			m_linksHeight = sz.cy;
		}
	}
	m_linksWidth += 10;


	m_itemHeight = m_textHeight * 3 + m_captionHeight;
	if(m_itemHeight < 50)
	{
		m_itemHeight = 50;
	}

	m_itemHeight += 8;

	SelectFont(hdc, oldFnt);
	ReleaseDC(m_hWnd, hdc);
}

BOOL CImportDlg::OnItemClick( HWND hWnd, CUSTOM_LIST_CLICK* clc )
{

	RECT rcLink;

	rcLink.top		= clc->rcItem.top + 4;
	rcLink.left		= clc->rcItem.right - m_linksWidth;
	rcLink.right	= clc->rcItem.right;
	rcLink.bottom	= rcLink.top + m_linksHeight;

	POINT pt;
	pt.x	= clc->x;
	pt.y	= clc->y;
	if(PtInRect(&rcLink, pt))
	{
		HMENU hMenu = CreatePopupMenu();
		AppendMenu(hMenu, MF_STRING, 1, m_engine->getStringDef(XUI_IMPORT_MENU_NEW));
		if(m_records[clc->itemID]->m_ID)
		{
			AppendMenu(hMenu, MF_STRING, 2, m_engine->getStringDef(XUI_IMPORT_MENU_UPDATE));
		}
		AppendMenu(hMenu, MF_STRING, 3, m_engine->getStringDef(XUI_IMPORT_MENU_SKIP));

		pt.x = rcLink.left;
		pt.y = rcLink.bottom;
		MapWindowPoints(hWnd, NULL, &pt, 1);
		
		int ret = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
		switch(ret)
		{
		case 1:
			m_actions[clc->itemID] = IMPORT_ACT_IMPORT;
			m_ctlList->update(clc->itemID);
			break;
		case 2:
			m_actions[clc->itemID] = IMPORT_ACT_UPDATE;
			m_ctlList->update(clc->itemID);
			break;
		case 3:
			m_actions[clc->itemID] = IMPORT_ACT_NONE;
			m_ctlList->update(clc->itemID);
			break;
		}
		updateImportBtn();
	}
	return TRUE;
}

BOOL CImportDlg::OnItemDblClick( HWND hWnd, CUSTOM_LIST_CLICK* clc )
{
	CContactDlg dlg(m_engine, m_btn, NULL);

	dlg.m_rec			= *m_records[clc->itemID];
	dlg.m_categories	= m_btn->GetCategories();
	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		*m_records[clc->itemID] = dlg.m_rec;
		m_ctlList->update(clc->itemID);
	}

	return TRUE;
}

