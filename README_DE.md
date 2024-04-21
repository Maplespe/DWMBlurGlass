# DWMBlurGlass
Fügen Sie der Titelleiste des globalen Systems einen Unschärfeeffekt hinzu. Unterstützung für Win10 und Win11.

#
| [中文](/README_ZH.md) | [English](/README.md) | [Italiano](/README_IT.md) | [Français](/README_FR.md) | [Türkçe](/README_TR.md)
This project uses [LGNU V3 license](/COPYING.LESSER).

[![license](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Effekte
* Fügt der Titelleiste des globalen Systems einen Unschärfeeffekt hinzu.
* Anpassbarer globaler Unschärferadius.
* Anpassbare Hintergrundfarbenmischung.
* Anpassbare Textfarbe für die Titelleiste.
* Unterstützung für Themes von Drittanbietern.
* Passen Sie den Aero-Reflexionseffekt an.
* Stellen Sie die Höhe der Titelleistenschaltfläche im Win7-Stil wieder her.
* Unterstützung zum Aktivieren des Unschärfeeffekts für Programme, die die alte Win7-API DwmEnableBlurBehindWindow verwenden.

Bitte beachten Sie, dass der Unschärferadius globale Auswirkungen hat, was auch Orte wie das Startmenü, das Benachrichtigungscenter usw. umfasst.

![image](/Screenshot/001911.png)
![image](/Screenshot/10307.png)

## Kompatibilität
Unterstützung ab **Windows 10 2004** und bis zur **neuesten Version von Windows 11** (mit Ausnahme der Vorschauversion).

Kann mit Designs von Drittanbietern verwendet werden, um DWM weiter anzupassen.

Wir ändern nicht die Rendering-Logik der Anwendung selbst, die sich völlig von der Logik von "MicaForEveryone" unterscheidet und daher die Kompatibilität mit Programmen von Drittanbietern maximiert.

Kompatibel mit [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), funktioniert besser zusammen.

Kompatibel mit [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Es ist zu beachten, dass dieses Projekt zwar kompatibel mit TF ist, jedoch EBMv2 nicht vollständig kompatibel mit TFv2**)

## Katalog
- [Galerie](#Galerie)
- [Verwendung](#Verwendung)
- [Sprachdateien](#Sprachdateien)
- [Abhängigkeiten](#Abhängigkeiten)

## Galerie
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

> Aktivieren Sie „DWMAPI-Mica-Effekt überschreiben (Win11)“

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001911.png)

Verwendung von Themes von Drittanbietern

> Aktivieren Sie „Effekte auf Ränder erweitern (Win10)“

> Aktivieren Sie „Aero-Reflexionseffekt (Win10)“

> Aktivieren Sie „Höhe der Titelleistenschaltfläche reduzieren (Win7-Stil)“

![image](/Screenshot/025454_DE.png)

</details>

## Verwendung

### Installation
1. Laden Sie das kompilierte Programmarchiv von der Seite [Release](https://github.com/Maplespe/DWMBlurGlass/releases) herunter.
2. Entpacken Sie es an einen Speicherort wie „C:\Programme“.
<details><summary><b>3. Führen Sie das GUI-Programm DWMBlurGlass.exe aus und klicken Sie auf „Installieren“.</b></summary>

![image](/Screenshot/012746.png)

>Wenn die Meldung „Installation war erfolgreich!“ erscheint - Sie aber noch keine gültige Symboldatei heruntergeladen haben, laden Sie diese bitte unter Reiter „Symbole“ herunter, erst dann kann DWMBlurGlass verwendet werden.

>**Beachten Sie, dass Sie in Zukunft möglicherweise ähnliche Benachrichtigungen erhalten, insbesondere nach Systemaktualisierungen.**

![image](/Screenshot/012924.png)

</details>

### Deinstallation
1. Führen Sie das GUI-Programm DWMBlurGlass.exe aus und klicken Sie auf Deinstallieren.
2. Löschen Sie relevante Dateien aus dem Programm Ordner

## Sprachdateien
Wir bieten als Grundsprachen Englisch, vereinfachtes Chinesisch und Deutsch an.
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
* MiaoUI interface library v2 (Private)
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Da das GUI-Programm private Bibliotheken verwendet, können nur Insider die GUI kompilieren, was keinen Einfluss auf die Kompilierung der Bibliotheken hat.
