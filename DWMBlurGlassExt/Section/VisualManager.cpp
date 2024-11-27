#include <ranges>
#include "../Backdrops/BackdropFactory.hpp"
#include "CustomBackdropEffect.h"
#include "CommonDef.h"
#include <mutex>

using namespace MDWMBlurGlass;

namespace MDWMBlurGlassExt::CVisualManager
{
	std::unordered_map<DWM::CTopLevelWindow*, com_ptr<DCompPrivate::ICompositedBackdropVisual>> g_backdropMap{};
	void RedrawTopLevelWindow(DWM::CTopLevelWindow* window)
	{
		__try
		{
			auto kind{ CustomBackdrop::GetActualBackdropKind(window) };
			// 0x10000 UpdateText
			// 0x20000 UpdateIcon
			// 0x100000 UpdateColorization
			// ...

			if (os::buildNumber >= 22621)
			{
				if (kind == CompositedBackdropKind::SystemBackdrop)
				{
					window->OnSystemBackdropUpdated();
				}
			}
			else if (kind != CompositedBackdropKind::Accent)
			{
				window->SetDirtyFlags(0x10000);
			}
			if (kind == CompositedBackdropKind::Accent)
			{
				window->OnAccentPolicyUpdated();
			}
			if (os::buildNumber >= 18362)
			{
				window->OnClipUpdated();
			}
			else
			{
				window->OnBlurBehindUpdated();
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {}
	}


	size_t GetCount()
	{
		return g_backdropMap.size();
	}

	com_ptr<DCompPrivate::ICompositedBackdropVisual> GetOrCreate(DWM::CTopLevelWindow* window, bool createIfNecessary,
		bool silent)
	{
		auto it{ g_backdropMap.find(window) };

		if (createIfNecessary)
		{
			auto data{ window->GetData() };

			if (
				data &&
				it == g_backdropMap.end()
				)
			{
				HWND targetWindow{ data->GetHWND() };
				HWND shellWindow{ DWM::GetShellWindowForCurrentDesktop() };

				if (targetWindow != shellWindow)
				{
					auto result{ g_backdropMap.emplace(window, winrt::make<CCompositedBackdropVisual>(window)) };
					if (result.second == true)
					{
						it = result.first;
	
						if (!silent)
						{
							RedrawTopLevelWindow(window);
						}
					}
				}
			}
		}

		return it == g_backdropMap.end() ? nullptr : it->second;
	}

	com_ptr<DCompPrivate::ICompositedBackdropVisual> GetOrCreateForAccentBlurRect(DWM::CTopLevelWindow* window,
		LPCRECT accentBlurRect, bool createIfNecessary, bool silent)
	{
		auto it{ g_backdropMap.find(window) };

		auto result{ GetOrCreate(window, createIfNecessary, true) };
		if (result && it == g_backdropMap.end())
		{
			result->SetAccentRect(accentBlurRect);
			if (!silent)
			{
				RedrawTopLevelWindow(window);
			}
		}

		return result;
	}

	void TryClone(DWM::CTopLevelWindow* src, DWM::CTopLevelWindow* dst,
		DCompPrivate::ICompositedBackdropVisual** visual)
	{
		auto legacyVisual{ src->GetLegacyVisual() };
		if (auto backdrop{ GetOrCreate(src, false, false) }; backdrop && legacyVisual)
		{
			auto it{ g_backdropMap.find(dst) };
			if (it == g_backdropMap.end())
			{
				com_ptr<DCompPrivate::ICompositedBackdropVisual> clonedBackdrop{ nullptr };

				clonedBackdrop = legacyVisual->IsCloneAllowed() ? winrt::make<CClonedCompositedBackdropVisual>(dst, reinterpret_cast<CCompositedBackdropVisual*>(backdrop.get())) : winrt::make<CClonedPeekingBackdropVisual>(src, dst);

				auto result{ g_backdropMap.emplace(dst, clonedBackdrop) };
				if (result.second == true) { it = result.first; }
			}

			if (visual)
			{
				*visual = (it == g_backdropMap.end() ? nullptr : it->second.get());
			}
		}
	}

	void Remove(DWM::CTopLevelWindow* window, bool silent)
	{
		auto it{ g_backdropMap.find(window) };

		if (it != g_backdropMap.end())
		{
			g_backdropMap.erase(it);

			if (!silent)
			{
				RedrawTopLevelWindow(window);
			}
		}
	}

	void Trim(DWM::CTopLevelWindow* window)
	{
		auto it{ g_backdropMap.find(window) };

		if (it != g_backdropMap.end() && it->second->CanBeTrimmed())
		{
			g_backdropMap.erase(it);
		}
	}

	void Shutdown()
	{
		std::vector<DWM::CTopLevelWindow*> windowCollection{};
		for (const auto& window : g_backdropMap | std::views::keys)
		{
			windowCollection.push_back(window);
		}
		g_backdropMap.clear();

		for (auto& window : windowCollection)
		{
			RedrawTopLevelWindow(window);
		}

		BackdropFactory::Shutdown();
	}

	void RefreshEffectConfig()
	{
		CGlassReflectionVisual::UpdateReflectionSurface(Utils::GetCurrentDir() + L"\\data\\aeropeek.png");
		CGlassReflectionVisual::UpdateIntensity(CommonDef::g_configData.reflection ? CommonDef::g_configData.glassIntensity : 0.f);
		BackdropFactory::RefreshConfig();
	}
}
