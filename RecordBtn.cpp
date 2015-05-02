#include "globals.h"
#include ".\recordbtn.h"
#include "abookbtn.h"
#include "md5.h"
#include "dc.h"


CRecordBtn::CRecordBtn(CAddressBook* parent, UINT recID, IRecordParent*	recParent)
{
	ZeroMemory(m_recalcDigest, sizeof(m_recalcDigest));

	m_recParent		= recParent;
	m_text = TEXT("");
	m_parent = parent;
	m_recID = recID;
	InitializeCriticalSection(&m_sync);
	std::wstring name = m_parent->GetText(m_recID, L"{displayname}");
	StringCchCopy(m_Name, MAX_PATH, name.c_str());
	CRecord rec;
	if(parent->GetRecord(recID, rec))
	{
		m_btnText = rec.GetFieldValue(L"displayname");
	}
}

CRecordBtn::~CRecordBtn(void)
{
	DeleteCriticalSection(&m_sync);
	m_html = nullptr;
}

void CRecordBtn::GetChildUID(LPWSTR uid)
{
	wsprintf(uid, TEXT("rec:%d"), m_recID);
}

void CRecordBtn::GetSize(SIZE* sz, BOOL actual)
{
	Lock();
	if (m_html)
	{
		RECT txtMargins = { 0, 0, 0, 0 };
		m_container->GetContentMargins(&txtMargins);

		int max_width = m_container->GetMaxMenuWidth() - txtMargins.left - txtMargins.right;
		sz->cx = m_html->render(max_width);
		sz->cy = m_html->height();
	}
	Unlock();
}

int CRecordBtn::GetHeight(int width, int defHeight, HDC hDC)
{
	m_html->render(width);
	return m_html->height();
}

void CRecordBtn::OnDraw(HDC hDC, LPRECT rcItem)
{
	Lock();

	if (m_html)
	{
		tlb::dc hdc;
		hdc.begin_paint(hDC, rcItem);

		BitBlt(hdc, rcItem->left, rcItem->top,
			rcItem->right - rcItem->left,
			rcItem->bottom - rcItem->top, hDC, rcItem->left, rcItem->top, SRCCOPY);

		m_html->draw((litehtml::uint_ptr) ((cairo_t*)hdc), rcItem->left, rcItem->top, nullptr);
		hdc.end_paint(true);
	}

	Unlock();
}

UINT CRecordBtn::GetModeFlags(void)
{
	return BTN_FLAG_NOTEXT;
}

void CRecordBtn::GetButtonText(LPWSTR text) 
{
	if(!m_btnText.empty())
	{
		StringCchCopy(text, 500, m_btnText.c_str());
	}
}

BOOL CRecordBtn::OnRun(void)
{
	return TRUE;
}

UINT CRecordBtn::ReadChanges(void)
{
	if(m_recParent->isDeletedRecord(m_recID))
	{
		return 2;
	}
	std::wstring txt = m_parent->GetText(m_recID);
	if (txt != m_text || needRecalc())
	{
		create_html_document(txt.c_str());

		return READ_CHANGES_RECALC | READ_CHANGES_REDRAW;
	}
	return 0;
}

BOOL CRecordBtn::OnDelete(void)
{
	m_parent->DeleteRecord(m_recID);
	m_recParent->NotfyDelete();
	return TRUE;
}

void CRecordBtn::create_html_document(LPCTSTR txt)
{
	Lock();

	COLORREF clr = m_container->GetTextColor();
	WCHAR txt_clr[10];
	wsprintf(txt_clr, L"#%02X%02X%02X", GetRValue(clr), GetGValue(clr), GetBValue(clr));

	std::wstring user_css = load_text_from_resource(L"abook.css", L"CSS");
	replace_str(user_css, std::wstring(L"$color$"), std::wstring(txt_clr));

	litehtml::css user_style;
	user_style.parse_stylesheet(user_css.c_str(), L"", 0, litehtml::media_query_list::ptr());

	m_text = txt;
	m_html = litehtml::document::createFromString(m_text.c_str(), this, &m_parent->get_web_context(), &user_style);
	getRecalcDigest(m_recalcDigest);

	Unlock();
}

LPWSTR CRecordBtn::GetTipCaption(int tipID)
{
/*
	CRecord rec;
	if(m_parent->GetRecord(m_recID, rec))
	{
		std::wstring dname = rec.GetFieldValue(L"displayname");
		if(!dname.empty())
		{
			LPWSTR name = (LPWSTR) CoTaskMemAlloc((dname.length() + 1) * sizeof(TCHAR));
			lstrcpy(name, dname.c_str());
			return name;
		} else
		{
			LPWSTR name = (LPWSTR) CoTaskMemAlloc((lstrlen(L"") + 1) * sizeof(TCHAR));
			lstrcpy(name, L"");
			return name;
		}
	}
*/
	return NULL;
}

LPWSTR CRecordBtn::GetTipText(int tipID)
{
	CRecord rec;
	if(m_parent->GetRecord(m_recID, rec))
	{
		std::wstring strTip;
		BOOL isFirst = TRUE;
		for(int i=1; i <= 7; i++)
		{
			TCHAR valName[50];
			wsprintf(valName, L"phone%d", i);
			std::wstring phone = rec.GetFieldValue(valName);
			if(!phone.empty())
			{
				std::wstring label = rec.GetFieldLabel(rec.GetFieldType(valName).c_str(), &m_parent->m_xui);

				wsprintf(valName, L"phonenote%d", i);
				std::wstring note = rec.GetFieldValue(valName);

				if(!isFirst)
				{
					strTip += TEXT("<br>");
				}
				strTip += label;
				strTip += TEXT(":<t><b>");
				strTip += phone;
				if(!note.empty())
				{
					strTip += TEXT(" - ");
					strTip += note;
				}
				strTip += TEXT("</b>");
				isFirst = FALSE;
			}
		}
		for(auto& rec_iter : rec.m_fields)
		{
			if (StrCmpI(rec_iter.name.c_str(), L"displayname") && StrCmpNI(rec_iter.name.c_str(), L"phone", 5))
			{
				std::wstring label = rec.GetFieldLabel(rec_iter.name.c_str(), &m_parent->m_xui);
				if(!isFirst)
				{
					strTip += TEXT("<br>");
				}
				strTip += label;
				strTip += TEXT(":<t><b>");
				strTip += rec_iter.value;
				strTip += TEXT("</b>");
				isFirst = FALSE;
			}
		}
		LPWSTR name = (LPWSTR) CoTaskMemAlloc((strTip.length() + 1) * sizeof(TCHAR));
		lstrcpy(name, strTip.c_str());
		return name;
	}
	return NULL;
}

LPWSTR CRecordBtn::GetTipFullHTML(int tipID)
{
	CRecord rec;
	if (m_parent->GetRecord(m_recID, rec))
	{
		std::wstring strTip = load_text_from_resource(m_parent->use_interactive_tips() ? L"tips-int.html" : L"tips.html", L"TEMPLATE");

		rec.ApplyTemplate(strTip, &m_parent->m_xui, m_parent->get_progs());

		LPWSTR name = (LPWSTR)CoTaskMemAlloc((strTip.length() + 1) * sizeof(TCHAR));
		lstrcpy(name, strTip.c_str());
		return name;
	}
	return NULL;
}

void* CRecordBtn::GetTipImage(int tipID, LPCWSTR url, int& width, int& height, BOOL redraw_on_ready)
{
	void* ret = nullptr;
	width = 0;
	height = 0;

	cairo_container::image_ptr img = m_parent->GetIcon(m_recID, url);
	if (img)
	{
		width = img->getWidth();
		height = img->getHeight();

		ret = CoTaskMemAlloc(width * height * 4);
		memcpy(ret, img->getBits(), width * height * 4);
	}

	return ret;
}

void CRecordBtn::OnBeginDrag(IDataObject* lpObj)
{
	{ // Set the RECORD_CLIPBOARD_FORMAT
		FORMATETC fmt;
		fmt.cfFormat = (WORD) g_clipRecord;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.ptd = NULL;
		fmt.tymed = TYMED_HGLOBAL;

		STGMEDIUM med;
		ZeroMemory(&med, sizeof(med));

		med.tymed = TYMED_HGLOBAL;
		med.pUnkForRelease = NULL;
		med.hGlobal = GlobalAlloc(GHND, sizeof(UINT));
		DWORD* df = (DWORD*) GlobalLock(med.hGlobal);
		df[0] = m_recID;
		GlobalUnlock(med.hGlobal);
		lpObj->SetData(&fmt, &med, TRUE);
	}

	{ // Set the PARENT_CLIPBOARD_FORMAT
		FORMATETC fmt;
		fmt.cfFormat = (WORD) g_clipParent;
		fmt.dwAspect = DVASPECT_CONTENT;
		fmt.lindex = -1;
		fmt.ptd = NULL;
		fmt.tymed = TYMED_HGLOBAL;

		STGMEDIUM med;
		ZeroMemory(&med, sizeof(med));

		med.tymed = TYMED_HGLOBAL;
		med.pUnkForRelease = NULL;
		med.hGlobal = GlobalAlloc(GHND, sizeof(IRecordParent*));
		IRecordParent** df = (IRecordParent**) GlobalLock(med.hGlobal);
		df[0] = m_recParent;
		GlobalUnlock(med.hGlobal);
		lpObj->SetData(&fmt, &med, TRUE);
	}
}

BOOL CRecordBtn::applyMargins()
{
	return TRUE;
}

BOOL CRecordBtn::OnMouseOver( long keys, int x, int y )
{
	if (m_html)
	{
		litehtml::position::vector boxes;
		if (m_html->on_mouse_over(x - m_rc.left, y - m_rc.top, x - m_rc.left, y - m_rc.top, boxes))
		{
			m_container->Redraw();
		}
	}
	return TRUE;
}

BOOL CRecordBtn::OnMouseLeave( long keys )
{
	if (m_html)
	{
		litehtml::position::vector boxes;
		if (m_html->on_mouse_leave(boxes))
		{
			m_container->Redraw();
		}
	}

	return TRUE;
}

BOOL CRecordBtn::OnMouseEnter( long keys, int x, int y )
{
	if (m_html)
	{
		litehtml::position::vector boxes;
		if (m_html->on_mouse_over(x - m_rc.left, y - m_rc.top, x - m_rc.left, y - m_rc.top, boxes))
		{
			m_container->Redraw();
		}
	}
	return TRUE;
}

BOOL CRecordBtn::OnLButtonDown( long key, int x, int y )
{
	if (m_html)
	{
		litehtml::position::vector boxes;
		if (m_html->on_lbutton_down(x - m_rc.left, y - m_rc.top, x - m_rc.left, y - m_rc.top, boxes))
		{
			m_container->Redraw();
		}
	}
	return TRUE;
}

BOOL CRecordBtn::OnLButtonUp(long key, int x, int y)
{
	if (m_html)
	{
		litehtml::position::vector boxes;
		if (m_html->on_lbutton_up(x - m_rc.left, y - m_rc.top, x - m_rc.left, y - m_rc.top, boxes))
		{
			m_container->Redraw();
		}
	}
	if (!m_run_url.empty())
	{
		if (m_run_url[0] != L':')
		{
			ShellExecute(NULL, NULL, m_run_url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			m_parent->runLink(m_run_url.c_str());
		}
		m_run_url.clear();
	}
	else
	{
		m_parent->onEditContact(m_recID, m_recParent);
	}
	return TRUE;
}

BOOL CRecordBtn::OnSetCursor( WPARAM wParam, LPARAM lParam )
{
	if (m_html)
	{
		if (m_cursor == L"pointer")
		{
			SetCursor(LoadCursor(NULL, IDC_HAND));
			return TRUE;
		}
	}
	return FALSE;
}

void CRecordBtn::SetContainer(CTlbContainer* container)
{
	CTlbButton::SetContainer(container);

	std::wstring txt = m_parent->GetText(m_recID);
	create_html_document(txt.c_str());
}

HICON CRecordBtn::GetTipIcon(int tipID)
{
	cairo_container::image_ptr img = m_parent->GetIcon(m_recID, L"photo", 100, 100);
	if(img)
	{
		return (HICON) 0xFFFFFFFF;
	}
	return NULL;
}

UINT CRecordBtn::GetTipFlags(int tipID)
{
	if (!m_parent->use_interactive_tips()) return 0;
	return TIP_F_INTERACTIVE;
}

BOOL CRecordBtn::OnTipClick(int tipID, LPCWSTR url)
{
	std::wstring run = url ? url : L"";

	if (!run.empty())
	{
		if (run[0] != L':')
		{
			ShellExecute(NULL, NULL, run.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		else
		{
			m_parent->runLink(run.c_str());
		}
		run.clear();
	}

	return TRUE;
}

int CRecordBtn::GetTipsIconSize(int tipID)
{
	return 100;
}

void CRecordBtn::DrawTipsIcon( HDC hDC, POINT pt, int tipID )
{
	cairo_container::image_ptr img = m_parent->GetIcon(m_recID, L"photo", 100, 100);
	if(img)
	{
		img->draw(hDC, pt.x, pt.y);
	}
}

BOOL CRecordBtn::needRecalc()
{
	unsigned char digest[16];
	getRecalcDigest(digest);
	if(!memcmp(digest, m_recalcDigest, 16))
	{
		return FALSE;
	}
	return TRUE;
}

void CRecordBtn::make_url(LPCWSTR url, LPCWSTR basepath, std::wstring& out)
{
	out = url;
}

cairo_container::image_ptr CRecordBtn::get_image(LPCWSTR url, bool redraw_on_ready)
{
	return m_parent->GetIcon(m_recID, url);
}

void CRecordBtn::get_client_rect(litehtml::position& client)
{

}

int CRecordBtn::get_default_font_size()
{
	return m_parent->m_defFontSizePx;
}

const litehtml::tchar_t* CRecordBtn::get_default_font_name()
{
	return m_parent->m_defFont;
}

void CRecordBtn::set_caption(const litehtml::tchar_t* caption)
{

}

void CRecordBtn::set_base_url(const litehtml::tchar_t* base_url)
{

}

void CRecordBtn::link(litehtml::document* doc, litehtml::element::ptr el)
{

}

void CRecordBtn::on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el)
{
	m_run_url = url ? url : L"";
}

void CRecordBtn::set_cursor(const litehtml::tchar_t* cursor)
{
	m_cursor = cursor ? cursor : L"";
}

void CRecordBtn::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
	std::wstring base_path = m_parent->GetBasePath();
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

void CRecordBtn::getRecalcDigest(unsigned char digest[16])
{
	MD5_CTX ctx;
	MD5Init(&ctx);
	RECT rcMargins = {0, 0, 0, 0};
	m_container->GetContentMargins(&rcMargins);
	COLORREF glow = m_container->GetTextGlowColor();
	COLORREF text_color = m_container->GetTextColor();

	MD5Update(&ctx, (unsigned char*) &rcMargins,				sizeof(RECT));
	MD5Update(&ctx, (unsigned char*) m_parent->m_defFont,		lstrlen(m_parent->m_defFont) * sizeof(WCHAR));
	MD5Update(&ctx, (unsigned char*) &m_parent->m_defFontSize,	sizeof(m_parent->m_defFontSize));
	MD5Update(&ctx, (unsigned char*)&glow, sizeof(glow));
	MD5Update(&ctx, (unsigned char*)&text_color, sizeof(text_color));

	MD5Final(digest, &ctx);
}
