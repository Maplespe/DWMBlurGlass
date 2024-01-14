/**
 * FileName: Dialog.cpp
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
#include "Dialog.h"

namespace MDWMBlurGlass
{
	using namespace Render;

	void Dialog::ScaleContent(float scale)
	{
		class update : UINodeBase
		{
		public:
			void updateScale()
			{
				UpdateScale();
			}
		};
		std::function<void(UINodeBase*, _m_rcscale)> setChild = [this, &setChild](UINodeBase* node, _m_rcscale scale)
		{
			node->SetScale(scale, false);
			std::vector<UINodeBase*> nodeList;
			node->GetChildrenList(nodeList);

			for (auto& child : nodeList)
				setChild(child, scale);
		};

		std::vector<UINodeBase*> nodeList;
		m_content->GetChildrenList(nodeList);

		_m_rcscale rcscale = { scale, scale, scale, scale };
		m_content->SetScale(rcscale);
		for (auto& _child : nodeList)
			setChild(_child, rcscale);

		reinterpret_cast<update*>(m_content)->updateScale();
	}

	void MultiplePages::SwitchNavPage(const std::wstring& subPageName, int page)
	{
		Ctrl::UIControl* cur = m_page->Child(subPageName + std::to_wstring(page));
		Ctrl::UIControl* old = m_page->Child(subPageName + std::to_wstring(m_inani ? m_lastpage[0] : m_curpage));

		if (m_inani)
		{
			m_inani = false;
			m_ani->StopTask(m_task, true);
			old->SetVisible(false);
			m_page->Child(subPageName + std::to_wstring(m_lastpage[1]))->SetVisible(false);
		}
		cur->SetAlpha(0, false);
		cur->SetVisible(true);

		bool reverse = page < m_curpage;
		cur->SetPos(reverse ? -255 : 0, 0, false);
		int srcY = old->GetPos().y;
		auto aniProc = [old, cur, reverse, page, srcY, this](const MAnimation::MathsCalc* calc, float per)
		{
			auto pos = calc->calc(MAnimation::Exponential_Out, 0, 255);
			auto alpha = 255 - pos * 2;
			if (alpha < 0) alpha = 0;
			old->SetAlpha((_m_byte)alpha, false);
			cur->SetAlpha((_m_byte)pos, false);
			cur->SetPos(reverse ? -255 + pos : 255 - pos, srcY);
			if (per == 100.f)
			{
				cur->SetPos(0, srcY);
				old->SetVisible(false, true);

				m_curpage = page;
				m_inani = false;
			}
			return m_inani;
		};
		//当前正在进行动画的页面参数
		m_lastpage[0] = page;
		m_lastpage[1] = m_curpage;
		m_inani = true;
		m_task = m_ani->CreateTask(aniProc, 300);
	}
}