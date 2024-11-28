# DWMBlurGlass
Fügen Sie benutzerdefinierte Effekte zur Titelleiste des globalen Systems hinzu, unterstützt Windows 10 und Windows 11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [italiano](/README_IT.md) | [français](/README_FR.md) | [Türkçe](/README_TR.md) | [español](/README_ES.md)
This project uses [LGNU V3 license](/COPYING.LESSER).

## !!! Laden Sie DWMBlurGlass nicht von irgendwo anders herunter!!!!
> [!WARNING]
> Wir haben festgestellt, dass jemand vorgibt, wir zu sein und DWMBlurGlass mit einem Schadcode-Implantat postet.
> 
> Um zu verhindern, dass so etwas noch einmal passiert, laden Sie die Software bitte nicht von inoffiziellen Adressen herunter!
> 
> **Wir haben auch keinen offiziellen Discord.**
> 
> Wir vertreiben ausschließlich Software auf [Github](https://github.com/Maplespe/DWMBlurGlass/releases), [Bilibili](https://space.bilibili.com/87195798) Und [winmoes](https://winmoes.com).
> 
> Außerdem werden alle neuen Versionen zum Testen zuerst in den Testzweig verschoben, anstatt Binärdateien im Voraus zu veröffentlichen.

[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Katalog
- [Effekte](#Effekte)
- [Kompatibilität](#Kompatibilität)
- [Galerie](#Galerie)
- [Material-Effects](#material-effects)
  - [Unschärfe](#blur)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [Verwendung](#Verwendung)
  - [Installieren](#Installieren)
  - [Deinstallieren](#Deinstallieren)
- [Sprachdateien](#Sprachdateien)
- [Abhängigkeiten](#Abhängigkeiten)

## Effekte
* Fügt einen benutzerdefinierten Effekt zur Titelleiste des globalen Systems hinzu.
* Anpassbarer globaler Unschärferadius oder nur Unschärferadius der Titelleiste.
* Anpassbare Mischfarben für die Titelleiste.
* Anpassbare Textfarbe für die Titelleiste.
* Aeroreflexionen und Parallaxeneffekte sind verfügbar.
* Stellen Sie die Höhe der Titelleistenschaltflächen im Windows 7-Stil wieder her.
* Unterstützung zum Aktivieren des Unschärfeeffekts für Programme, die die alte Windows 7-API (DwmEnableBlurBehindWindow) verwenden.
* Unterstützt die Effekte „Blur“, „Aero“, „Acrylic“ und „Mica (nur Win11)“.
* Individuell anpassbare automatische Umschaltung zwischen Hell-/Dunkel-Farbmodus.
* Die Unschärfemethoden „CustomBlur“, „AccentBlur“ und „SystemBackdrop“ sind verfügbar.
* Theme-Unterstützung von Drittanbietern.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Kompatibilität
Unterstützt ab **Windows 10 2004** und bis zur **neuesten Version von Windows 11** (Einige Unschärfemethoden werden in Windows Insider-Versionen nicht unterstützt).

Kann mit Designs von Drittanbietern verwendet werden, um DWM weiter anzupassen.

Wir ändern nicht die Rendering-Logik der Anwendung selbst, die sich völlig von der Logik von "MicaForEveryone" unterscheidet und daher die Kompatibilität mit Programmen von Drittanbietern maximiert.

Wir haben DWM umgekehrt analysiert und eine benutzerdefinierte Unschärfemethode erstellt, um atemberaubende visuelle Effekte zu erzielen. Wenn Sie sich jedoch für die Unschärfemethode „SystemBackdrop“ entscheiden, verwendet sie die öffentlich verfügbaren Schnittstellen des Systems und hat den gleichen Effekt wie MicaForEveryone.

Die Verwendung mit MicaForEveryone wird nicht empfohlen, wir garantieren keine Kompatibilität damit.

Kompatibel mit [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), funktioniert besser zusammen.

Kompatibel mit [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Es ist zu beachten, dass EBMv2, obwohl dieses Projekt mit TF kompatibel ist, nicht vollständig mit TFv2 kompatibel ist**)

## Galerie
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> Aktivieren Sie „DWMAPI-Mica-Effekt überschreiben (win11)“

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Verwendung von Themes von Drittanbietern

> Aktivieren Sie „Effekte auf Ränder erweitern (win10)“

> Aktivieren Sie „Aero-Reflexionseffekt (Win10)“

> Aktivieren Sie „Höhe der Titelleistenschaltfläche reduzieren (Win7-Stil)“

![image](/Screenshot/025410.png)

</details>

## Material Effects
### Unschärfe
> Grundlegende reine Unschärfe. Nichts Besonderes.

![image](/Screenshot/blur.png)

### Aero
> Der Glaseffekt von Windows 7 mit Sättigungs- und Belichtungseffekten im Hintergrund, wenn ein Fenster inaktiv ist.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> Das Acrylrezept: Hintergrund, Unschärfe, Ausschlussmischung, Sättigung, Farb-/Tönungsüberlagerung und Rauschen.

![image](/Screenshot/acrylic.png)

### Mica
> Das Mica-Rezept: verschwommenes Hintergrundbild, Sättigung und Farb-/Tönungsüberlagerung.

![image](/Screenshot/mica.png)

### MicaAlt
Alle oben genannten Effekte können angepasst werden, um Farben zu mischen.

MicaAlt ist Glimmer mit einem gräulichen Farbton. Sie können die Mischfarbe selbst ändern, um den MicaAlt-Effekt zu erzielen.

## Verwendung

### Installieren
1. Laden Sie das kompilierte Programmarchiv von herunter [Release](https://github.com/Maplespe/DWMBlurGlass/releases) Seite.
2. Entpacken Sie es an einen Speicherort wie „C:\Programme“.
<details><summary><b>3. Führen Sie das GUI-Programm DWMBlurGlass.exe aus und klicken Sie auf „Installieren“.</b></summary>

![image](/Screenshot/012746.png)

>Wenn beim Klicken auf „Installieren“ nichts passiert, müssen Sie auf die Seite „Symbole“ und dann auf „Herunterladen“ klicken.

>**Möglicherweise erhalten Sie in Zukunft eine Benachrichtigung über fehlende Symbole, insbesondere nach Systemaktualisierungen.**

![image](/Screenshot/012924.png)

</details>

### Deinstallieren
1. Führen Sie das GUI-Programm DWMBlurGlass.exe aus und klicken Sie auf Deinstallieren.
2. Löschen Sie relevante Dateien

## Sprachdateien
Wir bieten mehrere Sprachen an, darunter Englisch, vereinfachtes Chinesisch, Spanisch, Portugiesisch und mehr.
Wenn Sie uns bei der Übersetzung in andere Sprachen helfen möchten, finden Sie unten Informationen zu den Sprachdateiformaten.

1. Zuerst müssen Sie dieses Repository forken und lokal klonen.
2. Öffnen Sie den Ordner „Languagefiles“, wählen Sie eine vorhandene Sprache wie „en-US.xml“ aus und erstellen Sie eine Kopie.
3. Benennen Sie den Code in den Namen der [Zielsprache] um (https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) und öffnen Sie die XML-Datei in Ihrem bevorzugtem Texteditor.
4. Ändern Sie in der zweiten Zeile im Feld „`local`“ den Code in Ihren Zielsprachencode, der mit dem Dateinamen identisch sein sollte (ohne die Erweiterung .xml).
5. Sie können Ihren Namen in das Feld „Autor“ eingeben.
6. Als nächstes übersetzen Sie bitte die Feldwerte in das XML-Format (achten Sie darauf, die Feldnamen nicht zu übersetzen).
Das korrekte Format ist: „<config>Config</config>“ in „<config>xxxx</config>“.
7. Speichern Sie Ihre Datei, wenn Sie fertig sind, und kopieren Sie sie in das Verzeichnis „data\lang“ in dem Ordner, in dem sich das Programm DWMBlurGlass.exe befindet.
8. Öffnen Sie als nächstes DWMBlurGlass.exe und testen Sie die Sprachdatei, um festzustellen, ob sie ordnungsgemäß funktioniert. Wenn dies nicht der Fall ist, überprüfen Sie die Sprachcodeeinstellungen und stellen Sie sicher, dass die Datei der XML-Formatspezifikation entspricht. (Hierfür können Sie bspw. den Onlinedienst www.xmlvalidation.com nutzen)
9. Übertragen Sie die Datei abschließend in Ihr eigenes geforktes Repository und senden Sie die Pull-Anfrage an den Hauptzweig des Ursprünglichen Projekts.
10. Nachdem die Anfrage genehmigt wurde, wird ihre Datei mit einem zukünftigen Software-Update veröffentlicht.
   

## Abhängigkeiten
* [MiaoUI Lite interface library v2](https://github.com/Maplespe/MiaoUILite)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Separate Upstream-Implementierung der CustomBlur-Methode, danke an ALTaleX für Recherche und Unterstützung.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)