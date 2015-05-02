#pragma once
#include <vector>

#define SDT_BEGINNODE	0
#define SDT_ENDNODE		1
#define SDT_DATA		2

struct STREAM_DATA
{
	LPTSTR	id;
	DWORD	type;
	DWORD	cbSize;
	LPBYTE	lpData;
};

class CPluginStream
{
	std::vector<STREAM_DATA> m_data;
	std::vector<int>		 m_nodes;
public:
	CPluginStream(void);
	virtual ~CPluginStream(void);
	// Loading data from stream
	BOOL Load(IStream* stream);
	// Save data to stream
	BOOL Save(IStream* stream);
	void Clear(void);
	DWORD GetDWORD(LPCTSTR id, DWORD defValue = 0);
	WORD GetWORD(LPCTSTR id, WORD defValue = 0);
	LPCTSTR GetString(LPCTSTR id, LPCTSTR defValue = NULL);
	LPBYTE GetBIN(LPCTSTR id, DWORD* cbSize = NULL);
	void SaveDWORD(LPCTSTR id, DWORD data);
	void SaveWORD(LPCTSTR id, WORD data);
	void SaveBIN(LPCTSTR id, LPBYTE data, DWORD cbData);
	void SaveString(LPCTSTR id, LPCTSTR data);
	void BeginNode(LPCTSTR id);
	BOOL EndNode();
	BOOL OpenNode(LPCTSTR id);
	BOOL CloseNode();
private:
	STREAM_DATA* findData(LPCTSTR id);
	void saveData(LPCTSTR id, DWORD type, LPBYTE data, DWORD cbSize);
};
