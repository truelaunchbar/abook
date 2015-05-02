#pragma once
#include <dib.h>

namespace tlb
{
	class color
	{
	public:
		RGBQUAD	clr;

		color()
		{
			ZeroMemory(&clr, sizeof(clr));
		}

		color(BYTE red, BYTE green, BYTE blue, BYTE alpha = 0xFF)
		{
			clr.rgbRed		= red;
			clr.rgbGreen	= green;
			clr.rgbBlue		= blue;
			clr.rgbReserved	= alpha;
		}

		color(COLORREF val)
		{
			clr.rgbRed		= GetRValue(val);
			clr.rgbGreen	= GetGValue(val);
			clr.rgbBlue		= GetBValue(val);
			clr.rgbReserved	= 255;
		}

		color(const color& val)
		{
			clr = val.clr;
		}

		void operator=(const color& val)
		{
			clr = val.clr;
		}

		COLORREF ToCOLORREF()
		{
			return RGB(red(), green(), blue());
		}

		BYTE red() const			{ return clr.rgbRed;				}
		BYTE green() const			{ return clr.rgbGreen;				}
		BYTE blue() const			{ return clr.rgbBlue;				}
		BYTE alpha() const			{ return clr.rgbReserved;			}

		double red_f() const		{ return clr.rgbRed / 255.0;		}
		double green_f() const		{ return clr.rgbGreen / 255.0;		}
		double blue_f() const		{ return clr.rgbBlue / 255.0;		}
		double alpha_f() const		{ return clr.rgbReserved / 255.0;	}
	};


	class dc
	{
		cairo_surface_t*	m_surface;
		cairo_t*			m_cr;
		simpledib::dib		m_dib;
		BOOL				m_owndata;
	public:
		dc();
		~dc();

		bool	create(int cx, int cy);
		bool	create(HDC hdc, HBITMAP bmp, LPRGBQUAD bits, int width, int height, cairo_t* cr, cairo_surface_t* surface);
		void	begin_paint(HDC hdc, LPRECT rcDraw);
		void	end_paint(bool copy = true);
		void	draw(tlb::dc& hdc, LPRECT rcDraw);
		HBITMAP	detach_bitmap();
		bool	is_valid()						{ return m_cr ? true : false; }

		void	destroy();
		HDC		hdc()		{	return m_dib.hdc();		}
		int		width()		{	return m_dib.width();	}
		int		height()	{	return m_dib.height();	}
		LPRGBQUAD bits()	{	return m_dib.bits();	}
		void	clear_rect(LPRECT rcClear);
		void	save();
		void	restore();
		void	set_clip(LPCRECT rcClip, int fnMode);
		void	reset_clip();
		void	rounded_rect(int x, int y, int width, int height, int radius, int line_width);
		
		void	rectangle(LPCRECT rc, int line_width = 0);
		void	set_color(COLORREF clr)			{	set_color(tlb::color(clr));		}
		void	set_color(tlb::color clr)		{	cairo_set_source_rgba(m_cr, clr.red_f(), clr.green_f(), clr.blue_f(), clr.alpha_f());	}
		void	fill(bool preserve = false);
		void	draw_image(CTxDIB* bmp, int x, int y, int cx, int cy);
		void	tile_image(CTxDIB* bmp, int x, int y, int cx, int cy);
		void	tile_image(CTxDIB* bmp, LPCRECT rcDraw)	{ tile_image(bmp, rcDraw->left, rcDraw->top, rcDraw->right - rcDraw->left, rcDraw->bottom - rcDraw->top); }

		operator cairo_t*()			{	return m_cr;			}
		operator cairo_surface_t*()	{	return m_surface;		}
		operator simpledib::dib*()	{	return &m_dib;			}
		operator HDC()				{	return m_dib;			}
	private:
		void init_cairo();
	};

	class skined_image
	{
		CTxDIB	m_dibLeftTop;
		CTxDIB	m_dibTop;
		CTxDIB	m_dibRightTop;

		CTxDIB	m_dibLeftCenter;
		CTxDIB	m_dibCenter;
		CTxDIB	m_dibRightCenter;

		CTxDIB	m_dibLeftBottom;
		CTxDIB	m_dibBottom;
		CTxDIB	m_dibRightBottom;

		MARGINS	m_margins;
		BOOL	m_tileX;
		BOOL	m_tileY;
	public:
		skined_image();
		~skined_image();

		BOOL load(LPCWSTR fileName, MARGINS* mg, BOOL tileX, BOOL tileY);
		BOOL load(CTxDIB* dib, MARGINS* mg, BOOL tileX, BOOL tileY);

		void draw(tlb::dc& hdc, LPRECT rcDraw, LPRECT rcClip);
	};
}