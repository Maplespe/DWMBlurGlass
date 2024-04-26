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
#include "../framework.h"

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
		{ dwmcore, "CRenderData::DrawImageResource_FillMode" },
		{ dwmcore, "CFilterEffect::CalcInversedWorldInputBoundsFromVisibleWorldOutputBoundsRecursive" },

		{ udwm, "CDesktopManager::s_pDesktopManagerInstance" },
		{ udwm, "CDesktopManager::MonitorDpiFromPoint"},

		{ udwm, "CSolidColorLegacyMilBrushProxy::Update" },
		{ udwm, "CWindowList::GetExtendedFrameBounds" },
		{ udwm, "CWindowList::UpdateAccentBlurRect" },
		{ udwm, "CWindowList::GetSyncedWindowDataByHwnd" },
		{ udwm, "ResourceHelper::CreateGeometryFromHRGN" },
		{ udwm, "HrgnFromRects" },

		{ udwm, "CAccent::UpdateAccentPolicy" },
		{ udwm, "CAccent::UpdateAccentBlurRect" },
		{ udwm, "CAccent::s_IsPolicyActive" },

		{ udwm, "CTopLevelWindow::TreatAsActiveWindow" },
		{ udwm, "CTopLevelWindow::s_ChooseWindowFrameFromStyle" },
		{ udwm, "CTopLevelWindow::ValidateVisual" },
		{ udwm, "CTopLevelWindow::UpdateWindowVisuals" },
		{ udwm, "CTopLevelWindow::UpdateNCAreaButton" },
		{ udwm, "CTopLevelWindow::GetActualWindowRect" },
		{ udwm, "CTopLevelWindow::GetBorderMargins" },
		{ udwm, "CTopLevelWindow::UpdateText" },
		{ udwm, "CTopLevelWindow::InitializeVisualTreeClone" },
		{ udwm, "CTopLevelWindow::UpdateNCAreaBackground" },
		{ udwm, "CTopLevelWindow::OnAccentPolicyUpdated" },
		{ udwm, "CTopLevelWindow::OnClipUpdated" },
		{ udwm, "CTopLevelWindow::~CTopLevelWindow" },

		{ udwm, "CGlassColorizationParameters::AdjustWindowColorization" },

		{ udwm, "CText::SetColor" },
		{ udwm, "CText::SetText" },
		{ udwm, "CText::SetSize" },
		{ udwm, "CText::UpdateAlignmentTransform" },

		{ udwm, "CWindowList::BlurBehindChange" },
		{ udwm, "CWindowList::GetSyncedWindowData" },

		{ udwm, "CButton::UpdateLayout" },

		{ udwm, "VisualCollection::Remove" },
		{ udwm, "VisualCollection::RemoveAll" },
		{ udwm, "VisualCollection::InsertRelative" },

		{ udwm, "CDesktopManager::HandleInteropDeviceLost" },

		{ udwm, "CVisualProxy::SetClip" },
		{ udwm, "CVisualProxy::SetSize" },

		{ udwm, "CVisual::Create" },
		{ udwm, "CVisual::SetInsetFromParent" },
		{ udwm, "CVisual::SetInsetFromParentRight" },
		{ udwm, "CVisual::SetSize" },
		{ udwm, "CVisual::SetOffset" },
		{ udwm, "CVisual::InitializeVisualTreeClone" },
		{ udwm, "CVisual::Unhide" },
		{ udwm, "CVisual::Hide" },
		{ udwm, "CVisual::ConnectToParent" },
		{ udwm, "CVisual::SetOpacity" },
		{ udwm, "CVisual::UpdateOpacity" },
		{ udwm, "CVisual::SetScale" },
		{ udwm, "CVisual::SendSetOpacity" },
		{ udwm, "CVisual::RenderRecursive" },
		{ udwm, "CVisual::SetDirtyChildren" },
		{ udwm, "CVisual::CreateFromSharedHandle" },
		{ udwm, "CVisual::InitializeFromSharedHandle" },
		{ udwm, "CVisual::MoveToFront" },
		{ udwm, "CVisual::Initialize" },

		{ udwm, "CContainerVisual" },
		{ udwm, "CContainerVisual::Create" },

		{ udwm, "CRenderDataVisual::AddInstruction" },
		{ udwm, "CRenderDataVisual::ClearInstructions" },
		{ udwm, "CDrawGeometryInstruction::Create" },
		{ udwm, "CCanvasVisual::Create" },

		{ udwm, "CWindowData::IsImmersiveWindow" },

		{ udwm, "CMatrixTransformProxy::Update" }
	});
}