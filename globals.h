#pragma	  once

#define TLB_USE_TXDIB
#define TLB_USE_XUILIB
#define TLB_USE_CAIRO
#define TLB_USE_SIMPLEDIB

#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <uxtheme.h>
#include <tlbpdklib.h>
#include <shlwapi.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <stdlib.h>
#include <vector>
#include <litehtml.h>
#include <cairo_container.h>
#include <cairo_font.h>
#include <algorithm>
#include <mutex>
#include "template.h"

EXTERN_C GUID FAR CLSID_ABOOK;

// TODO: Add all includes definitions here

#define RECORD_CLIPBOARD_FORMAT		TEXT("tlbAddressBookRecord")
extern UINT g_clipRecord;
#define PARENT_CLIPBOARD_FORMAT		TEXT("tlbAddressBookParent")
extern UINT g_clipParent;

#define ASSERT
#define TRACE
#define AfxFindResourceHandle g_hInst;

#define FREE_CLEAR_STR(str) if(str) delete str; str = NULL;
#define MAKE_STR(str, cpstr) FREE_CLEAR_STR(str); if(cpstr) { str = new WCHAR[lstrlen(cpstr) + 1]; lstrcpy(str, cpstr); }
#define IsCharNumeric(ch) (IsCharAlphaNumeric(ch) && !IsCharAlpha(ch))

inline bool is_empty(LPCWSTR val)
{
	if (!val || val && !val[0])
	{
		return true;
	}
	return false;
}

struct AB_FIELDS
{
	LPCWSTR	strID;
	LPCWSTR	attr;
	LPWSTR	defText;
	LPCWSTR	fldName;
	BOOL	inList;
	BOOL	haveImage;
	LPCWSTR	redirNameTo;
	BOOL	suppotsCustomCmd;
	LPCWSTR	groupID;
	LPCWSTR	groupAttr;
	LPWSTR	groupDefText;
};

struct DEF_TEMPLATES
{
	LPCWSTR	strID;
	LPCWSTR	attr;
	LPWSTR	defText;

	UINT	text;
};

extern DEF_TEMPLATES g_defTemplates[];


extern AB_FIELDS g_defText[];


extern HINSTANCE g_hInst;

extern void GetFileVersion(LPCTSTR fileName, DWORD& versionMS, DWORD& versionLS);
extern void Version2Str(DWORD versionMS, DWORD versionLS, LPTSTR str);
extern void DrawVGradient(HDC dc, const RECT& rectClient, COLORREF m_clrStart, COLORREF m_clrEnd, BYTE alpha);
extern void replace_str(std::wstring& dst, std::wstring& substr, std::wstring& replaceWith);
extern LPWSTR load_utf8_file(LPCWSTR fileName);
extern std::wstring load_text_from_resource(LPCWSTR id, LPCWSTR group);

#define DEFAULT_TEMPLATE	TEXT("<table width=100% cellspacing=0 cellpadding=0>\r\n<tr>\r\n	<td colspan=3>&nbsp;<h3>{displayname}</h3></td>\r\n</tr>\r\n{ifnotempty:phone1} \r\n<tr>\r\n	<td width=16></td>\r\n	<td width=16>{img:phonetype1}</td>\r\n	<td>&nbsp;{phone1}</td>\r\n</tr>\r\n{endif}\r\n{ifnotempty:phone2} \r\n<tr>\r\n	<td width=16></td>\r\n	<td width=16>{img:phonetype2}</td>\r\n	<td>&nbsp;{phone2}</td>\r\n</tr>\r\n{endif}\r\n</table>\r\n{ifnotempty:notes} \r\n<table width=100% cellspacing=0 cellpadding=0>\r\n<tr>\r\n	<td width=18>{img:note}</td><td><pre>{notes}</pre></td>\r\n</tr>\r\n</table>\r\n{endif}\r\n<table bgcolor=\"#6666FF\" width=100% cellspacing=0 cellpadding=0 height=2>\r\n<tr><td></td></tr>\r\n</table>")
