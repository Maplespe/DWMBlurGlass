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

## 效果
* 为全局系统标题栏添加模糊效果
* 可自定义全局模糊强度
* 可自定义混合背景颜色
* 可自定义标题栏文本颜色
* 第三方主题支持
* 自定义Aero反射底图效果
* 可还原win7样式标题栏按钮高度
* 支持为使用旧版win7API DwmEnableBlurBehindWindow 的程序启用模糊效果

请注意 模糊强度将影响全局 例如开始菜单、通知中心等位置...

![image](/Screenshot/001701.png)

## 兼容性
最低支持 **Windows 10 2004** 最高支持到 **Windows 11 最新版**(不包括预览版)

可以与第三方主题一起使用进一步定制DWM.

我们不会修改应用程序本身的渲染逻辑，与MicaForEveryone原理完全不同，因此也最大程度与第三方程序兼容.

与[ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica)兼容，一起使用效果更佳.

与[TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts)兼容. (**需要注意的是即使本项目与TF兼容，但EBMv2与TFv2并不完全兼容**)

## 目录
- [预览](#预览)
- [如何使用](#如何使用)
- [语言文件](#语言文件)
- [依赖](#依赖)

## 预览
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/78930.png)

> 启用 "覆盖使用DWMAPI设置的云母效果(win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

使用第三方主题

> 启用 "扩展效果到边框 (win10)"
> 启用 "Aero反光效果 (win10)"

![image](/Screenshot/025410.png)

</details>

## 如何使用

### 安装
1. 从 [Release](https://github.com/Maplespe/DWMBlurGlass/releases) 页面下载压缩包.
2. 解压到一个位置 例如 "`C:\Program Files`".
<details><summary><b>3. 运行 DWMBlurGlass.exe GUI 程序 并点击安装.</b></summary>

![image](/Screenshot/013025.png)

>如果提示 "安装成功! 但您还没有下载有效的符号文件，在"符号文件"页面下载后才能生效" 则需要前往"符号"页面下载符号

>**请注意，之后可能还会收到类似提示，特别是系统更新后 届时下载新的符号即可**
>
>如果下载失败 则可能需要使用代理，在中国大陆部分地区可能无法访问微软服务器

![image](/Screenshot/013100.png)

</details>

### 卸载
1. 运行 DWMBlurGlass.exe GUI 程序 并点击卸载.
2. 手动删除相关剩余文件.

#### 更新
1. 通过图形用户界面程序卸载 DWMBlurGlass，因为它会解除与 DWM 的连接。
2. 下载 GitHub 上的最新版本
3. 将其解压到安装 DWMBlurGlass 的位置（例如 C:/Program Files/DWMBlurGlass_x64）
4. 重新安装 DWMBlurGlass。它可能会保留您的首选项。


## 语言文件
我们提供基本的语言 英语和简体中文 如果您愿意帮助我们翻译其他语言，请参阅以下内容，了解语言文件格式.
1. 首先需要fork并克隆此储存库到本地.
2. 打开"`Languagefiles`"文件夹 并选择一个现有的语言文件 例如"`zh-CN.xml`" 并复制它.
3. 重命名此文件为[目标区域代码名称](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names)并使用您喜欢的文本编辑器编辑此xml文档.
4. 修改第二行的"`local`"字段的值为目标区域代码名称(它应该和文件名一样 但不包括xml后缀).
5. 可以在"`author`"字段中填写您的名称.
6. 接下来翻译剩下的字段值(注意不要翻译字段名称) 正确的格式为: `<config>Config</config>` 翻译为 `<config>xxxx</config>`.
7. 完成后保存文件，并将其复制到 DWMBlurGlass.exe 程序所在文件夹下的 "`data\lang`" 目录中.
8. 接下来，打开 DWMBlurGlass.exe，测试语言文件是否正常工作，如果不正常，请检查语言代码设置，并检查文件是否符合 xml 格式规范.
9. 最后，将文件提交到自己fork的仓库，并向项目的主分支发送拉取请求.
10. 请求通过后，您的文件将随未来的软件更新一起发布.

## 依赖
* MiaoUI interface library v2 (Private)
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

由于GUI程序使用了私有的库，因此只有内部人员才可以编译GUI，这并不影响库的编译.
