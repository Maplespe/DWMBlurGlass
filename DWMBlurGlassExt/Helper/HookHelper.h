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
#include "../framework.h"
#include "../minhook/MinHook.h"

namespace MDWMBlurGlassExt
{
	bool MHostLoadProcOffsetList();
	PVOID MHostGetProcAddress(LPCSTR pszProcName);

	template<typename T>
	T* MHostGetProcAddress(LPCSTR pszProcName)
	{
		return reinterpret_cast<T*>(MHostGetProcAddress(pszProcName));
	}

	bool WriteMemory(PVOID memoryAddress, std::function<void()> callback);
	size_t WriteIAT(PVOID baseAddress, std::string_view dllName, std::unordered_map<LPCSTR, PVOID> hookMap);

	__forceinline PVOID* GetObjectVTable(void* This)
	{
		return reinterpret_cast<PVOID*>(*reinterpret_cast<PVOID*>(This));
	}

	template<typename T>
	T class_method_cast(void* pfun)
	{
		static union
		{
			struct { void* __pfun; };
			T __method = nullptr;
		} cast{ .__pfun = pfun };
		return cast.__method;
	}

	template<typename T>
	T class_method_cast(LPCSTR name)
	{
		return class_method_cast<T>(MHostGetProcAddress(name));
	}

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

	enum class call_type
	{
		before,	// 在原函数调用之前调用此挂钩例程
		after,	// 在原函数调用完成后调用此挂钩例程
		final	// 无论怎么样都要进行调用，在代理函数返回之前进行调用
	};
	enum class handling_type
	{
		none,			// 默认处理方式
		return_now,		// 通知挂钩管理器不再向下调用其它挂钩例程或者包括原函数，并立即返回（对call_type::final无效）
		skip,			// 通知挂钩管理器不必继续调用原函数（仅适用于call_type::before）
	};
	template <typename T>
	struct detour_info
	{
		T* routine = nullptr;
		call_type ctype = call_type::before;
		handling_type htype = handling_type::none;
		size_t enabledCount = 0;

		detour_info() = default;
		detour_info(T* detour, call_type ct = call_type::before, handling_type ht = handling_type::none) : routine(detour), ctype(ct), htype(ht) {}
	};
	template <typename T> detour_info(T* detour, call_type ct, handling_type ht) -> detour_info<T>;
	template <typename ReturnType>
	struct HookDispatcherRVImpl
	{
		static inline thread_local void* s_return_value_ptr = nullptr;

		// 对于call_type::after或final的函数可以获取到函数的返回值并进行修改
		// call_type::before的挂钩例程调用该函数将导致未定义行为
		// 当ReturnType为void时，该方法将会被移除
		static inline auto& return_value()
		{
			return *reinterpret_cast<ReturnType*>(s_return_value_ptr);
		}
	};
	template <>
	struct HookDispatcherRVImpl<void>
	{
	};

	template <typename T, size_t total, typename ReturnType, typename... Args>
	class HookDispatcherImpl : public HookDispatcherRVImpl<ReturnType>
	{
	protected:
		using return_type = ReturnType;
		using function_type = ReturnType(Args...);
		using pointer = ReturnType(*)(Args...);

		enum { arity = sizeof...(Args) };
		template <size_t index>
		struct args
		{
			static_assert(index < arity, "索引超出范围，该值必须小于参数个数！");
			using type = typename std::tuple_element<index, std::tuple<Args...>>::type;
		};
		using tuple_type = std::tuple<std::remove_cv_t<std::remove_reference_t<Args>>...>;
		using bare_tuple_type = std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>;

		std::atomic_size_t m_ref_count = 0;
		std::array<detour_info<function_type>, total> m_detours = {};
		static inline HookDispatcherImpl<T, total, ReturnType, Args...>* s_this = nullptr;
		static inline thread_local handling_type s_htype = handling_type::none;

		__forceinline static T* get_derived_this()
		{
			return static_cast<T*>(s_this);
		}
		template <call_type ctype>
		__forceinline static bool __stdcall call_routine(Args&&... args)
		{
			for (const auto& info : s_this->m_detours)
			{
				if (!info.enabledCount)
					continue;
				if (info.ctype != ctype)
					continue;
				s_htype = info.htype;
				info.routine(std::forward<Args>(args)...);

				if constexpr (ctype != call_type::final)
				{
					if(s_htype == handling_type::return_now)
						return true;
				}
			}

			return false;
		}
		static return_type __stdcall proxy_function(Args... args)
		{
			s_htype = handling_type::none;

			auto cleanUp = MDWMBlurGlass::RAIIHelper::scope_exit([&]()
				{
					call_routine<call_type::final>(std::forward<Args>(args)...);
				});

			if constexpr (std::is_same_v<return_type, void>)
			{
				if (call_routine<call_type::before>(std::forward<Args>(args)...))
				{
					return;
				}

				if (s_htype != handling_type::skip)
				{
					get_derived_this()->call_org(std::forward<Args>(args)...);
				}

				if (call_routine<call_type::after>(std::forward<Args>(args)...))
				{
					return;
				}
			}
			else
			{
				return_type result{};
				HookDispatcherRVImpl<return_type>::s_return_value_ptr = &result;

				if (call_routine<call_type::before>(std::forward<Args>(args)...))
				{
					return result;
				}

				if (s_htype != handling_type::skip)
				{
					result = get_derived_this()->call_org(std::forward<Args>(args)...);
				}

				if (call_routine<call_type::after>(std::forward<Args>(args)...))
				{
					return result;
				}

				return result;
			}
		}
		HookDispatcherImpl(const HookDispatcherImpl&) = delete;
		HookDispatcherImpl(HookDispatcherImpl&&) = delete;
		HookDispatcherImpl() = delete;
		HookDispatcherImpl(const std::array<detour_info<function_type>, total>& detours) : m_detours(detours)
		{
			static_assert(total >= 1, "挂钩例程的个数需至少1个！");
			s_this = this;
		}
	public:
		// 告诉挂钩管理器启用/禁用索引为index的挂钩例程
		// 如果进行了修改则返回true
		template <size_t index, bool enable>
		bool enable_hook_routine()
		{
			static_assert(index < total, "索引超出范围，该值必须小于挂钩例程个数！");

			std::atomic_ref<size_t> enabledCount{ m_detours[index].enabledCount };
#ifdef _DEBUG
			if (enabledCount == 0 && !enable)
			{
				FatalAppExitW(0, L"enable_hook_routine 收到了一个错误调用！");
			}
#endif
			enabledCount += enable ? 1 : (enabledCount ? -1 : 0);
			OutputDebugStringW(
				std::format(L"hook procedure ref count changed: {}[index: {}, {}]\n", size_t(enabledCount), index, enable).c_str()
			);

			if ((enabledCount == 1 && enable) || (enabledCount == 0 && !enable))
			{
				m_ref_count += enable ? 1 : -1;
				OutputDebugStringW(
					std::format(L"global ref count changed: {}[index: {}, {}]\n", size_t(m_ref_count), index, enable).c_str()
				);
				return true;
			}
			return false;
		}
		// 禁用所有挂钩例程并取消API挂钩
		// 如果已经有相关的引用则返回true
		bool shutdown()
		{
			for (auto& info : m_detours)
			{
				info.enabledCount = 0;
			}
			if (!m_ref_count)
			{
				return false;
			}
			m_ref_count = 0;
			return true;
		}
		// 告诉挂钩管理器该次调用的处理方式被临时更改
		inline void modify_handle_type_for_temporary(handling_type htype)
		{
			s_htype = htype;
		}
	};

	template <size_t total, typename ReturnType, typename... Args>
	class MinHookDispatcherImpl : MinHook<ReturnType(Args...)>, public HookDispatcherImpl<MinHookDispatcherImpl<total, ReturnType, Args...>, total, ReturnType, Args...>
	{
	protected:
		using this_type = MinHookDispatcherImpl<total, ReturnType, Args...>;
		using base_type = HookDispatcherImpl<this_type, total, ReturnType, Args...>;
		using mini_hook = MinHook<ReturnType(Args...)>;
		MinHookDispatcherImpl(LPCSTR target, const std::array<detour_info<ReturnType(Args...)>, total>& detours) :
			mini_hook(target, reinterpret_cast<ReturnType(*)(Args...)>(base_type::proxy_function)),
			base_type(detours)
		{
		}
		~MinHookDispatcherImpl()
		{
			this_type::shutdown();
		}
	public:
		MinHookDispatcherImpl(const MinHookDispatcherImpl&) = delete;
		MinHookDispatcherImpl(MinHookDispatcherImpl&&) = delete;
		MinHookDispatcherImpl() = delete;

		using mini_hook::call_org;
		// 告诉挂钩管理器启用/禁用索引为index的挂钩例程
		// 如果API挂钩已经启用或禁用，将始终返回MH_OK
		// 否则返回初始化API挂钩的返回值
		template <size_t index, bool enable>
		MH_STATUS enable_hook_routine()
		{
			if ((base_type::s_this)->enable_hook_routine<index, enable>())
			{
				if constexpr (enable)
				{
					if (base_type::m_ref_count == 1)
					{
						return mini_hook::Attach();
					}
				}
				else
				{
					if (!base_type::m_ref_count)
					{
						return mini_hook::Detach();
					}
				}
			}
			return MH_OK;
		}
		MH_STATUS shutdown()
		{
			if (base_type::shutdown())
			{
				return mini_hook::Detach();
			}

			return MH_OK;
		}
		// 初始化的时候你并没有向target参数传入目标地址，导致钩子没有创建成功
		// 该函数允许你在启用任何一个挂钩例程之前调用以初始化进行补偿
		MH_STATUS reset_target(void* target)
		{
			mini_hook::reset_target(target);

			// 钩子已经被启用了
			if (base_type::m_ref_count)
			{
				return mini_hook::Attach();
			}
		}
	};

	template <size_t total, typename T>
	class MiniHookDispatcher;

	template <size_t total, typename ReturnType, typename... Args>
	class MiniHookDispatcher<total, ReturnType(Args...)> : public MinHookDispatcherImpl<total, ReturnType, Args...>
	{
	public:
		[[nodiscard]] inline explicit operator MH_STATUS() const noexcept
		{
			return this->m_status;
		}
		[[nodiscard]] inline explicit operator bool() const noexcept
		{
			return bool(this->m_status == MH_OK);
		}
		MiniHookDispatcher(const MiniHookDispatcher&) = delete;
		MiniHookDispatcher(MiniHookDispatcher&&) = delete;
		MiniHookDispatcher() = delete;
		MiniHookDispatcher(LPCSTR target, const std::array<detour_info<ReturnType(Args...)>, total>& detours) : MinHookDispatcherImpl<total, ReturnType, Args...>(target, detours) {}
		~MiniHookDispatcher() = default;
	};
	template <size_t total, typename T> MiniHookDispatcher(LPCSTR target, const std::array<detour_info<T>, total>& detours) -> MiniHookDispatcher<total, T>;
}