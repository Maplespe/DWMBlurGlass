/**
 * FileName: ColorPickerDlg.cpp
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
#include "ColorPickerDlg.h"

namespace MDWMBlurGlass
{
	ColorPickerDlg::ColorPickerDlg(UIControl* parent, XML::MuiXML* ui) : Page(parent, ui)
	{
		std::wstring_view geometry = MXMLCODE(
		<part>
			<fill_rect rc="0,0,0,0" color="220,220,220,255" />
		</part>
		<part>
			<fill_rect rc="0,0,0,0" color="220,220,220,255" />
		</part>
		<part>
			<fill_rect rc="0,0,0,0" color="220,220,220,255" />
		</part>
		<part>
			<fill_rect rc="0,0,0,0" color="220,220,220,255" />
		</part>
		);
		ui->Mgr()->AddGeometryStyle(L"colorpickerbtn", geometry);

		std::wstring_view xml = MXMLCODE(
		<PropGroup id="colorpicker_edit" limitText="3" number="true" inset="3,1,1,1" wordAutoSel="true" />

		<UIControl name="colorpicker" bgColor="80,80,80,60" size="100%,100%" autoSize="false" align="Center" visible="false">
			<UIControl bgColor="255,255,255,255" frameWidth="1" frameColor="222,222,222,255" frameRound="10" name="content"
			autoSize="false" size="453,342" align="Absolute" shadowColor="0,0,0,60" shadowOffset="0,30" shadowExtend="5" shadowRadius="35">
				<UILabel text="#colorpicker_dlg_sel" pos="15,18" fontSize="14" />
				<UIColorPicker autoSize="false" frame="15,45,250,250" frameWidth="1" frameColor="150,150,150,255" name="colorpicker_color" />
				<UISlider style="style_pickerbar" btnStyle="colorpickerbtn" frame="278,45,22,250" maxValue="359" value="359" vertical="true" btnSize="0,2" 
				trackInset="0,0,0,0" bottomShow="true" name="colorpicker_hbar" />
				<UIControl frame="314,45,100%,165" align="LinearH">
					<UIControl size="50%,100%" align="LinearV">
						<UILabel text="Red:" pos="0,4" fontSize="14" />
						<UILabel text="Green:" pos="0,5" fontSize="14" />
						<UILabel text="Blue:" pos="0,5" fontSize="14" />

						<UILabel text="Hue:" pos="0,26" fontSize="14" />
						<UILabel text="Saturate:" pos="0,5" fontSize="14" />
						<UILabel text="Value:" pos="0,5" fontSize="14" />
					</UIControl>
					<UIControl size="100%,100%" align="LinearV">
						<UIEditBox frame="0,1,44,20" prop="colorpicker_edit" name="colorpicker_r" text="0" />
						<UIEditBox frame="0,3,44,20" prop="colorpicker_edit" name="colorpicker_g" text="0" />
						<UIEditBox frame="0,3,44,20" prop="colorpicker_edit" name="colorpicker_b" text="0" />

						<UIEditBox frame="0,20,44,20" prop="colorpicker_edit" name="colorpicker_h" text="359" />
						<UIEditBox frame="0,3,44,20" prop="colorpicker_edit" name="colorpicker_s" text="0" />
						<UIEditBox frame="0,3,44,20" prop="colorpicker_edit" name="colorpicker_v" text="0" />
					</UIControl>
				</UIControl>
				<UIControl frame="430,134,20,65" align="LinearV">
					<UILabel pos="0,0" text="°" />
					<UILabel pos="0,5" text="%" />
					<UILabel pos="0,6" text="%" />
				</UIControl>
				<UIControl frame="314,224,100%,75" align="LinearV">
					<UILabel pos="0,0" text="#colorpicker_dlg_c" />
					<UIControl align="LinearH" autoSize="true">
						<UIControl frameWidth="1" frameColor="150,150,150,255" name="colorpicker_new" frame="0,5,50,50" />
						<UIControl frameWidth="1" frameColor="150,150,150,255" name="colorpicker_cur" frame="10,5,50,50" />
					</UIControl>
				</UIControl>
				<UIControl frame="200,300,100%,100%" align="LinearHLB">
					<UIButton pos="10,10" text="#msgdlg_no" name="color_dialog_no" animate="false" autoSize="true" minSize="72,27" maxSize="100,26" inset="5,2,5,2" />
					<UIButton pos="10,10" text="#msgdlg_yes" name="color_dialog_yes" animate="false" autoSize="true" minSize="72,27" maxSize="100,26" inset="5,2,5,2" />
				</UIControl>
				<UILabel fontSize="12" text="HEX: #" pos="15,305" />
				<UIEditBox frame="55,302,75,20" limitText="8" inset="3,1,1,1" wordAutoSel="true" text="0" name="colorpicker_hex" />
				<UIControl frame="150,305,100%,20" align="LinearH" name="colorpicker_ablock">
					<UILabel text="#colorpicker_dlg_alpha" />
					<UISlider frame="5,0,165,18" value="255" maxValue="255" name="colorpicker_alpha" />
					<UILabel autoSize="false" frame="5,0,100,20" text="100%" name="colorpicker_aper" />
				</UIControl>
			</UIControl>
		</UIControl>
		);

		if (!ui->CreateUIFromXML(parent, xml.data()))
		{
			throw std::invalid_argument("Invalid XML code!");
		}

		m_page = parent->Child(L"colorpicker");
		m_colorpicker = m_page->Child<UIColorPicker>(L"colorpicker_color");
		m_alphaper = m_page->Child<UILabel>(L"colorpicker_aper");
		m_alphav = m_page->Child<UISlider>(L"colorpicker_alpha");
		m_page->Child(L"colorpicker_hbar")->SetCacheType(UICacheType::Disable);
	}

	//更新颜色和文本显示
	void ColorPickerDlg::updateDisplay(_m_color color)
	{
		UIBkgndStyle bgColor;
		bgColor.FrameColor = Color::M_RGBA(83, 83, 83, 255);
		bgColor.FrameWidth = 1;
		bgColor.bkgndColor = color;
		_m_byte alpha = (_m_byte)m_alphav->GetCurValue();
		if (m_alpha)
		{
			using namespace Color;
			bgColor.bkgndColor = M_RGBA(M_GetRValue(color), M_GetGValue(color), M_GetBValue(color), alpha);
		}
		m_page->Child<UIControl>(L"colorpicker_new")->SetBackground(bgColor);
		auto hex = _m_color(bgColor.bkgndColor).HEX(true, m_alpha);
		std::ranges::transform(hex, hex.begin(), ::toupper);
		m_page->Child<UIEditBox>(L"colorpicker_hex")->SetCurText(hex);
	}

	//更新RGB值显示
	void ColorPickerDlg::updateRGB(_m_color color)
	{
		m_page->Child<UIEditBox>(L"colorpicker_r")->SetCurText(std::to_wstring(Color::M_GetRValue(color)));
		m_page->Child<UIEditBox>(L"colorpicker_g")->SetCurText(std::to_wstring(Color::M_GetGValue(color)));
		m_page->Child<UIEditBox>(L"colorpicker_b")->SetCurText(std::to_wstring(Color::M_GetBValue(color)));
	}

	//更新HSV值显示
	void ColorPickerDlg::updateHSV(UIColorPicker::HSV&& hsv)
	{
		m_page->Child<UIEditBox>(L"colorpicker_h")->SetCurText(std::to_wstring(hsv.hue));
		m_page->Child<UIEditBox>(L"colorpicker_s")->SetCurText(std::to_wstring(hsv.sat));
		m_page->Child<UIEditBox>(L"colorpicker_v")->SetCurText(std::to_wstring(hsv.val));
		m_page->Child<UISlider>(L"colorpicker_hbar")->SetCurValue(hsv.hue);
	}

	void ColorPickerDlg::updateAlpha(int value)
	{
		value = int(round((float)value / 255.f * 100.f));
		m_alphaper->SetAttribute(L"text", std::to_wstring(value) + L"%", false);
		updateDisplay(m_colorpicker->GetRGBAColor());
	}

	//RGB颜色已更改
	void ColorPickerDlg::changeRGB()
	{
		auto R = _wtoi(m_page->Child<UIEditBox>(L"colorpicker_r")->GetCurText().c_str());
		auto G = _wtoi(m_page->Child<UIEditBox>(L"colorpicker_g")->GetCurText().c_str());
		auto B = _wtoi(m_page->Child<UIEditBox>(L"colorpicker_b")->GetCurText().c_str());
		m_colorpicker->SetRGBAColor(Color::M_RGBA((_m_byte)R, (_m_byte)G, (_m_byte)B, 255));
	}

	bool ColorPickerDlg::EventProc(UINotifyEvent event, UIControl* control, _m_param param)
	{
		bool ret = true;

		switch (event)
		{
		case Event_ColorPicker_Change:
		{
			if (_MNAME(L"colorpicker_color"))
			{
				auto ctrl = (UIColorPicker*)control;
				auto color = (_m_color)param;

				updateRGB(color);
				updateHSV(ctrl->GetHSVColor());
				updateDisplay(color);
			}
			else
				ret = false;
		}
		break;
		case Event_Slider_Change:
		{
			if (_MNAME(L"colorpicker_hbar"))
			{
				auto hsv = m_colorpicker->GetHSVColor();
				hsv.hue = (_m_ushort)static_cast<UISlider*>(control)->GetCurValue();
				m_colorpicker->SetHSVColor(hsv);
				updateRGB(m_colorpicker->GetRGBAColor());
				updateDisplay(m_colorpicker->GetRGBAColor());
				m_page->Child<UIEditBox>(L"colorpicker_h")->SetCurText(std::to_wstring(hsv.hue));
			}
			else if (_MNAME(L"colorpicker_alpha"))
			{
				updateAlpha((int)param);
			}
			else
				ret = false;
		}
		break;
		case Event_Focus_False:
		{
			//防止文本越界
			auto textFormat = [](UIEditBox* edit, int max)
			{
				auto text = edit->GetCurText();
				if (text.empty()) edit->SetCurText(L"0");
				int value = _wtoi(text.c_str());

				if (value > max)
					edit->SetCurText(std::to_wstring(max));
				else if (value < 0)
					edit->SetCurText(L"0");
			};
			auto hexFormat = [](std::wstring_view input)
			{
				std::wstring result;
				for (const wchar_t ch : input) 
				{
					if ((ch >= L'0' && ch <= L'9') || (ch >= L'A' && ch <= L'F')) 
						result += ch;
					else
						result += L'F';
				}
				return result;
			};
			if (_MNAME(L"colorpicker_r")
				|| _MNAME(L"colorpicker_g")
				|| _MNAME(L"colorpicker_b"))
			{
				auto edit = (UIEditBox*)control;
				textFormat(edit, 255);
				changeRGB();
				updateHSV(m_colorpicker->GetHSVColor());
				updateDisplay(m_colorpicker->GetRGBAColor());
			}
			else if (_MNAME(L"colorpicker_h"))
			{
				auto edit = (UIEditBox*)control;
				textFormat(edit, 359);
				auto hsv = m_colorpicker->GetHSVColor();
				hsv.hue = (_m_ushort)_wtoi(edit->GetCurText().c_str());
				m_colorpicker->SetHSVColor(hsv);
				m_page->Child<UISlider>(L"colorpicker_hbar")->SetCurValue(hsv.hue);
				m_page->Child<UIEditBox>(L"colorpicker_h")->SetCurText(std::to_wstring(hsv.hue));
				updateRGB(m_colorpicker->GetRGBAColor());
			}
			else if (_MNAME(L"colorpicker_s"))
			{
				auto edit = (UIEditBox*)control;
				textFormat(edit, 100);
				auto hsv = m_colorpicker->GetHSVColor();
				hsv.sat = (_m_ushort)_wtoi(edit->GetCurText().c_str());
				m_colorpicker->SetHSVColor(hsv, false);

				auto color = m_colorpicker->GetRGBAColor();
				updateRGB(color);
				updateDisplay(color);
			}
			else if (_MNAME(L"colorpicker_v"))
			{
				auto edit = (UIEditBox*)control;
				textFormat(edit, 100);
				auto hsv = m_colorpicker->GetHSVColor();
				hsv.val = (_m_ushort)_wtoi(edit->GetCurText().c_str());
				m_colorpicker->SetHSVColor(hsv, false);

				auto color = m_colorpicker->GetRGBAColor();
				updateRGB(color);
				updateDisplay(color);
			}
			else if(_MNAME(L"colorpicker_hex"))
			{
				auto edit = (UIEditBox*)control;
				auto src = edit->GetCurText();
				std::ranges::transform(src, src.begin(), ::toupper);
				auto format = hexFormat(src);

				edit->SetCurText(format);
				_m_color color(format, true, m_alpha);

				m_alphav->SetCurValue(Color::M_GetAValue(color));
				int avalue = int(round((float)m_alphav->GetCurValue() / 255.f * 100.f));
				m_alphaper->SetAttribute(L"text", std::to_wstring(avalue) + L"%", false);
				updateRGB(color);
				updateDisplay(color);
				m_colorpicker->SetRGBAColor(color);
			}
			else
				ret = false;
		}
		break;
		case Event_Mouse_LClick:
		{
			if (_MNAME(L"color_dialog_yes"))
			{
				Show(false, true);
			}
			else if (_MNAME(L"color_dialog_no"))
			{
				Show(false, false);
			}
			else
				ret = false;
		}
		break;
		default:
			ret = false;
			break;
		}

		return ret;
	}

	void ColorPickerDlg::ShowColorPicker(_m_color showColor, bool alpha, std::function<void(bool, _m_color)> callback)
	{
		if (alpha)
		{
			m_page->Child(L"content")->SetSize(453, 367, true);
			m_page->Child(L"colorpicker_ablock")->SetVisible(true, false);
			m_alphav->SetCurValue(Color::M_GetAValue(showColor));
			updateAlpha(m_alphav->GetCurValue());
		}
		else
		{
			m_page->Child(L"content")->SetSize(453, 342, true);
			m_page->Child(L"colorpicker_ablock")->SetVisible(false, false);
			m_alphav->SetCurValue(255);
		}

		m_alpha = alpha;

		m_page->Child<UIEditBox>(L"colorpicker_hex")->SetLimitText(m_alpha ? 8 : 6);

		m_callback = std::move(callback);

		m_colorpicker->SetRGBAColor(showColor);
		updateRGB(showColor);
		updateHSV(m_colorpicker->GetHSVColor());
		updateDisplay(showColor);
		//显示当前颜色
		UIBkgndStyle bgColor;
		bgColor.FrameColor = Color::M_RGBA(83, 83, 83, 255);
		bgColor.FrameWidth = 1;
		bgColor.bkgndColor = showColor;
		m_page->Child(L"colorpicker_cur")->SetBackground(bgColor);

		Show(true);
	}

	void ColorPickerDlg::Show(bool show, bool isok)
	{
		m_page->SetVisible(show);
		if (m_callback)
		{
			using namespace Color;
			_m_color color = m_colorpicker->GetRGBAColor();
			if (m_alpha)
				color = M_RGBA(M_GetRValue(color), M_GetGValue(color), M_GetBValue(color),
					(_m_byte)m_alphav->GetCurValue());
			m_callback(isok, color);
		}
	}
}