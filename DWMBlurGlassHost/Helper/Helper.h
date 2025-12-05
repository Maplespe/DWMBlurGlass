/**
 * FileName: Helper.h
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
#include "../framework.h"
#include "Common.h"

namespace MDWMBlurGlass
{
	extern void EnumFiles(std::wstring_view path, std::wstring_view append, std::vector<std::wstring>& fileList);

	extern std::wstring ReadFileSting(std::wstring_view filePath);

	extern std::pair<std::wstring, std::wstring> GetSystemLocaleAndParent();

	extern DWORD GetProcessId(std::wstring_view name);

	extern bool Inject(DWORD processId, std::wstring_view dllName, std::wstring& err);

	extern bool UnInject(DWORD processId, std::wstring_view dllName, std::wstring& err);

	extern bool ModuleIsExists(DWORD processId, std::wstring_view dllName);

	extern bool IsRunasAdmin();

	extern std::wstring ReplaceString(std::wstring str, std::wstring_view old_value, std::wstring_view new_value);
}