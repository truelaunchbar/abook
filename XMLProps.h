#pragma once
#include <atlbase.h>
#include <MsXml.h>

class CXMLProps
{
private:
	CComPtr<IXMLDOMDocument> m_doc;
	IXMLDOMNode** m_nodes;
	int			  m_count;
	int			  m_bufSize;
	LPTSTR		  m_fileName;
public:
	CXMLProps(void);
	CXMLProps(CXMLProps& xml, LPTSTR topNode);
	virtual ~CXMLProps(void);
	BOOL OpenFile(LPCWSTR fileName, LPCTSTR topNode);
	BOOL OpenText(LPSTR txt, LPCTSTR topNode);
	BOOL OpenText(LPWSTR txt, LPCTSTR topNode);
	BOOL FirstNode(LPTSTR nodeName);
protected:
	void PushNode(IXMLDOMNode* node);
	IXMLDOMNode* GetNode() { return m_count ? m_nodes[m_count - 1] : NULL; };
public:
	BOOL OpenNode(LPTSTR nodeName, BOOL create = FALSE);
	void CloseNode(void);
	BOOL OpenNextNode(void);
	
	LPTSTR  GetNodeText(LPTSTR nodeName, LPCTSTR defValue);
	LPTSTR	GetNodeText();
	LPTSTR	GetNodeName();
	LPTSTR	GetNodeXML();
	BOOL	AddNode(CXMLProps* props);
	BOOL	GetValue(LPTSTR valName, VARIANT* val);
	UINT	GetUINTValue(LPTSTR valName, UINT defValue);
	int		GetINTValue(LPTSTR valName, int defValue);
	double	GetDBLValue(LPTSTR valName, double defValue);
	LPTSTR	GetSTRValue(LPTSTR valName, LPCTSTR defValue);
	void	GetDATETIMEValue(LPTSTR valName, SYSTEMTIME* value);
	BOOL	CreateNode(LPTSTR nodeName, LPCTSTR cValue = NULL);
	
	BOOL	SetValue(LPTSTR valName, VARIANT* val);
	BOOL	SetSTRValue(LPTSTR valName, LPCTSTR value, BOOL del_empty = TRUE);
	BOOL	SetSTRValue(LPTSTR valName, LPCTSTR value, LPCTSTR defValue);
	BOOL	SetUINTValue(LPTSTR valName, UINT value, UINT defValue = 0, BOOL validDefValue = FALSE);
	BOOL	SetINTValue(LPTSTR valName, int value, INT defValue = 0, BOOL validDefValue = FALSE);
	BOOL	SetDBLValue(LPTSTR valName, double value, double defValue = 0, BOOL validDefValue = FALSE);
	BOOL	SetDATETIMEValue(LPTSTR valName, SYSTEMTIME* value);
	BOOL	SetNodeText(LPTSTR nodeName, LPCTSTR value);
	BOOL	SetNodeText(LPCTSTR value);
	long	get_attributes_count();
	BOOL	RemoveAttribute(LPCWSTR attrName);
	
	BOOL	Save(LPTSTR fileName);
	BOOL	CreateFile(LPCTSTR fileName, LPCTSTR topNode);
	void	RemoveAllChilds(void);
	BOOL	RemoveNode(LPTSTR nodeName);
	BOOL	RemoveNode();
};
