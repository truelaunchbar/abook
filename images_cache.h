#pragma once

class images_cache
{
	cairo_container::images_map	m_images;
	std::vector<std::wstring>	m_paths;
	std::mutex					m_lock;
public:
	images_cache();
	void add_path(LPCWSTR path)
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_paths.push_back(std::wstring(path));
	}
	void clear()
	{
		std::lock_guard<std::mutex> lock(m_lock);
		m_paths.clear();
		m_images.clear();
	}
	void init_by_template(const CUSTOM_TEMPLATE& tpl, CTlbContainer* container);
	cairo_container::image_ptr getImage(LPCWSTR url);
};