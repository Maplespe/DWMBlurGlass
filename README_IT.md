# DWMBlurGlass
Aggiunge l'effetto sfocato alla barra del titolo del sistema globale, supporta Win10 e Win11.

#
| [中文](/README_ZH.md) | [English](/README.md) | [Deutsch](/README_DE.md) | 
This project uses [LGNU V3 license](/COPYING.LESSER).

[![licenza](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Github Tutte le versioni](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub ultima versione](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Effetti
* Aggiunge un effetto sfocato alla barra del titolo del sistema globale.
* Raggio di sfocatura globale personalizzabile.
* Colori di sfondo sfumati personalizzabili.
* Colore del testo della barra del titolo personalizzabile.
* Supporto per temi di terze parti.
* Personalizza l'effetto riflesso Aero.
* Ripristina l'altezza del pulsante della barra del titolo in stile Win7.
* Supporto per abilitare l'effetto sfocatura per i programmi che utilizzano la vecchia API Win7 DwmEnableBlurBehindWindow.

Tieni presente che il raggio di sfocatura influenzerà il globale, che include anche posizioni come il menu Start, il centro notifiche, ecc...

![image](/Screenshot/001911.png)
![image](/Screenshot/10307.png)

## Compatibilita
Supportato a partire da **Windows 10 2004** e fino all'**ultima versione di Windows 11** (esclusa la versione di anteprima).

Può essere utilizzato con temi di terze parti per personalizzare ulteriormente DWM.

Non modifichiamo la logica di rendering dell'applicazione stessa, che è completamente diversa dalla logica di MicaForEveryone e quindi massimizza la compatibilità con programmi di terze parti.
Compatibile con [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), Funziona meglio assieme.

Compatibile con [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Va notato che anche se questo progetto è compatibile con TF, EBMv2 non è completamente compatibile con TFv2**)

## Catalogo
- [Galleria](#galleria)
- [Come si usa](#come-si-usa)
- [File di lingua](#file-di-lingua)
- [Dipendenze](#dipendenze)

## Galleria
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

> Abilita "Sostituisci effetto mica DWMAPI (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001911.png)

Utilizzo di temi di terze parti

> Abilita "Estendi effetti ai bordi (win10)"

> Abilita "Effetto riflessione aerodinamica (win10)"

> Abilita "Riduci l'altezza del pulsante della barra del titolo (stile Win7)"

![image](/Screenshot/025454.png)

</details>

## Come si usa

### Installazione
1. Scarica l'archivio del programma compilato sulla pagina [Release](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Estrailo in una posizione come "`C:\Program Files`".
<details><summary><b>3. Eseguire il programma GUI DWMBlurGlass.exe e fare clic su Installa.</b></summary>

![image](/Screenshot/012746.png)

>Se viene visualizzato il messaggio "L'installazione è riuscita! Ma non hai ancora scaricato un file di simboli valido, scaricalo dalla pagina "Simboli" prima di poterlo utilizzare!" quindi devi fare clic sulla pagina Simboli e fare clic su Scarica prima di poterlo utilizzare.

>**Tieni presente che potresti ricevere notifiche simili in futuro, soprattutto dopo gli aggiornamenti di sistema.**

![image](/Screenshot/012924.png)

</details>

### Disinstalla
1. Run the DWMBlurGlass.exe GUI program and click Uninstall.
2. Delete relevant files

## File di lingua
Offriamo le lingue di base inglese e cinese semplificato.
Se desideri aiutarci a tradurre in altre lingue, vedi sotto per i formati dei file della lingua.

1. Innanzitutto, devi creare un fork di questo repository e clonarlo localmente.
2. Apri la cartella "`Languagefiles`" e seleziona una lingua esistente come "`en-US.xml`" e creane una copia.
3. Rinominare il codice con il nome della [lingua di destinazione](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) e apri il file xml nel tuo editor di testo preferito.
4. Nella seconda riga, nel campo "`local`", modificalo nel codice della lingua di destinazione, che dovrebbe essere uguale al nome del file (senza l'estensione .xml).
5. Puoi inserire il tuo nome nel campo "`author`".
6. Successivamente, traduci i valori del campo nel formato xml (fai attenzione a non tradurre i nomi dei campi). Il formato corretto è: `<config>Config</config>` in `<config>xxxx</config>`.
7. Al termine, salvare il file e copiarlo nella directory "data\lang" nella cartella in cui si trova il programma DWMBlurGlass.exe.
8. Successivamente, apri DWMBlurGlass.exe e prova il file della lingua per vedere se funziona correttamente. In caso contrario, controlla le impostazioni del codice della lingua e controlla che il file sia conforme alla specifica del formato xml.
9. Infine, esegui il commit del file nel tuo repository biforcato e invia richieste pull al ramo principale del progetto.
10. Una volta approvata la richiesta, il file verrà rilasciato con un futuro aggiornamento del software.


## Dipendenze
* MiaoUI interface library v2 (Private)
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries](https://github.com/Microsoft/wil)

Poiché il programma GUI utilizza librerie private, solo gli addetti ai lavori possono compilare la GUI, il che non influisce sulla compilazione delle librerie.
