# DWMBlurGlass
Ajoute un effet personnalisé à la barre de titre du système global, prend en charge win10 et win11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [Italiano](/README_IT.md) | [Français](/README_FR.md) | [Türkçe](/README_TR.md)
Ce projet utilise la [licence LGNU V3](/COPYING.LESSER).

Autres langues :
> La documentation suivante peut être obsolète, veuillez vous référer à la documentation actuelle.
>
> [Deutsch](/README_DE.md)


[![licence](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Toutes les versions GitHub](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![Version GitHub](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Catalogue
- [Effets](#effets)
- [Compatibilité](#compatibilité)
- [Galerie](#galerie)
- [Effets Matériels](#effets-matériels)
  - [Flou](#flou)
  - [Aero](#aero)
  - [Acrylique](#acrylique)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [Comment utiliser](#comment-utiliser)
  - [Installer](#installer)
  - [Désinstaller](#désinstaller)
- [Fichiers de langue](#fichiers-de-langue)
- [Dépendances](#dépendances)

## Effets
* Ajoute un effet personnalisé à la barre de titre du système global.
* Rayon de flou global personnalisable ou rayon de flou de la barre de titre uniquement.
* Couleurs de mélange de la barre de titre personnalisables.
* Couleur du texte de la barre de titre personnalisable.
* Reflets Aero et effets de parallaxe peuvent être ajoutés.
* Restaure la hauteur des boutons de la barre de titre du style Win7.
* Prise en charge de l'activation de l'effet de flou pour les programmes utilisant l'ancienne API DwmEnableBlurBehindWindow de Win7.
* Prise en charge des effets `Blur`, `Aero`, `Acrylique`, `Mica (Win11 uniquement)`.
* Mode couleur claire/sombre personnalisable individuellement avec commutation automatique.
* Les méthodes de flou `CustomBlur`, `AccentBlur` et `SystemBackdrop` sont disponibles.
* Prise en charge des thèmes tiers.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Compatibilité
Pris en charge à partir de **Windows 10 2004** jusqu'à la **dernière version de Windows 11** (Certaines méthodes de flou ne sont pas prises en charge dans la version de prévisualisation).

Peut être utilisé avec des thèmes tiers pour personnaliser davantage DWM.

Nous ne modifions pas la logique de rendu de l'application elle-même, ce qui est totalement différent de la logique de MicaForEveryone et maximise donc la compatibilité avec les programmes tiers.

Nous effectuons une analyse inverse du dwm et codons une méthode de flou personnalisée pour apporter des effets visuels étonnants, mais si vous choisissez la méthode de flou "`SystemBackdrop`", elle utilise les interfaces publiques disponibles du système et a le même effet que MicaForEveryone.

Non recommandé pour une utilisation avec MicaForEveryone, nous ne garantissons pas la compatibilité avec celui-ci.

Compatible avec [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), fonctionne mieux ensemble.

Compatible avec [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Il convient de noter que même si ce projet est compatible avec TF, EBMv2 n'est pas entièrement compatible avec TFv2**)

## Galerie
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> Activer "Remplacer l'effet Mica DWMAPI (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Utilisation de thèmes tiers

> Activer "Étendre les effets aux bordures (win10)"

> Activer "Effet de réflexion Aero (win10)"

> Activer "Réduire la hauteur des boutons de la barre de titre (style win7)"

![image](/Screenshot/025410.png)

</details>

## Effets Matériels
### Flou
> Flou pur de base. Rien de spécial.

![image](/Screenshot/blur.png)

### Aero
> Effet de verre de Windows 7, avec des effets de saturation et d'exposition sur l'arrière-plan lorsqu'il perd le focus.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylique
> La recette acrylique : arrière-plan, flou, exclusion de mélange, saturation, superposition de couleur/teinte, bruit

![image](/Screenshot/acrylic.png)

### Mica
> La recette du mica : fond d'écran flou, saturation, superposition de couleur/teinte

![image](/Screenshot/mica.png)

### MicaAlt
Tous les effets ci-dessus peuvent être personnalisés pour mélanger les couleurs.

MicaAlt est Mica avec une teinte grisâtre, vous pouvez modifier la couleur de mélange vous-même pour obtenir l'effet MicaAlt.

## Comment utiliser

### Installer
1. Téléchargez l'archive du programme compilé depuis la [page de sortie](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Décompressez-le à un emplacement tel que "`C:\Program Files`".
<details><summary><b>3. Exécutez le programme DWMBlurGlass.exe GUI et cliquez sur Installer.</b></summary>

![image](/Screenshot/012746.png)

> Si le message "L'installation a réussi ! Mais vous n'avez pas encore téléchargé un fichier de symboles valide, veuillez le télécharger depuis la page "Symboles" avant de pouvoir l'utiliser !" s'affiche, vous devez cliquer sur la page des Symboles et cliquer sur Télécharger avant de pouvoir l'utiliser.

>**Notez que vous pourriez recevoir des notifications similaires à l'avenir, en particulier après des mises à jour système.**

![image](/Screenshot/012924.png)

</details>

### Désinstaller
1. Exécutez le programme DWMBlurGlass.exe GUI et cliquez sur Désinstaller.
2. Supprimez les fichiers pertinents

## Fichiers de langue
Nous proposons les langues de base anglais et chinois simplifié.
Si vous souhaitez nous aider à traduire dans d'autres langues, veuillez voir ci-dessous pour les formats de fichier de langue.

1. Tout d'abord, vous devez fork ce dépôt et le cloner localement.
2. Ouvrez le dossier "`Languagefiles`" et sélectionnez une langue existante telle que "`en-US.xml`" et faites une copie.
3. Renommez le code avec le nom de la [langue cible](https://learn.microsoft.com/fr-fr/windows/win32/intl/locale-names) et ouvrez le fichier xml dans votre éditeur de texte préféré.
4. À la deuxième ligne, dans le champ "`local`", changez-le pour le code de votre langue cible, qui devrait être le même que le nom de fichier (sans l'extension .xml).
5. Vous pouvez mettre votre nom dans le champ "`author`".
6. Ensuite, veuillez traduire les valeurs de champ dans le format xml (faites attention à ne pas traduire les noms de champ) Le format correct est:`<config>Config</config>` à `<config>xxxx</config>`.
7. Enregistrez votre fichier lorsque vous avez terminé et copiez-le dans le répertoire "data\lang" du dossier où se trouve le programme DWMBlurGlass.exe.
8. Ensuite, ouvrez DWMBlurGlass.exe et testez le fichier de langue pour voir s'il fonctionne correctement. S'il ne fonctionne pas, vérifiez les paramètres du code de langue et vérifiez que le fichier est conforme à la spécification du format xml.
9. Enfin, validez le fichier dans votre propre dépôt forké et envoyez des demandes de fusion à la branche principale du projet.
10. Une fois la demande approuvée, votre fichier sera publié avec une future mise à jour logicielle.

## Dépendances
* Bibliothèque d'interface MiaoUI v2 (Privée)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Implémentation en amont séparée de la méthode de flou CustomBlur, merci à ALTaleX pour la recherche et le soutien.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Bibliothèques d'implémentation Windows](https://github.com/Microsoft/wil)

Comme le programme GUI utilise des bibliothèques privées, seuls les initiés peuvent compiler le GUI, ce qui n'affecte pas la compilation des bibliothèques.

