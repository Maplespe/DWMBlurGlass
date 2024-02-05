/**
 * FileName: BackdropMaterials.h
 *
 * Copyright (C) 2024 Maplespe、ALTaleX531
 *
 * This file is part of MToolBox and DWMBlurGlass and AcrylicEverywhere.
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
#include "DWMStruct.h"
#include "DcompPrivate.h"
#include "winrt_impl.h"
#include "wil.h"

namespace MDWMBlurGlassExt
{
	namespace comp = MDWMBlurGlass::comp;

	struct CBackdropResources
	{
		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		comp::CompositionBrush lightMode_Active_Brush{ nullptr };
		comp::CompositionBrush darkMode_Active_Brush{ nullptr };
		comp::CompositionBrush lightMode_Inactive_Brush{ nullptr };
		comp::CompositionBrush darkMode_Inactive_Brush{ nullptr };

		static comp::CompositionBrush CreateCrossFadeEffectBrush(
			const comp::Compositor& compositor,
			const comp::CompositionBrush& from,
			const comp::CompositionBrush& to
		);
		static comp::ScalarKeyFrameAnimation CreateCrossFadeAnimation(
			const comp::Compositor& compositor,
			winrt::Windows::Foundation::TimeSpan const& crossfadeTime
		);
		inline comp::CompositionBrush ChooseIdealBrush(bool darkMode, bool active);

		void ClearBrushResource();
	};

	struct CAcrylicResources : CBackdropResources
	{
		comp::CompositionBrush noiceBrush{ nullptr };

		comp::CompositionBrush CreateBrush(
			const comp::Compositor& compositor,
			const winrt::Windows::UI::Color& tintColor,
			const winrt::Windows::UI::Color& luminosityColor,
			float tintOpacity,
			float luminosityOpacity,
			float blurAmount
		);
		HRESULT EnsureNoiceSurfaceBrush();
		HRESULT EnsureAcrylicBrush();
	};
	struct CMicaResources : CBackdropResources
	{
		comp::CompositionBrush CreateBrush(
			const comp::Compositor& compositor,
			const winrt::Windows::UI::Color& tintColor,
			const winrt::Windows::UI::Color& luminosityColor,
			float tintOpacity,
			float luminosityOpacity
		);
		HRESULT EnsureMicaBrush();
	};
	struct CAeroResources : CBackdropResources
	{
		comp::CompositionBrush CreateBrush(
			const comp::Compositor& compositor,
			const winrt::Windows::UI::Color& tintColor,
			float tintOpacity,
			float exposureAmount,
			float blurAmount
		);
		bool onlyBlur = false;
		HRESULT EnsureAeroBrush();
	};
	struct CGlassReflectionResources
	{
		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		comp::CompositionDrawingSurface drawingSurface{ nullptr };
		WCHAR filePath[MAX_PATH + 1]{};

		HRESULT EnsureGlassSurface();
	};

	struct CBackdropEffect
	{
		bool useDarkMode{ false };
		bool windowActivated{ false };

		winrt::com_ptr<DCompPrivate::IDCompositionDesktopDevicePartner> interopDCompDevice{ nullptr };
		winrt::com_ptr<DWM::CVisual> udwmVisual{ nullptr };
		winrt::com_ptr<DCompPrivate::InteropCompositionTarget> interopCompositionTarget{ nullptr };
		comp::CompositionBrush currentBrush{ nullptr };
		comp::SpriteVisual spriteVisual{ nullptr };
		winrt::Windows::Foundation::Numerics::float2 currentSize{ 0.f, 0.f };
		std::chrono::milliseconds crossFadeTime{ 187 };

		STDMETHOD(InitializeDCompAndVisual)();
		STDMETHOD(InitializeVisualTreeClone)(CBackdropEffect* backdrop);
		STDMETHOD(UpdateBackdrop)(DWM::CTopLevelWindow* topLevelWindow);
		STDMETHOD(EnsureBackdropResources)() PURE;
	};

	struct CAcrylicBackdrop : CBackdropEffect
	{
		static CAcrylicResources s_sharedResources;

		STDMETHOD(UpdateBackdrop)(DWM::CTopLevelWindow* topLevelWindow) override;
		STDMETHOD(EnsureBackdropResources)() override;
	};
	struct CMicaBackdrop : CBackdropEffect
	{
		static CMicaResources s_sharedResources;

		STDMETHOD(UpdateBackdrop)(DWM::CTopLevelWindow* topLevelWindow) override;
		STDMETHOD(EnsureBackdropResources)() override;
	};
	struct CAeroBackdrop : CBackdropEffect
	{
		static CAeroResources s_sharedResources;

		CAeroBackdrop() { crossFadeTime = std::chrono::milliseconds{87}; }
		STDMETHOD(UpdateBackdrop)(DWM::CTopLevelWindow* topLevelWindow) override;
		STDMETHOD(EnsureBackdropResources)() override;
	};
	struct CGlassReflectionBackdrop : CBackdropEffect
	{
		static CGlassReflectionResources s_sharedResources;
		winrt::Windows::Foundation::Numerics::float2 relativeOffset{};
		winrt::Windows::Foundation::Numerics::float2 fixedOffset{};
		RECT currentWindowRect{};
		RECT currentMonitorRect{};
		HMONITOR currentMonitor{ nullptr };
		comp::CompositionSurfaceBrush glassSurfaceBrush{ nullptr };

		STDMETHOD(UpdateBackdrop)(DWM::CTopLevelWindow* topLevelWindow) override;
		STDMETHOD(EnsureBackdropResources)() override;
	};

	class CCompositedBackdrop
	{
		MDWMBlurGlass::effectType m_type{ MDWMBlurGlass::effectType::Blur };
		wil::unique_hrgn m_clipRgn{ nullptr };
		winrt::com_ptr<DWM::CVisual> m_visual{ nullptr };
		std::unique_ptr<CBackdropEffect> m_backdropEffect{ nullptr };
		std::unique_ptr<CGlassReflectionBackdrop> m_glassReflection{ nullptr };
		
		void ConnectPrimaryBackdropToParent();
		void ConnectGlassReflectionToParent();
		void ConnectBorderFillToParent();
	public:
		CCompositedBackdrop(MDWMBlurGlass::effectType type, bool glassReflection);
		CCompositedBackdrop(CCompositedBackdrop&& backdrop) = delete;
		CCompositedBackdrop(const CCompositedBackdrop& backdrop) = delete;
		~CCompositedBackdrop();
		void InitializeVisualTreeClone(CCompositedBackdrop* backdrop);

		void UpdateGlassReflection(bool enable);
		void UpdateBackdropType(MDWMBlurGlass::effectType type);
		void Update(DWM::CTopLevelWindow* window, HRGN hrgn);
		DWM::CVisual* GetVisual() const { return m_visual.get(); }
		CGlassReflectionBackdrop* GetGlassReflection() const { return m_glassReflection.get(); };
		HRGN GetClipRegion() const { return m_clipRgn.get(); };
	};

	class CBackdropManager
	{
	public:
		std::shared_ptr<CCompositedBackdrop> GetOrCreateBackdrop(DWM::CTopLevelWindow* topLevelWindow,  bool createIfNecessary = false);
		std::shared_ptr<CCompositedBackdrop> CreateWithGivenBackdrop(DWM::CTopLevelWindow* topLevelWindow, std::shared_ptr<CCompositedBackdrop> backdrop);
		std::shared_ptr<CCompositedBackdrop> Remove(DWM::CTopLevelWindow* topLevelWindow);
		void RefreshEffectConfig();
		void Shutdown();
		bool MatchVisualCollection(DWM::VisualCollection* visualCollection);
	private:
		std::unordered_map<DWM::CTopLevelWindow*, std::shared_ptr<CCompositedBackdrop>> m_backdropMap;
	};
}