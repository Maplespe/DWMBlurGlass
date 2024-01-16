/**
 * FileName: MainWindow.h
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

namespace MDWMBlurGlass
{
	inline const std::wstring g_vernum = L"1.0.1";

	bool MainWindow_EventProc(Mui::MWindowCtx*, Mui::UINotifyEvent event, Mui::Ctrl::UIControl* control, Mui::_m_param param);

	bool MainWindow_InitWindow(const Mui::MWindowCtx*, Mui::Ctrl::UIControl* root, Mui::XML::MuiXML* ui);

	void MainWindow_Destroy();

	void ShowColorPicker(Mui::_m_color showColor, bool alpha, std::function<void(bool, Mui::_m_color)> callback);
}