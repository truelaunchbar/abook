#include "globals.h"
#include "edittemplatedlg.h"
#include "resource.h"
#include <Richedit.h>
#include "xuistrings.h"
#include "dc.h"
#include "XMLProps.h"

CEditTemplateDlg::CEditTemplateDlg(CXUIEngine* engine, LPCWSTR fontName, int fontSize, const SIZE& szPhoto, litehtml::context* context, CTlbContainer* container) : CXUIDialog(L"res:template.xml", engine)
{
	m_container = container;
	m_web_context = context;
	m_photo_size = szPhoto;

	m_rec.load_sample();

	HDC hdc = GetDC(NULL);
	m_defFontSize = MulDiv(fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	m_defFont = fontName;
}

CEditTemplateDlg::~CEditTemplateDlg(void)
{
}

void CEditTemplateDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();

	SendMessage(m_ctlTemplate->get_wnd(), EM_SETEVENTMASK, 0, ENM_CHANGE);

	CHARFORMAT fmt;
	ZeroMemory(&fmt, sizeof(fmt));
	fmt.cbSize				= sizeof(fmt);
	fmt.dwMask				= CFM_FACE | CFM_SIZE;
	fmt.bPitchAndFamily		= FIXED_PITCH;

	HDC hdc = GetDC(m_hWnd);
	fmt.yHeight = MulDiv(8 * 20, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(m_hWnd, hdc);

	lstrcpy(fmt.szFaceName, L"Courier New");

	SendMessage(m_ctlTemplate->get_wnd(), EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&fmt);
	m_ctlTemplate->value_STR(m_template.text.c_str());

	m_ctlTemplate->set_TabStopFocus();
	SendMessage(m_ctlTemplate->get_wnd(), EM_SETSEL, -1, 0);

	int tabstops = 14;
	SendMessage(m_ctlTemplate->get_wnd(), EM_SETTABSTOPS, 1, (LPARAM) &tabstops);

	OnUpdatePreview();
	update_caption();
}


BOOL CEditTemplateDlg::OnEndDialog(UINT code)
{
	if(code == IDOK)
	{
		m_template.text = m_ctlTemplate->value_STRdef(L"");
		if (m_template.type == tpl_type_custom && m_template.id)
		{
			m_template.save(m_container);
		}
	}
	return TRUE;
}

void CEditTemplateDlg::make_url(LPCWSTR url, LPCWSTR basepath, std::wstring& out)
{
	out = url;
}

cairo_container::image_ptr CEditTemplateDlg::get_image(LPCWSTR url, bool redraw_on_ready)
{
	return m_rec.get_image(url, &m_images, m_photo_size.cx, m_photo_size.cy);
}

void CEditTemplateDlg::get_client_rect(litehtml::position& client)
{

}

int CEditTemplateDlg::get_default_font_size()
{
	return m_defFontSize;
}

const litehtml::tchar_t* CEditTemplateDlg::get_default_font_name()
{
	return m_defFont.c_str();
}

void CEditTemplateDlg::set_caption(const litehtml::tchar_t* caption)
{

}

void CEditTemplateDlg::set_base_url(const litehtml::tchar_t* base_url)
{

}

void CEditTemplateDlg::link(litehtml::document* doc, litehtml::element::ptr el)
{

}

void CEditTemplateDlg::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
	std::wstring base_path = m_template.get_base_path(m_container);
	if (!base_path.empty())
	{
		base_path += url;
		auto txt = std::auto_ptr<WCHAR>(load_utf8_file(base_path.c_str()));
		if (txt.get())
		{
			text = txt.get();
		}
	}
}

void CEditTemplateDlg::on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el)
{

}

void CEditTemplateDlg::set_cursor(const litehtml::tchar_t* cursor)
{

}

BOOL CEditTemplateDlg::OnLabels(LPRECT rcButton)
{
	HMENU hMenu = create_fields_menu();

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0)
	{
		TCHAR fld[255];
		wsprintf(fld, TEXT("{label:%s}"), g_defText[ret-1].fldName);
		SendMessage(m_ctlTemplate->get_wnd(), EM_REPLACESEL, TRUE, (LPARAM) fld);
		OnUpdatePreview();
	}
	DestroyMenu(hMenu);
	
	return TRUE;
}

BOOL CEditTemplateDlg::OnDelete(LPRECT rcButton)
{
	HMENU hMenu = CreatePopupMenu();

	int idx = 0;
	int cmd = 1;

	auto templates = load_templates(m_container, true);

	for (const auto& tpl : tpls)
	{
		if (tpl.type == tpl_type_custom)
		{
			templates.push_back(tpl);
		}
	}

	if (templates.empty())
	{
		return TRUE;
	}

	for (const auto& tpl : templates)
	{
		if (tpl.id)
		{
			InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, tpl.name.c_str());
		}
		else
		{
			InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, (L"[" + tpl.name + L"]").c_str());
		}
	}

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0 && ret <= (int) templates.size())
	{
		if (templates[ret - 1].id == 0)
		{
			erase_custom_tpl(templates[ret - 1]);
		}
		else
		{
			bool erase_id = false;
			if (templates[ret - 1].id == m_template.id && templates[ret - 1].type == m_template.type)
			{
				erase_id = true;
			}

			if (templates[ret - 1].erase(m_container))
			{
				if (erase_id)
				{
					m_template.id = 0;
					update_caption();
				}
			}
		}
	}
	DestroyMenu(hMenu);

	return TRUE;
}

BOOL CEditTemplateDlg::OnUpdatePreview()
{
	m_images.init_by_template(m_template, m_container);
	cairo_container::clear_images();

	CProgram::vector progs;

	std::wstring tpl = m_ctlTemplate->value_STRdef(L"");
	m_rec.ApplyTemplate(tpl, m_engine, progs);

	WCHAR txt_clr[10];
	wsprintf(txt_clr, L"#%02X%02X%02X", 0, 0, 0);

	std::wstring user_css;

	HRSRC hResource = ::FindResource(g_hInst, L"abook.css", L"CSS");
	if (hResource)
	{
		DWORD imageSize = ::SizeofResource(g_hInst, hResource);
		if (imageSize)
		{
			LPCSTR pResourceData = (LPCSTR) ::LockResource(::LoadResource(g_hInst, hResource));
			if (pResourceData)
			{
				LPWSTR css = new WCHAR[imageSize * 3];
				int ret = MultiByteToWideChar(CP_UTF8, 0, pResourceData, imageSize, css, imageSize * 3);
				css[ret] = 0;
				user_css = css;
				delete css;
			}
		}
	}
	replace_str(user_css, std::wstring(L"$color$"), std::wstring(txt_clr));

	litehtml::css user_style;
	user_style.parse_stylesheet(user_css.c_str(), L"", 0, litehtml::media_query_list::ptr());

	m_html = litehtml::document::createFromString(tpl.c_str(), this, m_web_context, &user_style);
	m_ctlPreview->redraw(TRUE);

	return TRUE;
}

HMENU CEditTemplateDlg::create_fields_menu(BOOL with_images)
{
	std::map<std::wstring, std::wstring> groups;

	for (int i = 0; g_defText[i].fldName; i++)
	{
		if (!with_images)
		{
			if (g_defText[i].inList)
			{
				if (g_defText[i].groupID)
				{
					groups[g_defText[i].groupID] = g_defText[i].groupDefText;
				}
			}
		}
		else
		{
			if (g_defText[i].haveImage)
			{
				if (g_defText[i].groupID)
				{
					groups[g_defText[i].groupID] = g_defText[i].groupDefText;
				}
			}
		}
	}

	HMENU hMenu = CreatePopupMenu();
	HMENU submenu;
	for (auto grp_i = groups.begin(); grp_i != groups.end(); grp_i++)
	{
		submenu = CreatePopupMenu();
		for (int i = 0; g_defText[i].fldName; i++)
		{
			if (g_defText[i].groupID)
			{
				if (!with_images)
				{
					if (g_defText[i].inList && grp_i->first == g_defText[i].groupID)
					{
						InsertMenu(submenu, -1, MF_STRING | MF_BYPOSITION, i + 1, m_engine->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
					}
				}
				else
				{
					if (g_defText[i].haveImage && grp_i->first == g_defText[i].groupID)
					{
						InsertMenu(submenu, -1, MF_STRING | MF_BYPOSITION, i + 1, m_engine->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
					}
				}
			}
		}
		InsertMenu(hMenu, -1, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT_PTR)submenu, m_engine->getStringDef(grp_i->first.c_str(), NULL, (LPWSTR)grp_i->second.c_str()));
	}

	for (int i = 0; g_defText[i].fldName; i++)
	{
		if (!g_defText[i].groupID)
		{
			if (!with_images)
			{
				if (g_defText[i].inList)
				{
					InsertMenu(hMenu, i, MF_STRING | MF_BYPOSITION, i + 1, m_engine->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
				}
			}
			else
			{
				if (g_defText[i].haveImage)
				{
					InsertMenu(hMenu, i, MF_STRING | MF_BYPOSITION, i + 1, m_engine->getStringDef(g_defText[i].strID, g_defText[i].attr, g_defText[i].defText));
				}
			}
		}
	}

	return hMenu;
}

void CEditTemplateDlg::update_caption()
{
	std::wstring caption;
	if (m_template.name.empty() || m_template.type == tpl_type_custom && !m_template.id)
	{
		caption = m_engine->getStringDef(XUI_EDIT_TEMPLATE);
	}
	else
	{
		caption = m_template.name + L" - " + m_engine->getStringDef(XUI_EDIT_TEMPLATE);
	}
	SetWindowText(get_wnd(), caption.c_str());
}

BOOL CEditTemplateDlg::OnSave(LPRECT rcButton)
{
	HMENU hMenu = CreatePopupMenu();

	int idx = 0;
	int cmd = 1;

	auto templates = load_templates(m_container, true);

	for (const auto& tpl : tpls)
	{
		if (tpl.type == tpl_type_custom)
		{
			templates.push_back(tpl);
		}
	}


	for (const auto& tpl : templates)
	{
		if (tpl.id)
		{
			InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, tpl.name.c_str());
		}
		else
		{
			InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, (L"[" + tpl.name + L"]").c_str());
		}
	}

	if (!templates.empty())
	{
		InsertMenu(hMenu, idx++, MF_SEPARATOR | MF_BYPOSITION, 0, TEXT(""));
	}


	InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, m_engine->getStringDef(XUI_STR_SAVEAS));

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0)
	{
		if (ret <= (UINT)templates.size())
		{
			templates[ret - 1].text = m_ctlTemplate->value_STRdef(L"");
			if (templates[ret - 1].save(m_container))
			{
				erase_custom_tpl(m_template);
				m_template = templates[ret - 1];
			}
		} else
		{
			LPCWSTR tpl_text = m_ctlTemplate->value_STR();
			int id = 0;
			if (tpl_text)
			{
				CNewTemplateDlg dlg(m_engine);
				if (dlg.DoModal(m_hWnd) == IDOK)
				{
					CUSTOM_TEMPLATE tpl = m_template;
					tpl.type = tpl_type_custom;
					tpl.id = 0;
					tpl.text = tpl_text;
					tpl.name = dlg.m_name;
					if (tpl.save(m_container))
					{
						erase_custom_tpl(m_template);
						m_template = std::move(tpl);
					}
				}
			}
		}
		update_caption();
	}
	DestroyMenu(hMenu);

	return TRUE;
}

void CEditTemplateDlg::erase_custom_tpl(const CUSTOM_TEMPLATE& tpl)
{
	auto tpl_i = std::find_if(tpls.begin(), tpls.end(), [&tpl](CUSTOM_TEMPLATE& t)
	{
		if (t.name == tpl.name && t.path == tpl.path)
		{
			return true;
		}
		return false;
	});
	if (tpl_i != tpls.end())
	{
		tpls.erase(tpl_i);
	}
}

BOOL CEditTemplateDlg::OnLoad(LPRECT rcButton)
{
	HMENU hMenu = CreatePopupMenu();

	int idx = 0;
	int cmd = 1;

	auto templates = load_templates(m_container);

	for (const auto& tpl : tpls)
	{
		if (tpl.type == tpl_type_custom)
		{
			templates.push_back(tpl);
		}
	}


	for (const auto& tpl : templates)
	{
		if (tpl.type == tpl_type_skin)
		{
			InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, tpl.name.c_str());
		}
		else
		{
			if (!tpl.id)
			{
				InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, (L"[" + tpl.name + L"]").c_str());
			}
			else
			{
				InsertMenu(hMenu, idx++, MF_STRING | MF_BYPOSITION, cmd++, (L"(" + tpl.name + L")").c_str());
			}
		}
	}

	if (!templates.empty())
	{
		InsertMenu(hMenu, idx++, MF_SEPARATOR | MF_BYPOSITION, 0, TEXT(""));
	}

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if (ret > 0 && ret <= (int) templates.size())
	{
		m_template = templates[ret - 1];
		m_template.load_text(m_container);
		m_ctlTemplate->value_STR(m_template.text.c_str());
		m_template.type = tpl_type_custom;
		OnUpdatePreview();
		update_caption();
	}
	DestroyMenu(hMenu);

	return TRUE;
}

BOOL CEditTemplateDlg::OnFields(LPRECT rcButton)
{
	HMENU hMenu = create_fields_menu();

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0)
	{
		TCHAR fld[255];
		wsprintf(fld, TEXT("{%s}"), g_defText[ret-1].fldName);
		SendMessage(m_ctlTemplate->get_wnd(), EM_REPLACESEL, TRUE, (LPARAM) fld);
		OnUpdatePreview();
	}
	DestroyMenu(hMenu);

	return TRUE;
}

BOOL CEditTemplateDlg::OnNotEmpty(LPRECT rcButton)
{
	HMENU hMenu = create_fields_menu();

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0)
	{
		CHARRANGE rng;
		SendMessage(m_ctlTemplate->get_wnd(), EM_EXGETSEL, 0, (LPARAM) &rng);
		int selSZ = rng.cpMax - rng.cpMin;
		if(selSZ < 0)
		{
			selSZ = GetWindowTextLength(m_ctlTemplate->get_wnd());
		}
		LPTSTR oldText = new TCHAR[selSZ + 10];
		SendMessage(m_ctlTemplate->get_wnd(), EM_GETSELTEXT, 0, (LPARAM) oldText);


		LPTSTR newText = new TCHAR[selSZ + 100];
		lstrcpy(newText, TEXT("{ifnotempty:"));
		lstrcat(newText, g_defText[ret-1].fldName);
		lstrcat(newText, TEXT("}"));
		lstrcat(newText, oldText);
		lstrcat(newText, TEXT("{endif}"));

		SendMessage(m_ctlTemplate->get_wnd(), EM_REPLACESEL, TRUE, (LPARAM) newText);
		delete newText;
		delete oldText;
	}
	DestroyMenu(hMenu);

	return TRUE;
}

BOOL CEditTemplateDlg::OnImages(LPRECT rcButton)
{
	HMENU hMenu = create_fields_menu(TRUE);

	UINT ret = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN, rcButton->left, rcButton->bottom, NULL, m_hWnd, NULL);
	if(ret > 0)
	{
		TCHAR fld[255];
		wsprintf(fld, TEXT("{img:%s}"), g_defText[ret-1].fldName);
		SendMessage(m_ctlTemplate->get_wnd(), EM_REPLACESEL, TRUE, (LPARAM) fld);
		OnUpdatePreview();
	}
	DestroyMenu(hMenu);

	return TRUE;
}

BOOL CEditTemplateDlg::OnDrawPreview( HDC hdc, LPRECT rcDraw )
{
	if (m_html)
	{
		m_html->render(rcDraw->right - rcDraw->left);

		tlb::dc tlb_hdc;
		tlb_hdc.begin_paint(hdc, rcDraw);

		BitBlt(tlb_hdc, rcDraw->left, rcDraw->top,
			rcDraw->right - rcDraw->left,
			rcDraw->bottom - rcDraw->top, hdc, rcDraw->left, rcDraw->top, SRCCOPY);

		m_html->draw((litehtml::uint_ptr) ((cairo_t*)tlb_hdc), rcDraw->left, rcDraw->top, nullptr);
		tlb_hdc.end_paint(true);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

CNewTemplateDlg::CNewTemplateDlg(CXUIEngine* engine) : CXUIDialog(L"res:template-name.xml", engine)
{
	m_name[0] = 0;
}

CNewTemplateDlg::~CNewTemplateDlg()
{
}

BOOL CNewTemplateDlg::OnEndDialog(UINT code)
{
	if(code == IDOK)
	{
		StringCchCopy(m_name, 255, find(L"name")->value_STRdef(L""));
		if(!m_name[0])
		{
			showTipMessage(L"name", L"error");
			return FALSE;
		}
	}
	return TRUE;
}
