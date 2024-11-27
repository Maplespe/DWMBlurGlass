/**
 * FileName: BlurRadiusTweaker.cpp
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
#include "BlurRadiusTweaker.h"
#include "HookDef.h"
#include "CommonDef.h"

namespace MDWMBlurGlassExt::BlurRadiusTweaker
{
	using namespace CommonDef;
	using namespace MDWMBlurGlass;

	std::atomic_bool g_startup = false;

	//Win11 BlurAmount
	MinHook g_funCRenderingTechnique_ExecuteBlur
	{
		"CRenderingTechnique::ExecuteBlur", CRenderingTechnique_ExecuteBlur
	};

	MinHook g_funCRenderingTechnique_ExecuteBlur24h2
	{
		"CRenderingTechnique::ExecuteBlur", CRenderingTechnique_ExecuteBlur24h2
	};

	//Win10 WinUI HostBackdrop Brush
	MinHook g_funCCustomBlur_BuildEffect
	{
		"CCustomBlur::BuildEffect", CCustomBlur_BuildEffect
	};
	MinHook g_funCCustomBlur_DetermineOutputScale
	{
		"CCustomBlur::DetermineOutputScale", CCustomBlur_DetermineOutputScale
	};

	void Attach()
	{
		if (g_startup) return;
		g_startup = true;

		if (os::buildNumber < 22000)
		{
			g_funCCustomBlur_BuildEffect.Attach();
			g_funCCustomBlur_DetermineOutputScale.Attach();
			g_CD2DContext_FillEffect_HookDispatcher.enable_hook_routine<0, true>();
		}
		else if (os::buildNumber >= 22000)
		{
			if(os::buildNumber >= 26100)
				g_funCRenderingTechnique_ExecuteBlur24h2.Attach();
			else
				g_funCRenderingTechnique_ExecuteBlur.Attach();
		}

	}

	void Detach()
	{
		if (!g_startup) return;

		if (os::buildNumber < 22000)
		{
			g_CD2DContext_FillEffect_HookDispatcher.enable_hook_routine<0, false>();
			g_funCCustomBlur_DetermineOutputScale.Detach();
			g_funCCustomBlur_BuildEffect.Detach();
		}
		else if (os::buildNumber >= 22000)
		{
			if (os::buildNumber >= 26100)
				g_funCRenderingTechnique_ExecuteBlur24h2.Detach();
			else
				g_funCRenderingTechnique_ExecuteBlur.Detach();
		}

		g_startup = false;
	}

	void Refresh()
	{
		if (g_configData.blurmethod != blurMethod::DWMAPIBlur && g_configData.customAmount && !g_startup)
			Attach();
		else if ((!g_configData.customAmount || g_configData.blurmethod == blurMethod::DWMAPIBlur) && g_startup)
			Detach();
	}

	DWORD64 CD2DContext_FillEffect(ID2D1DeviceContext** This, const DWM::ID2DContextOwner* a2, ID2D1Effect* inputEffect,
		const D2D_RECT_F* srcRect, const D2D_POINT_2F* dstPoint, D2D1_INTERPOLATION_MODE interMode,
		D2D1_COMPOSITE_MODE mode)
	{
		const UINT32 count = inputEffect->GetPropertyCount();
		for (UINT i = 0; i < count; i++)
		{
			wchar_t name[256];
			inputEffect->GetPropertyName(i, name, 256);

			if (wcscmp(name, L"StandardDeviation") != 0)
				continue;

			float dv = 0.f;
			inputEffect->GetValue(i, &dv);

			//HostBackdrop 
			if (count == 4)
			{
				g_CD2DContext_FillEffect_HookDispatcher.return_value() = g_CD2DContext_FillEffect_HookDispatcher.call_org(
					This, a2, inputEffect, srcRect, dstPoint, interMode, mode);
				g_CD2DContext_FillEffect_HookDispatcher.modify_handle_type_for_temporary(handling_type::return_now);
				return S_OK;
			}

			//无论如何 哪怕是0 都会有一点点的模糊(因为图像被预缩放2倍了) 所以如果要透明 可以不绘制
			if (g_configData.blurAmount == 0.f)
			{
				g_CD2DContext_FillEffect_HookDispatcher.return_value() = S_OK;
				g_CD2DContext_FillEffect_HookDispatcher.modify_handle_type_for_temporary(handling_type::skip);
				return S_OK;
			}

			//BlurBehind
			inputEffect->SetValue(D2D1_DIRECTIONALBLUR_PROP_STANDARD_DEVIATION, g_configData.blurAmount / 2.f);
		}
		return S_OK;
	}

	DWORD64 CRenderingTechnique_ExecuteBlur(DWM::CRenderingTechnique* This, DWM::CDrawingContext* a2,
		const DWM::EffectInput* a3, const D2D_VECTOR_2F* a4, DWM::EffectInput* a5)
	{
		auto blurVector = *a4;
		blurVector.x = blurVector.y = (g_configData.blurAmount / 2.f) / 10.f;
		return g_funCRenderingTechnique_ExecuteBlur.call_org(This, a2, a3, &blurVector, a5);
	}

	DWORD64 CRenderingTechnique_ExecuteBlur24h2(DWM::CRenderingTechnique* This, DWM::CDrawingContext* a2,
		const DWM::EffectInput* a3, const D2D_VECTOR_2F* a4, const D2D_SIZE_F* a5, DWM::EffectInput* a6)
	{
		auto blurVector = *a4;
		blurVector.x = blurVector.y = (g_configData.blurAmount / 2.f) / 10.f;
		return g_funCRenderingTechnique_ExecuteBlur24h2.call_org(This, a2, a3, &blurVector, a5, a6);
	}

	DWORD64 CCustomBlur_BuildEffect(DWM::CCustomBlur* This, ID2D1Image* backdropImage, const D2D_RECT_F* srcRect,
	                                const D2D_SIZE_F* kSize, D2D1_GAUSSIANBLUR_OPTIMIZATION a5, const D2D_VECTOR_2F* a6, D2D_VECTOR_2F* a7)
	{
		const auto ret = g_funCCustomBlur_BuildEffect.call_org(This, backdropImage, srcRect, kSize, a5, a6, a7);

		This->DirBlurKernelXEffect()->SetValue(0, g_configData.blurAmount);
		This->DirBlurKernelYEffect()->SetValue(0, g_configData.blurAmount);

		return ret;
	}

	float CCustomBlur_DetermineOutputScale(float a1, float a2, D2D1_GAUSSIANBLUR_OPTIMIZATION a3)
	{
		//hostBackdrop画刷会预先使用矩阵缩放为2.5倍 再利用BuildEffect中的D2D Scale效果缩放回去 然后利用2次自定义高斯内核模糊 达到快速高模糊度计算
		//这里设置为1 则不缩放 否则BuildEffect中的图像会非常模糊 马赛克一样
		return 1.f;
	}
}
