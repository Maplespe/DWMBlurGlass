/**
 * FileName: HookHelper.cpp
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
#include "HookHelper.h"
#include "DefFunctionList.h"
#include <ImageHlp.h>
#include <winrt.h>
#include <Psapi.h>
#pragma comment(lib, "dbghelp.lib")

namespace MDWMBlurGlassExt
{
	std::unordered_map<std::string, void*> g_procAddress;
	bool g_offsetListInited = false;

	bool MHostLoadProcOffsetList()
	{
		static bool loaded = false;
		if (loaded)
			return true;

		HMODULE hModule = LoadLibraryW((Utils::GetCurrentDir() + L"\\DWMBlurGlass.exe").c_str());
		if (!hModule) 
			return false;

		auto clean = RAIIHelper::scope_exit([&] { FreeLibrary(hModule); });

		const auto GetModuleOffset = (DWORD64(__stdcall*)(size_t))GetProcAddress(hModule, "GetModuleOffset");
		if(!GetModuleOffset)
			return false;

		void* moduleBase_dwmcore = GetModuleHandleW(L"dwmcore.dll");
		void* moduleBase_uDwm = GetModuleHandleW(L"uDwm.dll");
		for (size_t i = 0; i < g_hookFunList.size(); ++i)
		{
			auto& [type, funName] = g_hookFunList[i];
			void* base;
			if (type == dwmcore)
				base = moduleBase_dwmcore;
			else if (type == udwm)
				base = moduleBase_uDwm;
			else
				continue;
			g_procAddress.insert({ funName, (void*)((DWORD64)base + GetModuleOffset(i)) });
		}
		loaded = true;
		return true;
	}

	PVOID MHostGetProcAddress(LPCSTR pszProcName)
	{
		const auto iter = g_procAddress.find(pszProcName);
		if (iter == g_procAddress.end())
			return nullptr;
		return iter->second;
	}

	bool WriteMemory(PVOID memoryAddress, std::function<void()> callback) try
	{
		if (!memoryAddress || !callback)
		{
			return false;
		}

		MEMORY_BASIC_INFORMATION mbi{};
		winrt::check_bool(
			VirtualQuery(
				memoryAddress,
				&mbi,
				sizeof(MEMORY_BASIC_INFORMATION)
			)
		);
		winrt::check_bool(
			VirtualProtect(
				mbi.BaseAddress,
				mbi.RegionSize,
				PAGE_READWRITE,
				&mbi.Protect
			)
		);
		callback();
		winrt::check_bool(
			VirtualProtect(
				mbi.BaseAddress,
				mbi.RegionSize,
				mbi.Protect,
				&mbi.Protect
			)
		);
		return true;
	}
	catch(...)
	{
		return false;
	}

	void WalkIAT(PVOID baseAddress, std::string_view dllName, std::function<bool(PVOID* functionAddress, LPCSTR functionNameOrOrdinal, BOOL importedByName)> callback) try
	{
		winrt::check_bool(!dllName.empty());
		winrt::check_bool(baseAddress);
		winrt::check_bool(callback);

		ULONG size{ 0ul };
		auto importDescriptor
		{
			static_cast<PIMAGE_IMPORT_DESCRIPTOR>(
				ImageDirectoryEntryToData(
					baseAddress,
					TRUE,
					IMAGE_DIRECTORY_ENTRY_IMPORT,
					&size
				)
			)
		};

		winrt::check_pointer(importDescriptor);

		LPCSTR moduleName{ nullptr };
		while (importDescriptor->Name != 0)
		{
			moduleName = reinterpret_cast<LPCSTR>(reinterpret_cast<UINT_PTR>(baseAddress) + importDescriptor->Name);

			if (!_stricmp(moduleName, dllName.data()))
			{
				break;
			}

			importDescriptor++;
		}

		winrt::check_pointer(moduleName);
		winrt::check_bool(importDescriptor->Name != 0);

		auto thunk{ reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<UINT_PTR>(baseAddress) + importDescriptor->FirstThunk) };
		auto nameThunk = reinterpret_cast<PIMAGE_THUNK_DATA>(reinterpret_cast<UINT_PTR>(baseAddress) + importDescriptor->OriginalFirstThunk);

		bool result{ true };
		while (thunk->u1.Function)
		{
			LPCSTR functionName{ nullptr };
			auto functionAddress = reinterpret_cast<PVOID*>(&thunk->u1.Function);

			BOOL importedByName{ !IMAGE_SNAP_BY_ORDINAL(nameThunk->u1.Ordinal) };
			if (importedByName)
			{
				functionName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(reinterpret_cast<UINT_PTR>(baseAddress) + static_cast<RVA>(nameThunk->u1.AddressOfData))->Name;
			}
			else
			{
				functionName = MAKEINTRESOURCEA(IMAGE_ORDINAL(thunk->u1.Ordinal));
			}

			result = callback(functionAddress, functionName, importedByName);
			if (!result)
			{
				break;
			}

			thunk++;
			nameThunk++;
		}
	}
	catch (...)
	{
	}

	size_t WriteIAT(PVOID baseAddress, std::string_view dllName, std::unordered_map<LPCSTR, PVOID> hookMap)
	{
		size_t result{ hookMap.size() };

		WalkIAT(baseAddress, dllName, [&](PVOID* functionAddress, LPCSTR functionNameOrOrdinal, BOOL importedByName) -> bool
		{
			auto tempHookMap{ hookMap };
			for (auto [targetFunctionNameOrOrdinal, hookFunctionAddress] : tempHookMap)
			{
				if (
					(importedByName == TRUE && !strcmp(functionNameOrOrdinal, targetFunctionNameOrOrdinal)) ||
					(importedByName == FALSE && functionNameOrOrdinal == targetFunctionNameOrOrdinal)
					)
				{
					WriteMemory(functionAddress, [&]()
						{
							InterlockedExchangePointer(functionAddress, hookFunctionAddress);
						});
					hookMap.erase(targetFunctionNameOrOrdinal);

					break;
				}
			}

			if (hookMap.empty())
			{
				return false;
			}

			return true;
		});

		result -= hookMap.size();

		return result;
	}

	PVOID MemSafeSearch(LPCSTR moduleName, PVOID dst, size_t size)
	{
		HMODULE hModule = GetModuleHandleA(moduleName);
		MODULEINFO moduleInfo;
		if (hModule && K32GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO)))
		{
			PVOID startAddress = hModule;
			PVOID endAddress = PVOID((ULONG64)moduleInfo.lpBaseOfDll + moduleInfo.SizeOfImage);

			while (memcmp(startAddress, dst, size) != 0)
			{
				startAddress = reinterpret_cast<PVOID>(reinterpret_cast<ULONG_PTR>(startAddress) + 1);
				if (startAddress >= endAddress)
					return nullptr;
			}
			return startAddress;
		}
		return nullptr;
	}

	MinHookImpl::MinHookImpl(LPCSTR pszProcName, void* detour) : m_detour(detour), m_targetName(pszProcName)
	{
		m_status = HookAutoInitializer::status;
	}

	MH_STATUS MinHookImpl::Attach()
	{
		if (HookAutoInitializer::status == MH_OK)
		{
			if(!g_offsetListInited)
			{
				MHostLoadProcOffsetList();
				g_offsetListInited = true;
			}
			if(!m_target)
			{
				const auto iter = g_procAddress.find(m_targetName);
				if (iter == g_procAddress.end())
				{
					m_status = MH_ERROR_FUNCTION_NOT_FOUND;
					return m_status;
				}

				m_target = iter->second;
			}
			m_status = MH_CreateHook(
				m_target,
				m_detour,
				&m_org
			);

			return MH_EnableHook(m_target);
		}
		return status;
	}


}