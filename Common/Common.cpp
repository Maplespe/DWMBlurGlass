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

		bool SetIniString(std::wstring_view path, std::wstring_view appName, std::wstring_view keyName, std::wstring_view value)
		{
			return WritePrivateProfileStringW(appName.data(), keyName.data(), value.data(), path.data());
		}

		bool IsAppUseLightMode()
		{
			DWORD dwResult = 0;
			DWORD dwSize = sizeof(DWORD);
			auto hr = RegGetValueW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &dwResult, &dwSize);
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
		// newly added params since 2.1.0
		cfgData.overrideAccent = Utils::GetIniString(path, L"config", L"overrideAccent") == L"true";

		auto ret = Utils::GetIniString(path, L"config", L"extendRound");

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

		// newly added params since 2.1.0
		ret = Utils::GetIniString(path, L"config", L"glassIntensity");
		if (!ret.empty())
			cfgData.glassIntensity = (float)std::clamp(_wtof(ret.data()), 0.0, 1.0);
		ret = Utils::GetIniString(path, L"config.aero", L"ColorizationColorBalance");
		if (!ret.empty())
			cfgData.ColorizationColorBalance = (float)std::clamp(_wtof(ret.data()), 0.0, 100.0);

		ret = Utils::GetIniString(path, L"config.aero", L"ColorizationAfterglowBalance");
		if (!ret.empty())
			cfgData.ColorizationAfterglowBalance = (float)std::clamp(_wtof(ret.data()), 0.0, 100.0);

		ret = Utils::GetIniString(path, L"config.aero", L"ColorizationBlurBalance");
		if (!ret.empty())
			cfgData.ColorizationBlurBalance = (float)std::clamp(_wtof(ret.data()), 0.0, 100.0);

		

		ret = Utils::GetIniString(path, L"config", L"effectType");
		if (!ret.empty())
		{
			cfgData.effectType = (MDWMBlurGlass::effectType)std::clamp(_wtoi(ret.data()), 0, 3);
			if (cfgData.blurmethod != blurMethod::CustomBlur && cfgData.effectType > effectType::Acrylic)
				cfgData.effectType = effectType::Acrylic;
		}

		// newly added params since 2.1.0
		ret = Utils::GetIniString(path, L"config", L"crossfadeTime");
		if (!ret.empty())
		{
			cfgData.crossfadeTime = (UINT)_wtoll(ret.data());
		}
		return cfgData;
	}

	void ConfigData::SaveToFile(std::wstring_view path, const ConfigData& cfg)
	{
		Utils::SetIniString(path, L"config", L"applyglobal", cfg.applyglobal ? L"true" : L"false");
		Utils::SetIniString(path, L"config", L"extendBorder", cfg.extendBorder ? L"true" : L"false");
		Utils::SetIniString(path, L"config", L"reflection", cfg.reflection ? L"true" : L"false");
		Utils::SetIniString(path, L"config", L"oldBtnHeight", cfg.oldBtnHeight ? L"true" : L"false");
		Utils::SetIniString(path, L"config", L"customAmount", cfg.customAmount ? L"true" : L"false");

		// newly added params since 2.1.0
		Utils::SetIniString(path, L"config", L"overrideAccent", cfg.overrideAccent ? L"true" : L"false");

		Utils::SetIniString(path, L"config", L"blurAmount", std::to_wstring(cfg.blurAmount));
		Utils::SetIniString(path, L"config", L"customBlurAmount", std::to_wstring(cfg.customBlurAmount));
		Utils::SetIniString(path, L"config", L"luminosityOpacity", std::to_wstring(cfg.luminosityOpacity));

		// newly added params since 2.1.0
		Utils::SetIniString(path, L"config", L"glassIntensity", std::to_wstring(cfg.glassIntensity));
		Utils::SetIniString(path, L"config.aero", L"ColorizationColorBalance", std::to_wstring(cfg.ColorizationColorBalance));
		Utils::SetIniString(path, L"config.aero", L"ColorizationAfterglowBalance", std::to_wstring(cfg.ColorizationAfterglowBalance));
		Utils::SetIniString(path, L"config.aero", L"BlurBalance", std::to_wstring(cfg.ColorizationBlurBalance));

		Utils::SetIniString(path, L"config", L"activeTextColor", std::to_wstring(cfg.activeTextColor));
		Utils::SetIniString(path, L"config", L"inactiveTextColor", std::to_wstring(cfg.inactiveTextColor));
		Utils::SetIniString(path, L"config", L"activeTextColorDark", std::to_wstring(cfg.activeTextColorDark));
		Utils::SetIniString(path, L"config", L"inactiveTextColorDark", std::to_wstring(cfg.inactiveTextColorDark));
		Utils::SetIniString(path, L"config", L"activeBlendColor", std::to_wstring(cfg.activeBlendColor));
		Utils::SetIniString(path, L"config", L"inactiveBlendColor", std::to_wstring(cfg.inactiveBlendColor));
		Utils::SetIniString(path, L"config", L"activeBlendColorDark", std::to_wstring(cfg.activeBlendColorDark));
		Utils::SetIniString(path, L"config", L"inactiveBlendColorDark", std::to_wstring(cfg.inactiveBlendColorDark));
		Utils::SetIniString(path, L"config", L"blurMethod", std::to_wstring((int)cfg.blurmethod));
		Utils::SetIniString(path, L"config", L"effectType", std::to_wstring((int)cfg.effectType));

		// newly added params since 2.1.0
		Utils::SetIniString(path, L"config", L"crossfadeTime", std::to_wstring(cfg.crossfadeTime));
	}
}
