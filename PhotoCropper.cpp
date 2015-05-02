#include "globals.h"
#include "PhotoCropper.h"
#include <WindowsX.h>
#include "resource.h"

CPhotoCropper::CPhotoCropper(CXUIElement* parent, CXUIEngine* engine) : CXUIElement(parent, engine)
{
	m_fileName		= NULL;
	m_image			= NULL;
	m_imgWidth		= 0;
	m_imgHeight		= 0;
	m_imgLeft		= 0;
	m_imgTop		= 0;
	m_downX			= 0;
	m_downY			= 0;
	m_bInCapture	= FALSE;

	m_curTake		= LoadCursor(engine->get_hInstance(), MAKEINTRESOURCE(IDC_TAKE));
	m_curGrab		= LoadCursor(engine->get_hInstance(), MAKEINTRESOURCE(IDC_GRAB));

	WNDCLASS wc;
	if(!GetClassInfo(engine->get_hInstance(), XUI_PHOTOCROP_CLASS, &wc))
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC) CPhotoCropper::WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= engine->get_hInstance();
		wc.hIcon			= NULL;
		wc.hCursor			= m_curTake;
		wc.hbrBackground	= (HBRUSH)(COLOR_BTNFACE + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= XUI_PHOTOCROP_CLASS;

		RegisterClass(&wc);
	}
}

CPhotoCropper::~CPhotoCropper(void)
{
	FREE_CLEAR_STR(m_fileName);
	DestroyCursor(m_curTake);
	DestroyCursor(m_curGrab);
	if(m_image)
	{
		delete m_image;
	}
}

void CPhotoCropper::Init()
{
	DWORD wStyle = WS_CHILD;

	if(get_disabled())	wStyle |= WS_DISABLED;
	if(!get_hidden())	wStyle |= WS_VISIBLE;

	m_hWnd = CreateWindowEx(0, XUI_PHOTOCROP_CLASS, TEXT(""), wStyle, m_left, m_top, m_width, m_height, m_parent->get_parentWnd(), (HMENU) m_id, m_engine->get_hInstance(), (LPVOID) this);

	m_minWidth  = CROP_WINDOW_SIZE;
	m_minHeight = CROP_WINDOW_SIZE;
}

LRESULT CALLBACK CPhotoCropper::WndProc( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam )
{
	CPhotoCropper* pThis = NULL;
	if(IsWindow(hWnd))
	{
		pThis = (CPhotoCropper*)GetProp(hWnd, TEXT("CPhotoCropperThis"));
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
		case WM_SETCURSOR:
			if(pThis->m_bInCapture)
			{
				SetCursor(pThis->m_curGrab);
			} else
			{
				SetCursor(pThis->m_curTake);
			}
			return TRUE;
		case WM_LBUTTONDOWN:
			pThis->OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
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
			RemoveProp(hWnd, TEXT("CPhotoCropperThis"));
			return 0;
		case WM_CREATE:
			{
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
				pThis = (CPhotoCropper*) lpcs->lpCreateParams;
				SetProp(hWnd, TEXT("CPhotoCropperThis"), (HANDLE) pThis);
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
				SendMessage(GetParent(pThis->m_hWnd), WM_ERASEBKGND, (WPARAM) memDC, NULL);
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

void CPhotoCropper::OnPaint( HDC hdc, LPRECT rcDraw )
{
	using namespace Gdiplus;
	Graphics gdi(hdc);

	RectF	rcSrc;
	RectF	rcDst;

	rcSrc.X			= (double) m_imgLeft / m_zoomLevel;
	rcSrc.Y			= (double) m_imgTop / m_zoomLevel;
	rcSrc.Width		= CROP_WINDOW_SIZE / m_zoomLevel;
	rcSrc.Height	= CROP_WINDOW_SIZE / m_zoomLevel;

	rcDst.X			= rcDraw->left;
	rcDst.Y			= rcDraw->top;
	rcDst.Width		= CROP_WINDOW_SIZE;
	rcDst.Height	= CROP_WINDOW_SIZE;

	gdi.SetInterpolationMode(InterpolationModeLowQuality);
	gdi.DrawImage(m_image, rcDst, rcSrc.X, rcSrc.Y, rcSrc.Width, rcSrc.Height, UnitPixel);
}

void CPhotoCropper::setImage( LPCWSTR fileName )
{
	using namespace Gdiplus;

	MAKE_STR(m_fileName, fileName);
	if(m_image)
	{
		delete m_image;
		m_image = NULL;
	}
	m_image = Image::FromFile(fileName);
	if(m_image)
	{
		m_imgWidth	= m_image->GetWidth();
		m_imgHeight = m_image->GetHeight();

		int maxSize		= max(m_imgWidth, m_imgHeight);
		m_minZoomLevel	= (double) CROP_WINDOW_SIZE / (double) maxSize;
		m_zoomLevel		= m_minZoomLevel;
	}
}

void CPhotoCropper::setZoom( int zoom )
{
	double realWidth = CROP_MAX_ZOOM - m_minZoomLevel;
	double dlgWidth = 1000;

	double oldX = (double) (m_imgLeft + CROP_WINDOW_SIZE / 2) / m_zoomLevel;
	double oldY = (double) (m_imgTop + CROP_WINDOW_SIZE / 2) / m_zoomLevel;

	m_zoomLevel = m_minZoomLevel + (double) zoom * realWidth / dlgWidth;

	double newX = (double) (m_imgLeft + CROP_WINDOW_SIZE / 2) / m_zoomLevel;
	double newY = (double) (m_imgTop + CROP_WINDOW_SIZE / 2) / m_zoomLevel;

	m_imgLeft -= (newX - oldX) * m_zoomLevel;
	m_imgTop  -= (newY - oldY) * m_zoomLevel;

	InvalidateRect(m_hWnd, NULL, TRUE);
}

void CPhotoCropper::OnLButtonDown( int x, int y )
{
	m_downX			= x;
	m_downY			= y;
	m_downImgX		= m_imgLeft;
	m_downImgY		= m_imgTop;
	m_bInCapture	= TRUE;
	SetCapture(m_hWnd);
	SetCursor(m_curGrab);
}

void CPhotoCropper::OnLButtonUp( int x, int y )
{
	m_bInCapture = FALSE;
	ReleaseCapture();
	SetCursor(m_curTake);
}

void CPhotoCropper::OnMouseMove( int x, int y )
{
	if(m_bInCapture)
	{
		m_imgLeft = m_downImgX + (m_downX - x);
		m_imgTop  = m_downImgY + (m_downY - y);

		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}

int CPhotoCropper::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	using namespace Gdiplus;

	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}    
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

BOOL CPhotoCropper::save( LPCWSTR fileName )
{
	using namespace Gdiplus;
	if(!m_image) return FALSE;

	RectF	rcSrc;
	RectF	rcDst;

	rcSrc.X			= (double) m_imgLeft / m_zoomLevel;
	rcSrc.Y			= (double) m_imgTop / m_zoomLevel;
	rcSrc.Width		= CROP_WINDOW_SIZE / m_zoomLevel;
	rcSrc.Height	= CROP_WINDOW_SIZE / m_zoomLevel;

	if(rcSrc.X < 0)	rcSrc.X = 0;
	if(rcSrc.Y < 0)	rcSrc.Y = 0;

	if(rcSrc.X + rcSrc.Width  > m_imgWidth)		rcSrc.Width		= m_imgWidth  - rcSrc.X;
	if(rcSrc.Y + rcSrc.Height > m_imgHeight)	rcSrc.Height	= m_imgHeight - rcSrc.Y;

	rcDst.X			= 0;
	rcDst.Y			= 0;
	rcDst.Width		= min(256, rcSrc.Width * m_zoomLevel * (256.0 / (double) CROP_WINDOW_SIZE));
	rcDst.Height	= min(256, rcSrc.Height * m_zoomLevel * (256.0 / (double) CROP_WINDOW_SIZE));

	Bitmap bmp(rcDst.Width, rcDst.Height, PixelFormat32bppARGB);
	Graphics memdc(&bmp);
	memdc.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	memdc.DrawImage(m_image, rcDst, rcSrc.X, rcSrc.Y, rcSrc.Width, rcSrc.Height, UnitPixel);

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);
	if(bmp.Save(fileName, &pngClsid, NULL) == Ok)
	{
		return TRUE;
	}
	return FALSE;
}
