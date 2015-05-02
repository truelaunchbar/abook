#pragma once
#include "abookbtn.h"

#define BTN_ID_SAVE		1
#define BTN_ID_LOAD		2
#define BTN_ID_FIELDS	3
#define BTN_ID_LABELS	4
#define BTN_ID_NOTEMPTY	5
#define BTN_ID_IMAGES	6
#define BTN_ID_DELETE	7

class CEditTemplateDlg : public CXUIDialog,
						 public cairo_container
{
	CRecord					m_rec;
	images_cache			m_images;
	litehtml::document::ptr	m_html;

	CXUIFreeDraw*			m_ctlPreview;
	CXUIRTF*				m_ctlTemplate;

	int						m_defFontSize;
	std::wstring			m_defFont;
	SIZE					m_photo_size;
	litehtml::context*		m_web_context;
	CTlbContainer*			m_container;

	LPTSTR		getText();
public:
	CUSTOM_TEMPLATE m_template;
	CUSTOM_TEMPLATE::vector tpls;

	CEditTemplateDlg(CXUIEngine* engine, LPCWSTR fontName, int fontSize, const SIZE& szPhoto, litehtml::context* context, CTlbContainer* container);
	virtual ~CEditTemplateDlg(void);

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM	(L"preview",	L"freedraw",	m_ctlPreview)
		XUI_BIND_ITEM	(L"template",	L"rtf",			m_ctlTemplate)
	XUI_END_BIND_MAP

	XUI_BEGIN_EVENT_MAP
		XUI_HANDLE_CLICKED				(L"updatePreview",	OnUpdatePreview)
		XUI_HANDLE_FREEDRAW				(L"preview",		OnDrawPreview)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbSave",			OnSave)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbLoad",			OnLoad)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbDelete",		OnDelete)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbFields",		OnFields)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbLabels",		OnLabels)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbImages",		OnImages)
		XUI_HANDLE_TOOLBAR_DROPDOWN		(L"tbNotEmpty",		OnNotEmpty)
	XUI_END_EVENT_MAP


	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);

	// cairo_container members
	virtual void	make_url(LPCWSTR url, LPCWSTR basepath, std::wstring& out);
	virtual cairo_container::image_ptr get_image(LPCWSTR url, bool redraw_on_ready);
	virtual void	get_client_rect(litehtml::position& client);
	virtual int		get_default_font_size();
	virtual const litehtml::tchar_t* get_default_font_name();
	virtual	void	set_caption(const litehtml::tchar_t* caption);
	virtual	void	set_base_url(const litehtml::tchar_t* base_url);
	virtual void	link(litehtml::document* doc, litehtml::element::ptr el);
	virtual void	import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl);
	virtual void	on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el);
	virtual	void	set_cursor(const litehtml::tchar_t* cursor);

private:
	BOOL OnDrawPreview(HDC hdc, LPRECT rcDraw);
	BOOL OnFields(LPRECT rcButton);
	BOOL OnLabels(LPRECT rcButton);
	BOOL OnImages(LPRECT rcButton);
	BOOL OnNotEmpty(LPRECT rcButton);
	BOOL OnLoad(LPRECT rcButton);
	BOOL OnSave(LPRECT rcButton);

	void erase_custom_tpl(const CUSTOM_TEMPLATE& tpl);
	BOOL OnDelete(LPRECT rcButton);
	BOOL OnUpdatePreview();
	HMENU create_fields_menu(BOOL with_images = FALSE);
	void update_caption();
};

class CNewTemplateDlg : public CXUIDialog
{
public:
	WCHAR m_name[255];

	CNewTemplateDlg(CXUIEngine* engine);
	~CNewTemplateDlg();

	virtual BOOL OnEndDialog(UINT code);
};