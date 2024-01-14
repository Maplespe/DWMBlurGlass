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
#include "Common.h"

namespace MDWMBlurGlass
{
    class MainWindowPage : MultiplePages
    {
    public:
        MainWindowPage(Ctrl::UIControl* parent, XML::MuiXML* ui);

        bool EventProc(UINotifyEvent event, Ctrl::UIControl* control, _m_param param) override;

    private:
        void RefreshStatus();
        void RefreshSymStatus();
        void SetButtonEnable(bool enable);

        void LoadConfig(std::wstring_view path);
        void SaveConfig(std::wstring_view path);

        void RefreshDWMConfig();

        Ctrl::UIEffectLayer* m_effLayer = nullptr;
        Ctrl::UISlider* m_blurValue = nullptr;
        Ctrl::UILabel* m_blurValueLabel = nullptr;

        Ctrl::UIButton* m_saveBtn = nullptr;
        Ctrl::UIButton* m_restBtn = nullptr;

        ConfigData m_cfgData;
    };
}