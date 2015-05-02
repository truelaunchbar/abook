#pragma	  once

#include <atlstr.h>
#include "vcard.h"
#include "PluginStream.h"
#include "ABookEngine.h"
#include "TxThread.h"
#include <vector>
#include <string>
#include "record.h"
#include "images_cache.h"

#define WM_EDITRECORD	(WM_USER + 1000)
#define WM_DELRECORD	(WM_USER + 1001)

#define RECORD_MENU		0x01

#define MID_ADDCONTACT	0
#define MID_IMPORT		1

#define HOTKEY_NEWCONTACT	1
#define HOTKEY_MENU			2

#define GROUP_TYPE_NONE		0
#define GROUP_TYPE_ALPHA	1
#define GROUP_TYPE_FIELD	2

class IRecordParent
{
public:
	virtual BOOL	isDeletedRecord(UINT recID) = 0;
	virtual void	NotfyDelete() = 0;
	virtual void	NotifyChange() = 0;
};

#define AMID_NAME	1

class CAddressBook : public CTlbButton,
					 public IRecordParent,
					 public CTxThread
{
private:
	UINT	m_version;	// version of plugin

	//menu images
	CTxDIB		m_menuImgImport;
	CTxDIB		m_menuImgNew;

	CRITICAL_SECTION	m_sync;

	void LockItems() { EnterCriticalSection(&m_sync); }
	void UnlockItems() { LeaveCriticalSection(&m_sync); }

	int					m_StartMenuID;

	UINT				m_nexID;

	HANDLE				m_changesHandle;
	HFONT				m_itemFont;
	HFONT				m_completeFont;

	WORD				m_hkNew;
	WORD				m_hkMenu;

	HICON				m_tipIcon;

	CRecord::vector		m_records;
	CProgram::vector	m_progs;

	images_cache		m_images;

	CUSTOM_TEMPLATE			m_template;
	CUSTOM_TEMPLATE::vector	m_templates;

	DWORD				m_groupType;
	WCHAR				m_groupField[255];

	SIZE				m_photoSize;

	LPTSTR				m_Description;
	litehtml::context	m_web_context;
	BOOL				m_interactiveTips;

public:
	TCHAR			m_defFont[32];
	int				m_defFontSize;
	int				m_defFontSizePx;
	CABookEngine	m_xui;

	CAddressBook();
	~CAddressBook();

	DWORD    GetGroupType() { return m_groupType; }
	LPCWSTR  GetGroupField() { return m_groupField; }

	BOOL isDeletedRecord(UINT recID);
	
	BOOL Save(IStream *data);
	BOOL Load(IStream *data);

	BOOL OnProperties();
	UINT GetSupportedActions(void);
	UINT GetModeFlags(void);
	void GetSize(SIZE* sz, BOOL actual);
	BOOL OnCreate();

	BOOL openProperties();
	HICON GetTipIcon(int tipID);
	LPWSTR GetTipCaption(int tipID);
	LPWSTR GetTipText(int tipID);
	BOOL isMenu(void);
	UINT GetMenuFlags(void);
	BOOL SetIconLocation(LPCWSTR szLocation);

	virtual BOOL SupportGlassMenu();

	HANDLE	GetChangesHandle();
	void	PrepareChanges(HANDLE hChanges);
	void	CloseChangesHandle(HANDLE hChanges);
	BOOL	FirstChildData(CHILDS_DATA* data);
	BOOL	NextChildData(CHILDS_DATA* data);
	BOOL	FreeChildData(CHILDS_DATA* data);
	BOOL	CreateChild(CREATE_CHILD_DATA* childData);

	virtual BOOL FirstChildDataF(CHILDS_DATA* data, LPWSTR filter, HANDLE hStop);
	virtual BOOL NextChildDataF(CHILDS_DATA* data, LPWSTR filter, HANDLE hStop);

	virtual BOOL SupportSearchMenu();

	DWORD AcceptDropObject(IDataObject* lpObj, DWORD dwKeys);
	DWORD DropObject(IDataObject* lpObj, DWORD dwEffect);

	int QueryContextMenu(HMENU hMenu, int index, int cmdFirst, int cmdLast);
	BOOL OnContextMenuCommand(int ID);
	BOOL OnSetLCID(DWORD dwLCID, HMODULE hInstance);

	BOOL LoadIcon(int size);
	void runLink(LPCWSTR link);
	virtual DWORD ThreadProc();
	std::wstring GetText(UINT recID, LPCWSTR tpl = NULL);
	std::wstring GetBasePath();
	CProgram::vector& get_progs()
	{
		return m_progs;
	}
	BOOL use_interactive_tips()
	{
		return m_interactiveTips;
	}

	int GetHotkeysCount(void);
	void OnHotkey(UINT hkID);
	BOOL GetHotKeyData(int idx, HOTKEY_DATA* hkData);
	void onImport(void);
	std::vector<UINT> GetRecordsByFirstLetter(TCHAR alpha);
	void onEditContact(UINT id, IRecordParent* parent);
	BOOL GetRecord(UINT recID, CRecord& rec);
	void DeleteRecord(UINT recID);
	void NotfyDelete(void);
	void GetAlphaGroupText(WCHAR alpha, LPWSTR txt, int cbTXT);
	void NotifyChange(void);
	std::vector<UINT> GetRecordsByField(LPCWSTR field, LPCWSTR value);
	std::vector<std::wstring> GetValuesOfField(LPCWSTR field);
	std::vector<UINT> GetFieldEmptyRecords(LPCWSTR field);
	std::vector<std::wstring> GetCategories(void);
	cairo_container::image_ptr GetIcon(UINT recID, LPCTSTR iconID, int width = -1, int height = -1);
	
	BOOL SupportSortMenu(void);
	void CompareButtons(COMPARE_BUTTONS_DATA* cpBtns);
	BOOL QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData);
	void UpdateRecord(UINT recID, CRecord& rec, BOOL save = TRUE);
	UINT FindSimilarRecord(CRecord& rec);

	litehtml::context& get_web_context();

private:
	void onNewContact(void);
	CRecord::vector::iterator find_record(UINT recID);
	std::wstring GetFirstLetters(void);
	void ActivateWindow(HWND wndActivate);
	void load_master_css();
	void init_font();
};

inline litehtml::context& CAddressBook::get_web_context()
{
	return m_web_context;
}
