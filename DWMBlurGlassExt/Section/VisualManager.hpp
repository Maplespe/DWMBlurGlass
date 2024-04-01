#pragma once
#include "DWMStruct.h"
#include "CompositedBackdrop.hpp"
#include "CommonDef.h"
#include <ranges>

namespace MDWMBlurGlassExt
{
	class CVisualManager
	{
	public:
		winrt::com_ptr<CCompositedBackdrop> GetOrCreateBackdrop(DWM::CTopLevelWindow* target, bool createIfNecessary = false)
		{
			auto it{ m_backdropMap.find(target) };

			if (createIfNecessary)
			{
				if (it == m_backdropMap.end())
				{
					auto result{ m_backdropMap.emplace(target, winrt::make_self<CCompositedBackdrop>(target)) };
					if (result.second == true) 
					{ 
						it = result.first; 
						it->second->UpdateEffectType(CommonDef::g_configData.effectType);
						it->second->UpdateGlassReflectionState(CommonDef::g_configData.reflection);
						it->second->UpdateBordersState(!CommonDef::g_configData.extendBorder);
					}
				}
			}

			return it == m_backdropMap.end() ? nullptr : it->second;
		}
		winrt::com_ptr<CCompositedAccentBackdrop> GetOrCreateAccentBackdrop(DWM::CTopLevelWindow* target, bool createIfNecessary = false)
		{
			auto it{ m_accentBackdropMap.find(target) };

			if (createIfNecessary)
			{
				if (it == m_accentBackdropMap.end())
				{
					auto result{ m_accentBackdropMap.emplace(target, winrt::make_self<CCompositedAccentBackdrop>(target)) };
					if (result.second == true) 
					{
						it = result.first;
						it->second->UpdateEffectType(CommonDef::g_configData.effectType);
						it->second->UpdateGlassReflectionState(CommonDef::g_configData.reflection);
					}
				}
			}

			return it == m_accentBackdropMap.end() ? nullptr : it->second;
		}
		winrt::com_ptr<CCompositedBackdrop> TryCloneBackdropForWindow(DWM::CTopLevelWindow* src, DWM::CTopLevelWindow* dest)
		{
			if (auto backdrop{ GetOrCreateBackdrop(src) }; backdrop)
			{
				auto it{ m_backdropMap.find(dest) };
				auto clonedBackdrop{ winrt::make_self<CCompositedBackdrop>(backdrop.get(), dest)};

				if (it == m_backdropMap.end())
				{
					auto result{ m_backdropMap.emplace(dest, clonedBackdrop) };
					if (result.second == true) { it = result.first; }
				}
				else
				{
					std::swap(
						clonedBackdrop,
						it->second
					);
				}

				return it == m_backdropMap.end() ? nullptr : it->second;
			}

			return nullptr;
		}
		winrt::com_ptr<CCompositedBackdrop> TryCloneAnimatedBackdropForWindow(DWM::CTopLevelWindow* src, DWM::CSecondaryWindowRepresentation* dest)
		{
			if (auto backdrop{ GetOrCreateBackdrop(src) }; backdrop)
			{
				auto it{ m_animatedBackdropMap.find(dest) };
				if (it != m_animatedBackdropMap.end())
				{
					m_animatedBackdropMap.erase(it);
				}
				auto clonedBackdrop{ winrt::make_self<CCompositedBackdrop>(backdrop.get()) };
				auto result{ m_animatedBackdropMap.emplace(dest, clonedBackdrop) };
				if (result.second == true) { it = result.first; }

				return it == m_animatedBackdropMap.end() ? nullptr : it->second;
			}

			return nullptr;
		}
		winrt::com_ptr<CCompositedAccentBackdrop> TryCloneAnimatedAccentBackdropForWindow(DWM::CTopLevelWindow* src, DWM::CSecondaryWindowRepresentation* dest)
		{
			if (auto backdrop{ GetOrCreateAccentBackdrop(src) }; backdrop)
			{
				auto it{ m_animatedAccentBackdropMap.find(dest) };
				if (it != m_animatedAccentBackdropMap.end())
				{
					m_animatedAccentBackdropMap.erase(it);
				}
				auto clonedBackdrop{ winrt::make_self<CCompositedAccentBackdrop>(backdrop.get()) };
				auto result{ m_animatedAccentBackdropMap.emplace(dest, clonedBackdrop) };
				if (result.second == true) { it = result.first; }

				return it == m_animatedAccentBackdropMap.end() ? nullptr : it->second;
			}

			return nullptr;
		}
		winrt::com_ptr<CCompositedAccentBackdrop> TryCloneAccentBackdropForWindow(DWM::CTopLevelWindow* src, DWM::CTopLevelWindow* dest)
		{
			if (auto backdrop{ GetOrCreateAccentBackdrop(src) }; backdrop)
			{
				auto it{ m_accentBackdropMap.find(dest) };
				auto clonedBackdrop{ winrt::make_self<CCompositedAccentBackdrop>(backdrop.get(), dest) };

				if (it == m_accentBackdropMap.end())
				{
					auto result{ m_accentBackdropMap.emplace(dest, clonedBackdrop) };
					if (result.second == true) { it = result.first; }
				}
				else
				{
					std::swap(
						clonedBackdrop,
						it->second
					);
				}

				return it == m_accentBackdropMap.end() ? nullptr : it->second;
			}

			return nullptr;
		}
		void Remove(DWM::CTopLevelWindow* target)
		{
			auto it{ m_backdropMap.find(target) };

			if (it != m_backdropMap.end())
			{
				m_backdropMap.erase(it);
			}
		}
		void RemoveAnimatedBackdrop(DWM::CSecondaryWindowRepresentation* target)
		{
			auto it{ m_animatedBackdropMap.find(target) };

			if (it != m_animatedBackdropMap.end())
			{
				m_animatedBackdropMap.erase(it);
			}
		}
		void RemoveAnimatedAccentBackdrop(DWM::CSecondaryWindowRepresentation* target)
		{
			auto it{ m_animatedAccentBackdropMap.find(target) };

			if (it != m_animatedAccentBackdropMap.end())
			{
				m_animatedAccentBackdropMap.erase(it);
			}
		}
		void RemoveAccent(DWM::CTopLevelWindow* target)
		{
			auto it{ m_accentBackdropMap.find(target) };

			if (it != m_accentBackdropMap.end())
			{
				m_accentBackdropMap.erase(it);
			}
		}
		void Shutdown()
		{
			m_backdropMap.clear();
			m_accentBackdropMap.clear();
			m_animatedBackdropMap.clear();
			m_animatedAccentBackdropMap.clear();
		}
		void RefreshEffectConfig()
		{
			CAeroBackdrop::s_sharedResources.interopDCompDevice = nullptr;
			CMicaBackdrop::s_sharedResources.interopDCompDevice = nullptr;
			CAcrylicBackdrop::s_sharedResources.interopDCompDevice = nullptr;
			CBlurBackdrop::s_sharedResources.interopDCompDevice = nullptr;
			CGlassReflectionBackdrop::s_sharedResources.interopDCompDevice = nullptr;

			for (auto& effect : m_backdropMap | std::views::values)
			{
				effect->Reset();
				effect->UpdateEffectType(CommonDef::g_configData.effectType);
				effect->UpdateGlassReflectionState(CommonDef::g_configData.reflection);
				effect->UpdateBordersState(!CommonDef::g_configData.extendBorder);
			}

			for (auto& effect : m_accentBackdropMap | std::views::values)
			{
				effect->Reset();
				effect->UpdateEffectType(CommonDef::g_configData.effectType);
				effect->UpdateGlassReflectionState(CommonDef::g_configData.reflection);
			}
		}
	private:
		std::unordered_map<DWM::CTopLevelWindow*, winrt::com_ptr<CCompositedBackdrop>> m_backdropMap{};
		std::unordered_map<DWM::CTopLevelWindow*, winrt::com_ptr<CCompositedAccentBackdrop>> m_accentBackdropMap{};
		std::unordered_map<DWM::CSecondaryWindowRepresentation*, winrt::com_ptr<CCompositedBackdrop>> m_animatedBackdropMap{};
		std::unordered_map<DWM::CSecondaryWindowRepresentation*, winrt::com_ptr<CCompositedAccentBackdrop>> m_animatedAccentBackdropMap{};
	};
}