# DWMBlurGlass
Thêm hiệu ứng tuỳ chỉnh lên thanh tiêu đề toàn hệ thống, hỗ trợ Windows 10 và Windows 11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [italiano](/README_IT.md) | [français](/README_FR.md) | [Türkçe](/README_TR.md) | [español](/README_ES.md) | [Tiếng Việt](/README_VI.md)
Dự án này sử dụng [Giấy phép LGNU V3](/COPYING.LESSER).

Các ngôn ngữ khác:
> Các tài liệu sau có thể đã lỗi thời, vui lòng tham khảo bản hiện tại.
>
> [Deutsch](/README_DE.md)

## !!! Không tải DWMBlurGlass từ bất kỳ nơi nào khác!!!!
> [!WARNING]
> Chúng tôi đã phát hiện có người đang mạo danh chúng tôi và phát hành DWMBlurGlass đính kèm mã độc.
> 
> Để tránh tình trạng này tiếp diễn, vui lòng không tải phần mềm từ các nguốn không chính thống!
> 
> **Chúng tôi cũng không có Discord chính thức nào cả.**
> 
> Chúng tôi chỉ phát hành phần mềm trên [Github](https://github.com/Maplespe/DWMBlurGlass/releases), [Bilibili](https://space.bilibili.com/87195798) và [winmoes](https://winmoes.com).
> 
> Đồng thời, bất kỳ phiên bản thử nghiệm mới nào đều được đẩy qua nhánh thử nghiệm trước, thay vì được phát hành luôn.

[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Mục lục
- [Hiệu ứng](#effects)
- [Tương thích](#compatibility)
- [Thư viện ảnh](#gallery)
- [Hiệu ứng vật liệu](#material-effects)
  - [Blur](#blur)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [Cách dùng](#how-to-use)
  - [Cài đặt](#install)
  - [Gỡ cài đặt](#uninstall)
- [Tập tin ngôn ngữ](#language-files)
- [Phụ thuộc](#dependencies)

## Hiệu ứng
* Thêm hiệu ứng tuỳ chỉnh lên thanh tiêu đề toàn hệ thống.
* Tuỳ chỉnh được bán kính làm mờ chung hoặc riêng thanh tiêu đề.
* Tuỳ chỉnh được màu blend thanh tiêu đề.
* Tuỳ chỉnh được màu chữ thanh tiêu đề.
* Có các hiệu ứng phản chiếu Aero và di chuyển thị sai.
* Khôi phục kích thước nút thanh tiêu đề kiểu Windows 7.
* Hỗ trợ kích hoạt hiệu ứng làm mờ cho chương trình sử dụng API DwmEnableBlurBehindWindow cũ từ Windows 7.
* Hỗ trợ các hiệu ứng `Blur`, `Aero`, `Acrylic`, và `Mica (chỉ Win11)`.
* Tuỳ chỉnh được riêng từng chế độ màu Sáng/Tối tự động.
* Các phương thức làm mờ `CustomBlur`, `AccentBlur` và `SystemBackdrop` có sẵn.
* Hỗ trợ chủ đề bên thứ ba.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Tương thích
Hỗ trợ từ **Windows 10 2004** đến **phiên bản mới nhất của Windows 11** (Một vài phương thức làm mờ không được hỗ trợ trên các phiên bản Windows Insider).

Có thể dùng chung với chủ đề bên thứ ba để tuỳ chỉnh thêm DWM.

Chúng tôi không thay đổi logic render của ứng dụng, khác hoàn toàn logic của MicaForEveryone và vì thế tối đa hóa khả năng tương thích với ứng dụng bên thứ ba.

Chúng tôi đã phân tích ngược DWM và tạo ra phương thức mờ riêng để mang lại hiệu ứng trực quan tuyệt vời, nhưng nếu bạn chọn phương thức "`SystemBackdrop`", nó sử dụng những gì hệ thống có sẵn công khai và hiệu ứng giống MicaForEveryone.

Không khuyến nghị sử dụng chung với MicaForEveryone, chúng tôi không đảm bảo tương thích với nó.

Tương thích với [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), tốt hơn khi dùng chung.

Tương thích với [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Lưu ý rằng mặc dù dự án này tương thích với TF, EBMv2 không hoàn toàn tương thích với TFv2**)

## Thư viện ảnh
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> Kích hoạt "Ghi đè hiệu ứng DWMAPI (Windows 11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Sử dụng chủ đề bên thứ ba

> Kích hoạt "Mở rộng hiệu ứng ra viền cửa sổ (Windows 10)"

> Kích hoạt "Bật hiệu ứng phản chiếu Aero"

> Kích hoạt "Khôi phục kích thước nút trên thanh tiêu đề kiểu Win7"

![image](/Screenshot/025410.png)

</details>

## Hiệu ứng vật liệu
### Blur
> Làm mờ cơ bản. Không có gì đặc biệt.

![image](/Screenshot/blur.png)

### Aero
> Hiệu ứng kính của Windows 7, với màu with hiệu ứng bão hòa và độ phơi sáng sau nền khi một của sổ không hoạt động.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> Công thức acrylic: nền, làm mờ, hòa trộn loại trừ, bão hòa, phủ màu/lớp màu và nhiễu.

![image](/Screenshot/acrylic.png)

### Mica
> Công thức Mica: hình nền mờ, bão hòa và phủ màu/lớp màu.

![image](/Screenshot/mica.png)

### MicaAlt
Tất cả các hiệu ứng trên có thể tuỳ chỉnh tới các màu blend.

MicaAlt là Mica với tone màu xám, bạn có thể tự chỉnh màu blend để có hiệu ứng MicaAlt.

## Cách dùng

### Cài đặt
1. Tải tập tin nén lưu trữ chương trình đã biên soạn từ trang [Release](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Giải nén nó ra một vị trí ví dụ "`C:\Program Files`".
<details><summary><b>3. Chạy chương trình GUI DWMBlurGlass.exe và nhấn Cài đặt.</b></summary>

![image](/Screenshot/012746.png)

>Nếu không có gì xảy ra sau khi bạn nhấn cài đặt, thì bạn cần phải nhấn vào thẻ Symbol và nhấn Tải về.

>**Bạn có thể nhận một thông báo về việc thiếu symbol trong tương lai, nhất là sau khi cập nhật hệ thống.**

![image](/Screenshot/012924.png)

</details>

### Gỡ cài đặt
1. Chạy chương trình GUI DWMBlurGlass.exe và nhấn Gỡ cài đặt.
2. Xoá các tập tin liên quan

## Tập tin ngôn ngữ
Chúng tôi cung cấp một vài ngôn ngữ, chẳng hạn Tiếng Anh, Tiếng Trung Giản thểe, Tiếng Tây Ban Nha, Tiếng Bồ Đào Nha và hơn nữa.
Nếu bạn muốn giúp chúng tôi phiên dịch sang các ngôn ngữ khác, vui lòng xem bên dưới về định dạng tập tin ngôn ngữ.

1. Đầu tiên, bạn cần fork kho lưu trữ này và sao chép (clone) nó xuống thiết bị cục bộ.
2. Mở thư mục "`Languagefiles`" và tạo một bản sao của một ngôn ngữ có sẵn như "`en-US.xml`".
3. Đổi tên mã ngôn ngữ sang [ngôn ngữ của bạn](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) và mở tập tin xml bằng trình chỉnh sửa văn bản ưa thích của bạn.
4. Ở dòng thứ hai, trong mục "`local`", đổi nó thành mã ngôn ngữ đích của bạn, giống như tên tập tin (ngoại trừ phần mở rộng .xml).
5. Bạn có thể đặt tên mình vào mục "`author`".
6. Tiếp theo, vui lòng dịch các giá trị trong từng mục theo định dạng xml (chú ý không dịch luôn tên các mục) Định dạng đúng là:`<config>Config</config>` sang `<config>xxxx</config>`.
7. Lưu tập tin của bạn sau khi xong và sao chép nó vào thư mục "data\lang" nơi chứa chương trình DWMBlurGlass.exe.
8. Tiếp theo, mở DWMBlurGlass.exe và kiểm tra xem tập tin ngôn ngữ có hoạt động đúng không. Nếu không, kiểm tra cài đặt mã ngôn ngữ và tập tin theo đúng cấu trúc định dạng xml.
9. Cuối cùng, tải tập tin lên kho lưu trữ đã fork của bạn và gửi yêu cầu pull lên nhánh main của dự án.
10. Sau khi yêu cầu được chấp thuận, tập tin của bạn sẽ được phát hành trong một phiên bản cập nhật phần mềm trong tương lai.
   

## Phụ thuộc
* MiaoUI interface library v2 (Riêng tư)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Phương thức upstream riêng biệt của phương pháp CustomBlur, cảm ơn ALTaleX vì nghiên cứu và hỗ trợ.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Vì chương trình GUI sử dụng thư viện riêng tư, chỉ người nội bộ mới có thể biên dịch GUI, điều đó không ảnh hưởng việc biên dịch thư viện.
