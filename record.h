#pragma once
#include "feild.h"
#include "applications.h"

class CPluginStream;
class images_cache;

class CRecord
{
public:
	typedef std::shared_ptr<CRecord>	ptr;
	typedef std::vector<CRecord::ptr>	vector;
public:
	UINT						m_ID;
	UINT						m_flags;
	FIELD::vector				m_fields;
	LPBYTE						m_image;
	DWORD						m_cbImage;
	cairo_container::image_ptr	m_img;
	SIZE						m_imgSize;
public:
	CRecord();
	CRecord(const CRecord& val);
	~CRecord();

	cairo_container::image_ptr GetPhoto(int cx, int cy);
	void Load(CPluginStream* stream);
	void Save(CPluginStream* stream);
	void InitImage(LPBYTE imgBin = NULL, DWORD cbImage = 0, BOOL copy = FALSE);
	void ClearImage(void);
	void ApplyTemplate(std::wstring& tpl, CXUIEngine* xui, CProgram::vector& progs);

	bool if_not_empty(const std::wstring &fld);
	bool if_not_empty_one(const std::wstring &fld);
	void load_sample();
	bool filter(LPCWSTR filter_str);

	CRecord& operator=(const CRecord& val);
	std::wstring GetFieldValue(LPCWSTR fldName);
	std::wstring GetFieldType(LPCWSTR fldName);
	void DeleteField(LPCWSTR fldName);
	void SetFieldValue(LPCWSTR fldName, LPCWSTR value);
	std::wstring GetFieldLabel(LPCWSTR fldName, CXUIEngine* xui);
	std::wstring GetFieldLabel(int fldIDX, CXUIEngine* xui);
	cairo_container::image_ptr get_image(LPCWSTR url, images_cache* cache, int width = 0, int height = 0);

private:
	FIELD::vector::iterator find_field(LPCWSTR fldName);
};
