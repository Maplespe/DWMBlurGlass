/**
 * FileName: Common.cpp
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
#include "Common.h"
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

namespace MDWMBlurGlass
{
	const HINSTANCE this_inst = reinterpret_cast<HINSTANCE>(&__ImageBase);

	namespace Utils
	{
		std::wstring GetCurrentDir()
		{
			wchar_t sPath[MAX_PATH];
			GetModuleFileNameW(this_inst, sPath, MAX_PATH);
			std::wstring path = sPath;
			path = path.substr(0, path.rfind(L'\\'));

			return path;
		}

		std::wstring GetIniString(std::wstring_view path, std::wstring_view appName, std::wstring_view keyName)
		{
			if (!PathFileExistsW(path.data()))
				return {};

			const HANDLE pFile = CreateFileW(path.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (!pFile)
				return {};

			LARGE_INTEGER fileSize;
			if (!GetFileSizeEx(pFile, &fileSize))
				return {};

			wchar_t* data = new wchar_t[fileSize.QuadPart];
			memset(data, 0, sizeof(wchar_t) * fileSize.QuadPart);
			GetPrivateProfileStringW(appName.data(), keyName.data(), nullptr, data, (DWORD)fileSize.QuadPart, path.data());

			std::wstring ret = data;
			delete[] data;

			CloseHandle(pFile);
			return ret;
		}
	}
}