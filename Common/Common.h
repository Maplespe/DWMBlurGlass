/**
 * FileName: Common.h
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
#include <Windows.h>
#include <string>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;
namespace MDWMBlurGlass
{
	extern const HINSTANCE this_inst;

	enum class blurMethod
	{
		CustomBlur,
		AccentBlur,
		DWMAPIBlur
	};

	enum class effectType
	{
		None=-1,
		Blur,
		Aero,
		Acrylic,
		Mica
	};

	struct ConfigData
	{
		bool applyglobal = false;
		bool extendBorder = false;
		bool reflection = false;
		bool oldBtnHeight = false;
		bool customAmount = false;
		// newly added params since 2.1.0
		bool overrideAccent = false;
		
		int extendRound = 10;
		float blurAmount = 20.f;
		float customBlurAmount = 20.f;
		float luminosityOpacity = 0.65f;
		// newly added params since 2.1.0
		float glassIntensity = 1.f;

		// these settings are optimal for the default Sky color from Windows 7
		// newly added params since 2.1.0
		float ColorizationColorBalance = 8.0f;
		float ColorizationAfterglowBalance = 43.0f;
		float ColorizationBlurBalance = 49.0f;

		COLORREF activeTextColor = 0xFF000000;
		COLORREF inactiveTextColor = 0xFFB4B4B4;
		COLORREF activeBlendColor = 0x64FFFFFF;
		COLORREF inactiveBlendColor = 0x64FFFFFF;

		COLORREF activeTextColorDark = 0xFFFFFFFF;
		COLORREF inactiveTextColorDark = 0xFFB4B4B4;
		COLORREF activeBlendColorDark = 0x64000000;
		COLORREF inactiveBlendColorDark = 0x64000000;


		blurMethod blurmethod = blurMethod::CustomBlur;
		effectType effectType = effectType::Blur;
		// newly added params since 2.1.0
		UINT crossfadeTime = 87;

		bool isDefault()
		{
			static ConfigData _default;
			return memcmp(this, &_default, sizeof ConfigData) == 0;
		}

		static ConfigData LoadFromFile(std::wstring_view path);
		static void SaveToFile(std::wstring_view path, const ConfigData& cfg);
	};

	enum class MHostNotifyType
	{
		Refresh,
		Shutdown
	};

	enum MHostModuleType
	{
		dwmcore,
		udwm
	};

	auto constexpr DWMBlurGlassNotifyClassName = L"MDWMBlurGlassExtNotify";

	namespace RAIIHelper
	{
		template <typename TLambda>
		class lambda_call
		{
		public:
			lambda_call(const lambda_call&) = delete;
			lambda_call& operator=(const lambda_call&) = delete;
			lambda_call& operator=(lambda_call&& other) = delete;

			inline explicit lambda_call(TLambda&& lambda) noexcept : m_lambda(std::move(lambda))
			{
				static_assert(std::is_same<decltype(lambda()), void>::value, "scope_exit的lambda函数一定不能有返回值");
				static_assert(!std::is_lvalue_reference<TLambda>::value && !std::is_rvalue_reference<TLambda>::value, "scope_exit只能直接与lambda函数一起使用");
			}
			inline lambda_call(lambda_call&& other) noexcept : m_lambda(std::move(other.m_lambda)), m_call(other.m_call)
			{
				other.m_call = false;
			}
			inline ~lambda_call() noexcept
			{
				reset();
			}
			// 确保scope_exit lambda不会被调用
			inline void release() noexcept
			{
				m_call = false;
			}
			// 立刻执行scope_exit lambda如果还没有运行的话；确保它不再次运行
			inline void reset() noexcept
			{
				if (m_call)
				{
					m_call = false;
					m_lambda();
				}
			}
			// 返回true如果scope_exit lambda仍要被执行
			[[nodiscard]] inline explicit operator bool() const noexcept
			{
				return m_call;
			}
		protected:
			TLambda m_lambda;
			bool m_call = true;
		};
		/*
			返回一个对象，该对象在销毁时执行给定的lambda函数，请使用auto捕获返回的对象
			使用reset()提前执行lambda或使用release()避免执行
			在lambda中抛出的异常将引发快速失败
			你可以认为这个对象的作用跟finally差不多
		*/
		template <typename TLambda>
		[[nodiscard]] inline auto scope_exit(TLambda&& lambda) noexcept
		{
			return lambda_call<TLambda>(std::forward<TLambda>(lambda));
		}
	}

	namespace Utils
	{
		extern std::wstring GetCurrentDir();
		extern std::wstring GetIniString(std::wstring_view path, std::wstring_view appName, std::wstring_view keyName);
		extern bool SetIniString(std::wstring_view path, std::wstring_view appName, std::wstring_view keyName, std::wstring_view value);
		extern bool IsAppUseLightMode();
	}
}