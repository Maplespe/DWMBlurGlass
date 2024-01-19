/**
 * FileName: MainPage.cpp
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
#include "MainPage.h"
#include "Common.h"
#include "../UIManager.h"
#include "../Helper/Helper.h"
#include "../MHostHelper.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace MDWMBlurGlass
{
	using namespace Ctrl;

	MainWindowPage::MainWindowPage(UIControl* parent, XML::MuiXML* ui) : MultiplePages(parent, ui)
	{
		std::wstring_view xml = LR"(
<PropGroup id="group" frameColor="222,222,222,255" frameWidth="1" bgColor="255,255,255,200" frameRound="6" padding="10,10,10,10" />
<PropGroup id="cdsp" normalColor="222,222,222,255" hoverColor="100,100,100,255" pressColor="150,150,150,255" autoSize="false" size="190,30" />
<UINavBar frame="15,15,0,0" name="navbar" fontSize="14" />
<UIControl frame="0,50,485,100%" name="page0" align="LinearV">
    <UIControl frame="15,5,15f,75" align="LinearV" prop="group">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_status" />
            <UILabel frame="10,2,60,20" text="#status" />
            <UILabel frame="10,2,60,20" name="dllstate" text="Not installed" fontColor="255,128,0,255" />
        </UIControl>
        <UIControl frame="0,10,100,100" autoSize="true">
            <UIButton frame="0,0,90,26" name="install" text="#install" />
            <UIButton frame="5,0,90,26" name="uninstall" text="#uninstall" />
            <UIControl frame="10,5,1,15" bgColor="211,211,211,255" />
            <UIButton frame="10,0,120,26" name="savecfg" text="#saveconfig" enable="false" />
        </UIControl>
    </UIControl>
    <UIControl frame="15,8,15f,15f" align="LinearV" prop="group">
        <UIControl frame="0,0,100,100" autoSize="true">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_control" />
            <UILabel frame="10,2,60,20" text="#effectset" />
        </UIControl>
		<UICheckBox frame="0,11" text="#applyglobal" name="applyglobal" />
		<UICheckBox frame="0,11" text="#extendborder" name="extendBorder" />
		<UICheckBox frame="0,11" text="#reflection" name="reflection" />
		<UICheckBox frame="0,11" text="#oldBtnHeight" name="oldBtnHeight" />
        <UIControl frame="0,10,100,100" autoSize="true">
            <UILabel frame="0,1,60,20" text="#blurvalue" />
            <UISlider frame="10,0,200,16" name="blurslider" maxValue="50" />
            <UILabel frame="5,1,60,20" name="blurvalue" text="50" />
        </UIControl>
        <UIControl frame="0,10,100,100" autoSize="true" align="LinearH">
            <UIControl frame="0,0,100,100" autoSize="true" align="LinearV">
                <UILabel frame="0,11,60,20" text="#blendcolor" />
				<UILabel frame="0,22,60,20" text="#inactiveblendcolor" />
                <UILabel frame="0,20,60,20" text="#activecolor" />
                <UILabel frame="0,21,60,20" text="#inactivecolor" />
            </UIControl>
            <UIControl frame="0,0,100,100" autoSize="true" align="LinearV">
                <ColorDisplay frame="10,5,190,30" name="activeBlendColor" prop="cdsp" curColor="255,255,255,100" showAlpha="true" />
				<ColorDisplay frame="10,5,190,30" name="inactiveBlendColor" prop="cdsp" curColor="255,255,255,100" showAlpha="true" />
                <ColorDisplay frame="10,5,190,30" name="activetext" prop="cdsp" curColor="0,0,0,255" />
                <ColorDisplay frame="10,5,190,30" name="inactivetext" prop="cdsp" curColor="180,180,180,255" />
            </UIControl>
        </UIControl>
        <UIControl frame="0,18,100,100" autoSize="true">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_preview" />
            <UILabel frame="10,2,60,20" text="#preview" />
        </UIControl>
        <UIImgBox frame="0,10,10f,90" autoSize="false" frameColor="222,222,222,255" frameWidth="1" img="prebackground" imgStyle="3">
            <UIControl frame="20,20,20f,100" frameColor="222,222,222,255" frameWidth="1" align="LinearV">
                <UIEffectLayer frame="0,0,100%,30" name="preblur" bgColor="255,255,255,100" align="LinearH">
                    <UIImgBox frame="7,7,16,16" autoSize="false" img="icon_title" />
                    <UILabel frame="5,8,60,20" name="sampletitle" text="#sampletitle" />
                </UIEffectLayer>
                <UIControl frame="0,0,100%,100" bgColor="255,255,255,255" frameColor="222,222,222,255" frameWidth="1" />
            </UIControl>
        </UIImgBox>
    </UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page1" visible="false" align="LinearV">
    <UIControl frame="15,5,15f,115" align="LinearV" prop="group">
        <UIControl frame="0,0,10f,30">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_symbol" />
            <UILabel frame="10,2,60,20" text="#symbolfile" />
            <UILabel frame="10,2,60,20" name="symstate" text="invalid" fontColor="255,128,0,255" />
            <UIControl frame="0,0,100%,100%" align="LinearHL">
                <UIButton frame="0,0,120,26" name="downloadsym" text="#download" />
            </UIControl>
        </UIControl>
        <UILabel frame="0,5,10f,10f" autoSize="false" text="#symtip" fontColor="169,169,169,255" />
    </UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page2" visible="false" align="LinearV">
    <UIControl frame="15,5,15f,75" align="LinearV" prop="group">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,20" autoSize="false" img="icon_config" />
            <UILabel frame="10,3,60,20" text="#configfile" />
        </UIControl>
        <UIControl frame="0,10,100,100" autoSize="true">
            <UIButton frame="0,0,90,26" name="importcfg" text="#import" />
            <UIButton frame="5,0,90,26" name="exportcfg" text="#export" />
            <UIControl frame="10,5,1,15" bgColor="211,211,211,255" />
            <UIButton frame="10,0,130,26" name="restorecfg" text="#restore" />
        </UIControl>
    </UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page3" visible="false" align="LinearV">
    <UIControl frame="15,5,15f,120" align="LinearV" prop="group">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,20" autoSize="false" img="icon_about" />
            <UILabel frame="10,3,60,20" text="#about" />
        </UIControl>
        <UILabel frame="0,5,60,20" text="#aboutinfo" />
        <UILabel frame="0,5,60,20" text="https://github.com/Maplespe/DWMBlurGlass" fontColor="30,144,255,255" hyperlink="true" url="https://github.com/Maplespe/DWMBlurGlass" />
        <UILabel frame="0,5,60,20" text="#curlang" />
        <UILabel frame="0,5,60,20" name="langauthor" text="[zh-CN] 翻译贡献者: Maplespe" />
    </UIControl>
</UIControl>
		)";

		if(!ui->CreateUIFromXML(parent, xml.data()))
			throw std::invalid_argument("Invalid XML code!");

		auto navbar = parent->Child<UINavBar>(L"navbar");
		navbar->AddItem(ui->GetStringValue(L"general"));
		navbar->AddItem(ui->GetStringValue(L"symbol"));
		navbar->AddItem(ui->GetStringValue(L"config"));
		navbar->AddItem(ui->GetStringValue(L"about"));

        auto langinfo = parent->Child<UILabel>(L"langauthor");
        auto info = GetCurrentLangInfo();
        std::wstring str = L"[" + info.local + L"] " + ui->GetStringValue(L"langauthor") + L" ";
        str += info.author;
        langinfo->SetAttribute(L"text", str);

        m_page = parent;
        m_blurValue = m_page->Child<UISlider>(L"blurslider");
        m_effLayer = m_page->Child<UIEffectLayer>(L"preblur");
        m_blurValueLabel = m_page->Child<UILabel>(L"blurvalue");
        m_saveBtn = m_page->Child<UIButton>(L"savecfg");
        m_restBtn = m_page->Child<UIButton>(L"restorecfg");

        RefreshStatus();
        RefreshSymStatus();
        LoadConfig(Utils::GetCurrentDir() + L"\\data\\config.ini");
	}

	bool MainWindowPage::EventProc(UINotifyEvent event, UIControl* control, _m_param param)
	{
        bool ret = true;
        HWND hWnd = (HWND)control->GetParentWin()->GetWindowHandle();
        switch (event)
        {
			case Event_NavBar_ItemChange:
	        {
                if (_MNAME(L"navbar"))
                    SwitchNavPage(L"page", (int)param);
                else
                    ret = false;
	        }
            break;
			case Event_Mouse_LClick:
	        {
                if (_MNAME(L"install"))
                {
                    std::wstring errinfo;
                    if (InstallScheduledTasks(errinfo))
                    {
                        std::wstring err;
                        bool symbolState = MHostGetSymbolState();
                        if (symbolState && !LoadDWMExtension(err, m_ui))
                        {
                            MessageBoxW(hWnd, (m_ui->GetStringValue(L"loadfail") + err).c_str(), L"Error", MB_ICONERROR);
                            return false;
                        }
                        if (symbolState)
                            RefreshSysConfig();
                        MessageBoxW(hWnd,
                            m_ui->GetStringValue(symbolState ? L"installsucs" : L"installsucs1").c_str(),
                            m_ui->GetStringValue(L"install").c_str(),
                            MB_ICONINFORMATION
                        );
                    }
                    else
                        MessageBoxW(hWnd,
                            (m_ui->GetStringValue(L"installfail") + errinfo).c_str(),
                            m_ui->GetStringValue(L"install").c_str(),
                            MB_ICONERROR
                        );
                    RefreshStatus();
                }
                else if (_MNAME(L"uninstall"))
                {
                    std::wstring errinfo;
                    ShutdownDWMExtension(errinfo);
                    if (DeleteScheduledTasks(errinfo))
                    {
                        RefreshSysConfig();

                        MessageBoxW(hWnd,
                            m_ui->GetStringValue(L"uninstallsucs").c_str(),
                            m_ui->GetStringValue(L"uninstall").c_str(),
                            MB_ICONINFORMATION
                        );
                    }
                    else
                        MessageBoxW(hWnd,
                            (m_ui->GetStringValue(L"uninstallfail") + errinfo).c_str(),
                            m_ui->GetStringValue(L"uninstall").c_str(),
                            MB_ICONERROR
                        );
                    RefreshStatus();
                }
                else if(_MNAME(L"downloadsym"))
                {
                    control->SetEnabled(false);
                    static_cast<UIButton*>(control)->SetAttribute(L"text", m_ui->GetStringValue(L"symdowning"));
                    auto navbar = m_page->Child(L"navbar");
                	navbar->SetEnabled(false);
                    std::thread([navbar, control, this, hWnd]
                    {
                        if (!MHostDownloadSymbol())
                            MessageBoxW(hWnd,
                                m_ui->GetStringValue(L"symdownfail").c_str(),
                                m_ui->GetStringValue(L"download").c_str(),
                                MB_ICONERROR
                            );
                        else
                            RefreshSymStatus();
                    	static_cast<UIButton*>(control)->SetAttribute(L"text", m_ui->GetStringValue(L"download"), false);
                        navbar->SetEnabled(true, false);
                        m_page->UpdateLayout();
                        
                        if (IsInstallTasks())
                        {
                            std::wstring err;
                            if (!LoadDWMExtension(err, m_ui))
                                MessageBoxW(hWnd, (m_ui->GetStringValue(L"loadfail") + err).c_str(), L"Error", MB_ICONERROR);
                            else
                                RefreshSysConfig();
                        }
                    }).detach();
                }
                else if (_MNAME(L"savecfg"))
                {
                    SaveConfig(Utils::GetCurrentDir() + L"\\data\\config.ini");
                    control->SetEnabled(false);
                    RefreshDWMConfig();
                }
                else if (_MNAME(L"importcfg"))
                {
                    static bool confirmed = false;
                    if (!confirmed)
                    {
                        if (MessageBoxW(hWnd, m_ui->GetStringValue(L"importwarn").c_str(),
                            m_ui->GetStringValue(L"import").c_str(), MB_ICONQUESTION | MB_YESNO) != IDYES)
                            return false;
                        confirmed = true;
                    }

                    std::vector<std::wstring> seledFile;
                    bool isOpen = BrowseForFile(true, false, { { L"Config", L"*.ini" } }, hWnd, seledFile);
                    if (!isOpen)
                        return false;

                    LoadConfig(seledFile[0]);
                    SaveConfig(seledFile[0]);
                    SetButtonEnable(false);
                    RefreshDWMConfig();
                }
                else if (_MNAME(L"exportcfg"))
                {
                    std::vector<std::wstring> seledFile;
                    bool isOpen = BrowseForFile(false, false, { { L"Config", L"*.ini" } }, hWnd, seledFile, L"ini");
                    if (!isOpen)
                        return false;

                    SaveConfig(seledFile[0]);
                }
                else if(_MNAME(L"restorecfg"))
                {
                    if (MessageBoxW(hWnd, m_ui->GetStringValue(L"restorecfg").c_str(),
                        m_ui->GetStringValue(L"restore").c_str(), MB_ICONQUESTION | MB_YESNO) != IDYES)
                        return false;

                    LoadConfig({});
                    SaveConfig(Utils::GetCurrentDir() + L"\\data\\config.ini");
                    SetButtonEnable(false);
                    RefreshDWMConfig();
                    control->SetEnabled(false);
                }
                else if(_MNAME(L"applyglobal"))
                {
                    m_cfgData.applyglobal = static_cast<UICheckBox*>(control)->GetSel();
                    SetButtonEnable(true);
                }
                else if (_MNAME(L"extendBorder"))
                {
                    m_cfgData.extendBorder = static_cast<UICheckBox*>(control)->GetSel();
                    SetButtonEnable(true);
                }
                else if (_MNAME(L"reflection"))
                {
                    m_cfgData.reflection = static_cast<UICheckBox*>(control)->GetSel();
                    SetButtonEnable(true);
                }
                else if (_MNAME(L"oldBtnHeight"))
                {
                    m_cfgData.oldBtnHeight = static_cast<UICheckBox*>(control)->GetSel();
                    SetButtonEnable(true);
                }
                else
                    ret = false;
	        }
            break;
            case Event_Slider_Change:
            {
                if (_MNAME(L"blurslider"))
                {
                    m_blurValueLabel->SetAttribute(L"text", std::to_wstring((int)param), false);
                    m_effLayer->SetEffectValue((float)param);
                    m_cfgData.blurAmount = (float)param;
                    SetButtonEnable(true);
                    m_page->UpdateLayout();
                }
                else
                    ret = false;
            }
            break;
            case Event_ColorPicker_Change:
            {
                if (_MNAME(L"activetext"))
                {
                	m_page->Child<UILabel>(L"sampletitle")->SetAttributeSrc(L"fontColor", (_m_color)param);
                    m_cfgData.activeTextColor = (_m_color)param;
                }
                else if (_MNAME(L"inactivetext"))
                {
                    m_cfgData.inactiveTextColor = (_m_color)param;
                }
                else if(_MNAME(L"activeBlendColor"))
                {
                    auto bkgnd = m_effLayer->GetBkgndStyle();
                    bkgnd.bkgndColor = (_m_color)param;
                    m_effLayer->SetBackground(bkgnd);
                    m_cfgData.activeBlendColor = (_m_color)param;
                }
                else if (_MNAME(L"inactiveBlendColor"))
                {
                    m_cfgData.inactiveBlendColor = (_m_color)param;
                }
                else
                    ret = false;

                if (ret)
                    SetButtonEnable(true);
            }
            break;
        default:
            ret = false;
            break;
        }
		return ret;
	}

    void MainWindowPage::RefreshStatus()
    {
        auto label = m_page->Child<UILabel>(L"dllstate");
        bool installed = IsInstallTasks();
        label->SetAttribute(L"text", m_ui->GetStringValue(installed ? L"status1" : L"status0"));
        label->SetAttributeSrc(L"fontColor", installed ? Color::M_RGBA(0, 192, 0, 255) : Color::M_RGBA(255, 128, 0, 255));
        m_page->Child(L"install")->SetEnabled(!installed);
        m_page->Child(L"uninstall")->SetEnabled(installed);
    }

    void MainWindowPage::RefreshSymStatus()
    {
        auto label = m_page->Child<UILabel>(L"symstate");
        bool installed = MHostGetSymbolState();
        label->SetAttribute(L"text", m_ui->GetStringValue(installed ? L"symstatus1" : L"symstatus0"));
        label->SetAttributeSrc(L"fontColor", installed ? Color::M_RGBA(0, 192, 0, 255) : Color::M_RGBA(255, 128, 0, 255));
        m_page->Child(L"downloadsym")->SetEnabled(!installed);
    }

    void MainWindowPage::SetButtonEnable(bool enable)
    {
        m_saveBtn->SetEnabled(enable);
        m_restBtn->SetEnabled(!m_cfgData.isDefault());
    }

    void MainWindowPage::LoadConfig(std::wstring_view path)
    {
        m_cfgData = {};

        auto ret = Utils::GetIniString(path, L"config", L"blurAmount");

        m_cfgData.applyglobal = Utils::GetIniString(path, L"config", L"applyglobal") == L"true";
        m_cfgData.extendBorder = Utils::GetIniString(path, L"config", L"extendBorder") == L"true";
        m_cfgData.reflection = Utils::GetIniString(path, L"config", L"reflection") == L"true";
        m_cfgData.oldBtnHeight = Utils::GetIniString(path, L"config", L"oldBtnHeight") == L"true";

        if (!ret.empty())
            m_cfgData.blurAmount = (float)Helper::M_Clamp(0.0, 50.0, _wtof(ret.data()));

        ret = Utils::GetIniString(path, L"config", L"activeTextColor");
        if (!ret.empty())
            m_cfgData.activeTextColor = (COLORREF)_wtoll(ret.data());

        ret = Utils::GetIniString(path, L"config", L"inactiveTextColor");
        if (!ret.empty())
            m_cfgData.inactiveTextColor = (COLORREF)_wtoll(ret.data());

        ret = Utils::GetIniString(path, L"config", L"activeBlendColor");
        if (!ret.empty())
            m_cfgData.activeBlendColor = (COLORREF)_wtoll(ret.data());

        ret = Utils::GetIniString(path, L"config", L"inactiveBlendColor");
        if (!ret.empty())
            m_cfgData.inactiveBlendColor = (COLORREF)_wtoll(ret.data());

        m_page->Child<UICheckBox>(L"applyglobal")->SetSel(m_cfgData.applyglobal);
        m_page->Child<UICheckBox>(L"extendBorder")->SetSel(m_cfgData.extendBorder);
        m_page->Child<UICheckBox>(L"reflection")->SetSel(m_cfgData.reflection);
        m_page->Child<UICheckBox>(L"oldBtnHeight")->SetSel(m_cfgData.oldBtnHeight);

        _m_color textColor = (m_cfgData.activeTextColor & 0x00FFFFFF) | 0xFF000000;
        m_page->Child<UILabel>(L"sampletitle")->SetAttributeSrc(L"fontColor", textColor, false);
        m_page->Child(L"activetext")->SetAttribute(L"curColor", Color::M_RGBA_STR(textColor), false);

        textColor = (m_cfgData.inactiveTextColor & 0x00FFFFFF) | 0xFF000000;
        m_page->Child(L"inactivetext")->SetAttribute(L"curColor", Color::M_RGBA_STR(textColor), false);

        auto bkgnd = m_effLayer->GetBkgndStyle();
        bkgnd.bkgndColor = m_cfgData.activeBlendColor;
        m_effLayer->SetBackground(bkgnd);
        m_effLayer->SetEffectValue(m_cfgData.blurAmount);

        m_page->Child(L"activeBlendColor")->SetAttribute(L"curColor", Color::M_RGBA_STR(bkgnd.bkgndColor), false);
        m_page->Child(L"inactiveBlendColor")->SetAttribute(L"curColor", Color::M_RGBA_STR(m_cfgData.inactiveBlendColor), false);

        m_blurValue->SetCurValue((int)m_cfgData.blurAmount, false);
        m_blurValueLabel->SetAttribute(L"text", std::to_wstring((int)m_cfgData.blurAmount), false);

        m_page->UpdateLayout();
    }

    void MainWindowPage::SaveConfig(std::wstring_view path)
    {
        SetIniString(path, L"config", L"applyglobal", m_cfgData.applyglobal ? L"true" : L"false");
        SetIniString(path, L"config", L"extendBorder", m_cfgData.extendBorder ? L"true" : L"false");
        SetIniString(path, L"config", L"reflection", m_cfgData.reflection ? L"true" : L"false");
        SetIniString(path, L"config", L"oldBtnHeight", m_cfgData.oldBtnHeight ? L"true" : L"false");
        SetIniString(path, L"config", L"blurAmount", std::to_wstring(m_cfgData.blurAmount));
        SetIniString(path, L"config", L"activeTextColor", std::to_wstring(m_cfgData.activeTextColor));
        SetIniString(path, L"config", L"inactiveTextColor", std::to_wstring(m_cfgData.inactiveTextColor));
        SetIniString(path, L"config", L"activeBlendColor", std::to_wstring(m_cfgData.activeBlendColor));
        SetIniString(path, L"config", L"inactiveBlendColor", std::to_wstring(m_cfgData.inactiveBlendColor));
    }

    void MainWindowPage::RefreshDWMConfig()
    {
        MHostNotify(MHostNotifyType::Refresh);
        if(IsInstallTasks())
        {
            RefreshSysConfig();
        }
    }

    void MainWindowPage::RefreshSysConfig()
    {
        BOOL enable = TRUE;
        SystemParametersInfoW(SPI_SETGRADIENTCAPTIONS, 0, &enable, SPIF_SENDCHANGE);
        SendNotifyMessageW(HWND_BROADCAST, WM_DWMCOLORIZATIONCOLORCHANGED, m_cfgData.activeBlendColor, 1);
    }
}