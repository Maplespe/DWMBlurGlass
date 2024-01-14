/**
 * FileName: SymbolResolver.h
 *
 * Copyright (C) 2024 Maplespe、ALTaleX531
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
#include "winrt.h"
#include "../framework.h"
#include <ImageHlp.h>

namespace MDWMBlurGlass
{
	class SymbolResolver
	{
	public:
		SymbolResolver();
		~SymbolResolver() noexcept;

		// Pass "*!*" to mask to search all.
		HRESULT Walk(std::wstring_view dllName, std::string_view mask, std::function<bool(PSYMBOL_INFO, ULONG)> callback);

		HRESULT Walk(std::wstring_view dllName, std::string_view mask, std::wstring_view srv, std::function<bool(PSYMBOL_INFO, ULONG)> callback);

		// Return true if symbol successfully loaded.
		bool GetSymbolStatus();
		// Return true if symbol need to be downloaded.
		bool GetSymbolSource();
	private:
		static BOOL CALLBACK EnumSymbolsCallback(
			PSYMBOL_INFO pSymInfo,
			ULONG SymbolSize,
			PVOID UserContext
		);
		static BOOL CALLBACK SymCallback(
			HANDLE hProcess,
			ULONG ActionCode,
			ULONG64 CallbackData,
			ULONG64 UserContext
		);

		bool m_printInfo{ false };
		bool m_symbolsOK{ false };
		bool m_requireInternet{ false };
	};
}