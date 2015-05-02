#pragma once

#include <map>

class CAddressBook;
class IRecordParent;

#define MID_REC_MOVETO		0
#define MID_REC_BEGINCATS	1

class CRecordBtn :	public CTlbButton,
					public cairo_container
{
	CAddressBook*	m_parent;
	UINT			m_recID;
	IRecordParent*	m_recParent;
	unsigned char	m_recalcDigest[16];
	
	CRITICAL_SECTION m_sync;

	litehtml::document::ptr	m_html;
	std::wstring	m_btnText;

	std::wstring	m_text;
	std::wstring	m_cursor;
	std::wstring	m_run_url;
	
	void Lock() { EnterCriticalSection(&m_sync); }
	void Unlock() { LeaveCriticalSection(&m_sync); }
public:
	CRecordBtn(CAddressBook* parent, UINT recID, IRecordParent*	recParent);
	~CRecordBtn(void);

	virtual void GetChildUID(LPWSTR uid);
	virtual void GetSize(SIZE* sz, BOOL actual);
	virtual int  GetHeight(int width, int defHeight, HDC hDC);
	virtual void OnDraw(HDC hDC, LPRECT rcItem);
	virtual LPWSTR GetTipCaption(int tipID);
	virtual LPWSTR GetTipText(int tipID);
	virtual LPWSTR GetTipFullHTML(int tipID);
	virtual void* GetTipImage(int tipID, LPCWSTR url, int& width, int& height, BOOL redraw_on_ready);
	virtual HICON GetTipIcon(int tipID);
	virtual UINT GetTipFlags(int tipID);
	virtual BOOL OnTipClick(int tipID, LPCWSTR url);
	virtual int GetTipsIconSize(int tipID);
	virtual void DrawTipsIcon(HDC hDC, POINT pt, int tipID);
	virtual void OnBeginDrag(IDataObject* lpObj);

	void getRecalcDigest(unsigned char digest[16]);
	BOOL needRecalc();

	// cairo_container members
	virtual void	make_url(LPCWSTR url, LPCWSTR basepath, std::wstring& out);
	virtual cairo_container::image_ptr get_image(LPCWSTR url, bool redraw_on_ready);
	virtual void	get_client_rect(litehtml::position& client);
	virtual int		get_default_font_size();
	virtual const litehtml::tchar_t* get_default_font_name();
	virtual	void	set_caption(const litehtml::tchar_t* caption);
	virtual	void	set_base_url(const litehtml::tchar_t* base_url);
	virtual void	link(litehtml::document* doc, litehtml::element::ptr el);
	virtual void	on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el);
	virtual	void	set_cursor(const litehtml::tchar_t* cursor);
	virtual void	import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl);

	virtual BOOL OnMouseOver(long keys, int x, int y);
	virtual BOOL OnMouseLeave(long keys);
	virtual BOOL OnMouseEnter(long keys, int x, int y);
	virtual BOOL OnLButtonDown(long key, int x, int y);
	virtual BOOL OnLButtonUp(long key, int x, int y);
	virtual BOOL OnSetCursor(WPARAM wParam, LPARAM lParam);
	virtual void SetContainer(CTlbContainer* container);

	virtual BOOL applyMargins();

	UINT GetModeFlags(void);
	void GetButtonText(LPWSTR text);
	BOOL OnRun(void);
	UINT ReadChanges(void);
	BOOL OnDelete(void);

private:
	void create_html_document(LPCTSTR txt);
};
