/**
 * FileName: Dialog.h
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

namespace MDWMBlurGlass
{
	using namespace Mui;

	class Page
	{
	public:
		Page(Ctrl::UIControl* parent, XML::MuiXML* ui)
		{
			M_ASSERT(parent && ui)
			m_parent = parent;
			m_ui = ui;
		}

		virtual bool EventProc(UINotifyEvent event, Ctrl::UIControl* control, _m_param param) = 0;

	protected:
		Ctrl::UIControl* m_parent = nullptr;
		Ctrl::UIControl* m_page = nullptr;
		XML::MuiXML* m_ui = nullptr;
	};

	class MultiplePages : public Page
	{
	public:
		MultiplePages(Ctrl::UIControl* parent, XML::MuiXML* ui)
			: Page(parent, ui) {}

		void SwitchNavPage(const std::wstring& subPageName, int page);

	private:
		int m_curpage = 0;
	};
}