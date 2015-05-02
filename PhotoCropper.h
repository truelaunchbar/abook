#pragma once
#include <GdiPlus.h>

#define XUI_PHOTOCROP_CLASS	L"XUI_PHOTOCROP_CLASS"
#define CROP_WINDOW_SIZE	400
#define CROP_MAX_ZOOM		3

class CPhotoCropper : public CXUIElement
{
	HCURSOR			m_curTake;
	HCURSOR			m_curGrab;

	LPWSTR			m_fileName;
	Gdiplus::Image*	m_image;
	int				m_imgWidth;
	int				m_imgHeight;
	int				m_imgLeft;
	int				m_imgTop;
	double			m_minZoomLevel;
	double			m_zoomLevel;

	int				m_downX;
	int				m_downY;
	int				m_downImgX;
	int				m_downImgY;
	BOOL			m_bInCapture;
public:
	CPhotoCropper(CXUIElement* parent, CXUIEngine* engine);
	virtual ~CPhotoCropper(void);

	IMPLEMENT_INTERFACE(L"photocrop")

	virtual void Init();

	void setImage(LPCWSTR fileName);
	void setZoom(int zoom);
	BOOL save(LPCWSTR fileName);

private:
	int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
	void OnPaint(HDC hdc, LPRECT rcDraw);
	void OnLButtonDown(int x, int y);
	void OnLButtonUp(int x, int y);
	void OnMouseMove(int x, int y);
};
