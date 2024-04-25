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
#include <functional>
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

#define GetCfgValueInternal(keyName, fun) GetConfigValue(keyName, [&](std::wstring value) fun);
#pragma push_macro("GetCfgValueInternal")

	ConfigData ConfigData::LoadFromFile(std::wstring_view path)
	{
		auto GetConfigBool = [&path](std::wstring_view keyName, bool defaultValue = false)
		{
			if (const auto value = Utils::GetIniString(path, L"config", keyName); !value.empty())
				return value == L"true";
			return defaultValue;
		};

		auto GetConfigValue = [&path](std::wstring_view keyName,
			const std::function<void(std::wstring)>& setfun)
		{
			const auto value = Utils::GetIniString(path, L"config", keyName);
			if (value.empty() || !setfun)
				return;
			setfun(value);
		};

		ConfigData cfgData;
		cfgData.applyglobal = GetConfigBool(L"applyglobal");
		cfgData.extendBorder = GetConfigBool(L"extendBorder");
		cfgData.reflection = GetConfigBool(L"reflection");
		cfgData.oldBtnHeight = GetConfigBool(L"oldBtnHeight");
		cfgData.customAmount = GetConfigBool(L"customAmount");
		cfgData.useAccentColor = GetConfigBool(L"useAccentColor");
		cfgData.crossFade = GetConfigBool(L"crossFade", true);
		cfgData.overrideAccent = GetConfigBool(L"overrideAccent");

		GetCfgValueInternal(L"extendRound",
		{
			cfgData.extendRound = std::clamp(_wtoi(value.data()), 0, 16);
		});

		GetCfgValueInternal(L"blurAmount",
		{
			cfgData.blurAmount = (float)std::clamp(_wtof(value.data()), 0.0, 50.0);
		});

		GetCfgValueInternal(L"customBlurAmount",
		{
			cfgData.customBlurAmount = (float)std::clamp(_wtof(value.data()), 0.0, 50.0);
		});

		GetCfgValueInternal(L"luminosityOpacity",
		{
			cfgData.luminosityOpacity = (float)std::clamp(_wtof(value.data()), 0.0, 1.0);
		});

		GetCfgValueInternal(L"activeTextColor",
		{
			cfgData.activeTextColor = (COLORREF)_wtoll(value.data());
			cfgData.activeTextColor = (cfgData.activeTextColor & 0x00FFFFFF) | 0xFF000000;
		});

		GetCfgValueInternal(L"inactiveTextColor",
		{
			cfgData.inactiveTextColor = (COLORREF)_wtoll(value.data());
			cfgData.inactiveTextColor = (cfgData.inactiveTextColor & 0x00FFFFFF) | 0xFF000000;
		});

		GetCfgValueInternal(L"activeTextColorDark",
		{
			cfgData.activeTextColorDark = (COLORREF)_wtoll(value.data());
			cfgData.activeTextColorDark = (cfgData.activeTextColorDark & 0x00FFFFFF) | 0xFF000000;
		});
		
		GetCfgValueInternal(L"inactiveTextColorDark",
		{
			cfgData.inactiveTextColorDark = (COLORREF)_wtoll(value.data());
			cfgData.inactiveTextColorDark = (cfgData.inactiveTextColorDark & 0x00FFFFFF) | 0xFF000000;
		});

		GetCfgValueInternal(L"activeBlendColor",
		{
			cfgData.activeBlendColor = (COLORREF)_wtoll(value.data());
		});

		GetCfgValueInternal(L"inactiveBlendColor",
		{
			cfgData.inactiveBlendColor = (COLORREF)_wtoll(value.data());
		});

		GetCfgValueInternal(L"activeBlendColorDark",
		{
			cfgData.activeBlendColorDark = (COLORREF)_wtoll(value.data());
		});

		GetCfgValueInternal(L"inactiveBlendColorDark",
		{
			cfgData.inactiveBlendColorDark = (COLORREF)_wtoll(value.data());
		});

		GetCfgValueInternal(L"blurMethod",
		{
			cfgData.blurmethod = (blurMethod)std::clamp(_wtoi(value.data()), 0, 2);
		});

		GetCfgValueInternal(L"glassIntensity",
		{
			cfgData.glassIntensity = (float)std::clamp(_wtof(value.data()), 0.0, 1.0);
		});

		GetCfgValueInternal(L"activeColorBalance",
		{
			cfgData.activeColorBalance = (float)std::clamp(_wtof(value.data()), 0.0, 1.0);
		});

		GetCfgValueInternal(L"inactiveColorBalance",
		{
			cfgData.inactiveColorBalance = (float)std::clamp(_wtof(value.data()), 0.0, 1.0);
		});

		GetCfgValueInternal(L"activeBlurBalance",
		{
			cfgData.activeBlurBalance = (float)std::clamp(_wtof(value.data()), -1.0, 1.0);
		});

		GetCfgValueInternal(L"inactiveBlurBalance",
		{
			cfgData.inactiveBlurBalance = (float)std::clamp(_wtof(value.data()), -1.0, 1.0);
		});

		GetCfgValueInternal(L"effectType",
		{
			cfgData.effectType = (MDWMBlurGlass::effectType)std::clamp(_wtoi(value.data()), -1, 3);
			if (cfgData.blurmethod != blurMethod::CustomBlur && cfgData.effectType > effectType::Acrylic)
				cfgData.effectType = effectType::Acrylic;
		});

		GetCfgValueInternal(L"crossfadeTime",
		{
			cfgData.crossfadeTime = (UINT)std::clamp(_wtoi(value.data()), 0, 2000);
		});
		return cfgData;
	}

#pragma pop_macro("GetCfgValueInternal")

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
			 { L"glassIntensity", make_wstring(cfg.glassIntensity) },
			 { L"activeColorBalance", make_wstring(cfg.activeColorBalance) },
			 { L"inactiveColorBalance", make_wstring(cfg.inactiveColorBalance) },
			 { L"activeBlurBalance", make_wstring(cfg.activeBlurBalance) },
			 { L"inactiveBlurBalance", make_wstring(cfg.inactiveBlurBalance) },
			 { L"blurMethod", make_wstring((int)cfg.blurmethod) },
			 { L"effectType", make_wstring((int)cfg.effectType) },
			 { L"crossfadeTime", make_wstring(cfg.crossfadeTime) }
		}); const auto& [key, value] : regkeyList)
		{
			Utils::SetIniString(path, L"config", key, value);
		}
	}
}
