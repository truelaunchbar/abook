#pragma once

class CABookEngine : public CXUIEngine
{
public:
	CABookEngine(HINSTANCE hInst);
	virtual ~CABookEngine(void);

	virtual void			DrawImage(HDC hdc, int x, int y, int width, int height, LPWSTR imgUrl);
	virtual void			DrawFrame(HDC hdc, int x, int y, int width, int height, LPWSTR imgUrl, int frames, int frame, int framesOrient);
	virtual CXUIElement*	createXUIElement(LPCWSTR name, CXUIElement* parent);
};
