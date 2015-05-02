#include "globals.h"
#include "dc.h"
#define _USE_MATH_DEFINES
#include <math.h>

tlb::dc::dc()
{
	m_surface	= NULL;
	m_cr		= NULL;
	m_owndata	= FALSE;
}

tlb::dc::~dc()
{
	destroy();
}

void tlb::dc::begin_paint( HDC hdc, LPRECT rcDraw )
{
	destroy();
	m_dib.beginPaint(hdc, rcDraw);
	init_cairo();
}

void tlb::dc::end_paint(bool copy)
{
	m_dib.endPaint(copy);
}

void tlb::dc::destroy()
{
	m_dib.destroy();
	if(m_owndata)
	{
		if(m_cr)		cairo_destroy(m_cr);
		if(m_surface)	cairo_surface_destroy(m_surface);
	}
	m_cr		= NULL;
	m_surface	= NULL;
}

bool tlb::dc::create( int cx, int cy )
{
	destroy();
	bool ret = m_dib.create(cx, cy, true);
	init_cairo();
	return ret;
}

bool tlb::dc::create( HDC hdc, HBITMAP bmp, LPRGBQUAD bits, int width, int height, cairo_t* cr, cairo_surface_t* surface )
{
	destroy();
	m_dib.create(hdc, bmp, bits, width, height);
	m_cr		= cr;
	m_surface	= surface;
	m_owndata	= FALSE;
	return true;
}

void tlb::dc::init_cairo()
{
	m_surface	= cairo_image_surface_create_for_data((unsigned char*) m_dib.bits(), CAIRO_FORMAT_ARGB32, m_dib.width(), m_dib.height(), m_dib.width() * 4);
	m_cr		= cairo_create(m_surface);

	POINT pt;
	GetWindowOrgEx(m_dib.hdc(), &pt);
	if(pt.x != 0 || pt.y != 0)
	{
		cairo_translate(m_cr, -pt.x, -pt.y);
	}
	m_owndata = TRUE;
}

void tlb::dc::clear_rect( LPRECT rcClear )
{
	if(!is_valid())	return;

	save();

	cairo_rectangle(m_cr, rcClear->left, rcClear->top, rcClear->right - rcClear->left, rcClear->bottom - rcClear->top);
	cairo_clip(m_cr);
	cairo_set_operator(m_cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(m_cr, 0, 0, 0, 0);
	cairo_paint(m_cr);

	restore();
}

void tlb::dc::save()
{
	if(!is_valid())	return;

	cairo_save(m_cr);
}

void tlb::dc::restore()
{
	if(!is_valid())	return;

	cairo_restore(m_cr);
}

void tlb::dc::rectangle( LPCRECT rc, int line_width )
{
	if(!is_valid())	return;

	double x	= (double) rc->left + (double) line_width / 2.0;
	double y	= (double) rc->top + (double) line_width / 2.0;
	double w	= rc->right - rc->left - line_width;
	double h	= rc->bottom - rc->top - line_width;
	cairo_rectangle(m_cr, x, y, w, h);
}

void tlb::dc::fill(bool preserve)
{
	if(!is_valid())	return;

	if(preserve)
	{
		cairo_fill_preserve(m_cr);
	} else
	{
		cairo_fill(m_cr);
	}
}

void tlb::dc::draw_image( CTxDIB* bmp, int x, int y, int cx, int cy )
{
	if(!is_valid())	return;

	cairo_save(m_cr);

	cairo_matrix_t flib_m;
	cairo_matrix_init(&flib_m, 1, 0, 0, -1, 0, 0);

	cairo_surface_t* img = NULL;

	CTxDIB rbmp;

	if(cx != bmp->getWidth() || cy != bmp->getHeight())
	{
		bmp->resample(cx, cy, &rbmp);
		img = cairo_image_surface_create_for_data((unsigned char*) rbmp.getBits(), CAIRO_FORMAT_ARGB32, rbmp.getWidth(), rbmp.getHeight(), rbmp.getWidth() * 4);
		cairo_matrix_translate(&flib_m, 0, -rbmp.getHeight());
		cairo_matrix_translate(&flib_m, x, -y);
	} else
	{
		img = cairo_image_surface_create_for_data((unsigned char*) bmp->getBits(), CAIRO_FORMAT_ARGB32, bmp->getWidth(), bmp->getHeight(), bmp->getWidth() * 4);
		cairo_matrix_translate(&flib_m, 0, -bmp->getHeight());
		cairo_matrix_translate(&flib_m, x, -y);
	}

	cairo_transform(m_cr, &flib_m);
	cairo_set_source_surface(m_cr, img, 0, 0);
	if(bmp->getMaxAlpha() == 255)
	{
		cairo_paint(m_cr);
	} else
	{
		cairo_paint_with_alpha(m_cr, (double) bmp->getMaxAlpha() / 255.0);
	}

	cairo_restore(m_cr);

	cairo_surface_destroy(img);
}

void tlb::dc::set_clip( LPCRECT rcClip, int fnMode )
{
	if(!is_valid())	return;

	POINT ptView = {0, 0};
	GetWindowOrgEx(m_dib, &ptView);
	HRGN rgn = CreateRectRgn(
		rcClip->left - ptView.x,
		rcClip->top - ptView.y,
		rcClip->right - ptView.x,
		rcClip->bottom - ptView.y);
	ExtSelectClipRgn(m_dib, rgn, fnMode);
	DeleteObject(rgn);

	rectangle(rcClip);
	cairo_clip(m_cr);
}

void tlb::dc::reset_clip()
{
	if(!is_valid())	return;

	SelectClipRgn(m_dib, NULL);
	cairo_reset_clip(m_cr);
}

HBITMAP tlb::dc::detach_bitmap()
{
	HBITMAP bmp = m_dib.detach_bitmap();
	destroy();
	return bmp;
}

void tlb::dc::rounded_rect( int x, int y, int width, int height, int radius, int line_width )
{
	if(!is_valid())	return;

	double xx	= x;
	double yy	= y;
	double w	= width;
	double h	= height;
	double r	= radius;

	if(line_width != 0)
	{
		xx += line_width / 2.0;
		yy += line_width / 2.0;
		w  -= line_width;
		h  -= line_width;
	}

	cairo_new_path(m_cr);

	cairo_move_to	(m_cr, xx + r, yy);
	cairo_arc		(m_cr, xx + w - r,	yy + r,		r, M_PI + M_PI / 2, M_PI * 2        );
	cairo_arc		(m_cr, xx + w - r,	yy + h - r,	r, 0,               M_PI / 2        );
	cairo_arc		(m_cr, xx + r,		yy + h - r,	r, M_PI/2,          M_PI            );
	cairo_arc		(m_cr, xx + r,		yy + r,		r, M_PI,            270 * M_PI / 180);
	cairo_close_path (m_cr);
}

void tlb::dc::tile_image( CTxDIB* bmp, int x, int y, int cx, int cy )
{
	if(!is_valid())	return;

	save();

	cairo_surface_t* img = cairo_image_surface_create_for_data((unsigned char*) bmp->getBits(), CAIRO_FORMAT_ARGB32, bmp->getWidth(), bmp->getHeight(), bmp->getWidth() * 4);
	cairo_pattern_t *pattern = cairo_pattern_create_for_surface(img);
	cairo_matrix_t flib_m;
	cairo_matrix_init(&flib_m, 1, 0, 0, -1, 0, 0);
	cairo_matrix_translate(&flib_m, -x, -y);
	cairo_pattern_set_extend (pattern, CAIRO_EXTEND_REPEAT);
	cairo_pattern_set_matrix (pattern, &flib_m);

	cairo_set_source(m_cr, pattern);
	cairo_rectangle(m_cr, x, y, cx, cy);
	cairo_clip(m_cr);

	if(bmp->getMaxAlpha() == 255)
	{
		cairo_paint(m_cr);
	} else
	{
		cairo_paint_with_alpha(m_cr, (double) bmp->getMaxAlpha() / 255.0);
	}

	restore();

	cairo_pattern_destroy(pattern);
	cairo_surface_destroy(img);
}

void tlb::dc::draw( tlb::dc& hdc, LPRECT rcDraw )
{
	if(!is_valid())	return;

	hdc.save();

	hdc.rectangle(rcDraw);
	cairo_clip(hdc);

	cairo_set_source_surface(hdc, m_surface, 0, 0);
	cairo_paint(hdc);

	hdc.restore();
}

tlb::skined_image::skined_image()
{
	ZeroMemory(&m_margins, sizeof(m_margins));
	m_tileX	= FALSE;
	m_tileY	= FALSE;
}

tlb::skined_image::~skined_image()
{

}

BOOL tlb::skined_image::load( LPCWSTR fileName, MARGINS* mg, BOOL tileX, BOOL tileY )
{
	CTxDIB dib;
	if(dib.load(fileName))
	{
		return load(&dib, mg, tileX, tileY);
	}
	return FALSE;
}

BOOL tlb::skined_image::load( CTxDIB* dib, MARGINS* mg, BOOL tileX, BOOL tileY )
{
	if(!dib)	return FALSE;

	m_margins	= *mg;
	m_tileX		= tileX;
	m_tileY		= tileY;

	if(m_margins.cxLeftWidth)
	{
		if(m_margins.cyTopHeight)
		{
			dib->crop(	0, 
						0, 
						m_margins.cxLeftWidth, 
						m_margins.cyTopHeight, 
						&m_dibLeftTop);
		}
		if(m_margins.cyBottomHeight)
		{
			dib->crop(	0, 
						dib->getHeight() - m_margins.cyBottomHeight, 
						m_margins.cxLeftWidth, 
						dib->getHeight(), 
						&m_dibLeftBottom);
		}
		dib->crop(	0, 
					m_margins.cyTopHeight, 
					m_margins.cxLeftWidth, 
					dib->getHeight() - m_margins.cyBottomHeight, 
					&m_dibLeftCenter);
	}

	if(m_margins.cxRightWidth)
	{
		if(m_margins.cyTopHeight)
		{
			dib->crop(	dib->getWidth() - m_margins.cxRightWidth, 
						0, 
						dib->getWidth(), 
						m_margins.cyTopHeight, 
						&m_dibRightTop);
		}
		if(m_margins.cyBottomHeight)
		{
			dib->crop(	dib->getWidth() - m_margins.cxRightWidth, 
						dib->getHeight() - m_margins.cyBottomHeight, 
						dib->getWidth(), 
						dib->getHeight(), 
						&m_dibRightBottom);
		}
		dib->crop(	dib->getWidth() - m_margins.cxRightWidth, 
					m_margins.cyTopHeight, 
					dib->getWidth(), 
					dib->getHeight() - m_margins.cyBottomHeight, 
					&m_dibRightCenter);
	}

	if(m_margins.cyTopHeight)
	{
		dib->crop(	m_margins.cxLeftWidth, 
					0, 
					dib->getWidth() - m_margins.cxRightWidth, 
					m_margins.cyTopHeight, 
					&m_dibTop);
	}

	if(m_margins.cyBottomHeight)
	{
		dib->crop(	m_margins.cxLeftWidth, 
					dib->getHeight() - m_margins.cyBottomHeight, 
					dib->getWidth() - m_margins.cxRightWidth, 
					dib->getHeight(), 
					&m_dibBottom);
	}

	dib->crop(	m_margins.cxLeftWidth,
				m_margins.cyTopHeight,
				dib->getWidth() - m_margins.cxRightWidth,
				dib->getHeight() - m_margins.cyBottomHeight,
				&m_dibCenter);

	return TRUE;
}

void tlb::skined_image::draw( tlb::dc& hdc, LPRECT rcDraw, LPRECT rcClip )
{
	if(!hdc.is_valid())	return;

	RECT rcPart;
	RECT rcTmp;
	if(m_margins.cxLeftWidth)
	{
		if(m_margins.cyTopHeight)
		{
			rcPart.left		= rcDraw->left;
			rcPart.right	= rcPart.left + m_margins.cxLeftWidth;
			rcPart.top		= rcDraw->top;
			rcPart.bottom	= rcPart.top + m_margins.cyTopHeight;
			if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
			{
				hdc.draw_image(&m_dibLeftTop, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			}
		}

		if(m_margins.cyBottomHeight)
		{
			rcPart.left		= rcDraw->left;
			rcPart.right	= rcPart.left + m_margins.cxLeftWidth;
			rcPart.top		= rcDraw->bottom - m_margins.cyBottomHeight;
			rcPart.bottom	= rcPart.top + m_margins.cyBottomHeight;
			if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
			{
				hdc.draw_image(&m_dibLeftBottom, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			}
		}

		rcPart.left		= rcDraw->left;
		rcPart.right	= rcPart.left + m_margins.cxLeftWidth;
		rcPart.top		= rcDraw->top + m_margins.cyTopHeight;
		rcPart.bottom	= rcDraw->bottom - m_margins.cyBottomHeight;
		if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
		{
			if(!m_tileY)
			{
				hdc.draw_image(&m_dibLeftCenter, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			} else
			{
				hdc.tile_image(&m_dibLeftCenter, &rcPart);
			}
		}
	}

	if(m_margins.cxRightWidth)
	{
		if(m_margins.cyTopHeight)
		{
			rcPart.left		= rcDraw->right - m_margins.cxRightWidth;
			rcPart.right	= rcPart.left + m_margins.cxRightWidth;
			rcPart.top		= rcDraw->top;
			rcPart.bottom	= rcPart.top + m_margins.cyTopHeight;
			if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
			{
				hdc.draw_image(&m_dibRightTop, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			}
		}

		if(m_margins.cyBottomHeight)
		{
			rcPart.left		= rcDraw->right - m_margins.cxRightWidth;
			rcPart.right	= rcPart.left + m_margins.cxRightWidth;
			rcPart.top		= rcDraw->bottom - m_margins.cyBottomHeight;
			rcPart.bottom	= rcPart.top + m_margins.cyBottomHeight;
			if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
			{
				hdc.draw_image(&m_dibRightBottom, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			}
		}

		rcPart.left		= rcDraw->right - m_margins.cxRightWidth;
		rcPart.right	= rcPart.left + m_margins.cxRightWidth;
		rcPart.top		= rcDraw->top + m_margins.cyTopHeight;
		rcPart.bottom	= rcDraw->bottom - m_margins.cyBottomHeight;
		if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
		{
			if(!m_tileY)
			{
				hdc.draw_image(&m_dibRightCenter, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			} else
			{
				hdc.tile_image(&m_dibRightCenter, &rcPart);
			}
		}

	}

	if(m_margins.cyTopHeight)
	{
		rcPart.left		= rcDraw->left + m_margins.cxLeftWidth;
		rcPart.right	= rcDraw->right - m_margins.cxRightWidth;
		rcPart.top		= rcDraw->top;
		rcPart.bottom	= rcDraw->top + m_margins.cyTopHeight;
		if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
		{
			if(!m_tileX)
			{
				hdc.draw_image(&m_dibTop, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			} else
			{
				hdc.tile_image(&m_dibTop, &rcPart);
			}
		}
	}

	if(m_margins.cyBottomHeight)
	{
		rcPart.left		= rcDraw->left + m_margins.cxLeftWidth;
		rcPart.right	= rcDraw->right - m_margins.cxRightWidth;
		rcPart.top		= rcDraw->bottom - m_margins.cyBottomHeight;
		rcPart.bottom	= rcPart.top + m_margins.cyBottomHeight;
		if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
		{
			if(!m_tileX)
			{
				hdc.draw_image(&m_dibBottom, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
			} else
			{
				hdc.tile_image(&m_dibBottom, &rcPart);
			}
		}
	}

	rcPart.left		= rcDraw->left + m_margins.cxLeftWidth;
	rcPart.right	= rcDraw->right - m_margins.cxRightWidth;
	rcPart.top		= rcDraw->top + m_margins.cyTopHeight;
	rcPart.bottom	= rcDraw->bottom - m_margins.cyBottomHeight;
	if(!rcClip || rcClip && IntersectRect(&rcTmp, rcClip, &rcPart))
	{
		if(!m_tileY && !m_tileX)
		{
			hdc.draw_image(&m_dibCenter, rcPart.left, rcPart.top, rcPart.right - rcPart.left, rcPart.bottom - rcPart.top);
		} else if(m_tileX && m_tileY)
		{
			hdc.tile_image(&m_dibCenter, &rcPart);
		} else if(m_tileX && !m_tileY)
		{
			CTxDIB dib;
			m_dibCenter.resample(m_dibCenter.getWidth(), rcPart.bottom - rcPart.top, &dib);
			hdc.tile_image(&dib, &rcPart);
		} else
		{
			CTxDIB dib;
			m_dibCenter.resample(rcPart.right - rcPart.left, m_dibCenter.getHeight(), &dib);
			hdc.tile_image(&dib, &rcPart);
		}
	}
}
