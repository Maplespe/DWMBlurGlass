/**
 * FileName: MDcompRender.h
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
#include "winrt.h"

namespace MDWMBlurGlass
{
	class MRender_DComp : public Mui::Render::MRender_D2D
	{
	public:
		bool InitRender(Mui::_m_uint width, Mui::_m_uint height) override;
		HRESULT InitBackdrop();

	private:
		com_ptr<abi::comp::ICompositionGraphicsDevice>		m_compositionDevice = nullptr;
		com_ptr<abi::comp::ICompositionBackdropBrush>		m_brush = nullptr;
		com_ptr<abi::comp::ICompositor>						m_compositor = nullptr;
		com_ptr<abi::desktop::IDesktopWindowTarget>			m_backdropTarget = nullptr;
		com_ptr<abi::comp::ISpriteVisual>					m_backdropVisual = nullptr;
		com_ptr<abi::comp::ICompositionSurfaceBrush>		m_surfaceBrush = nullptr;
	};
}