#include "globals.h"
#include "xuicustomlist.h"

CXUICustomList::CXUICustomList( CXUIElement* parent, CXUIEngine* engine ) : CXUIElement(parent, engine)
{
	m_scrollPos = 0;

	WNDCLASS wc;
	if(!GetClassInfo(engine->get_hInstance(), XUI_CUSTLIST_CLASS, &wc))
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wc.lpfnWndProc		= (WNDPROC) CXUICustomList::WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= engine->get_hInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= XUI_CUSTLIST_CLASS;

		RegisterClass(&wc);
	}
}

CXUICustomList::~CXUICustomList()
{

}

void CXUICustomList::Init()
{
	DWORD wStyle = WS_CHILD | WS_VSCROLL | WS_BORDER;

	if(get_disabled())	wStyle |= WS_DISABLED;
	if(!get_hidden())	wStyle |= WS_VISIBLE;

	m_hWnd = CreateWindowEx(WS_EX_STATICEDGE, XUI_CUSTLIST_CLASS, TEXT(""), wStyle, m_left, m_top, m_width, m_height, m_parent->get_parentWnd(), (HMENU) m_id, m_engine->get_hInstance(), (LPVOID) this);
}

LRESULT CALLBACK CXUICustomList::WndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
	CXUICustomList* pThis = NULL;
	if(IsWindow(hWnd))
	{
		pThis = (CXUICustomList*)GetProp(hWnd, TEXT("CRecordListCtlThis"));
	}

	MSG msg;
	memset(&msg, 0, sizeof(msg));
	msg.hwnd = hWnd;
	msg.lParam = lParam;
	msg.message = uMessage;
	msg.wParam = wParam;
	LRESULT res = NULL;

	if(pThis || uMessage == WM_CREATE)
	{
		switch (uMessage)
		{
		case WM_MOUSEWHEEL:
			pThis->OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
			return 0;
		case WM_SIZE:
			pThis->OnSize();
			break;
		case WM_VSCROLL:
			pThis->OnVScroll(HIWORD(wParam), LOWORD(wParam));
			return 0;
		case WM_LBUTTONDOWN:
			pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONDBLCLK:
			pThis->OnLButtonDblClick(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_LBUTTONUP:
			pThis->OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_MOUSEMOVE:
			pThis->OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		case WM_ERASEBKGND:
			return TRUE;
		case WM_DESTROY:
			RemoveProp(hWnd, TEXT("CRecordListCtlThis"));
			return 0;
		case WM_CREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
				pThis = (CXUICustomList*) lpcs->lpCreateParams;
				SetProp(hWnd, TEXT("CRecordListCtlThis"), (HANDLE) pThis);
				pThis->m_hWnd = hWnd;
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(pThis->m_hWnd, &ps);

				RECT rcClient;
				GetClientRect(pThis->m_hWnd, &rcClient);

				RECT rcDraw = {0, 0, 0, 0};
				rcDraw.right	= rcClient.right - rcClient.left;
				rcDraw.bottom	= rcClient.bottom - rcClient.top;
				HDC memDC	= CreateCompatibleDC(hdc);
				HBITMAP bmp = CreateCompatibleBitmap(hdc, rcDraw.right, rcDraw.bottom);
				HBITMAP oldBmp = (HBITMAP) SelectObject(memDC, bmp);
				FillRect(memDC, &rcDraw, (HBRUSH) (COLOR_WINDOW + 1));
				pThis->OnPaint(memDC, &rcDraw);
				BitBlt(hdc, rcClient.left, rcClient.top, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, memDC, 0, 0, SRCCOPY);
				SelectObject(memDC, oldBmp);
				DeleteObject(bmp);
				DeleteDC(memDC);

				EndPaint(pThis->m_hWnd, &ps);
			}
			return TRUE;
		}
	}

	return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void CXUICustomList::OnPaint( HDC hdc, LPRECT rcDraw )
{
	for(size_t i = 0; i < m_items.size(); i++)
	{
		CUSTOM_LIST_DRAWITEM di;
		di.itemID	= m_items[i].itemID;

		getItemRect((int) i, &di.rcItem);
		RECT rcTmp;
		if(IntersectRect(&rcTmp, rcDraw, &di.rcItem))
		{
			raiseEvent(XUI_EVENT_CUSTOMLIST_DRAWITEM, (WPARAM) hdc, (LPARAM) &di);
		}
	}
}

void CXUICustomList::OnLButtonDown( int x, int y )
{
	for(size_t i=0; i < m_items.size(); i++)
	{
		RECT rcItem;
		getItemRect((int) i, &rcItem);
		if(y >= rcItem.top && y < rcItem.bottom)
		{
			CUSTOM_LIST_CLICK clc;
			clc.itemID	= m_items[i].itemID;
			clc.rcItem	= rcItem;
			clc.x		= x;
			clc.y		= y;
			raiseEvent(XUI_EVENT_CUSTOMLIST_ITEMCLICK, (WPARAM) m_hWnd, (LPARAM) &clc);
			break;
		}
	}
}

void CXUICustomList::OnLButtonUp( int x, int y )
{

}

void CXUICustomList::OnMouseMove( int x, int y )
{

}

void CXUICustomList::getItemRect( int idx, LPRECT rcItem )
{
	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);
	rcItem->left	= rcClient.left;
	rcItem->right	= rcClient.right;
	rcItem->top		= rcClient.top + m_items[idx].top - m_scrollPos;
	rcItem->bottom	= rcItem->top + m_items[idx].height;
}

void CXUICustomList::addItem( UINT_PTR itemID, int height )
{
	ITEM item;
	item.height = height;
	item.itemID		= itemID;
	m_items.push_back(item);
}

void CXUICustomList::delItem( UINT_PTR itemID )
{
	for(size_t i=0; i < m_items.size(); i++)
	{
		if(m_items[i].itemID == itemID)
		{
			m_items.erase(m_items.begin() + i);
			break;
		}
	}
}

void CXUICustomList::clear()
{
	m_items.clear();
}

void CXUICustomList::update()
{
	m_scrollPos = 0;
	int top = 0;
	for(size_t i=0; i < m_items.size(); i++)
	{
		m_items[i].top = top;
		top += m_items[i].height;
	}
	updateScrollBar();
}

void CXUICustomList::update( UINT_PTR id )
{
	for(size_t i = 0; i < m_items.size(); i++)
	{
		if(m_items[i].itemID == id)
		{
			RECT rcItem;
			getItemRect((int) i, &rcItem);
			InvalidateRect(m_hWnd, &rcItem, FALSE);
			break;
		}
	}
}

void CXUICustomList::updateScrollBar()
{
	if(m_items.empty())
	{
		return;
	}

	RECT rcClient;
	GetClientRect(m_hWnd, &rcClient);

	int maxHeight = m_items.rbegin()->top + m_items.rbegin()->height;
	int wndHeight = rcClient.bottom - rcClient.top;

	if(maxHeight > wndHeight)
	{
		ShowScrollBar(m_hWnd, SB_VERT, TRUE);

		if(m_scrollPos > maxHeight - wndHeight)
		{
			m_scrollPos = maxHeight - wndHeight;
		}
		if(m_scrollPos < 0)
		{
			m_scrollPos = 0;
		}

		SetScrollRange(m_hWnd, SB_VERT, 0, maxHeight - wndHeight, TRUE);
		SetScrollPos(m_hWnd, SB_VERT, m_scrollPos, TRUE);
	} else
	{
		ShowScrollBar(m_hWnd, SB_VERT, FALSE);
	}
}

void CXUICustomList::OnVScroll( int pos, int type )
{
	switch(type)
	{
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_scrollPos = pos;
		break;
	}
	updateScrollBar();
	InvalidateRect(m_hWnd, NULL, FALSE);
}

void CXUICustomList::OnSize()
{
	updateScrollBar();
}

void CXUICustomList::OnMouseWheel( int delta )
{
	static int savedDelta = 0;
	int zDelta = delta;
	if(abs(zDelta) < WHEEL_DELTA)
	{
		if(savedDelta > 0 && zDelta < 0 || savedDelta < 0 && zDelta > 0)
		{
			savedDelta = 0;
		}
		savedDelta += zDelta;
		if(abs(savedDelta) >= WHEEL_DELTA)
		{
			zDelta = savedDelta;
			savedDelta = 0;
		}
	}
	INT lines = 1;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
	m_scrollPos += -zDelta * 10 * lines / WHEEL_DELTA, TRUE;

	updateScrollBar();
	InvalidateRect(m_hWnd, NULL, FALSE);
}

void CXUICustomList::OnLButtonDblClick( int x, int y )
{
	for(size_t i=0; i < m_items.size(); i++)
	{
		RECT rcItem;
		getItemRect((int) i, &rcItem);
		if(y >= rcItem.top && y < rcItem.bottom)
		{
			CUSTOM_LIST_CLICK clc;
			clc.itemID	= m_items[i].itemID;
			clc.rcItem	= rcItem;
			clc.x		= x;
			clc.y		= y;
			raiseEvent(XUI_EVENT_CUSTOMLIST_ITEMDBLCLICK, (WPARAM) m_hWnd, (LPARAM) &clc);
			break;
		}
	}
}
