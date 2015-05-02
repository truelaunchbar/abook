#include "globals.h"
#include "ABookEngine.h"
#include "PhotoCropper.h"
#include "xuicustomlist.h"

CABookEngine::CABookEngine(HINSTANCE hInst) : CXUIEngine(hInst)
{
}

CABookEngine::~CABookEngine(void)
{
}

void CABookEngine::DrawImage( HDC hdc, int x, int y, int width, int height, LPWSTR imgUrl )
{
	CTxDIB img;
	img.load(FindResource(m_hInst, imgUrl, TEXT("XUILIB")), m_hInst);
	if(img.isValid())
	{
		if(!lstrcmpi(L"delhkicon.png", imgUrl))
		{
			int w = width;
			int h = height;

			width  -= 8;
			height -= 8;

			if(width < 24)
			{
				if(width < 20)
				{
					img.load(FindResource(m_hInst, L"delhkicon16.png", TEXT("XUIIMG")), m_hInst);
					width = height = 16;
				} else
				{
					img.load(FindResource(m_hInst, L"delhkicon20.png", TEXT("XUIIMG")), m_hInst);
					width = height = 20;
				}
			} else
			{
				width = height = 24;
			}
			if(width != img.getWidth() || height != img.getHeight())
			{
				img.resample(width, height);
			}
			x = x + w / 2 - width  / 2;
			y = y + h / 2 - height / 2;
		}
		img.draw(hdc, x, y, width, height);
	}
}

void CABookEngine::DrawFrame( HDC hdc, int x, int y, int width, int height, LPWSTR imgUrl, int frames, int frame, int framesOrient )
{
	CTxDIB img;
	img.load(FindResource(m_hInst, imgUrl, TEXT("XUILIB")), m_hInst);
	if(img.isValid())
	{
		CTxDIB imgFrame;
		int frameWidth	= img.getWidth();
		int frameHeight	= img.getHeight();
		if(frames)
		{
			if(framesOrient)
			{
				frameWidth = frameWidth / frames;
				img.crop(frame * frameWidth, 0, (frame + 1) * frameWidth, img.getHeight(), &imgFrame);
			} else
			{
				frameHeight = frameHeight / frames;
				img.crop(0, frame * frameHeight, img.getWidth(), (frame + 1) * frameHeight, &imgFrame);
			}
		} else
		{
			imgFrame = img;
		}
		MARGINS mg = {0, 0, 0, 0};
		if(!lstrcmpi(imgUrl, L"delhkbg.png"))
		{
			mg.cxLeftWidth		= 3;
			mg.cxRightWidth		= 3;
			mg.cyTopHeight		= 3;
			mg.cyBottomHeight	= 3;
		}
		RECT rcDraw;
		rcDraw.left		= x;
		rcDraw.top		= y;
		rcDraw.right	= x + width;
		rcDraw.bottom	= y + height;

		CTxSkinDIB skdib;
		skdib.load(&imgFrame, &mg, FALSE, FALSE);
		skdib.draw(hdc, &rcDraw, &rcDraw);
	}
}

CXUIElement* CABookEngine::createXUIElement( LPCWSTR name, CXUIElement* parent )
{
	CXUIElement* ret = NULL;
	if(!StrCmpI(name, TEXT("photocrop")))
	{
		ret = new CPhotoCropper(parent, this);
	} else if(!StrCmpI(name, TEXT("customlist")))
	{
		ret = new CXUICustomList(parent, this);
	} else
	{
		ret = CXUIEngine::createXUIElement(name, parent);
	}
	return ret;
}