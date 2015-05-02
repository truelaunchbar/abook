#pragma once

class FIELD
{
public:
	typedef std::vector<FIELD>	vector;

	UINT			flags;
	std::wstring	name;
	std::wstring	displayName;
	std::wstring	value;

	FIELD()
	{
		flags = 0;
	}
	FIELD(const FIELD& val)
	{
		flags = val.flags;
		name = val.name;
		displayName = val.displayName;
		value = val.value;
	}
	FIELD& operator=(const FIELD& val)
	{
		flags = val.flags;
		name = val.name;
		displayName = val.displayName;
		value = val.value;
		return *this;
	}
};
