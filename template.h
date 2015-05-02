#pragma once

enum tpl_type
{
	tpl_type_custom,
	tpl_type_skin
};

// tpl_type_custom:
//		path - path of the root skin
//		id	 - template id in the templates.xml (> 0)
// tpl_type_skin
//		path - path of the skin
//		id	 = 0
// tpl_type_old
//		path - empty
//		id	 = 0
//		name and text are initialized
// the templates with tpl_type_old must be saved into templates.xml on first save

struct CUSTOM_TEMPLATE
{
	typedef std::vector<CUSTOM_TEMPLATE> vector;

	int				id;
	tpl_type		type;
	std::wstring	name;
	std::wstring	text;
	std::wstring	path;

	CUSTOM_TEMPLATE()
	{
		type = tpl_type_custom;
		id	 = 0;
	}

	CUSTOM_TEMPLATE(LPCWSTR vName, LPCWSTR vText, LPCWSTR vPath, tpl_type tp, int vId)
	{
		name = vName ? vName : L"";
		text = vText ? vText : L"";
		path = vPath ? vPath : L"";
		type = tp;
		id = vId;
	}

	CUSTOM_TEMPLATE(const CUSTOM_TEMPLATE& val)
	{
		name = val.name;
		text = val.text;
		path = val.path;
		type = val.type;
		id = val.id;
	}

	CUSTOM_TEMPLATE(CUSTOM_TEMPLATE&& val)
	{
		name = std::move(val.name);
		text = std::move(val.text);
		path = std::move(val.path);
		type = val.type;
		id = val.id;
	}

	CUSTOM_TEMPLATE& operator=(const CUSTOM_TEMPLATE& val)
	{
		name = val.name;
		text = val.text;
		path = val.path;
		type = val.type;
		id = val.id;
		return *this;
	}

	CUSTOM_TEMPLATE& operator=(CUSTOM_TEMPLATE&& val)
	{
		name = std::move(val.name);
		text = std::move(val.text);
		path = std::move(val.path);
		type = val.type;
		id = val.id;
		return *this;
	}

	void load_text(CTlbContainer* container);
	bool save(CTlbContainer* container);
	bool erase(CTlbContainer* container);
	std::wstring get_base_path(CTlbContainer* container);

};

extern void get_templates_file(CTlbContainer* container, LPWSTR custom_tpls_file);
extern CUSTOM_TEMPLATE::vector load_templates(CTlbContainer* container, bool custom_only = false);
extern std::wstring expand_skin_path(CTlbContainer* container, const std::wstring& path);
extern std::wstring get_skin_name(CTlbContainer* container, const std::wstring& path);

