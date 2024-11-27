/**
 * FileName: ColorPickerDlg.h
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
#include "Dialog.h"

namespace MDWMBlurGlass
{
	using namespace Ctrl;

	class ColorPickerDlg : public Page
	{
	public:
		ColorPickerDlg(UIControl* parent, XML::MuiXML* ui);

		bool EventProc(UINotifyEvent event, UIControl* control, _m_param param) override;

		void ShowColorPicker(_m_color showColor, bool alpha, std::function<void(bool, _m_color)> callback);

	private:
		void Show(bool show, bool isok = false);

		void updateDisplay(_m_color color);
		void updateRGB(_m_color color);
		void updateHSV(UIColorPicker::HSV&& hsv);
		void updateAlpha(int value);
		void changeRGB();

		UIColorPicker* m_colorpicker = nullptr;
		UISlider* m_alphav = nullptr;
		UILabel* m_alphaper = nullptr;

		bool m_alpha = false;
		std::function<void(bool, _m_color)> m_callback = nullptr;
	};
}