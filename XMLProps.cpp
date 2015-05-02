#include "globals.h"
#include "xmlprops.h"
#include <comutil.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#pragma comment(lib, "comsupp.lib")

CXMLProps::CXMLProps(void)
{
	m_fileName = NULL;
	m_doc.CoCreateInstance(CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER);
	if(!m_doc.p)
	{
		// TODO: CoUninitialize
		CoInitialize(NULL);
		m_doc.CoCreateInstance(CLSID_DOMFreeThreadedDocument, NULL, CLSCTX_INPROC_SERVER);
	}
	m_bufSize = 10;
	m_nodes = (IXMLDOMNode**) malloc(sizeof(IXMLDOMNode*) * m_bufSize);
	if(m_doc.p)
	{
		m_doc->put_async(VARIANT_FALSE);
		m_doc->put_resolveExternals(VARIANT_FALSE);
	}
	m_count = 0;
}

CXMLProps::CXMLProps(CXMLProps& xml, LPTSTR topNode) : m_doc(xml.m_doc)
{
	m_fileName = NULL;
	m_bufSize = 10;
	m_nodes = (IXMLDOMNode**) malloc(sizeof(IXMLDOMNode*) * m_bufSize);
	m_count = 0;
	if(m_doc.p)
	{
		m_doc->put_async(VARIANT_FALSE);
		m_doc->put_resolveExternals(VARIANT_FALSE);
		CComBSTR bstrSS(topNode);
		IXMLDOMNode* rootNode = NULL;
		HRESULT hr = m_doc->selectSingleNode(bstrSS, &rootNode);
		if(hr != S_OK) return;
		PushNode(rootNode);
	}
}

CXMLProps::~CXMLProps(void)
{
	for(int i=0; i < m_count; i++)
	{
		m_nodes[i]->Release();
	}
	free(m_nodes);
	if(m_fileName) delete m_fileName;
	if(m_doc.p)
	{
		m_doc.Release();
	}
}

BOOL CXMLProps::OpenFile(LPCWSTR fileName, LPCTSTR topNode)
{
	if(!m_doc.p) return FALSE;
	// Load the XML document file...
	VARIANT_BOOL bSuccess = false;
	HRESULT hr = m_doc->load(CComVariant(fileName), &bSuccess);
	if(hr != S_OK) return FALSE;
	if(!bSuccess) return FALSE;

	CComBSTR bstrSS(topNode);
	IXMLDOMNode* rootNode = NULL;
	hr = m_doc->selectSingleNode(bstrSS, &rootNode);
	if(hr != S_OK) return FALSE;
	PushNode(rootNode);

	m_fileName = new TCHAR[lstrlen(fileName) + 1];
	lstrcpy(m_fileName, fileName);

	return TRUE;
}

BOOL CXMLProps::OpenText(LPSTR txt, LPCTSTR topNode)
{
	if(!m_doc.p) return FALSE;
	// Load the XML document file...
	VARIANT_BOOL bSuccess = false;
	HRESULT hr = m_doc->loadXML(CComBSTR(txt), &bSuccess);
	if(hr != S_OK) return FALSE;
	if(bSuccess == VARIANT_FALSE) return FALSE;

	CComBSTR bstrSS(topNode);
	IXMLDOMNode* rootNode = NULL;
	hr = m_doc->selectSingleNode(bstrSS, &rootNode);
	if(hr != S_OK) return FALSE;
	PushNode(rootNode);

	m_fileName = NULL;

	return TRUE;
}

BOOL CXMLProps::OpenText(LPWSTR txt, LPCTSTR topNode)
{
	if(!m_doc.p) return FALSE;
	// Load the XML document file...
	VARIANT_BOOL bSuccess = false;
	HRESULT hr = m_doc->loadXML(CComBSTR(txt), &bSuccess);
	if(hr != S_OK) return FALSE;
	if(bSuccess == VARIANT_FALSE) return FALSE;

	CComBSTR bstrSS(topNode);
	IXMLDOMNode* rootNode = NULL;
	hr = m_doc->selectSingleNode(bstrSS, &rootNode);
	if(hr != S_OK) return FALSE;
	PushNode(rootNode);

	m_fileName = NULL;

	return TRUE;
}

BOOL CXMLProps::FirstNode(LPTSTR nodeName)
{
	return 0;
}

void CXMLProps::PushNode(IXMLDOMNode* node)
{
	if(m_count + 1 > m_bufSize)
	{
		m_bufSize += 10;
		m_nodes = (IXMLDOMNode**) realloc(m_nodes, sizeof(IXMLDOMNode*) * m_bufSize);
	}
	m_nodes[m_count] = node;
	m_count++;
}

BOOL CXMLProps::OpenNode(LPTSTR nodeName, BOOL create)
{
	IXMLDOMNode* node = GetNode();
	if(node) 
	{
		IXMLDOMNode* newNode = NULL;
		CComBSTR bstrnodeName(nodeName);
		HRESULT hr = node->selectSingleNode(bstrnodeName, &newNode);
		if(hr == S_OK)
		{
			PushNode(newNode);
			return TRUE;
		}
	}
	
	if(create)
	{
		return CreateNode(nodeName);
	}

	return FALSE;
}

void CXMLProps::CloseNode(void)
{
	if(m_count != 0)
	{
		IXMLDOMNode* node = GetNode();
		if(node)
		{
			node->Release();
			m_count--;
		}
	}
}

BOOL CXMLProps::OpenNextNode(void)
{
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	IXMLDOMNode* newNode = NULL;
	HRESULT hr = node->get_nextSibling(&newNode);
	if(hr != S_OK) return FALSE;
	CloseNode();
	PushNode(newNode);
	return TRUE;
}

BOOL CXMLProps::GetValue(LPTSTR valName, VARIANT* val)
{
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	CComQIPtr<IXMLDOMElement> spXMLChildElement;
	spXMLChildElement = node;
	HRESULT hr = spXMLChildElement->getAttribute(CComBSTR(valName), val);
	if(hr != S_OK) return FALSE;
	return TRUE;
}

UINT CXMLProps::GetUINTValue(LPTSTR valName, UINT defValue)
{
	CComVariant val;
	if(GetValue(valName, &val))
	{
		if(val.ChangeType(VT_UINT) == S_OK)
		{
			return val.uintVal;
		}
	}
	return defValue;
}

void CXMLProps::GetDATETIMEValue(LPTSTR valName, SYSTEMTIME* value)
{
	ZeroMemory(value, sizeof(SYSTEMTIME));
	CComVariant val;
	if(GetValue(valName, &val))
	{
		if(val.ChangeType(VT_BSTR) == S_OK)
		{
			__int64 ft = _tstoi64((LPTSTR) val.bstrVal);
			FileTimeToSystemTime((LPFILETIME) &ft, value);
		}
	}
}

int CXMLProps::GetINTValue(LPTSTR valName, int defValue)
{
	CComVariant val;
	if(GetValue(valName, &val))
	{
		if(val.ChangeType(VT_INT) == S_OK)
		{
			return val.intVal;
		}
	}
	return defValue;
}

double CXMLProps::GetDBLValue(LPTSTR valName, double defValue)
{
	CComVariant val;
	if(GetValue(valName, &val))
	{
		if(val.ChangeType(VT_R8) == S_OK)
		{
			return val.dblVal;
		}
	}
	return defValue;
}

LPTSTR CXMLProps::GetSTRValue(LPTSTR valName, LPCTSTR defValue)
{
	CComVariant val;
	if(GetValue(valName, &val))
	{
		if(val.ChangeType(VT_BSTR) == S_OK)
		{
			LPTSTR ret = new TCHAR[SysStringLen(val.bstrVal) + 1];
			lstrcpy(ret, val.bstrVal);
			return ret;
		}
	}
	if(defValue)
	{
		LPTSTR ret = new TCHAR[lstrlen(defValue) + 1];
		lstrcpy(ret, defValue);
		return ret;
	}
	return NULL;
}

BOOL CXMLProps::CreateNode(LPTSTR nodeName, LPCTSTR cValue)
{
	if(!m_doc.p) return FALSE;
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	IXMLDOMNode* newNode = NULL;
	HRESULT hr = m_doc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(nodeName), NULL, &newNode);
	if(hr != S_OK) return FALSE;
	IXMLDOMNode* insertedNode = NULL;
	hr = node->appendChild(newNode, &insertedNode);
	if(hr == S_OK)
	{
		PushNode(insertedNode);
		newNode->Release();
		if(cValue)
		{
			insertedNode->put_text(CComBSTR(cValue));
		}
		return TRUE;
	}
	newNode->Release();
	return FALSE;
}

BOOL CXMLProps::SetSTRValue(LPTSTR valName, LPCTSTR value, BOOL del_empty)
{
	if(!del_empty || value && value[0])
	{
		if(value)
		{
			CComVariant val(value);
			return SetValue(valName, &val);
		} else
		{
			CComVariant val(L"");
			return SetValue(valName, &val);
		}
	}

	IXMLDOMNode* node = GetNode();
	if(node)
	{
		CComQIPtr<IXMLDOMElement> spXMLChildElement;
		spXMLChildElement = node;
		spXMLChildElement->removeAttribute(CComBSTR(valName));
	}
	return TRUE;
}

BOOL CXMLProps::SetSTRValue( LPTSTR valName, LPCTSTR value, LPCTSTR defValue /*= NULL*/ )
{
	if(value && defValue)
	{
		if(!lstrcmp(value, defValue))
		{
			return SetSTRValue(valName, NULL, TRUE);
		} else
		{
			return SetSTRValue(valName, value, TRUE);
		}
	}
	return SetSTRValue(valName, value, TRUE);
}

BOOL CXMLProps::SetNodeText(LPTSTR nodeName, LPCTSTR value)
{
	if(!OpenNode(nodeName))
	{
		if(!CreateNode(nodeName, NULL))
		{
			return FALSE;
		}
	}
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	if(node->put_text((BSTR) value) == S_OK)
	{
		CloseNode();
		return TRUE;
	}
	CloseNode();
	return FALSE;
}

BOOL CXMLProps::SetNodeText(LPCTSTR value)
{
	IXMLDOMNode* node = GetNode();
	if (!node) return FALSE;
	if (node->put_text((BSTR)value) == S_OK)
	{
		return TRUE;
	}
	return FALSE;
}

LPTSTR CXMLProps::GetNodeText()
{
	LPTSTR ret = NULL;
	IXMLDOMNode* node = GetNode();
	if(node)
	{
		BSTR txt = NULL;
		if(node->get_text(&txt) == S_OK)
		{
			ret = new TCHAR[SysStringLen(txt) + 1];
			lstrcpy(ret, txt);
			SysFreeString(txt);
		}
	}
	return ret;
}

LPTSTR CXMLProps::GetNodeText(LPTSTR nodeName, LPCTSTR defValue)
{
	LPTSTR ret = NULL;
	if(OpenNode(nodeName))
	{
		IXMLDOMNode* node = GetNode();
		if(node)
		{
			BSTR txt = NULL;
			if(node->get_text(&txt) == S_OK)
			{
				ret = new TCHAR[SysStringLen(txt) + 1];
				lstrcpy(ret, txt);
				SysFreeString(txt);
			}
		}
		CloseNode();
	}
	if(!ret && defValue)
	{
		ret = new TCHAR[lstrlen(defValue) + 1];
		lstrcpy(ret, defValue);
	}
	return ret;
}

BOOL CXMLProps::SetUINTValue(LPTSTR valName, UINT value, UINT defValue, BOOL validDefValue)
{
	if(!validDefValue || (validDefValue && value != defValue))
	{
		CComVariant val(value);
		return SetValue(valName, &val);
	} else
	{
		IXMLDOMNode* node = GetNode();
		if(node)
		{
			CComQIPtr<IXMLDOMElement> spXMLChildElement;
			spXMLChildElement = node;
			spXMLChildElement->removeAttribute(CComBSTR(valName));
		}
	}
	return TRUE;
}

BOOL CXMLProps::SetINTValue(LPTSTR valName, int value, INT defValue, BOOL validDefValue)
{
	if(!validDefValue || (validDefValue && value != defValue))
	{
		CComVariant val(value);
		return SetValue(valName, &val);
	} else
	{
		IXMLDOMNode* node = GetNode();
		if(node)
		{
			CComQIPtr<IXMLDOMElement> spXMLChildElement;
			spXMLChildElement = node;
			spXMLChildElement->removeAttribute(CComBSTR(valName));
		}
	}
	return TRUE;
}

BOOL CXMLProps::SetDBLValue(LPTSTR valName, double value, double defValue, BOOL validDefValue)
{
	if(!validDefValue || (validDefValue && value != defValue))
	{
		CComVariant val(value);
		return SetValue(valName, &val);
	} else
	{
		IXMLDOMNode* node = GetNode();
		if(node)
		{
			CComQIPtr<IXMLDOMElement> spXMLChildElement;
			spXMLChildElement = node;
			spXMLChildElement->removeAttribute(CComBSTR(valName));
		}
	}
	return TRUE;
}

BOOL CXMLProps::SetDATETIMEValue(LPTSTR valName, SYSTEMTIME* value)
{
	ULONGLONG fileTime;
	if(value->wYear)
	{
		SystemTimeToFileTime(value, (LPFILETIME) &fileTime);
		TCHAR strVal[255];
		StringCbPrintf(strVal, 255 * sizeof(TCHAR), TEXT("%I64u"), fileTime);
		CComVariant val(strVal);
		return SetValue(valName, &val);
	} else
	{
		return FALSE;
	}
}

BOOL CXMLProps::SetValue(LPTSTR valName, VARIANT* val)
{
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	CComQIPtr<IXMLDOMElement> spXMLChildElement;
	spXMLChildElement = node;
	HRESULT hr = spXMLChildElement->setAttribute(CComBSTR(valName), *val);
	if(hr != S_OK) return FALSE;
	return TRUE;
}

BOOL CXMLProps::Save(LPTSTR fileName)
{
	if(!m_doc.p) return FALSE;
	if(!fileName)
	{
		if(m_doc->save(CComVariant(m_fileName)) == S_OK)
		{
			return TRUE;
		}
	} else
	{
		if(m_doc->save(CComVariant(fileName)) == S_OK)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CXMLProps::CreateFile(LPCTSTR fileName, LPCTSTR topNode)
{
	if(!m_doc.p) return FALSE;
	IXMLDOMNode* newNode = NULL;

	IXMLDOMProcessingInstruction* pi = NULL;;
	HRESULT hr = m_doc->createProcessingInstruction(CComBSTR(TEXT("xml")), CComBSTR(TEXT("version=\"1.0\" encoding=\"UTF-8\"")), &pi);
	//m_doc->createNode(CComVariant(NODE_PROCESSING_INSTRUCTION), CComBSTR(TEXT("xml")), NULL, &newNode);
	if(hr != S_OK) return FALSE;
	IXMLDOMNode* insertedNode = NULL;
	hr = m_doc->appendChild(pi, &insertedNode);
	pi->Release();
	insertedNode->Release();
	if(hr != S_OK) return FALSE;

	hr = m_doc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(topNode), NULL, &newNode);
	if(hr != S_OK) return FALSE;
	insertedNode = NULL;
	hr = m_doc->appendChild(newNode, &insertedNode);
	if(hr == S_OK)
	{
		PushNode(insertedNode);
		newNode->Release();
		if(fileName)
		{
			m_fileName = new TCHAR[lstrlen(fileName) + 1];
			lstrcpy(m_fileName, fileName);
		} else
		{
			m_fileName = NULL;
		}
		return TRUE;
	}
	newNode->Release();
	return FALSE;
}

void CXMLProps::RemoveAllChilds(void)
{
	IXMLDOMNode* node = GetNode();
	if(!node) return;
	CComPtr<IXMLDOMNodeList> lst;
	node->get_childNodes(&lst);
	long len = 0;
	lst->get_length(&len);
	for(int i=0; i < len; i++)
	{
		IXMLDOMNode* oldChild = NULL;
		IXMLDOMNode* child = NULL;
		lst->get_item(0, &child);
		if(child)
		{
			if(node->removeChild(child, &oldChild) == S_OK)
			{
				oldChild->Release();
			}
			child->Release();
		}
	}
}

BOOL CXMLProps::RemoveNode(LPTSTR nodeName)
{
	IXMLDOMNode* node = GetNode();
	if(!node) return FALSE;
	IXMLDOMNode* delNode = NULL;
	CComBSTR bstrnodeName(nodeName);
	HRESULT hr = node->selectSingleNode(bstrnodeName, &delNode);
	if(hr != S_OK) return FALSE;
	IXMLDOMNode* oldChild = NULL;
	if(node->removeChild(delNode, &oldChild) == S_OK)
	{
		oldChild->Release();
	}
	delNode->Release();
	return TRUE;
}

BOOL CXMLProps::RemoveNode()
{
	IXMLDOMNode* delNode = GetNode();
	if(!delNode) return FALSE;
	delNode->AddRef();
	CloseNode();
	IXMLDOMNode* node = GetNode();
	if(!node)
	{
		delNode->Release();
		return FALSE;
	}
	IXMLDOMNode* oldChild = NULL;
	if(node->removeChild(delNode, &oldChild) == S_OK)
	{
		oldChild->Release();
	}
	delNode->Release();
	return TRUE;
}

LPTSTR CXMLProps::GetNodeXML()
{
	LPTSTR ret = NULL;
	IXMLDOMNode* node = GetNode();
	if(node)
	{
		BSTR txt = NULL;
		if(node->get_xml(&txt) == S_OK)
		{
			ret = new TCHAR[SysStringLen(txt) + 1];
			lstrcpy(ret, txt);
			SysFreeString(txt);
		}
	}
	return ret;
}

BOOL CXMLProps::AddNode( CXMLProps* props )
{
	IXMLDOMNode* node = GetNode();
	IXMLDOMNode* childNode = props->GetNode();
	IXMLDOMNode* insertedNode = NULL;
	HRESULT hr = node->appendChild(childNode, &insertedNode);
	if(hr == S_OK)
	{
		insertedNode->Release();
		return TRUE;
	}
	return FALSE;
}

long CXMLProps::get_attributes_count()
{
	long ret = 0;

	IXMLDOMNode* node = GetNode();
	IXMLDOMNamedNodeMap* attr_map = NULL;
	node->get_attributes(&attr_map);
	if(attr_map)
	{
		attr_map->get_length(&ret);
		attr_map->Release();
	}

	return ret;
}

LPTSTR CXMLProps::GetNodeName()
{
	LPTSTR ret = NULL;
	IXMLDOMNode* node = GetNode();
	if(node)
	{
		BSTR txt = NULL;
		if(node->get_nodeName(&txt) == S_OK)
		{
			ret = new TCHAR[SysStringLen(txt) + 1];
			lstrcpy(ret, txt);
			SysFreeString(txt);
		}
	}
	return ret;
}

BOOL CXMLProps::RemoveAttribute( LPCWSTR attrName )
{
	IXMLDOMNode* node = GetNode();
	if(node)
	{
		CComQIPtr<IXMLDOMElement> spXMLChildElement;
		spXMLChildElement = node;
		if(spXMLChildElement->removeAttribute(CComBSTR(attrName)) == S_OK)
		{
			return TRUE;
		}
	}
	return FALSE;
}
