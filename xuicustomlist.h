#pragma once
#include <vector>

#define XUI_CUSTLIST_CLASS	L"XUI_CUSTLIST_CLASS"

#define XUI_EVENT_CUSTOMLIST_DRAWITEM			L"cuslst-drawitem"
#define XUI_EVENT_CUSTOMLIST_ITEMCLICK			L"cuslst-itemclick"
#define XUI_EVENT_CUSTOMLIST_ITEMDBLCLICK		L"cuslst-itemdblclick"

struct CUSTOM_LIST_DRAWITEM
{
	UINT_PTR	itemID;
	RECT		rcItem;
};

struct CUSTOM_LIST_CLICK
{
	UINT_PTR	itemID;
	RECT		rcItem;
	int			x;
	int			y;
};

class CXUICustomList : public CXUIElement
{
	struct ITEM
	{
		UINT_PTR	itemID;
		int			top;
		int			height;
	};

	int					m_scrollPos;
	std::vector<ITEM>	m_items;
public:
	CXUICustomList(CXUIElement* parent, CXUIEngine* engine);
	virtual ~CXUICustomList();

	IMPLEMENT_INTERFACE(L"customlist")

	virtual void Init();

	void addItem(UINT_PTR itemID, int height);
	void delItem(UINT_PTR itemID);
	void clear();
	void update();
	void update(UINT_PTR id);
private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

	void OnPaint(HDC hdc, LPRECT rcDraw);
	void OnLButtonDown(int x, int y);
	void OnLButtonDblClick(int x, int y);
	void OnLButtonUp(int x, int y);
	void OnMouseMove(int x, int y);
	void OnVScroll(int pos, int type);
	void OnSize();
	void OnMouseWheel(int delta);

	void getItemRect(int idx, LPRECT rcItem);
	void updateScrollBar();
};

#define XUI_HANDLE_CUSTLST_DRAWITEM(elID, func)			if(IS_SAME_STR(elID, el->get_id()) && IS_SAME_STR(XUI_EVENT_CUSTOMLIST_DRAWITEM, evID)) \
														{ \
															return func((HDC) wParam, (CUSTOM_LIST_DRAWITEM*) lParam); \
														}

#define XUI_HANDLE_CUSTLST_ITEMCLICK(elID, func)		if(IS_SAME_STR(elID, el->get_id()) && IS_SAME_STR(XUI_EVENT_CUSTOMLIST_ITEMCLICK, evID)) \
														{ \
															return func((HWND) wParam, (CUSTOM_LIST_CLICK*) lParam); \
														}

#define XUI_HANDLE_CUSTLST_ITEMDBLCLICK(elID, func)		if(IS_SAME_STR(elID, el->get_id()) && IS_SAME_STR(XUI_EVENT_CUSTOMLIST_ITEMDBLCLICK, evID)) \
														{ \
															return func((HWND) wParam, (CUSTOM_LIST_CLICK*) lParam); \
														}
