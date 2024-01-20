/**
 * FileName: DefFunctionList.h
 *
 * Copyright (C) 2024 Maplespe
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
#include "Common.h"
#include <array>

namespace MDWMBlurGlassExt
{
	using namespace MDWMBlurGlass;

	constexpr auto g_hookFunList = std::to_array<std::pair<MHostModuleType, LPCSTR>>
	({
		{ dwmcore, "CRenderingTechnique::ExecuteBlur" },
		{ dwmcore, "CD2DContext::FillEffect" },
		{ dwmcore, "CCustomBlur::BuildEffect" },
		{ dwmcore, "CCustomBlur::DetermineOutputScale" },
		{ dwmcore, "CDrawingContext::DrawVisualTree" },
		{ dwmcore, "CDrawingContext::FillEffect" },
		{ dwmcore, "CRenderData::DrawImageResource_FillMode" },
		{ dwmcore, "CFilterEffect::CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive" },

		{ udwm, "CDesktopManager::s_pDesktopManagerInstance" },
		{ udwm, "CSolidColorLegacyMilBrushProxy::Update" },
		{ udwm, "CWindowList::GetExtendedFrameBounds" },
		{ udwm, "ResourceHelper::CreateGeometryFromHRGN" },
		{ udwm, "HrgnFromRects" },
		{ udwm, "CAccentBlurBehind::UpdateAccentBlurBehind" },
		{ udwm, "CTopLevelWindow::UpdateNCAreaGeometry" },
		{ udwm, "CAccent::UpdateAccentPolicy" },
		{ udwm, "CAccent::UpdateAccentBlurRect" },
		{ udwm, "CTopLevelWindow::s_ChooseWindowFrameFromStyle" },
		{ udwm, "CTopLevelWindow::OnAccentPolicyUpdated" },
		{ udwm, "CTopLevelWindow::ValidateVisual" },
		{ udwm, "CTopLevelWindow::UpdateWindowVisuals" },
		{ udwm, "CGlassColorizationParameters::AdjustWindowColorization" },
		{ udwm, "CTopLevelWindow::UpdateText" },
		{ udwm, "CText::SetColor" },
		{ udwm, "CText::SetText" },
		{ udwm, "CText::SetSize" },
		{ udwm, "CWindowList::BlurBehindChange" },
		{ udwm, "CWindowList::GetSyncedWindowData" },
		{ udwm, "CButton::UpdateLayout" }
	});
}