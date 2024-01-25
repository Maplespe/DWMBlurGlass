# DWMBlurGlass
Ajoutez un effet de flou à la barre de titre du système global, prend en charge win10 et win11.

给全局系统标题栏添加模糊效果，支持win10和win11
#
| [Chinois](/README_ZH.md) | [Anglais](/README.md) | [Allemand](/README_DE.md) | [Français](/README_FR.md) | 
Ce projet utilise la [licence LGNU V3](/COPYING.LESSER).

[![licence](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![Toutes les versions Github](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![Release GitHub](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/langue-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>  

## Effets
* Ajoute un effet de flou à la barre de titre du système global.
* Rayon de flou global personnalisable.
* Couleurs d'arrière-plan de mélange personnalisables.
* Couleur du texte de la barre de titre personnalisable.
* Support des thèmes tiers.
* Personnalisez l'effet de réflexion Aero.
* Restaure la hauteur des boutons de la barre de titre de style Win7.
* Prise en charge de l'activation de l'effet de flou pour les programmes utilisant l'ancienne API DwmEnableBlurBehindWindow de win7.

Veuillez noter que le rayon de flou affectera l'ensemble du système, y compris des endroits tels que le menu Démarrer, le centre de notification, etc...

![image](/Screenshot/001911.png)
![image](/Screenshot/10307.png)

## Compatibilité
Pris en charge aussi bas que **Windows 10 2004** et aussi haut que la **dernière version de Windows 11** (à l'exclusion de la version de prévisualisation).

Peut être utilisé avec des thèmes tiers pour personnaliser davantage DWM.

Nous ne modifions pas la logique de rendu de l'application elle-même, ce qui est totalement différent de la logique de MicaForEveryone et maximise donc la compatibilité avec les programmes tiers.

Compatible avec [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), fonctionne mieux ensemble.

Compatible avec [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Il convient de noter que même si ce projet est compatible avec TF, EBMv2 n'est pas entièrement compatible avec TFv2**)

## Catalogue
- [Galerie](#galerie)
- [Comment utiliser](#comment-utiliser)
- [Fichiers de langue](#fichiers-de-langue)
- [Dépendances](#dépendances)

## Galerie
<details><summary><b>Windows 11</b></summary>
  
![image](/Screenshot/10307.png)

> Activer "Remplacer l'effet mica DWMAPI (win11)"

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001911.png)

Utilisation de thèmes tiers

> Activer "Étendre les effets aux bordures (win10)"

> Activer "Effet de réflexion Aero (win10)"

> Activer "Réduire la hauteur des boutons de la barre de titre (style win7)"

![image](/Screenshot/025454.png)

</details>

## Comment utiliser

### Installer
1. Téléchargez l'archive du programme compilé depuis la [page des versions](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Décompressez-le dans un emplacement tel que "`C:\Program Files`".
<details><summary><b>3. Exécutez le programme GUI DWMBlurGlass.exe et cliquez sur Installer.</b></summary>

![image](/Screenshot/012746.png)

>Si le message "Installation réussie ! Mais vous n'avez pas téléchargé de fichier valide pour les symboles, téléchargez-le dans la page "Symboles" pour le faire marcher !" s'affiche, vous devez cliquer sur la page des Symboles et cliquer sur Télécharger avant de pouvoir l'utiliser.

>**Notez que vous pourriez recevoir des notifications similaires à l'avenir, surtout après des mises à jour système.**

![image](/Screenshot/012924.png)

</details>

### Désinstaller
1. Exécutez le programme GUI DWMBlurGlass.exe et cliquez sur Désinstaller.
2. Supprimez les fichiers pertinents.

## Fichiers de langue
Nous proposons des langues de base en anglais et en chinois simplifié.
Si vous souhaitez nous aider à traduire dans d'autres langues, veuillez consulter ci-dessous les formats de fichiers de langue.

1. Tout d'abord, vous devez forker ce dépôt et le cloner localement.
2. Ouvrez le dossier "`Languagefiles`" et sélectionnez une langue existante telle que "`en-US.xml`" et faites une copie.
3. Renommez le code avec le nom de la [langue cible](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) et ouvrez le fichier xml dans votre éditeur de texte préféré.
4. À la deuxième ligne, dans le champ "`local`", changez-le pour le code de votre langue cible, qui devrait être le même que le nom de fichier (sans l'extension .xml).
5. Vous pouvez mettre votre nom dans le champ "`author`".
6. Ensuite, veuillez traduire les valeurs des champs au format xml (faites attention de ne pas traduire les noms des champs). Le format correct est : `<config>Config</config>` à `<config>xxxx</config>`.
7. Enregistrez votre fichier une fois terminé et copiez-le dans le répertoire "data\lang" dans le dossier où se trouve le programme DWMBlurGlass.exe.
8. Ensuite, ouvrez DWMBlurGlass.exe et testez le fichier de langue pour voir s'il fonctionne correctement. Si ce n'est pas le cas, vérifiez les paramètres du code de langue et vérifiez que le fichier est conforme à la spécification du format xml.
9. Enfin, committez le fichier sur votre propre dépôt forké et envoyez des demandes de fusion vers la branche principale du projet.
10. Après approbation de la demande, votre fichier sera publié avec une future mise à jour du logiciel.

## Dépendances
* Bibliothèque d'interface MiaoUI v2 (Privée)
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Bibliothèques d'implémentation Windows](https://github.com/Microsoft/wil)

Étant donné que le programme GUI utilise des bibliothèques privées, seuls les initiés peuvent compiler le GUI, ce qui n'affecte pas la compilation des bibliothèques.
