#pragma once

#include "abookbtn.h"

class CAddressBook;

class CAlphaGroupBtn : public CTlbButton,
					   public IRecordParent
{
	CAddressBook*	m_parent;
	TCHAR			m_alpha;
	BOOL			m_isChanged;

	HANDLE			m_changesHandle;
public:
	CAlphaGroupBtn(CAddressBook* parent, TCHAR alpha);
	~CAlphaGroupBtn(void);

	BOOL isDeletedRecord(UINT recID);
	void NotfyDelete(void);
	void NotifyChange(void);

	void OnDraw(HDC hDC, LPRECT rcItem);

	void GetChildUID(LPWSTR uid);
	BOOL isMenu(void);

	BOOL SupportSortMenu(void);
	void CompareButtons(COMPARE_BUTTONS_DATA* cpBtns);
	BOOL QueryArrangeMenu(UINT idx, ARRANGE_MENU_DATA* qamData);

	virtual BOOL SupportGlassMenu();
	virtual BOOL applyMargins();

	HANDLE	GetChangesHandle();
	void	PrepareChanges(HANDLE hChanges);
	void	CloseChangesHandle(HANDLE hChanges);
	BOOL	FirstChildData(CHILDS_DATA* data);
	BOOL	NextChildData(CHILDS_DATA* data);
	BOOL	FreeChildData(CHILDS_DATA* data);
	BOOL	CreateChild(CREATE_CHILD_DATA* childData);
	UINT	GetMenuFlags(void);
	UINT ReadChanges(void);
};
