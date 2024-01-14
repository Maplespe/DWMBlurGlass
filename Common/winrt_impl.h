// 该头文件主要用于引入WinRT组件相关的头文件
#pragma once
#pragma push_macro("GetCurrentTime")
#undef GetCurrentTime

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>
#include <winrt/windows.system.h>
#include <winrt/windows.system.threading.h>
#include <winrt/windows.system.diagnostics.h>

#include <winrt/windows.storage.h>
#include <winrt/windows.storage.pickers.h>
#include <winrt/windows.storage.streams.h>

#include <winrt/windows.graphics.h>
#include <winrt/windows.graphics.directx.h>
#include <winrt/windows.graphics.effects.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>

#include <winrt/Windows.Graphics.h>
#include <winrt/Windows.Graphics.Effects.h>

#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.UI.Composition.effects.h>
#include <winrt/Windows.UI.Composition.interactions.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.h>

#include <winrt/windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.interop.h>
#include <winrt/windows.ui.xaml.media.h>
#include <winrt/windows.ui.xaml.media.animation.h>
#include <winrt/windows.ui.xaml.media.imaging.h>
#include <winrt/windows.ui.xaml.shapes.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/windows.ui.xaml.hosting.h>

#pragma pop_macro("GetCurrentTime")
#pragma comment(lib, "windowsapp.lib")

namespace MDWMBlurGlass
{
	namespace comp = winrt::Windows::UI::Composition;
	namespace desktop = winrt::Windows::UI::Composition::Desktop;
	namespace xaml = winrt::Windows::UI::Xaml;
	namespace media = winrt::Windows::UI::Xaml::Media;
	namespace controls = winrt::Windows::UI::Xaml::Controls;
	namespace dx = winrt::Windows::Graphics::DirectX;
	namespace fx = winrt::Windows::Graphics::Effects;
	namespace collections = winrt::Windows::Foundation::Collections;
	namespace streams = winrt::Windows::Storage::Streams;
}