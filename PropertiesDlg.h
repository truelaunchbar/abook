#pragma once
#include "abookbtn.h"

class CPropertiesDlg :	public CXUIDialog,
						public cairo_container
{
	int						m_groupFieldIDX;
	litehtml::document::ptr	m_html;
	CRecord					m_rec;
	images_cache			m_images;
	CUSTOM_TEMPLATE			m_newTemplate;
	CUSTOM_TEMPLATE			m_curTemplate;

	CXUIComboBox*			m_ctlTemplates;
	CXUIFreeDraw*			m_ctlPreview;
	CXUIList*				m_ctlProgs;
	CXUIUrl*				m_ctlBasedOn;
	litehtml::context*		m_web_context;
public:
	WORD					m_hkNew;
	WORD					m_hkMenu;
	CUSTOM_TEMPLATE			m_template;
	TCHAR					m_defFont[255];
	int						m_defFontSize;
	DWORD					m_groupType;
	WCHAR					m_groupField[255];
	LPTSTR					m_Description;
	SIZE					m_photoSize;
	CProgram::vector		m_progs;
	CTlbContainer*			m_container;
	BOOL					m_interactiveTips;

	CUSTOM_TEMPLATE::vector m_templates;

public:
	CPropertiesDlg(CXUIEngine* engine, litehtml::context* context, CTlbContainer* container);
	virtual ~CPropertiesDlg(void);

	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);

	XUI_BEGIN_DATA_MAP
		XUI_DATA_MAP_WORD	(L"hkNew",			m_hkNew)
		XUI_DATA_MAP_WORD	(L"hkMenu",			m_hkMenu)
		XUI_DATA_MAP_INT	(L"defFontSize",	m_defFontSize)
		XUI_DATA_MAP_INT	(L"groupType",		m_groupType)
		XUI_DATA_MAP_INT	(L"groupField",		m_groupFieldIDX)
		XUI_DATA_MAP_STR	(L"defFont",		m_defFont)
		XUI_DATA_MAP_PSTR	(L"description",	m_Description)
		XUI_DATA_MAP_INT	(L"photoCX",		m_photoSize.cx)
		XUI_DATA_MAP_INT	(L"photoCY",		m_photoSize.cy)
		XUI_DATA_MAP_INT	(L"interactiveTips",m_interactiveTips)
	XUI_END_DATA_MAP

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM	(L"template",	L"combobox",	m_ctlTemplates)
		XUI_BIND_ITEM	(L"preview",	L"freedraw",	m_ctlPreview)
		XUI_BIND_ITEM	(L"progs",		L"list",		m_ctlProgs)
		XUI_BIND_ITEM	(L"base-tpl",	L"url",			m_ctlBasedOn)
	XUI_END_BIND_MAP

	XUI_BEGIN_EVENT_MAP
		XUI_HANDLE_FREEDRAW		(L"preview",			OnDrawPreview)
		XUI_HANDLE_CHANGED		(L"template",			OnTemplateChanged)
		XUI_HANDLE_CLICKED		(L"btnEditTemplate",	OnEditTemplate)
		XUI_HANDLE_CLICKED		(L"btnAddProg",			OnAddProg)
		XUI_HANDLE_CLICKED		(L"btnEditProg",		OnEditProg)
		XUI_HANDLE_CLICKED		(L"btnDelProg",			OnDelProg)
		XUI_HANDLE_LST_DBLCLK	(L"progs",				OnProgsDblClick)
		XUI_HANDLE_CLICKED		(L"base-tpl",			OnBasedOn)
	XUI_END_EVENT_MAP

	void SetTemplates(CUSTOM_TEMPLATE::vector& tpl);
	void SaveTemplates(CUSTOM_TEMPLATE::vector& tpl);

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
	void fillProgs(LPCWSTR selProg);
	void FillTemplates();
	static int CALLBACK MyEnumFonts(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam);

	BOOL OnDrawPreview(HDC hdc, LPRECT rcDraw);
	BOOL OnTemplateChanged();
	BOOL OnEditTemplate();
	BOOL OnBasedOn();

	void init_template(const  CUSTOM_TEMPLATE& tpl);
	BOOL OnAddProg();
	BOOL OnEditProg();
	BOOL OnDelProg();
	BOOL OnProgsDblClick(LPNMHDR hdr);
	void init_html();
	void update_based_on();
};
