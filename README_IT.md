# DWMBlurGlass
Aggiunge l'effetto sfocato alla barra del titolo del sistema globale, supporta Win10 e Win11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [Italiano](/README_IT.md) | [Français](/README_FR.md) | [Türkçe](/README_TR.md)
This project uses [LGNU V3 license](/COPYING.LESSER).

Altre lingue:
> La seguente documentazione potrebbe non essere aggiornata, fare riferimento alla documentazione corrente.
>
> [Deutsch](/README_DE.md)


[![licenza](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github Tutti i Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Catalogo
- [Effetti](#effetti)
- [Compatibilità](#compatibilità)
- [Galleria](#galleria)
- [Effetti Materiali](#effetti-materiali)
  - [Blur](#blur)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [Come si usa](#come-si-usa)
  - [Installazione](#installazione)
  - [Disinstallazione](#disinstallazione)
- [File di lingua](#file-di-lingua)
- [Dipendenze](#dipendenze)

## Effetti
* Aggiunge un effetto personalizzato alla barra del titolo del sistema globale.
* Solo raggio di sfocatura globale personalizzabile o raggio di sfocatura della barra del titolo.
* Colori di fusione della barra del titolo personalizzabili.
* Colore del testo della barra del titolo personalizzabile.
* È possibile aggiungere riflessi aerodinamici ed effetti di parallasse.
* Ripristina l'altezza del pulsante della barra del titolo in stile Win7.
* Supporto per abilitare l'effetto sfocatura per i programmi che utilizzano la vecchia API Win7 DwmEnableBlurBehindWindow.
* Supporta gli effetti "Blur", "Aero", "Acrylic", "Mica (solo Win11)".
* Modalità colore chiaro/scuro personalizzabile individualmente. Commutazione automatica del colore.
* Sono disponibili i metodi di sfocatura "CustomBlur", "AccentBlur" e "SystemBackdrop".
* Supporto per temi di terze parti.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Compatibilità
Supportato a partire da **Windows 10 2004** e fino all'**ultima versione di Windows 11** (alcuni metodi di sfocatura non sono supportati nella versione di anteprima).

Può essere utilizzato con temi di terze parti per personalizzare ulteriormente DWM.

Non modifichiamo la logica di rendering dell'applicazione stessa, che è completamente diversa dalla logica di MicaForEveryone e quindi massimizza la compatibilità con programmi di terze parti.

Analizziamo in modo inverso il dwm e codifichiamo un metodo di sfocatura personalizzato per ottenere effetti visivi sorprendenti, ma se scegli il metodo di sfocatura "`SystemBackdrop`", utilizza le interfacce del sistema disponibili pubblicamente e ha lo stesso effetto di MicaForEveryone.

Non consigliato per l'uso con MicaForEveryone, non garantiamo la compatibilità con esso.

Compatibile con [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), funziona meglio insieme.

Compatibile con [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Va notato che anche se questo progetto è compatibile con TF, EBMv2 non è completamente compatibile con TFv2**)

## Galleria
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> Abilita "Sostituisci l'effetto mica DWMAPI (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Utilizzando i temi di terze parti.

> Abilita "Estendi gli effetti ai bordi (win10)"

> Abilita "Abilita l'effetto riflesso Aero (win10)"

> Abilita "Riduci l'altezza del pulsante della barra del titolo (stile Win7)"

![image](/Screenshot/025410.png)

</details>

## Effetti Materiali
### Blur
> Sfocatura pura di base. Niente di speciale.

![image](/Screenshot/blur.png)

### Aero
> Effetto vetro di Windows 7, con effetti di saturazione ed esposizione sullo sfondo quando perde la messa a fuoco.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> La ricetta acrilica: sfondo, sfocatura, fusione di esclusione, saturazione, sovrapposizione colore/tinta, rumore

![image](/Screenshot/acrylic.png)

### Mica
> La ricetta della mica: sfondo sfocato, saturazione, sovrapposizione colore/tinta

![image](/Screenshot/mica.png)

### MicaAlt
Tutti gli effetti di cui sopra possono essere personalizzati per fondere i colori.

MicaAlt è Mica con tono grigiastro, puoi modificare tu stesso il colore di fusione per ottenere l'effetto MicaAlt.

## Come si usa

### Installazione
1. Scaricare l'archivio del programma compilato dalla pagina [Release](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Decomprimilo in una posizione come "`C:\Programmi`".
<details><summary><b>3. Eseguire il programma GUI DWMBlurGlass.exe e fare clic su Installa.</b></summary>

![image](/Screenshot/012746.png)

>Se viene visualizzato il messaggio "L'installazione è riuscita! Ma non hai ancora scaricato un file di simboli valido, scaricalo dalla pagina "Simboli" prima di poterlo utilizzare!" quindi devi fare clic sulla pagina Simboli e fare clic su Scarica prima di poterlo utilizzare.

>**Tieni presente che potresti ricevere notifiche simili in futuro, soprattutto dopo gli aggiornamenti di sistema.**

![image](/Screenshot/012924.png)

</details>

### Disinstallazione
1. Eseguire il programma GUI DWMBlurGlass.exe e fare clic su Disinstalla.
2. Elimina i file rilevanti

## File di lingua
Offriamo le lingue di base inglese e cinese semplificato.
Se desideri aiutarci a tradurre in altre lingue, vedi sotto per i formati dei file della lingua.

1. Innanzitutto, devi creare un fork di questo repository e clonarlo localmente.
2. Apri la cartella "`Languagefiles`" e seleziona una lingua esistente come "`en-US.xml`" e creane una copia.
3. Rinominare il codice con il nome della [lingua di destinazione](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) e aprire il file xml nel tuo editor di testo preferito.
4. Nella seconda riga, nel campo "`local`", modificalo nel codice della lingua di destinazione, che dovrebbe essere uguale al nome del file (senza l'estensione .xml).
5. Puoi inserire il tuo nome nel campo "`autore`".
6. Successivamente, traduci i valori del campo nel formato xml (fai attenzione a non tradurre i nomi dei campi). Il formato corretto è: `<config>Config</config>` in `<config>xxxx</config>`.
7. Al termine, salvare il file e copiarlo nella directory "data\lang" nella cartella in cui si trova il programma DWMBlurGlass.exe.
8. Successivamente, apri DWMBlurGlass.exe e prova il file della lingua per vedere se funziona correttamente. In caso contrario, controlla le impostazioni del codice della lingua e controlla che il file sia conforme alla specifica del formato xml.
9. Infine, esegui il commit del file nel tuo repository biforcato e invia richieste pull al ramo principale del progetto.
10. Una volta approvata la richiesta, il file verrà rilasciato con un futuro aggiornamento del software.
   

## Dipendenze
* MiaoUI interface library v2 (Private)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Implementazione upstream separata del metodo CustomBlur, grazie ad ALTaleX per la ricerca e il supporto.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Poiché il programma GUI utilizza librerie private, solo gli addetti ai lavori possono compilare la GUI, il che non influisce sulla compilazione delle librerie.
