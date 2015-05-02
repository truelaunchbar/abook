#include "globals.h"
#include "PropertiesDlg.h"
#include "xuistrings.h"
#include "resource.h"
#include "EditTemplateDlg.h"
#include "EditProgDlg.h"
#include "dc.h"

DEF_TEMPLATES g_defTemplates[] = 
{
	{XUI_STR_TPL_DEFAULT,	IDR_TPL_DEFAULT},
	{XUI_STR_TPL_MINI,		IDR_TPL_MINI},
	{XUI_STR_TPL_EXTRA,		IDR_TPL_EXTRAPHOTO},
	{0,0,0,					0}
};

LPCWSTR g_abGroupFields[] = 
{
	L"category",
	L"city",
	L"zip",
	L"state",
	L"country",
	L"organization",
	L"title",
	L"department",
	NULL
};


CPropertiesDlg::CPropertiesDlg(CXUIEngine* engine, litehtml::context* context, CTlbContainer* container) : CXUIDialog(L"res:properties.xml", engine)
{
	m_interactiveTips = FALSE;
	m_container		= container;
	m_web_context	= context;
	m_Description	= NULL;
	m_hkNew			= 0;
	m_defFont[0]	= 0;
	m_defFontSize	= 0;
	m_rec.load_sample();
}

CPropertiesDlg::~CPropertiesDlg(void)
{
	FREE_CLEAR_STR(m_Description);
	m_templates.clear();
}

void CPropertiesDlg::SetTemplates(CUSTOM_TEMPLATE::vector& tpl)
{
	m_templates = load_templates(m_container);
	for (auto& t : tpl)
	{
		if (t.type == tpl_type_custom && !t.id)
		{
			m_templates.push_back(t);
		}
	}
}

void CPropertiesDlg::SaveTemplates(CUSTOM_TEMPLATE::vector& tpl)
{
	tpl.clear();

	for (auto& t : m_templates)
	{
		if (t.type == tpl_type_custom && !t.id)
		{
			tpl.push_back(t);
		}
	}
}

void CPropertiesDlg::make_url(LPCWSTR url, LPCWSTR basepath, std::wstring& out)
{
	out = url;
}

cairo_container::image_ptr CPropertiesDlg::get_image(LPCWSTR url, bool redraw_on_ready)
{
	return m_rec.get_image(url, &m_images, find(L"photoCX")->value_INT(), find(L"photoCY")->value_INT());
}

void CPropertiesDlg::get_client_rect(litehtml::position& client)
{

}

int CPropertiesDlg::get_default_font_size()
{
	HDC hdc = GetDC(NULL);
	int ret = MulDiv(find(L"defFontSize")->value_INT(), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(NULL, hdc);
	return ret;
}

const litehtml::tchar_t* CPropertiesDlg::get_default_font_name()
{
	return find(L"defFont")->value_STRdef(L"Tahoma");
}

void CPropertiesDlg::set_caption(const litehtml::tchar_t* caption)
{

}

void CPropertiesDlg::set_base_url(const litehtml::tchar_t* base_url)
{

}

void CPropertiesDlg::link(litehtml::document* doc, litehtml::element::ptr el)
{

}

void CPropertiesDlg::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
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

void CPropertiesDlg::on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el)
{

}

void CPropertiesDlg::set_cursor(const litehtml::tchar_t* cursor)
{

}

void CPropertiesDlg::OnInitDialog()
{
	CXUIDialog::OnInitDialog();

	m_curTemplate = m_template;

	// init group field index
	m_groupFieldIDX = 0;
	for(int i=0; g_abGroupFields[i]; i++)
	{
		if(!StrCmpI(g_abGroupFields[i], m_groupField))
		{
			m_groupFieldIDX = i;
			break;
		}
	}

	// fill vertion tab
	TCHAR str[MAX_PATH];
	GetModuleFileName(g_hInst, str, MAX_PATH);
	DWORD versionLS, versionMS;
	GetFileVersion(str, versionMS, versionLS);

	TCHAR strVer[255];
	Version2Str(versionMS, versionLS, strVer);
	TCHAR strText[255];
	wsprintf(strText, TEXT("v%s"), strVer);

	find(L"version")->value_STR(strText);

	FillTemplates();
	fillProgs(NULL);

	// fill the fonts
	HDC hdc = GetDC(m_hWnd);
	LOGFONT lf;
	ZeroMemory(&lf, sizeof(lf));
	lf.lfCharSet = DEFAULT_CHARSET;
	EnumFontFamiliesEx(hdc, &lf, (FONTENUMPROC) MyEnumFonts, (LPARAM) find(L"defFont")->get_wnd(), 0);
	ReleaseDC(m_hWnd, hdc);

	dataExchange(FALSE, NULL);
	init_html();
	update_based_on();
}

BOOL CPropertiesDlg::OnEndDialog( UINT code )
{
	if(code == IDOK)
	{
		dataExchange(TRUE, NULL);
		lstrcpy(m_groupField, g_abGroupFields[m_groupFieldIDX]);
	}
	return TRUE;
}

void CPropertiesDlg::FillTemplates()
{
	m_ctlTemplates->clearItems();

	int selectedID = 0;

	for (int i = 0; i < (int)m_templates.size() && !selectedID; i++)
	{
		if (m_template.type == tpl_type_custom)
		{
			if (m_templates[i].type == tpl_type_custom)
			{
				if (m_template.id == m_templates[i].id)
				{
					if (!m_template.id)
					{
						if (m_template.name == m_templates[i].name)
						{
							selectedID = i + 1;
						}
					}
					else
					{
						selectedID = i + 1;
					}
				}
			}
		}
		else
		{
			if (m_template.path == m_templates[i].path)
			{
				selectedID = i + 1;
			}
		}
	}

	if (!selectedID)
	{
		m_ctlTemplates->addItem(0, m_engine->getStringDef(XUI_STR_TPL_CURRENT));
	}

	for(int i=0; i < (int) m_templates.size(); i++)
	{
		if (m_templates[i].type == tpl_type_custom)
		{
			if (m_templates[i].id)
			{
				m_ctlTemplates->addItem(i + 1, (L"(" + m_templates[i].name + L")").c_str());
			}
			else
			{
				m_ctlTemplates->addItem(i + 1, (L"[" + m_templates[i].name + L"]").c_str());
			}
		}
		else
		{
			m_ctlTemplates->addItem(i + 1, m_templates[i].name.c_str());
		}
	}

	m_ctlTemplates->value_INT(selectedID);
}

BOOL CPropertiesDlg::OnDrawPreview( HDC hdc, LPRECT rcDraw )
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

int CALLBACK CPropertiesDlg::MyEnumFonts(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	HWND hwndCB = (HWND) lParam;
	if(SendMessage(hwndCB, CB_FINDSTRING, NULL, (LPARAM) lpelfe->elfFullName) == CB_ERR)
	{
		SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM) lpelfe->elfFullName);
	}
	return TRUE;
}

BOOL CPropertiesDlg::OnTemplateChanged()
{
	int idx = m_ctlTemplates->value_INT();

	if(idx == 0)
	{
		if(!m_newTemplate.text.empty())
		{
			m_template = m_newTemplate;
		} else
		{
			m_template = m_curTemplate;
		}
	} else
	{
		idx = idx - 1;
		m_template = m_templates[idx];
	}
	m_template.load_text(m_container);
	init_html();
	m_ctlPreview->redraw(TRUE);
	update_based_on();

	return TRUE;
}

BOOL CPropertiesDlg::OnEditTemplate()
{
	dataExchange(TRUE, NULL);
	CEditTemplateDlg dlg(m_engine, m_defFont, m_defFontSize, m_photoSize, m_web_context, m_container);
	SaveTemplates(dlg.tpls);
	dlg.m_template = m_template;
	dlg.m_template.type = tpl_type_custom;
	if (dlg.DoModal(m_hWnd) == IDOK)
	{
		SetTemplates(dlg.tpls);
		init_template(dlg.m_template);
		m_newTemplate = dlg.m_template;
		FillTemplates();
	}
	return TRUE;
}

BOOL CPropertiesDlg::OnBasedOn()
{
	if (m_template.type == tpl_type_custom && !m_template.path.empty())
	{
		int idx = 0;
		for (const auto& tpl : m_templates)
		{
			idx++;
			if (tpl.type == tpl_type_skin && tpl.path == m_template.path)
			{
				init_template(tpl);
				m_ctlTemplates->value_INT(idx);
				break;
			}
		}
	}

	return TRUE;
}

void CPropertiesDlg::init_template(const CUSTOM_TEMPLATE& tpl)
{
	m_template = tpl;
	m_template.load_text(m_container);
	init_html();
	m_ctlPreview->redraw(TRUE);
	update_based_on();
}

BOOL CPropertiesDlg::OnAddProg()
{
	CEditProgDlg dlg(m_engine);
	if(dlg.DoModal(m_hWnd) == IDOK)
	{
		m_progs.push_back(dlg.m_prog);
		fillProgs(dlg.m_prog.getField().c_str());
	}
	return TRUE;
}

BOOL CPropertiesDlg::OnEditProg()
{
	int idx = (int) m_ctlProgs->getSelected();
	if(idx >= 0)
	{
		CEditProgDlg dlg(m_engine);
		dlg.m_prog = m_progs[idx];
		if(dlg.DoModal(m_hWnd) == IDOK)
		{
			m_progs[idx] = dlg.m_prog;
			fillProgs(dlg.m_prog.getField().c_str());
		}
	}
	return TRUE;
}

BOOL CPropertiesDlg::OnDelProg()
{
	int idx = (int) m_ctlProgs->getSelected();
	if(idx >= 0)
	{
		m_progs.erase(m_progs.begin() + idx);
		fillProgs(NULL);
	}
	return TRUE;
}

BOOL CPropertiesDlg::OnProgsDblClick( LPNMHDR hdr )
{
	return OnEditProg();
}

void CPropertiesDlg::init_html()
{
	m_images.init_by_template(m_template, m_container);
	cairo_container::clear_images();

	CProgram::vector progs;

	std::wstring tpl = m_template.text;
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
}

void CPropertiesDlg::update_based_on()
{
	std::wstring name;
	if (m_template.type == tpl_type_custom && !m_template.path.empty())
	{
		name = get_skin_name(m_container, m_template.path);
		m_ctlBasedOn->set_hidden(0);
	}
	else
	{
		name = m_engine->getStringDef(XUI_BASED_NONE);
		m_ctlBasedOn->set_hidden(1);
	}
	m_ctlBasedOn->setData(name.c_str(), L":notify:");
}

void CPropertiesDlg::fillProgs(LPCWSTR selProg)
{
	m_ctlProgs->clearItems();
	for(size_t i = 0; i < m_progs.size(); i++)
	{
		std::wstring fldName = m_progs[i].getField();
		for(int j = 0; g_defText[j].fldName; j++)
		{
			if(fldName == g_defText[j].fldName)
			{
				fldName = m_engine->getStringDef(g_defText[j].strID, g_defText[j].attr, g_defText[j].defText);
				break;
			}
		}
		int idx = m_ctlProgs->insertItem(fldName.c_str(), i);
		m_ctlProgs->setItemText(idx, 1, m_progs[i].getCmdLine().c_str());
		if(selProg && m_progs[i].getField() == selProg)
		{
			m_ctlProgs->selectItem(idx);
		}
	}
}
