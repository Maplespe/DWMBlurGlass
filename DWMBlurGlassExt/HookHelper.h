/**
 * FileName: HookHelper.h
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
#include "framework.h"
#include "minhook/MinHook.h"

namespace MDWMBlurGlassExt
{
	bool MHostLoadProcOffsetList();
	PVOID MHostGetProcAddress(LPCSTR pszProcName);

	bool WriteMemory(PVOID memoryAddress, std::function<void()> callback);
	size_t WriteIAT(PVOID baseAddress, std::string_view dllName, std::unordered_map<LPCSTR, PVOID> hookMap);

	struct HookAutoInitializer
	{
		MH_STATUS status = MH_OK;
		HookAutoInitializer()
		{
			static HookAutoInitializerImpl s_autoInitializer = {};
			status = s_autoInitializer.status;
		}
		~HookAutoInitializer() = default;

	private:
		struct HookAutoInitializerImpl
		{
			MH_STATUS status = MH_OK;
			HookAutoInitializerImpl()
			{
				/*if (!MHostLoadProcOffsetList())
				{
					status = MH_ERROR_MODULE_NOT_FOUND;
					return;
				}*/
				status = MH_Initialize();
			}
			~HookAutoInitializerImpl()
			{
				status = MH_Uninitialize();
			}
		};
	};

	class OrgCallForwarder
	{
	protected:
		void* m_org = nullptr;
	public:
		OrgCallForwarder() = default;
		~OrgCallForwarder() = default;

		template <typename T, typename... Args>
		__forceinline auto __stdcall call_org(Args&&... args)
		{
			return (reinterpret_cast<T*>(m_org))(std::forward<Args>(args)...);
		}
	};

	class MinHookImpl : public OrgCallForwarder, HookAutoInitializer
	{
	protected:
		MH_STATUS m_status = MH_ERROR_NOT_CREATED;
		void* m_target = nullptr;
		void* m_detour = nullptr;
		std::string m_targetName;
	public:
		[[nodiscard]] inline explicit operator MH_STATUS() const noexcept
		{
			return m_status;
		}
		[[nodiscard]] inline explicit operator bool() const noexcept
		{
			return bool(m_status == MH_OK);
		}
		MinHookImpl(LPCSTR pszProcName, void* detour);
		~MinHookImpl() { if(m_target) MH_RemoveHook(m_target); };

		MH_STATUS Attach();

		MH_STATUS Detach()
		{
			return MH_DisableHook(m_target);
		}
	};

	template <typename T>
	struct MinHook : public MinHookImpl
	{
		MinHook(LPCSTR pszProcName, T* detour) : MinHookImpl(pszProcName, reinterpret_cast<void*>(detour)) {}

		template <typename... Args>
		__forceinline auto __stdcall call_org(Args&&... args)
		{
			return OrgCallForwarder::call_org<T>(std::forward<Args>(args)...);
		}
	};
	template <typename T> MinHook(LPCSTR pszProcName, T* detour) -> MinHook<T>;
}