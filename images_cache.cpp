#include "globals.h"
#include "images_cache.h"

images_cache::images_cache()
{

}

void images_cache::init_by_template(const CUSTOM_TEMPLATE& tpl, CTlbContainer* container)
{
	clear();
	if (!tpl.path.empty())
	{
		std::wstring skin_path = expand_skin_path(container, tpl.path);
		if (!skin_path.empty())
		{
			WCHAR path[MAX_PATH];
			StringCchCopy(path, MAX_PATH, skin_path.c_str());
			PathRemoveFileSpec(path);
			PathAddBackslash(path);
			StringCchCat(path, MAX_PATH, L"images\\");
			add_path(path);
		}
	}

	WCHAR folder_path[MAX_PATH];
	GetModuleFileName(g_hInst, folder_path, MAX_PATH);
	PathRemoveFileSpec(folder_path);
	PathAddBackslash(folder_path);
	StringCchCat(folder_path, MAX_PATH, L"images\\");
	add_path(folder_path);
}

cairo_container::image_ptr images_cache::getImage(LPCWSTR url)
{
	std::lock_guard<std::mutex> lock(m_lock);

	auto iter = m_images.find(std::wstring(url));
	if (iter != m_images.end())
	{
		return iter->second;
	}
	for (auto& path : m_paths)
	{
		std::wstring file_name = path;
		file_name += url;
		if (PathFileExists(file_name.c_str()))
		{
			auto img = cairo_container::image_ptr(new CTxDIB);
			if (img->load(file_name.c_str()))
			{
				m_images[std::wstring(url)] = img;
				return img;
			}
		}
	}
	auto img = cairo_container::image_ptr(new CTxDIB);
	HRSRC res = FindResource(g_hInst, url, L"DEFIMAGES");
	if (res)
	{
		if (img->load(res, g_hInst))
		{
			m_images[std::wstring(url)] = img;
			return img;
		}
	}

	return nullptr;
}
