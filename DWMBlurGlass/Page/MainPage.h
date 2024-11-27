/**
 * FileName: MainPage.h
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
#include "../Extend/Dialog.h"
#include "../MainWindow.h"
#include "Common.h"

namespace MDWMBlurGlass
{
    class MainWindowPage : MultiplePages
    {
    public:
        MainWindowPage(Ctrl::UIControl* parent, XML::MuiXML* ui);

        bool EventProc(UINotifyEvent event, Ctrl::UIControl* control, _m_param param) override;
        bool SrcEventProc(MWindowCtx*, const MWndDefEventSource& defcallback, MEventCodeEnum code, _m_param param);

        void CreateTitleBar(XML::MuiXML* ui);

    private:
        void RefreshTopLineColor(bool active);
        void RefreshStatus();
        void RefreshSymStatus();
        void SetButtonEnable(bool enable);

        void LoadConfig(std::wstring_view path);
        void SaveConfig(std::wstring_view path);

        void RefreshDWMConfig();
        void RefreshSysConfig();

        void RefreshBlurPreview();

        void SwitchColorModePreview(bool light);
        void SwitchBlurMethod(blurMethod index);
        void SwitchBlurType(effectType type, bool skip = false);

		Ctrl::UIControl* m_effLayer = nullptr;

        Ctrl::UINavBar* m_cmodeNavbar = nullptr;

        Ctrl::UISlider* m_blurValue = nullptr;
        Ctrl::UILabel* m_blurValueLabel = nullptr;

        Ctrl::UISlider* m_customBlurValue = nullptr;
        Ctrl::UILabel* m_customBlurValueLabel = nullptr;

        Ctrl::UISlider* m_luminosityValue = nullptr;
        Ctrl::UILabel* m_luminosityValueLabel = nullptr;

        Ctrl::UISlider* m_blurBalanceValue = { nullptr };
        Ctrl::UILabel* m_blurBalanceLabel = { nullptr };

        Ctrl::UISlider* m_glowBalanceValue = { nullptr };
        Ctrl::UILabel* m_glowBalanceLabel = { nullptr };

        Ctrl::UISlider* m_glassIntensityValue = { nullptr };
        Ctrl::UILabel* m_glassIntensityLabel = { nullptr };

        Ctrl::UIButton* m_saveBtn[2] = { nullptr };
        Ctrl::UIButton* m_restBtn = nullptr;

        ConfigData m_cfgData;
    };
}