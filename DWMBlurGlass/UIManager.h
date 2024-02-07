/**
 * FileName: UIManager.h
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
#pragma once
#include "framework.h"

namespace MDWMBlurGlass
{
	bool LoadLanguageFileList();

	bool LoadBaseLanguageString(std::wstring_view local);

	std::wstring GetBaseLanguageString(std::wstring_view name);

	void ClearBaseLanguage();

	bool LoadLanguageString(Mui::XML::MuiXML* ui, std::wstring_view local, bool init, bool replaceCur = true);

	bool LoadDefualtUIStyle(Mui::XML::MuiXML* ui);

	struct LangInfo
	{
		std::wstring local;
		std::wstring author;
	};
	LangInfo GetCurrentLangInfo();
}