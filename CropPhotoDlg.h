#pragma once
#include "PhotoCropper.h"

class CCropPhotoDlg : public CXUIDialog
{
	LPWSTR			m_fileName;
	CPhotoCropper*	m_ctlCrop;
	CXUISlider*		m_ctlZoom;
	
public:
	LPBYTE			m_img;
	UINT			m_imgSize;

	CCropPhotoDlg(CXUIEngine* engine, LPCWSTR fileName);
	virtual ~CCropPhotoDlg(void);

	XUI_BEGIN_BIND_MAP
		XUI_BIND_ITEM(L"zoom",		L"slider",		m_ctlZoom)
		XUI_BIND_ITEM(L"crop",		L"photocrop",	m_ctlCrop)
	XUI_END_BIND_MAP

	XUI_BEGIN_EVENT_MAP
		XUI_HANDLE_HSCROLL(L"zoom",	OnZoomChanged)
	XUI_END_EVENT_MAP

	virtual void OnInitDialog();
	virtual BOOL OnEndDialog(UINT code);

private:
	BOOL OnZoomChanged(CXUIElement* el, LPARAM lParam);
};
