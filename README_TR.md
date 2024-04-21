# DWMBlurGlass
Küresel sistem başlık çubuğuna özel etkiler ekler, Windows 10 ve Windows 11'i destekler.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [Italiano](/README_IT.md) | [Français](/README_FR.md) | [Türkçe](/README_TR.md)
Bu proje [LGNU V3 lisansı](/COPYING.LESSER) kullanır.

Diğer diller:
> Aşağıdaki belgeleme eskimiş olabilir, lütfen şu anki belgelemeye bakın.
>
> [Deutsch](/README_DE.md)


[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Katalog
- [Etkiler](#Etkiler)
- [Uyumluluk](#Uyumluluk)
- [Geleri](#Geleri)
- [Materyal Etkileri](#Materyal-Etkileri)
  - [Buğu](#Buğu)
  - [Aero](#Aero)
  - [Acrylic](#Acrylic)
  - [Mica](#Mica)
  - [MicaAlt](#MicaAlt)
- [Nasıl kullanılır](#Nasıl-kullanılır)
  - [Kur](#Kur)
  - [Kaldır](#Kaldır)
- [Dil dosyaları](#Dil-dosyaları)
- [Bağımlılıklar](#Bağımlılıklar)

## Etkiler
* Küresel sistem başlık çubuğuna özel bir etki ekler.
* Özelleştirilebilir küresel buğu yarıçağı veya yalnızca başlık çubuğu buğu yarıçağı.
* Özelleştirilebilir başlık çubuğu karışım renkleri.
* Özelleştirilebilir başlık çubuğu metin rengi.
* Aero yansımaları ve ıraklık açısı etkileri mevcut.
* Windows 7 biçeminde başlık çubuğu buton yüksekliği seçeneği.
* Eski Windows 7 API DwmEnableBlurBehindWindow kullanan programlar için buğu etkisini etkinleştirme desteği.
* `Buğu`, `Aero`, `Acrylic`, ve `Mica (Yalnızca Win11)` etkilerini desktekler.
* Tek tek özelleştirilebilen otomatki Açık/Koyu renk modu arasında değiştirme.
* `CustomBlur`, `AccentBlur` ve `SystemBackdrop` buğulaştırma yöntemleri mevcut.
* Üçüncü parti tema desteği.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Uyumluluk
En düşük **Windows 10 2004** ve en yüksek **Windows 11'in en son sürümü**ne kadar desteklenir (Bazı buğulaştırma yöntemleri Windows Insider sürümlerinde desteklenmez).

DWM'yi daha da özelleştirmek için üçüncü taraf temalar ile kullanılabilir.

Uygulamanın kendisinin iş mantığını değiştirmeyiz, bu MicaForEveryone'ın mantığından tamamen farklıdır ve bu sayede üçüncü parti programlarla uyumluluğu en üst düzeye çıkarır.

DWM'yi tersine mühendislikle analiz ettik ve şaşırtıcı görsel etkiler getiren özel bir buğulaştırma yöntemi oluşturduk ancak "`SystemBackdrop`" buğulaştırma yöntemini seçerseniz, sistemdeki herkese açık arayüzleri kullanır ve MicaForEveryone ile aynı etkiye olur.

MicaForEveryone ile birlikte kullanılması önerilmez, uyumluluğunu garanti etmeyiz.

[ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica) ile uyumludur, birlikte daha iyi çalışır.

[TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts) ile uyumludur. (**Bu projenin TF ile uyumlu olduğunu hatırlamakla beraber, EBMv2'nin TFv2 ile tam uyumlu olmadığına dikkat edilmelidir**).

## Galeri
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> "DWMAPI etkisini geçersiz kıl (Windows 11)" açıkken

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Üçüncü parti tema kullanımı

> Etkiyi kenarlara genişlet (Windows 10) açıkken"

> "Aero yansıma etkisini etkinleştir" açıkken

> "Windows 7 biçeminde başlık çubuğu butonlarını etkinleştir" açıkken

![image](/Screenshot/025410.png)

</details>

## Materyal Etkileri
### Buğu
> Temel saf buğu. Özel bir şey değil.

![image](/Screenshot/blur.png)

### Aero
> Windows 7'nin cam etkisi, bir pencere etkin olmadığında arka planda doygunluk ve pozlama etkileri ile.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> Acrylic tarifi: arka plan, bulanıklık, dışlama karışımı, doygunluk, renk/ton kaplama ve gürültü.

![image](/Screenshot/acrylic.png)

### Mica
> Mika tarifi: bulanık duvar kağıdı, doygunluk ve renk/ton kaplama.

![image](/Screenshot/mica.png)

### MicaAlt
Yukarıdaki tüm etkiler renkleri karıştırmak için özelleştirilebilir.

MicaAlt, gri tonlamalı Mica'dır, MicaAlt etkisini elde etmek için karışım rengini kendiniz değiştirebilirsiniz.

## Nasıl kullanılır

### Kur
1. Derlenmiş program arşivini [Yayın](https://github.com/Maplespe/DWMBlurGlass/releases) sayfasından indirin.
2. "`C:\Program Files`" gibi bir konuma çıkartın.

<details><summary><b>3. DWMBlurGlass.exe GUI programını çalıştırın ve Yükle'ye tıklayın.</b></summary>

![image](/Screenshot/012746.png)

>Yükle'ye tıkladığınızda hiçbir şey olmazsa, Semboller sayfasına ve oradan İndir'e tıklamanız gerekmektedir.

>**Gelecekte, özellikle sistem güncellemelerinden sonra eksik sembollerle ilgili bir bildirim alabilirsiniz.**

![image](/Screenshot/012924.png)

</details>

### Kaldır
1. DWMBlurGlass.exe GUI programını çalıştırın ve Kaldır'a tıklayın.
2. İlgili dosyaları silin

## Dil dosyaları
İngilizce, Basitleştirilmiş Çince, İspanyolca, Portekizce ve daha fazla diller sunuyoruz.
Başka dillere çeviri yapmamıza yardımcı olmak isterseniz, lütfen dil dosyalarının biçimi için aşağıya göz atın.

1. İlk olarak, bu depoyu çatallamanız ve yerel olarak klonlamanız gerekiyor.
2. "`Languagefiles`" klasörünü açın ve "`en-US.xml`" gibi mevcut bir dili seçin ve bir kopyasını oluşturun.
3. Dosya adını [hedef dilin](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) adına göre değiştirin ve xml dosyasını favori metin düzenleyicinizde açın.
4. İkinci satırda, "`local`" alanını hedef dil kodunuz olarak değiştirin, bu kod dosya adı ile aynı olmalıdır (.xml uzantısı olmadan).
5. "`author`" alanına adınızı yazabilirsiniz.
6. Sonraki adımda, lütfen xml biçimindeki alan değerlerini çevirin (alan adlarını çevirmemeye dikkat edin). Doğru biçim şöyledir:`<config>Config</config>` yerine `<config>xxxx</config>`.
7. İşiniz bittiğinde dosyanızı kaydedin ve "data\lang" dizinine kopyalayın, bu dizin DWMBlurGlass.exe programının bulunduğu klasördedir.
8. Sonra, DWMBlurGlass.exe'yi açın ve dil dosyasının doğru çalışıp çalışmadığını sınayın. Çalışmıyorsa, dil kodu ayarlarını kontrol edin ve dosyanın xml formatına uygun olduğundan emin olun.
9. Son olarak, dosyayı kendi çatallanmış depoya işleyin ve projenin ana dalına bir çekme isteği gönderin.
10. İsteğiniz onaylandıktan sonra dosyanız gelecek bir yazılım güncellemesiyle yayınlanacaktır.
   

## Bağımlılıklar
* MiaoUI arayüz kütüphanesi v2 (Gizli)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - CustomBlur yönteminin ayrı bir üst akım uygulaması, araştırma ve destek için ALTaleX'e teşekkürler.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Gerçekleme Kütüphaneleri](https://github.com/Microsoft/wil)

GUI programı özel kütüphaneler kullandığı için, sadece içeriden olanlar GUI'yi derleyebilir. Bu durum kütüphanelerin derlenmesini etkilemez.
