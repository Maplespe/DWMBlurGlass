/**
 * FileName: UIManager.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "UIManager.h"
#include "Helper/Helper.h"
#include "pugixml/pugixml.hpp"
#include <unordered_map>

namespace MDWMBlurGlass
{
	std::unordered_map<std::wstring, std::wstring> g_langfileList;
	std::unordered_map<std::wstring, std::wstring> g_baselangList;

	bool LoadLanguageFileList()
	{
		std::wstring path = Utils::GetCurrentDir() + L"\\data\\lang\\";

		std::vector<std::wstring> fileList;
		EnumFiles(path, L"*.xml", fileList);

		if (fileList.empty())
			return false;

		for(auto& file : fileList)
		{
			pugi::xml_document xmldoc;

			if (!xmldoc.load_file(file.c_str()))
				continue;

			auto root = xmldoc.child(L"lang");
			if (root.empty())
				continue;

			auto local = root.attribute(L"local");
			if (local.empty())
				continue;

			g_langfileList.insert({ static_cast<std::wstring>(local.as_string()), file });
		}

		return !g_langfileList.empty();
	}

	std::wstring GetLanguageString(std::wstring_view name)
	{
		if (auto iter = g_baselangList.find(name.data()); iter != g_baselangList.end())
			return iter->second;
		return {};
	}

	void ClearBaseLanguage()
	{
		g_baselangList.clear();
	}

	bool LoadLanguageString(std::wstring_view local)
	{
		const auto iter = g_langfileList.find(local.data());
		if (iter == g_langfileList.end())
			return false;

		pugi::xml_document xmldoc;

		if (!xmldoc.load_file(iter->second.c_str()))
			return false;

		const auto root = xmldoc.child(L"lang");
		if (root.empty())
			return false;

		for(auto& element : root)
		{
			std::wstring name = element.name();
			std::wstring value = ReplaceString(element.text().get(), L"\\n", L"\n");

			if (name.empty() || value.empty())
				continue;

			g_baselangList.insert({ name, value });
		}
		return true;
	}
}