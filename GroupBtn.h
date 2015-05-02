#pragma once

class CAddressBook;

class CGroupBtn : public CTlbButton
{
	CAddressBook*	m_parent;
public:
	CGroupBtn(CAddressBook* parent);
	~CGroupBtn(void);
};
