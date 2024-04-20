# DWMBlurGlass
Add custom effects to the global system title bar, supports Windows 10 and Windows 11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [Italian](/README_IT.md) | [French](/README_FR.md) | [Türkçe](/README_TR.md)
This project uses [LGNU V3 license](/COPYING.LESSER).

Other languages:
> The following documentation may be out of date, please refer to the current documentation.
>
> [Deutsch](/README_DE.md)


[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Catalog
- [Effects](#effects)
- [Compatibility](#compatibility)
- [Gallery](#gallery)
- [Material Effects](#material-effects)
  - [Blur](#blur)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [How to use](#how-to-use)
  - [Install](#install)
  - [Uninstall](#uninstall)
- [Language files](#language-files)
- [Dependencies](#dependencies)

## Effects
* Adds a custom effect to the global system title bar.
* Customizable global blur radius or title bar blur radius only.
* Customizable title bar blend colors.
* Customizable title bar text color.
* Aero reflections and parallax effects are available.
* Restore Windows 7 style title bar button height.
* Support to enable blur effect for programs using old Windows 7 API DwmEnableBlurBehindWindow.
* Supports `Blur`, `Aero`, `Acrylic`, and `Mica (Win11 only)` effects.
* Individually customizable Light/Dark color mode automatic switching.
* `CustomBlur`, `AccentBlur` and `SystemBackdrop` blurring methods are available.
* Third-party theme support.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Compatibility
Supported as low as **Windows 10 2004** and as high as the **latest version of Windows 11** (Some blurring methods are not supported in Windows Insider versions).

Can be used with third party themes to further customize DWM.

We do not modify the rendering logic of the application itself, which is completely different from the logic of MicaForEveryone and therefore maximizes compatibility with third-party programs.

We reverse-analyzed DWM and created a custom blur method to bring stunning visual effects, but if you choose the "`SystemBackdrop`" blur method, it uses the system's publicly available interfaces and has the same effect as MicaForEveryone.

Not recommended for use with MicaForEveryone, we do not guarantee compatibility with it.

Compatible with [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), works better together.

Compatible with [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**It should be noted that even though this project is compatible with TF, EBMv2 is not fully compatible with TFv2**)

## Gallery
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> Enable "Override DWMAPI mica effect (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Using third-party themes

> Enable "Extend effects to borders (win10)"

> Enable "Aero reflection effect (win10)"

> Enable "Reduce title bar button height (win7 style)"

![image](/Screenshot/025410.png)

</details>

## Material Effects
### Blur
> Basic pure blur. Nothing special.

![image](/Screenshot/blur.png)

### Aero
> Windows 7's glass effect, with saturation and exposure effects on the background when a window is inactive.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> The acrylic recipe: background, blur, exclusion blend, saturation, color/tint overlay and noise.

![image](/Screenshot/acrylic.png)

### Mica
> The Mica recipe: blurred wallpaper, saturation and color/tint overlay.

![image](/Screenshot/mica.png)

### MicaAlt
All of the above effects can be customized to blend colors.

MicaAlt is Mica with a grayish tone, you can modify the blend color by yourself to get the MicaAlt effect.

## How to use

### Install
1. Download the compiled program archive from the [Release](https://github.com/Maplespe/DWMBlurGlass/releases) page.
2. Unzip it to a location such as "`C:\Program Files`".
<details><summary><b>3. Run the DWMBlurGlass.exe GUI program and click Install.</b></summary>

![image](/Screenshot/012746.png)

>If nothing happens when you click Install, then you need to click on the Symbols page and click Download.

>**You may receive a notification about missing symbols in the future, especially after system updates.**

![image](/Screenshot/012924.png)

</details>

### Uninstall
1. Run the DWMBlurGlass.exe GUI program and click Uninstall.
2. Delete relevant files

## Language files
We offer several languages, such as English, Simplified Chinese, Spanish, Portuguese and more.
If you would like to help us translate into other languages, please see below for language file formats.

1. First, you need to fork this repository and clone it locally.
2. Open the "`Languagefiles`" folder and select an existing language such as "`en-US.xml`" and make a copy.
3. Rename the code to the name of the [target language](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) and open the xml file in your favorite text editor.
4. In the second line, in the "`local`" field, change it to your target language code, which should be the same as the filename (without the .xml extension).
5. You can put your name in the "`author`" field.
6. Next, please translate the field values in the xml format (be careful not to translate the field names) The correct format is:`<config>Config</config>` to `<config>xxxx</config>`.
7. Save your file when finished and copy it to the "data\lang" directory in the folder where the DWMBlurGlass.exe program is located.
8. Next, open DWMBlurGlass.exe and test the language file to see if it works correctly. If it doesn't, check the language code settings and check that the file conforms to the xml format specification.
9. Finally, commit the file to your own forked repository and send a pull request to the main branch of the project.
10. After the request is approved, your file will be released with a future software update.
   

## Dependencies
* MiaoUI interface library v2 (Private)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Separate upstream implementation of the CustomBlur method, thanks to ALTaleX for research and support.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Since the GUI program uses private libraries, only insiders can compile the GUI, which does not affect the compilation of the libraries.
