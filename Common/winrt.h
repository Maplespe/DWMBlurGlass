#pragma once
// 该头文件主要用于引入WinRT组件相关的帮助函数，可放入预编译头
#include <roapi.h>
#include <ocidl.h>
#include <xamlom.h>
#include <DispatcherQueue.h>

#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include <Windows.Foundation.Collections.h>
#include <Windows.Web.Syndication.h>
#include <windows.system.h>
#include <windows.system.threading.h>
#include <windows.system.diagnostics.h>

#include <windows.storage.h>
#include <windows.storage.pickers.h>
#include <windows.storage.streams.h>

#include <windows.graphics.h>
#include <windows.graphics.directx.h>
#include <windows.graphics.effects.h>

#include <Windows.Foundation.h>
#include <Windows.Foundation.Collections.h>

#include <Windows.Graphics.h>
#include <Windows.Graphics.Effects.h>
#include <windows.graphics.effects.interop.h>

#include <windows.ui.h>
#include <Windows.UI.Composition.h>
#include <windows.ui.composition.interop.h>
#include <Windows.UI.Composition.Desktop.h>
#include <Windows.UI.Composition.effects.h>
#include <Windows.UI.Composition.interactions.h>

#include <windows.ui.xaml.h>
#include <windows.ui.xaml.interop.h>
#include <windows.ui.xaml.media.h>
#include <windows.ui.xaml.media.animation.h>
#include <windows.ui.xaml.media.imaging.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include <windows.ui.xaml.shapes.h>
#include <windows.ui.xaml.controls.h>
#include <windows.ui.xaml.hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#pragma pop_macro("GetCurrentTime")

#include <winrt/base.h>

namespace MDWMBlurGlass
{
	namespace abi
	{
		using namespace ABI::Windows;
		namespace comp = ABI::Windows::UI::Composition;
		namespace desktop = ABI::Windows::UI::Composition::Desktop;
		namespace xaml = ABI::Windows::UI::Xaml;
		namespace media = ABI::Windows::UI::Xaml::Media;
		namespace controls = ABI::Windows::UI::Xaml::Controls;
		namespace dx = ABI::Windows::Graphics::DirectX;
		namespace fx = ABI::Windows::Graphics::Effects;
		namespace collections = ABI::Windows::Foundation::Collections;
		namespace streams = ABI::Windows::Storage::Streams;
	}

	namespace Utils
	{
		inline HSTRING abi_of(const winrt::hstring& str)
		{
			return reinterpret_cast<HSTRING>(winrt::get_abi(str));
		}
	}

	using winrt::hresult_error;
	using winrt::check_bool;
	using winrt::check_hresult;
	using winrt::check_bool;
	using winrt::check_pointer;
	using winrt::check_win32;
	using winrt::throw_hresult;
	using winrt::throw_last_error;
	using winrt::to_hresult;
	using winrt::to_hstring;
	using winrt::to_string;

	using winrt::get_module_lock;
	using winrt::implements;
	using winrt::hstring;
	using winrt::com_ptr;
	using winrt::get_activation_factory;
	using winrt::get_abi;
	using winrt::put_abi;
	using winrt::copy_from_abi;
	using winrt::copy_to_abi;
	using winrt::attach_abi;
	using winrt::detach_abi;

	using winrt::make;
	using winrt::make_self;
}