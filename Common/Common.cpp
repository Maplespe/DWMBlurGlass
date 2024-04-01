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
#include <array>
#include <algorithm>
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
			auto clean = RAIIHelper::scope_exit([&] { CloseHandle(pFile); });

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
			return ret;
		}

		bool SetIniString(std::wstring_view path, std::wstring_view appName, std::wstring_view keyName, std::wstring_view value)
		{
			return WritePrivateProfileStringW(appName.data(), keyName.data(), value.data(), path.data());
		}

		bool IsAppUseLightMode()
		{
			DWORD dwResult = 0;
			DWORD dwSize = sizeof(DWORD);
			auto hr = RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
				L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &dwResult, &dwSize);
			return dwResult == 1;
		}
	}

	ConfigData ConfigData::LoadFromFile(std::wstring_view path)
	{
		ConfigData cfgData;
		cfgData.applyglobal = Utils::GetIniString(path, L"config", L"applyglobal") == L"true";
		cfgData.extendBorder = Utils::GetIniString(path, L"config", L"extendBorder") == L"true";
		cfgData.reflection = Utils::GetIniString(path, L"config", L"reflection") == L"true";
		cfgData.oldBtnHeight = Utils::GetIniString(path, L"config", L"oldBtnHeight") == L"true";
		cfgData.customAmount = Utils::GetIniString(path, L"config", L"customAmount") == L"true";
		cfgData.useAccentColor = Utils::GetIniString(path, L"config", L"useAccentColor") == L"true";

		auto ret = Utils::GetIniString(path, L"config", L"crossFade");
		if (!ret.empty())
			cfgData.crossFade = ret == L"true";

		ret = Utils::GetIniString(path, L"config", L"extendRound");
		if (!ret.empty())
			cfgData.extendRound = (float)std::clamp(_wtoi(ret.data()), 0, 16);

		ret = Utils::GetIniString(path, L"config", L"blurAmount");
		if (!ret.empty())
			cfgData.blurAmount = (float)std::clamp(_wtof(ret.data()), 0.0, 50.0);

		ret = Utils::GetIniString(path, L"config", L"customBlurAmount");
		if (!ret.empty())
			cfgData.customBlurAmount = (float)std::clamp(_wtof(ret.data()), 0.0, 50.0);

		ret = Utils::GetIniString(path, L"config", L"luminosityOpacity");
		if (!ret.empty())
			cfgData.luminosityOpacity = (float)std::clamp(_wtof(ret.data()), 0.0, 1.0);

		ret = Utils::GetIniString(path, L"config", L"activeTextColor");
		if (!ret.empty())
		{
			cfgData.activeTextColor = (COLORREF)_wtoll(ret.data());
			cfgData.activeTextColor = (cfgData.activeTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"inactiveTextColor");
		if (!ret.empty())
		{
			cfgData.inactiveTextColor = (COLORREF)_wtoll(ret.data());
			cfgData.inactiveTextColor = (cfgData.inactiveTextColor & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"activeTextColorDark");
		if (!ret.empty())
		{
			cfgData.activeTextColorDark = (COLORREF)_wtoll(ret.data());
			cfgData.activeTextColorDark = (cfgData.activeTextColorDark & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"inactiveTextColorDark");
		if (!ret.empty())
		{
			cfgData.inactiveTextColorDark = (COLORREF)_wtoll(ret.data());
			cfgData.inactiveTextColorDark = (cfgData.inactiveTextColorDark & 0x00FFFFFF) | 0xFF000000;
		}

		ret = Utils::GetIniString(path, L"config", L"activeBlendColor");
		if (!ret.empty())
			cfgData.activeBlendColor = (COLORREF)_wtoll(ret.data());

		ret = Utils::GetIniString(path, L"config", L"inactiveBlendColor");
		if (!ret.empty())
			cfgData.inactiveBlendColor = (COLORREF)_wtoll(ret.data());

		ret = Utils::GetIniString(path, L"config", L"activeBlendColorDark");
		if (!ret.empty())
			cfgData.activeBlendColorDark = (COLORREF)_wtoll(ret.data());

		ret = Utils::GetIniString(path, L"config", L"inactiveBlendColorDark");
		if (!ret.empty())
			cfgData.inactiveBlendColorDark = (COLORREF)_wtoll(ret.data());

		ret = Utils::GetIniString(path, L"config", L"blurMethod");
		if (!ret.empty())
			cfgData.blurmethod = (blurMethod)std::clamp(_wtoi(ret.data()), 0, 2);

		ret = Utils::GetIniString(path, L"config", L"effectType");
		if (!ret.empty())
		{
			cfgData.effectType = (MDWMBlurGlass::effectType)std::clamp(_wtoi(ret.data()), 0, 3);
			if (cfgData.blurmethod != blurMethod::CustomBlur && cfgData.effectType > effectType::Acrylic)
				cfgData.effectType = effectType::Acrylic;
		}
		return cfgData;
	}

	template<typename T>
	std::wstring make_wstring(T&& value)
	{
		return std::to_wstring(std::forward<T>(value));
	}

	std::wstring make_wstring(bool value)
	{
		return value ? L"true" : L"false";
	}

	void ConfigData::SaveToFile(std::wstring_view path, const ConfigData& cfg)
	{
		for (const auto regkeyList = std::to_array<std::pair<LPCWSTR, std::wstring>>
		({
			 { L"applyglobal", make_wstring(cfg.applyglobal) },
			 { L"extendBorder", make_wstring(cfg.extendBorder) },
			 { L"reflection", make_wstring(cfg.reflection) },
			 { L"oldBtnHeight", make_wstring(cfg.oldBtnHeight) },
			 { L"customAmount", make_wstring(cfg.customAmount) },
			 { L"crossFade", make_wstring(cfg.crossFade) },
			 { L"useAccentColor", make_wstring(cfg.useAccentColor) },
			 { L"blurAmount", make_wstring(cfg.blurAmount) },
			 { L"customBlurAmount", make_wstring(cfg.customBlurAmount) },
			 { L"luminosityOpacity", make_wstring(cfg.luminosityOpacity) },
			 { L"activeTextColor", make_wstring(cfg.activeTextColor) },
			 { L"inactiveTextColor", make_wstring(cfg.inactiveTextColor) },
			 { L"activeTextColorDark", make_wstring(cfg.activeTextColorDark) },
			 { L"inactiveTextColorDark", make_wstring(cfg.inactiveTextColorDark) },
			 { L"activeBlendColor", make_wstring(cfg.activeBlendColor) },
			 { L"inactiveBlendColor", make_wstring(cfg.inactiveBlendColor) },
			 { L"activeBlendColorDark", make_wstring(cfg.activeBlendColorDark) },
			 { L"inactiveBlendColorDark", make_wstring(cfg.inactiveBlendColorDark) },
			 { L"blurMethod", make_wstring((int)cfg.blurmethod) },
			 { L"effectType", make_wstring((int)cfg.effectType) }
		}); const auto& [key, value] : regkeyList)
		{
			Utils::SetIniString(path, L"config", key, value);
		}
	}
}
