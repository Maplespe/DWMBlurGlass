/**
 * FileName: ColorDisplay.cpp
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
#include "ColorDisplay.h"
#include "../MainWindow.h"

namespace Mui
{
	using namespace Helper;
	using namespace Color;

	void ColorDisplay::Register()
	{
		static auto method = [](UIControl* parent)
		{
			return new ColorDisplay(parent);
		};
		M_REGISTER_CTRL(method);
	}

	ColorDisplay::ColorDisplay(UIControl* parent)
	{
		m_cacheSupport = true;
		parent->AddChildren(this);
	}

	void ColorDisplay::SetAttribute(std::wstring_view attribName, std::wstring_view attrib, bool draw)
	{
		if (attribName == L"curColor")
			curColor = M_GetAttribValueColor(attrib);
		else if (attribName == L"normalColor")
			normalColor = M_GetAttribValueColor(attrib);
		else if (attribName == L"hoverColor")
			hoverColor = M_GetAttribValueColor(attrib);
		else if (attribName == L"pressColor")
			pressColor = M_GetAttribValueColor(attrib);
		else if (attribName == L"fontStyle")
		{
			fontStyle = *(UILabel::Attribute*)M_StoULong64(attrib);
			font = m_render->CreateFonts(L"", fontStyle.font, fontStyle.fontSize, fontStyle.fontCustom);
		}
		else if (attribName == L"text")
			fontStyle.text = attrib;
		else if (attribName == L"showAlpha")
			showalpha = attrib == L"true";
		else
		{
			UIControl::SetAttribute(attribName, attrib, draw);
			return;
		}
		m_cacheUpdate = true;
		if (draw) UpdateDisplay();
	}

	void ColorDisplay::SetCurColor(_m_color color, bool draw)
	{
		curColor = color;
		m_cacheUpdate = true;
		if (draw) UpdateDisplay();
	}

	void ColorDisplay::OnPaintProc(MPCPaintParam param)
	{
		_m_color dstColor = 0;
		switch (status)
		{
		case UIControlStatus_Normal:
			dstColor = normalColor;
			break;
		case UIControlStatus_Hover:
			dstColor = hoverColor;
			break;
		case UIControlStatus_Pressed:
			dstColor = pressColor;
			break;
		case UIControlStatus_Disable:
			dstColor = normalColor;
			break;
		default:
			break;
		}
		pen->SetColor(dstColor);

		auto scale = GetRectScale().scale();

		_m_uint width = _scale_to(1u, scale.cx);
		pen->SetWidth(width);

		param->render->DrawRectangle(*param->destRect, pen);

		int size = _scale_to(20, scale.cx);
		int offset = _scale_to(5, scale.cx);
		UIRect colorRc(param->destRect->left + offset, param->destRect->top + offset, size, size);

		brush->SetColor(curColor);

		float round = _scale_to(3.f, scale.cx);
		param->render->FillRoundedRect(colorRc, round, brush);
		pen->SetColor(M_RGBA(83, 83, 83, 255));
		param->render->DrawRoundedRect(colorRc, round, pen);

		colorRc.left = colorRc.right + offset;
		colorRc.right = param->destRect->right;

		brush->SetColor(IsEnabled() ? M_Black : M_RGBA(130, 130, 130, 255));

		std::wstring colorstr = showalpha ? L"RGBA(" : L"RGB(";
		colorstr += std::to_wstring(M_GetRValue(curColor)) + L"," + std::to_wstring(M_GetGValue(curColor))
			+ L"," + std::to_wstring(M_GetBValue(curColor));

		if (showalpha)
			colorstr += L"," + std::to_wstring(M_GetAValue(curColor)) + L")";
		else
			colorstr += L")";

		font->SetText(colorstr);
		font->SetFontSize(_scale_to(12, scale.cx), std::make_pair(0u, (_m_uint)font->GetText().length()));
		param->render->DrawTextLayout(font, colorRc, brush, TextAlign_VCenter);

		colorRc.left = param->destRect->left;
		colorRc.top = colorRc.bottom + offset;

		if (auto title = font->GetText(); !title.empty())
		{
			font->SetText(fontStyle.text);
			brush->SetColor(fontStyle.fontColor);

			font->SetFontSize(_scale_to(14, scale.cx), std::make_pair(0u, (_m_uint)font->GetText().length()));
			param->render->DrawTextLayout(font, colorRc, brush, TextAlign_Center);
		}
	}

	void ColorDisplay::OnLoadResource(MRenderCmd* render, bool recreate)
	{
		UINodeBase::OnLoadResource(render, recreate);

		auto scale = GetRectScale().scale();
		const float fontSize = M_MIN(scale.cx, scale.cy) * (float)fontStyle.fontSize;

		font = render->CreateFonts(L"", fontStyle.font, (_m_uint)fontSize, fontStyle.fontCustom);
		brush = render->CreateBrush(fontStyle.fontColor);
		pen = render->CreatePen(1, normalColor);
	}

	bool ColorDisplay::OnMouseMessage(MEventCodeEnum message, _m_param wParam, _m_param lParam)
	{
		bool ret = true;
		switch (message)
		{
		case M_MOUSE_HOVER:
			status = UIControlStatus_Hover;
			break;
		case M_MOUSE_LEAVE:
			status = UIControlStatus_Normal;
			break;
		case M_MOUSE_LBDOWN:
			status = UIControlStatus_Pressed;
			mousedown = true;
			break;
		case M_MOUSE_LBUP:
			status = UIControlStatus_Normal;
			if (mousedown)
			{
				UIPoint point((int)M_LOWORD((_m_long)lParam), (int)M_HIWORD((_m_long)lParam));
				SendEvent(Event_Mouse_LClick, (_m_param)&point);
				MDWMBlurGlass::ShowColorPicker(curColor, showalpha, [this](bool isok, _m_color dst)
				{
					if (!isok) return;
					curColor = dst;
					m_cacheUpdate = true;
					UpdateDisplay();
					SendEvent(Event_ColorPicker_Change, dst);
				});
			}
			mousedown = false;
			break;
		default:
			ret = false;
			break;
		}

		if (ret)
		{
			m_cacheUpdate = true;
			UpdateDisplay();
		}

		UIControl::OnMouseMessage(message, wParam, lParam);
		return ret;
	}
}