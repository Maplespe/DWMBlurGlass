#pragma once
#include "DWMStruct.h"
#include "CompositedBackdrop.hpp"

namespace MDWMBlurGlassExt::CVisualManager
{
	void RedrawTopLevelWindow(DWM::CTopLevelWindow* window);

	size_t GetCount();

	com_ptr<DCompPrivate::ICompositedBackdropVisual> GetOrCreate(DWM::CTopLevelWindow* window, bool createIfNecessary = false, bool silent = false);

	com_ptr<DCompPrivate::ICompositedBackdropVisual> GetOrCreateForAccentBlurRect(DWM::CTopLevelWindow* window,
		LPCRECT accentBlurRect, bool createIfNecessary = false, bool silent = false);

	void TryClone(DWM::CTopLevelWindow* src, DWM::CTopLevelWindow* dst, DCompPrivate::ICompositedBackdropVisual** visual = nullptr);

	void Remove(DWM::CTopLevelWindow* window, bool silent = false);

	void Trim(DWM::CTopLevelWindow* window);

	void Shutdown();

	void RefreshEffectConfig();

	/*void RefreshEffectConfig()
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
	}*/
}