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
#include "VersionHelper.h"
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace MDWMBlurGlass
{
	using namespace Ctrl;

	MainWindowPage::MainWindowPage(UIControl* parent, XML::MuiXML* ui) : MultiplePages(parent, ui)
	{
        std::wstring xml = LR"(
<PropGroup id="group" frameColor="222,222,222,255" frameWidth="1" bgColor="255,255,255,200" frameRound="6" padding="10,10,10,5" />
<PropGroup id="cdsp" normalColor="222,222,222,255" hoverColor="100,100,100,255" pressColor="150,150,150,255" autoSize="false" size="190,30" />
<UINavBar frame="15,50,0,0" name="navbar" fontSize="14" />
<UIControl frame="0,95,485,100%" name="page0" align="LinearV">
    <UIControl frame="15,5,15f,75" align="LinearV" prop="group">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_status" />
            <UILabel pos="10,2" text="#status" />
            <UILabel pos="10,2" name="dllstate" text="Not installed" fontColor="255,128,0,255" />
        </UIControl>
        <UIControl frame="0,10,100,100" autoSize="true">
            <UIButton pos="0,0" name="install" text="#install" autoSize="true" minSize="80,26" maxSize="105,26" inset="5,2,5,2" />
            <UIButton pos="5,0" name="uninstall" text="#uninstall" autoSize="true" minSize="80,26" maxSize="105,26" inset="5,2,5,2" />
            <UIControl frame="10,5,1,15" bgColor="211,211,211,255" />
            <UIButton pos="10,0" name="savecfg" text="#saveconfig" enable="false" autoSize="true" minSize="100,26" maxSize="190,26" inset="5,2,5,2" />
        </UIControl>
    </UIControl>
    <UIControl frame="15,8,15f,15f" align="LinearV" prop="group" name="generalGroup">
        <UIControl frame="0,0,100,100" autoSize="true">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_control" />
            <UILabel pos="10,2" text="#effectset" />
        </UIControl>
		<UICheckBox pos="0,11" text="#applyglobal" name="applyglobal" />
		<UICheckBox pos="0,11" text="#extendborder" name="extendBorder" />
		<UICheckBox pos="0,11" text="#reflection" name="reflection" />
		<UICheckBox pos="0,11" text="#oldBtnHeight" name="oldBtnHeight" />
        <UIControl frame="0,10,100,100" autoSize="true">
            <UICheckBox frame="0,1,60,20" text="#blurvalue" name="customAmount" />
            <UISlider frame="10,0,200,16" name="blurslider" maxValue="50" />
            <UILabel pos="5,1" name="blurvalue" text="50" />
        </UIControl>
		<UIControl frame="0,15,10f,1" bgColor="222,222,222,255" />
		<UINavBar frame="0,10,0,0" name="cmodenavbar" fontSize="12" barAnitime="0" barColor="162,158,180,255" />
        <UIControl frame="0,10,100,100" autoSize="true" align="LinearH">
            <UIControl frame="0,0,100,100" autoSize="true" align="LinearV">
                <UILabel pos="0,11" text="#blendcolor" />
				<UILabel pos="0,22" text="#inactiveblendcolor" />
                <UILabel pos="0,20" text="#activecolor" />
                <UILabel pos="0,21" text="#inactivecolor" />
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
            <UILabel pos="10,2" text="#preview" />
        </UIControl>
        <UIImgBox frame="0,10,10f,90" autoSize="false" frameColor="222,222,222,255" frameWidth="1" img="prebackground" imgStyle="3">
            <UIControl frame="20,20,20f,100" frameColor="222,222,222,255" frameWidth="1" align="LinearV">
                <UIEffectLayer frame="0,0,100%,30" name="preblur" bgColor="255,255,255,100" align="LinearH">
                    <UIImgBox frame="7,7,16,16" autoSize="false" img="icon_title" />
                    <UILabel pos="5,8" name="sampletitle" text="#sampletitle" />
                </UIEffectLayer>
                <UIControl frame="0,0,100%,100" bgColor="255,255,255,255" frameColor="222,222,222,255" frameWidth="1" />
            </UIControl>
        </UIImgBox>
    </UIControl>
</UIControl>)";

        xml += LR"(
<UIControl frame="0,50,485,100%" name="page1" visible="false" align="LinearV">
	<UIControl frame="15,5,15f,115" align="LinearV" prop="group" autoSize="true" maxSize="455,150" minSize="455,90">
		<UIControl frame="0,0,10f,30">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_code" />
            <UILabel pos="10,2" text="#blurmethod" />
            <UIControl frame="0,0,100%,100%" align="LinearHL">
                <UIComBox frame="0,0,260,26" name="blurmethod" fontSize="12" menuHeight="150" />
            </UIControl>
        </UIControl>
        <UILabel pos="0,5" fontColor="169,169,169,255" name="functip" />
    </UIControl>
	<UIControl frame="15,5,15f,120" align="LinearV" prop="group" enable="true" name="effectgroup">
		<UIControl frame="0,0,100%,30" align="LinearH">
			<UILabel pos="5,5" text="#effecttype" />
			<UIControl frame="0,0,100%,100%" align="LinearHL">
				<UIComBox frame="10,0,120,26" name="effecttype" fontSize="12" menuHeight="150" />
			</UIControl>
		</UIControl>
		<UIControl frame="0,5,100%,30" align="LinearH" name="customEffectGroup">
		    <UILabel pos="5,0" text="#blurvaluetitle" />
			<UILabel frame="20,0,30,20" name="titlebarAmountValue" text="50" autoSize="false" textAlign="2" />
			<UISlider frame="5,0,10f,16" name="titlebarAmountSlider" maxValue="50" maxSize="200,16" />
		</UIControl>
		<UIControl frame="0,5,100%,30" align="LinearH" name="customEffectGroup1">
		    <UILabel pos="5,0" text="#luminosity" />
			<UILabel frame="20,0,35,20" name="luminosityValue" text="50" autoSize="false" textAlign="2" />
			<UISlider frame="5,0,10f,16" name="luminositySlider" maxValue="100" maxSize="200,16" />
		</UIControl>
	</UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page2" visible="false" align="LinearV">
    <UIControl frame="15,5,15f,115" align="LinearV" prop="group" autoSize="true" maxSize="455,150" minSize="455,90">
        <UIControl frame="0,0,10f,30">
            <UIImgBox frame="0,0,18,18" autoSize="false" img="icon_symbol" />
            <UILabel pos="10,2" text="#symbolfile" />
            <UILabel pos="10,2" name="symstate" text="invalid" fontColor="255,128,0,255" />
            <UIControl frame="0,0,100%,100%" align="LinearHL">
                <UIButton pos="0,0" name="downloadsym" text="#download" autoSize="true" minSize="100,26" maxSize="210,26" inset="5,2,5,2" />
            </UIControl>
        </UIControl>
        <UILabel pos="0,5" text="#symtip" fontColor="169,169,169,255" />
    </UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page3" visible="false" align="LinearV">
    <UIControl frame="15,5,15f,75" align="LinearV" prop="group">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,20" autoSize="false" img="icon_config" />
            <UILabel pos="10,3" text="#configfile" />
        </UIControl>
        <UIControl frame="0,10,100,100" autoSize="true">
            <UIButton pos="0,0" name="importcfg" text="#import" autoSize="true" minSize="80,26" maxSize="100,26" inset="5,2,5,2" />
            <UIButton pos="5,0" name="exportcfg" text="#export" autoSize="true" minSize="80,26" maxSize="100,26" inset="5,2,5,2" />
            <UIControl frame="10,5,1,15" bgColor="211,211,211,255" />
            <UIButton pos="10,0" name="restorecfg" text="#restore" autoSize="true" minSize="100,26" maxSize="200,26" inset="5,2,5,2" />
        </UIControl>
    </UIControl>
</UIControl>
<UIControl frame="0,50,485,100%" name="page4" visible="false" align="LinearV">
    <UIControl pos="15,5" align="LinearV" prop="group" autoSize="true" maxSize="455,150" minSize="455,90">
        <UIControl autoSize="true">
            <UIImgBox frame="0,0,18,20" autoSize="false" img="icon_about" />
            <UILabel pos="10,3" text="#about" />
        </UIControl>
        <UILabel pos="0,5" text="#aboutinfo" />
        <UILabel pos="0,5" text="https://github.com/Maplespe/DWMBlurGlass" fontColor="30,144,255,255" hyperlink="true" url="https://github.com/Maplespe/DWMBlurGlass" />
        <UILabel pos="0,5" text="#curlang" />
        <UILabel pos="0,5" name="langauthor" />
		<UILabel frame="0,15,10f,20" text="Copyright © 2023-2024 Maplespe" autoSize="false" textAlign="2" hyperlink="true" url="https://github.com/Maplespe/" />
    </UIControl>
</UIControl>
		)";
        
		if(!ui->CreateUIFromXML(parent, xml))
			throw std::invalid_argument("Invalid XML code!");

		auto navbar = parent->Child<UINavBar>(L"navbar");
		navbar->AddItem(ui->GetStringValue(L"general"));
        navbar->AddItem(ui->GetStringValue(L"advanced"));
		navbar->AddItem(ui->GetStringValue(L"symbol"));
		navbar->AddItem(ui->GetStringValue(L"config"));
		navbar->AddItem(ui->GetStringValue(L"about"));

        navbar = parent->Child<UINavBar>(L"cmodenavbar");
        navbar->AddItem(ui->GetStringValue(L"cmodelight"));
        navbar->AddItem(ui->GetStringValue(L"cmodedark"));

        m_cmodeNavbar = navbar;

        auto combox = parent->Child<UIComBox>(L"blurmethod");
        auto addItem = [&combox](auto&& title)
        {
            ListItem* item = new ListItem();
            item->SetText(title);
            combox->AddItem(item, -1);
        };
        addItem(L"CustomBlur (IComposition)");
        addItem(L"OldBlur (AccentBlurBehind)");
        addItem(L"DWMAPI (SystemBackdrop)");

        combox = parent->Child<UIComBox>(L"effecttype");
        addItem(L"Blur");
        addItem(L"Aero");
        addItem(L"Acrylic");

        auto langinfo = parent->Child<UILabel>(L"langauthor");
        auto info = GetCurrentLangInfo();
        std::wstring str = L"[" + info.local + L"] " + ui->GetStringValue(L"langauthor") + L" ";
        str += info.author;
        langinfo->SetAttribute(L"text", str);

        m_page = parent;
        m_blurValue = m_page->Child<UISlider>(L"blurslider");
        m_effLayer = m_page->Child<UIEffectLayer>(L"preblur");
        m_blurValueLabel = m_page->Child<UILabel>(L"blurvalue");
        m_saveBtn[0] = m_page->Child<UIButton>(L"savecfg");
        m_restBtn = m_page->Child<UIButton>(L"restorecfg");

        m_customBlurValue = m_page->Child<UISlider>(L"titlebarAmountSlider");
        m_customBlurValueLabel = m_page->Child<UILabel>(L"titlebarAmountValue");

        m_luminosityValue = m_page->Child<UISlider>(L"luminositySlider");
        m_luminosityValueLabel = m_page->Child<UILabel>(L"luminosityValue");

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
                else if (_MNAME(L"cmodenavbar"))
                    SwitchColorModePreview(param == 0);
                else
                    ret = false;
	        }
            break;
			case Event_Mouse_LClick:
	        {
                if (_MNAME(L"Titlebar_Close"))
                {
                    SendMessageW(hWnd, WM_CLOSE, 0, 0);
                    return true;
                }
                if (_MNAME(L"Titlebar_Minisize"))
                {
                    ShowWindow(hWnd, SW_MINIMIZE);
                    return true;
                }
                else if (_MNAME(L"install"))
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
                else if (_MNAME(L"savecfg") || _MNAME(L"savecfg1"))
                {
                    SaveConfig(Utils::GetCurrentDir() + L"\\data\\config.ini");
                    SetButtonEnable(false);
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
                    SaveConfig(Utils::GetCurrentDir() + L"\\data\\config.ini");
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
                else if (_MNAME(L"customAmount"))
                {
                    m_cfgData.customAmount = static_cast<UICheckBox*>(control)->GetSel();
                    m_blurValue->SetEnabled(m_cfgData.customAmount);
                    RefreshBlurPreview();
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
                else if (_MNAME(L"titlebarAmountSlider"))
                {
                    m_customBlurValueLabel->SetAttribute(L"text", std::to_wstring((int)param), false);
                    m_cfgData.customBlurAmount = (float)param;
                    RefreshBlurPreview();
                    SetButtonEnable(true);
                    m_page->UpdateLayout();
                }
                else if (_MNAME(L"luminositySlider"))
                {
                    m_luminosityValueLabel->SetAttribute(L"text", std::to_wstring((int)param) + L"%", false);
                    m_cfgData.luminosityOpacity = (float)param / 100.f;
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
                    if(m_cmodeNavbar->GetCurSelItem() == 0)
						m_cfgData.activeTextColor = (_m_color)param;
                    else
                        m_cfgData.activeTextColorDark = (_m_color)param;
                }
                else if (_MNAME(L"inactivetext"))
                {
                    if (m_cmodeNavbar->GetCurSelItem() == 0)
                        m_cfgData.inactiveTextColor = (_m_color)param;
                    else
                        m_cfgData.inactiveTextColorDark = (_m_color)param;
                }
                else if(_MNAME(L"activeBlendColor"))
                {
                    if (m_cmodeNavbar->GetCurSelItem() == 0)
						m_cfgData.activeBlendColor = (_m_color)param;
                    else
                        m_cfgData.activeBlendColorDark = (_m_color)param;
                    SwitchColorModePreview(m_cmodeNavbar->GetCurSelItem() == 0);
                }
                else if (_MNAME(L"inactiveBlendColor"))
                {
                    if (m_cmodeNavbar->GetCurSelItem() == 0)
						m_cfgData.inactiveBlendColor = (_m_color)param;
                    else
                        m_cfgData.inactiveBlendColorDark = (_m_color)param;
                }
                else
                    ret = false;

                if (ret)
                    SetButtonEnable(true);
            }
            break;
        case Event_Mouse_LDown:
            {
                if (_MNAME(L"Titlebar"))
                {
                    SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
                    return true;
                }
                break;
			}
        case Event_ListBox_ItemChanged:
	        {
		        if(_MNAME(L"blurmethod"))
		        {
                    m_cfgData.blurmethod = (blurMethod)param;
                    m_cfgData.effectType = effectType::Blur;
                    SwitchBlurMethod((blurMethod)param);
                    SetButtonEnable(true);
		        }
                else if (_MNAME(L"effecttype"))
                {
                    m_cfgData.effectType = (effectType)param;
                    if (m_cfgData.blurmethod == blurMethod::CustomBlur)
                    {
                        m_page->Child(L"customEffectGroup")->SetEnabled(param != 3, false);
                        m_page->Child(L"effectgroup")->SetSize(15,
                            (m_cfgData.effectType == effectType::Acrylic || m_cfgData.effectType == effectType::Mica) ? 120 : 80, false);
                        //m_page->Child(L"customEffectGroup1")->SetVisible(m_cfgData.effectType == effectType::Acrylic, false);
                        m_page->UpdateLayout();
                    }
                    RefreshBlurPreview();
                    SetButtonEnable(true);
                }
	        }
        default:
            ret = false;
            break;
        }
		return ret;
	}

	bool MainWindowPage::SrcEventProc(MWindowCtx* ctx, const MWndDefEventSource& defcallback, MEventCodeEnum code,
		_m_param param)
	{
        const auto pm = (std::pair<_m_param, _m_param>*)param;
        auto message = static_cast<Window::UIWindowsWnd*>(ctx->Base())->ConvertEventCode(code);

        if (message == WM_ACTIVATE)
        {
            RefreshTopLineColor(LOWORD(pm->first) != WA_INACTIVE);
        }
        return false;
	}

	void MainWindowPage::CreateTitleBar(XML::MuiXML* ui)
	{
        std::wstring_view titlebar = LR"(
<UIControl size="100%,35" align="Absolute">
	<PropGroup id="btnProp" autoSize="false" animate="true" aniAlphaType="true" />
	<UIControl size="100%,1" name="borderColor" />
	<UIControl size="100%,100%" name="Titlebar" align="LinearHL">
		<UIButton size="35,30" style="style_closebtn" name="Titlebar_Close" />
		<UIButton size="35,30" style="style_minbtn" name="Titlebar_Minisize" />
		<UIControl bgColor="150,150,150,255" frame="3,12,1,8" />
		<UIButton style="style_savebtn" frame="3,0,35,30" name="savecfg1" enable="false" />
	</UIControl>
	<UILabel pos="10,10" fontSize="12" name="windTitle" />
</UIControl>
)";
        if (!ui->CreateUIFromXML(m_page, titlebar.data()))
            throw std::invalid_argument("Invalid XML code!");

        m_saveBtn[1] = m_page->Child<UIButton>(L"savecfg1");
        m_page->Child<UILabel>(L"windTitle")->SetAttribute(L"text", m_page->GetParentWin()->GetWindowTitle());

        RefreshTopLineColor(true);
	}

	void MainWindowPage::RefreshTopLineColor(bool active)
	{
        if (os::buildNumber >= 22000)
            return;
        UIBkgndStyle borderColor;
        if (active)
        {
            HKEY hKey = nullptr;
            DWORD dwValue = 0;
            if (RegOpenKeyExW(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\DWM", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                DWORD dwType = REG_DWORD;
                DWORD dwDataSize = sizeof(DWORD);

                RegQueryValueExW(hKey, L"ColorPrevalence", nullptr, &dwType, (LPBYTE)&dwValue, &dwDataSize);

                RegCloseKey(hKey);
            }
            if (dwValue == 1)
            {
                COLORREF color = 0;
                BOOL enable = TRUE;
                DwmGetColorizationColor(&color, &enable);
                //0xAARRGGBB
                BYTE alpha = Color::M_GetAValue(color);
                BYTE blue = Color::M_GetRValue(color);
                BYTE green = Color::M_GetGValue(color);
                BYTE red = Color::M_GetBValue(color);
                //0xAABBGGRR
                borderColor.bkgndColor = Color::M_RGBA(red, green, blue, alpha);
            }
            else
                borderColor.bkgndColor = Color::M_RGBA(107, 107, 107, 255);
        }
        else
        {
            borderColor.bkgndColor = Color::M_RGBA(40, 40, 40, 100);
        }
        m_parent->Child(L"borderColor")->SetBackground(borderColor);
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
        m_saveBtn[0]->SetEnabled(enable);
        m_saveBtn[1]->SetEnabled(enable);
        m_restBtn->SetEnabled(!m_cfgData.isDefault());
    }

    void MainWindowPage::LoadConfig(std::wstring_view path)
    {
        m_cfgData = ConfigData::LoadFromFile(path);

        m_page->Child<UICheckBox>(L"applyglobal")->SetSel(m_cfgData.applyglobal);
        m_page->Child<UICheckBox>(L"extendBorder")->SetSel(m_cfgData.extendBorder);
        m_page->Child<UICheckBox>(L"reflection")->SetSel(m_cfgData.reflection);
        m_page->Child<UICheckBox>(L"oldBtnHeight")->SetSel(m_cfgData.oldBtnHeight);

        m_page->Child<UICheckBox>(L"customAmount")->SetSel(m_cfgData.customAmount, false);

        RefreshBlurPreview();
        SwitchColorModePreview(true);

        m_blurValue->SetCurValue((int)m_cfgData.blurAmount, false);
        m_blurValue->SetEnabled(m_cfgData.customAmount, false);
        m_blurValueLabel->SetAttribute(L"text", std::to_wstring((int)m_cfgData.blurAmount), false);

        m_customBlurValue->SetCurValue((int)m_cfgData.customBlurAmount, false);
        m_customBlurValueLabel->SetAttribute(L"text", std::to_wstring((int)m_cfgData.customBlurAmount), false);

        m_luminosityValue->SetCurValue((int)(m_cfgData.luminosityOpacity * 100.f), false);
        m_luminosityValueLabel->SetAttribute(L"text", std::to_wstring(m_luminosityValue->GetCurValue()) + L"%", false);

        SwitchBlurMethod(m_cfgData.blurmethod);
        m_page->Child<UIComBox>(L"blurmethod")->SetCurSelItem((int)m_cfgData.blurmethod, false);
        m_page->Child<UIComBox>(L"effecttype")->SetCurSelItem((int)m_cfgData.effectType, false);
        bool enableLuminosity = (m_cfgData.effectType == effectType::Acrylic || m_cfgData.effectType == effectType::Mica)
            && m_cfgData.blurmethod == blurMethod::CustomBlur;
        m_page->Child(L"effectgroup")->SetSize(15, enableLuminosity ? 120 : 80, false);
        //m_page->Child(L"customEffectGroup1")->SetVisible(enableLuminosity, false);

        m_page->UpdateLayout();
    }

    void MainWindowPage::SaveConfig(std::wstring_view path)
    {
        ConfigData::SaveToFile(path, m_cfgData);
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
        BroadcastSystemMessageW(BSF_POSTMESSAGE, nullptr, WM_THEMECHANGED, 0, 0);
    }

    void MainWindowPage::RefreshBlurPreview()
    {
        auto blurValue = m_cfgData.customAmount ? m_cfgData.blurAmount : 20.f;
        if (!m_cfgData.customAmount && m_cfgData.blurmethod == blurMethod::CustomBlur && m_cfgData.effectType != effectType::Mica)
            blurValue = m_cfgData.customBlurAmount;
        if (m_cfgData.blurmethod == blurMethod::DWMAPIBlur)
            blurValue = 20.f;
        m_effLayer->SetEffectValue(blurValue);
    }

    void MainWindowPage::SwitchColorModePreview(bool light)
    {
        _m_color textColor = light ? m_cfgData.activeTextColor : m_cfgData.activeTextColorDark;
        m_page->Child<UILabel>(L"sampletitle")->SetAttributeSrc(L"fontColor", textColor, false);
        m_page->Child(L"activetext")->SetAttribute(L"curColor", Color::M_RGBA_STR(textColor), false);

		textColor = light ? m_cfgData.inactiveTextColor : m_cfgData.inactiveTextColorDark;
        m_page->Child(L"inactivetext")->SetAttribute(L"curColor", Color::M_RGBA_STR(textColor), false);

        auto bkgnd = m_effLayer->GetBkgndStyle();
        bkgnd.bkgndColor = light ? m_cfgData.activeBlendColor : m_cfgData.activeBlendColorDark;
        m_effLayer->SetBackground(bkgnd);

        m_page->Child(L"activeBlendColor")->SetAttribute(L"curColor", Color::M_RGBA_STR(bkgnd.bkgndColor), false);
        m_page->Child(L"inactiveBlendColor")->SetAttribute(L"curColor", Color::M_RGBA_STR(
            light ? m_cfgData.inactiveBlendColor : m_cfgData.inactiveBlendColorDark
        ), false);
    }

    void MainWindowPage::SwitchBlurMethod(blurMethod index)
    {
        std::wstring name = L"methodcustom";
        if (index == blurMethod::AccentBlur)
            name = L"methodaccent";
        else if (index == blurMethod::DWMAPIBlur)
            name = L"methoddwmapi";

        auto combox = m_page->Child<UIComBox>(L"effecttype");
        combox->GetItem(0)->SetText(index == blurMethod::DWMAPIBlur ? L"Mica" : L"Blur");
        combox->GetItem(1)->SetText(index == blurMethod::DWMAPIBlur ? L"Acrylic" : L"Aero");
        combox->GetItem(2)->SetText(index == blurMethod::DWMAPIBlur ? L"MicaAlt" : L"Acrylic");
        if (index == blurMethod::CustomBlur)
        {
            auto item = new ListItem();
            item->SetText(L"Mica");
            combox->AddItem(item, -1, false);
        }
        else
            combox->DeleteItem(3);

        combox->SetCurSelItem(0, false);

        RefreshBlurPreview();

        auto text = m_ui->GetStringValue(L"reflection");
        auto reflectionCheckbox = m_page->Child(L"reflection");
        reflectionCheckbox->SetAttribute(L"text", index == blurMethod::AccentBlur ? text + L" " + m_ui->GetStringValue(L"reflectiontip") : text, false);
        reflectionCheckbox->SetAttribute(L"fontColor", index == blurMethod::AccentBlur ? L"236,82,74,255" : L"0,0,0,255", false);

        m_page->Child(L"effectgroup")->SetSize(15, 80, false);
        //m_page->Child(L"customEffectGroup1")->SetVisible(false, false);
        m_page->Child(L"generalGroup")->SetEnabled(index != blurMethod::DWMAPIBlur, false);
        m_page->Child(L"effectgroup")->SetEnabled(index == blurMethod::CustomBlur || index == blurMethod::DWMAPIBlur, false);
        m_page->Child(L"customEffectGroup")->SetEnabled(index == blurMethod::CustomBlur, false);
        m_page->Child<UILabel>(L"functip")->SetAttribute(L"text", m_ui->GetStringValue(name), false);
        m_page->UpdateLayout();
    }
}