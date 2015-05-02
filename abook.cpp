#include "globals.h"
#include "abookbtn.h"

UINT g_clipRecord = 0;
UINT g_clipParent = 0;

// {AD43E27A-657A-44df-98C0-4713AB5721DC}
EXTERN_C GUID CLSID_ABOOK = 
{ 0xade143d4, 0x3fa6, 0x4d21, { 0x86, 0xbb, 0x62, 0x35, 0xa5, 0x7a, 0xdc, 0xbb } };

CRITICAL_SECTION	cairo_font::m_sync;

LPCLSID GetCLSID()
{
	return &CLSID_ABOOK;
}

LPSTR GetPluginName()
{
	return "Address Book";
}

void InitDLL()
{
	LoadLibrary(TEXT("riched20.dll"));
	g_clipRecord = RegisterClipboardFormat(RECORD_CLIPBOARD_FORMAT);
	g_clipParent = RegisterClipboardFormat(PARENT_CLIPBOARD_FORMAT);
	InitializeCriticalSectionAndSpinCount(&cairo_font::m_sync, 1000);
}

void UninitDLL()
{
	DeleteCriticalSection(&cairo_font::m_sync);
}

CTlbButton*	CreatePlugin()
{
	return new CAddressBook;
}

