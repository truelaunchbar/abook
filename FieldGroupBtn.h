#pragma once
#include "abookbtn.h"
class CAddressBook;

class CFieldGroupBtn : public CTlbButton,
					   public IRecordParent
{
	CAddressBook*	m_parent;
	WCHAR			m_field[255];
	LPWSTR			m_value;
	BOOL			m_isChanged;

	HANDLE			m_changesHandle;
public:
	CFieldGroupBtn(CAddressBook* parent, LPCWSTR field, LPCWSTR value);
	~CFieldGroupBtn(void);

	// IRecordParent
	BOOL	isDeletedRecord(UINT recID);
	void	NotfyDelete(void);
	void	NotifyChange(void);

	void	GetChildUID(LPWSTR uid);
	BOOL	isMenu(void);
	BOOL	LoadIcon(int size);

	HANDLE	GetChangesHandle();
	void	PrepareChanges(HANDLE hChanges);
	void	CloseChangesHandle(HANDLE hChanges);
	BOOL	FirstChildData(CHILDS_DATA* data);
	BOOL	NextChildData(CHILDS_DATA* data);
	BOOL	FreeChildData(CHILDS_DATA* data);
	BOOL	CreateChild(CREATE_CHILD_DATA* childData);
	UINT	GetMenuFlags(void);
	UINT	ReadChanges(void);

	virtual BOOL SupportGlassMenu();
	virtual BOOL applyMargins();

	BOOL SupportSortMenu(void);
	void CompareButtons(COMPARE_BUTTONS_DATA* cpBtns);
	BOOL QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData);

	DWORD	AcceptDropObject(IDataObject* lpObj, DWORD dwKeys);
	DWORD	DropObject(IDataObject* lpObj, DWORD dwEffect);
};
