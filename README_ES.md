# DWMBlurGlass
Agrega efectos personalizados de desenfoque a la barra de título global del sistema. Compatible con Windows 10 y Windows 11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [中文](/README_ZH.md) | [English](/README.md) | [italiano](/README_IT.md) | [français](/README_FR.md) | [Türkçe](/README_TR.md) | [español](/README_ES.md)
Este proyecto usa la [licencia LGPL v3](/COPYING.LESSER).

Otros idiomas:
> La siguiente documentación puede estar obsoleta, consulte la documentación actual.
>
> [Deutsch](/README_DE.md)


[![Licencia](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg?label=Licencia)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![GitHub All Releases](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg?label=Descargas)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![GitHub release](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg?label=Versión)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-C++-F34B7D.svg?label=Lenguaje"/>

## Índice
- [Efectos](#efectos)
- [Compatibilidad](#compatibilidad)
- [Galería](#galería)
- [Efectos de material](#efectos-de-material)
  - [Blur (desenfocado)](#blur-desenfocado)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [Instrucciones de uso](#instrucciones-de-uso)
  - [Instalar](#instalar)
  - [Desinstalar](#desinstalar)
- [Archivos de idioma](#archivos-de-idioma)
- [Dependencias](#dependencias)

## Efectos
* Agrega un efecto personalizado a la barra de título global del sistema.
* Permite modificar el radio de desenfoque global o sólo de la barra de título.
* Colores fusión de la barra de título personalizables.
* Color de texto de la barra de título personalizable.
* Permite usar reflejos Aero y efectos de paralaje.
* Permite restaurar el tamaño de los botones de la barra de título al estilo de Windows 7.
* Soporte para habilitar el efecto de desenfoque para programas que usan la antigua API de Windows 7 DwmEnableBlurBehindWindow.
* Soporta efectos `Blur (desenfocado)`, `Aero`, `Acrylic` y `Mica (sólo para Windows 11)`.
* Colores de modo claro y oscuro personalizables individualmente y con cambio automático.
* Métodos de desenfoque disponibles: `CustomBlur`, `AccentBlur` y `SystemBackdrop`.
* Soporte para temas de terceros.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## Compatibilidad
Es compatible desde **Windows 10 2004** hasta la **última versión de Windows 11** (algunos métodos de desenfoque no son compatibles con versiones de Windows Insider).

Se pueden aplicar temas de terceros para personalizar aún más DWM.

No modificamos la lógica de renderizado de la aplicación en sí, la cual es completamente diferente de la lógica de MicaForEveryone y, por lo tanto, maximiza la compatibilidad con programas de terceros.

Realizamos un análisis inverso de DWM y creamos un método de desenfoque personalizado para lograr efectos visuales impresionantes, pero si elige el método de desenfoque "`SystemBackdrop`", se usarán las interfaces públicamente disponibles del sistema y tiene el mismo efecto que MicaForEveryone.

No se recomienda su uso con MicaForEveryone, no garantizamos su compatibilidad.

Compatible con [ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), funcionan mejor en conjunto.

Compatible con [TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**Cabe destacar que, aunque este proyecto es compatible con TF, EBMv2 no es completamente compatible con TFv2**)

## Galería
<details><summary><b>Windows 11</b></summary>

![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

> «Redefinir efecto Mica establecido mediante DWMAPI (Win 11)» habilitado

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

Usando temas de terceros, las siguientes opciones fueron habilitadas:

> «Extender efectos a bordes (Win 10)»

> «Habilitar efecto de reflejo Aero»

> «Restaurar tamaño de botones de barra de título (estilo de Win 7)»

![image](/Screenshot/025410.png)

</details>

## Efectos de material
### Blur (desenfocado)
> Simple desenfoque básico. No tiene nada de particular.

![image](/Screenshot/blur.png)

### Aero
> El efecto cristal «Glass» de Windows 7, con efectos de saturación y exposición en el fondo cuando una ventana está inactiva.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> La receta Acrylic: fondo, desenfoque, fusión exclusión, saturación, superposición de color o tinte, y ruido.

![image](/Screenshot/acrylic.png)

### Mica
> La receta Mica: fondo de pantalla desenfocado, saturación, y superposición de color o tinte.

![image](/Screenshot/mica.png)

### MicaAlt
Todos los efectos anteriores se pueden personalizar para fusionar colores.

MicaAlt es Mica con un tono grisáceo, usted puede modificar el color fusión para obtener el efecto MicaAlt.

## Instrucciones de uso

### Instalar
1. Descargue el archivo del programa compilado desde la página de [Releases](https://github.com/Maplespe/DWMBlurGlass/releases).
2. Descomprímalo en una ubicación como "`C:\Program Files`".
<details><summary><b>3. Ejecute el programa GUI DWMBlurGlass.exe y haga clic en Instalar.</b></summary>

![image](/Screenshot/012746.png)

> Si no sucede nada al hacer clic en Instalar, entonces necesita hacer clic en la página de Símbolos y luego en Descargar.

> **Es posible que reciba una notificación sobre símbolos faltantes en el futuro, especialmente después de las actualizaciones del sistema.**

![image](/Screenshot/012924.png)

</details>

### Desinstalar
1. Ejecute el programa GUI DWMBlurGlass.exe y haga clic en Desinstalar.
2. Elimine los archivos relevantes.

## Archivos de idioma
Se ofrecen varias traducciones, como inglés, chino simplificado, español y portugués, entre otras.
Si desea ayudarnos a traducir a otros idiomas, consulte a continuación los formatos de archivo de idioma.

1. En primer lugar, debe hacer un *fork* (o bifurcar) este repositorio y clonarlo localmente.
2. Abra la carpeta "`Languagefiles`" y seleccione un idioma existente como "`en-US.xml`" y haga una copia.
3. Cambie el nombre del archivo al nombre del [idioma objetivo](https://learn.microsoft.com/es-es/windows/win32/intl/locale-names) y ábralo en su editor de texto favorito.
4. En la segunda línea, en el campo "`local`", cámbielo por el código de idioma objetivo, que debería ser el mismo que el nombre del archivo (sin la extensión .xml).
5. Puede poner su nombre en el campo "`author`".
6. A continuación, traduzca los valores de los campos en el formato XML (tenga cuidado de no traducir los nombres de los campos). El formato correcto es: `<config>Config</config>` a `<config>xxxx</config>`.
7. Guarde el archivo cuando haya terminado y cópielo en el directorio "data\lang" en la carpeta donde se encuentra el programa DWMBlurGlass.exe.
8. A continuación, abra DWMBlurGlass.exe y pruebe el archivo de idioma para ver si funciona correctamente. Si no lo hace, verifique la configuración del código de idioma y asegúrese de que el archivo cumpla con la especificación del formato XML.
9. Por último, haga un *commit* (o confirme los cambios) del archivo en su propio repositorio bifurcado y envíe un *pull request* (o solicitud de extracción) a la rama principal del proyecto.
10. Después de que se apruebe la solicitud, su archivo se lanzará con una futura actualización del software.


## Dependencias
* Biblioteca de interfaz MiaoUI v2 (privada)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - Implementación separada del método CustomBlur, gracias a ALTaleX por la investigación y el apoyo.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Bibliotecas de implementación de Windows (WIL)](https://github.com/Microsoft/wil)

Dado que la GUI usa una biblioteca privada, sólo los desarrolladores con acceso pueden compilarla, lo que no afecta la compilación de la biblioteca principal.
