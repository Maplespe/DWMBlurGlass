/**
 * FileName: ColorDisplay.h
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

namespace Mui
{
	using namespace Ctrl;

	class ColorDisplay : public UIControl
	{
	public:
		M_DEF_CTRL(L"ColorDisplay")
		{
			{ CtrlMgr::AttribType::labelStyle, L"fontStyle" }
		}
		M_DEF_CTRL_END

		ColorDisplay(UIControl* parent);

		void SetAttribute(std::wstring_view attribName, std::wstring_view attrib, bool draw = true) override;

		void SetCurColor(_m_color color, bool draw = true);

	protected:
		void OnPaintProc(MPCPaintParam param) override;

		void OnLoadResource(MRenderCmd* render, bool recreate) override;

		bool OnMouseMessage(MEventCodeEnum message, _m_param wParam, _m_param lParam) override;

	private:
		_m_color curColor = 0;
		_m_color hoverColor = 0;
		_m_color pressColor = 0;
		_m_color normalColor = 0;
		MFontPtr font = nullptr;
		MBrushPtr brush = nullptr;
		MPenPtr pen = nullptr;
		bool mousedown = false;
		bool showalpha = false;

		UIControlStatus status = UIControlStatus_Normal;

		UILabel::Attribute fontStyle;
	};
}