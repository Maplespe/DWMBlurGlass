# DWMBlurGlass
Add blur effect to global system title bar, support win10 and win11.

给全局系统标题栏添加模糊效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) |
This project uses [LGNU V3 license](/COPYING.LESSER).

[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Effects
* Adds a blur effect to the global system title bar.
* Customizable global blur radius.
* Customizable blend background colors.
* Customizable title bar text color.
* Third-party theme support.
* Customize the Aero reflection effect.

Please note that the blur radius will affect the global, which also includes locations such as the start menu, notification center, etc...

![image](/Screenshot/001911.png)

## Compatibility
Supported as low as **Windows 1709** and as high as the **latest version of Windows 11** (if not otherwise specified).

Can be used with third party themes to further customize DWM.

We do not modify the rendering logic of the application itself, which is completely different from the logic of MicaForEveryone and therefore maximizes compatibility with third-party programs.

Compatible with [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), works better together.

Compatible with [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**It should be noted that even though this project is compatible with TF, EBMv2 is not fully compatible with TFv2**)

## Catalog
- [Gallery](#gallery)
- [How to use](#how-to-use)
- [Language files](#language-files)
- [Dependencies](#dependencies)

## Gallery
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

> Enable "Override DWMAPI mica effect (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001911.png)

Using third-party themes

> Enable "Extend effects to borders (win10)"
> Enable "Aero reflection effect (win10)"

![image](/Screenshot/025454.png)

</details>

## How to use

### Install
1. Download the compiled program archive from the [Release](https://github.com/Maplespe/DWMBlurGlass/releases) page.
2. Unzip it to a location such as "`C:\Program Files`".
<details><summary><b>3. Run the DWMBlurGlass.exe GUI program and click Install.</b></summary>

![image](/Screenshot/012746.png)

>If the prompt "Installation was successful! But you have not yet downloaded a valid symbol file, please download it from the "Symbols" page before you can use it!" then you need to click on the Symbols page and click Download before you can use it.

>**Note that you may receive similar notifications in the future, especially after system updates.**

![image](/Screenshot/012924.png)

</details>

### Uninstall
1. Run the DWMBlurGlass.exe GUI program and click Uninstall.
2. Delete relevant files

## Language files
We offer basic languages English and Simplified Chinese.
If you would like to help us translate into other languages, please see below for language file formats.

1. First, you need to fork this repository and clone it locally.
2. Open the "`Languagefiles`" folder and select an existing language such as "`en-US.xml`" and make a copy.
3. Rename the code to the name of the [target language](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) and open the xml file in your favorite text editor.
4. In the second line, in the "`local`" field, change it to your target language code, which should be the same as the filename (without the .xml extension).
5. You can put your name in the "`author`" field.
6. Next, please translate the field values in the xml format (be careful not to translate the field names) The correct format is:`<config>Config</config>` to `<config>xxxx</config>`.
7. Save your file when finished and copy it to the "data\lang" directory in the folder where the DWMBlurGlass.exe program is located.
8. Next, open DWMBlurGlass.exe and test the language file to see if it works correctly. If it doesn't, check the language code settings and check that the file conforms to the xml format specification.
9. Finally, commit the file to your own forked repository and send pull requests to the main branch of the project.
10. After the request is approved, your file will be released with a future software update.
   

## Dependencies
* MiaoUI interface library v2 (Private)
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Since the GUI program uses private libraries, only insiders can compile the GUI, which does not affect the compilation of the libraries.
