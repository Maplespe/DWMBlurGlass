# DWMBlurGlass
הוספת אפקטי טשטוש מותאמים אישית לפס הכותרת הגלובלי של המערכת. תואם ל-Windows 10 ול-Windows 11.

给全局系统标题栏添加自定义效果，支持win10和win11
#
| [עברית](/README_HE.md) | [סינית](/README_ZH.md) | [אנגלית](/README.md) | [איטלקית](/README_IT.md) | [צרפתית](/README_FR.md) | [טורקית](/README_TR.md) | [ספרדית](/README_ES.md) | [גרמנית](/README_DE.md) | [פורטוגזית ברזילאית](/README_PTBR.md)

פרויקט זה משתמש ב[רישיון LGPL v3](/COPYING.LESSER).

## !!! אין להוריד את DWMBlurGlass משום מקור אחר מלבד המופיע כאן !!!
> [!WARNING]
> גילינו שמישהו מתחזה אלינו ומפיץ גרסה של DWMBlurGlass המכילה קוד זדוני.
>
> כדי למנוע הישנות מקרים כאלו, **אנא הורידו את התוכנה אך ורק מהכתובות הרשמיות!**
>
> **אין לנו שרת דיסקורד רשמי.**
>
> אנו מפיצים את הגרסאות **אך ורק דרך [Github](https://github.com/Maplespe/DWMBlurGlass/releases), [Bilibili](https://space.bilibili.com/87195798) ו-[winmoes](https://winmoes.com)**.
>
> כמו כן, כל גרסה חדשה נבדקת תחילה בענף test, ולא משתחררת קובץ הפעלה מראש.

[![רישיון](https://img.shields.io/github/license/Maplespe/DWMBlurGlass.svg?label=רישיון)](https://www.gnu.org/licenses/lgpl-3.0.en.html)
[![הורדות](https://img.shields.io/github/downloads/Maplespe/DWMBlurGlass/total.svg?label=הורדות)](https://github.com/Maplespe/DWMBlurGlass/releases)
[![גרסה](https://img.shields.io/github/release/Maplespe/DWMBlurGlass.svg?label=גרסה)](https://github.com/Maplespe/DWMBlurGlass/releases/latest)
<img src="https://img.shields.io/badge/language-c++-F34B7D.svg"/>
<img src="https://img.shields.io/github/last-commit/Maplespe/DWMBlurGlass.svg"/>

## תוכן עניינים
- [אפקטים](#אפקטים)
- [תאימות](#תאימות)
- [גלריה](#גלריה)
- [אפקטי חומר (Material)](#אפקטי-חומר-material)
  - [טשטוש (Blur)](#טשטוש-blur)
  - [Aero](#aero)
  - [Acrylic](#acrylic)
  - [Mica](#mica)
  - [MicaAlt](#micaalt)
- [הוראות שימוש](#הוראות-שימוש)
  - [התקנה](#התקנה)
  - [הסרה](#הסרה)
- [קבצי שפה](#קבצי-שפה)
- [תלויות](#תלויות)

## אפקטים
* הוספת אפקט מותאם אישית לפס הכותרת הגלובלי של המערכת.
* אפשרות לשנות את רדיוס הטשטוש הגלובלי או רק של פס הכותרת.
* צבעי מיזוג (blend) של פס הכותרת ניתנים להתאמה אישית.
* צבע טקסט של פס הכותרת ניתן להתאמה אישית.
* אפשרות להשתמש בהשתקפויות Aero ואפקטי פרלקסה.
* שחזר את גובה לחצן שורת הכותרת בסגנון Windows 7.
* שחזר את הלחצן של שורת הכותרת בסגנון Windows 7.
* תמיכה בהפעלת אפקט הטשטוש עבור תוכנות המשתמשות ב-API הישן של Windows 7, `DwmEnableBlurBehindWindow`.
* תמיכה באפקטים: `טשטוש (Blur)`, `Aero`, `Acrylic` ו-`Mica (ל-Windows 11 בלבד)`.
* צבעים למצב בהיר ולמצב כהה הניתנים להתאמה אישית בנפרד, עם החלפה אוטומטית.
* שיטות טשטוש זמינות: `CustomBlur`, `AccentBlur` ו-`SystemBackdrop`.
* תמיכה בערכות נושא של צד שלישי.

![image](/Screenshot/001701.png)
![image](/Screenshot/10307.png)

## תאימות
תואם החל מ-**Windows 10 2004** ועד ל**גרסה האחרונה של Windows 11** (חלק משיטות הטשטוש אינן נתמכות בגרסאות Windows Insider).

ניתן להחיל ערכות נושא של צד שלישי כדי להתאים את DWM אף יותר.

איננו משנים את לוגיקת הרינדור של היישום עצמו, שהיא שונה לחלוטין מהלוגיקה של MicaForEveryone, ובכך ממקסמת את התאימות עם תוכנות צד שלישי.

ביצענו ניתוח הנדסה לאחור (reverse engineering) של DWM ויצרנו שיטת טשטוש מותאמת אישית להשגת אפקטים חזותיים מרשימים. אך אם תבחרו בשיטת הטשטוש `SystemBackdrop`, ייעשה שימוש בממשקי המערכת הזמינים לציבור, והאפקט יהיה זהה לזה של MicaForEveryone.

השימוש בתוכנה במקביל ל-MicaForEveryone אינו מומלץ, ואיננו מבטיחים תאימות במקרה זה.

תואם ל-[ExplorerBlurMica](https://github.com/Maplespe/ExplorerBlurMica), והם עובדים טוב יותר יחד.

תואם ל-[TranslucentFlyouts](https://github.com/ALTaleX531/TranslucentFlyouts). (**יש לציין כי למרות שפרויקט זה תואם ל-TF, גרסת EBMv2 אינה תואמת באופן מלא ל-TFv2**)

## גלריה
<details><summary><b>Windows 11</b></summary>

![image](/Screenshot/10307.png)

![image](/Screenshot/102134.png)

- [x] ביטול אפקט נציץ DWMAPI (win11)

![image](/Screenshot/013521.png)
</details>

<details><summary><b>Windows 10</b></summary>

![image](/Screenshot/001701.png)

![image](/Screenshot/100750.png)

בשימוש בערכות נושא של צד שלישי, האפשרויות הבאות הופעלו:

- [x] הרחבת אפקטים לגבולות (win10)
- [x] אפקט השתקפות אוויר
- [x] שחזר את גודל כפתור שורת הכותרת בסגנון Win7

![image](/Screenshot/025410.png)

</details>

## אפקטי חומר (Material)
### טשטוש (Blur)
> טשטוש בסיסי ופשוט. אין בו שום דבר מיוחד.

![image](/Screenshot/blur.png)

### Aero
> אפקט הזכוכית (Glass) של Windows 7, עם אפקטי רוויה וחשיפה ברקע כאשר חלון אינו פעיל.

![image](/Screenshot/aero.png)

![image](/Screenshot/aero_inactive.png)

### Acrylic
> המתכון של Acrylic: רקע, טשטוש, מיזוג הדרה (exclusion blend), רוויה, שכבת-על של צבע או גוון, ורעש.

![image](/Screenshot/acrylic.png)

### Mica
> המתכון של Mica: טפט מטושטש, רוויה, ושכבת-על של צבע או גוון.

![image](/Screenshot/mica.png)

### MicaAlt
כל האפקטים שלעיל ניתנים להתאמה אישית למיזוג צבעים.

MicaAlt הוא למעשה Mica עם גוון אפרפר; ניתן לשנות את צבע המיזוג כדי לקבל את אפקט MicaAlt.

## הוראות שימוש

### התקנה
1. הורד את קובץ התוכנה המהודר (compiled) מעמוד ה-[הורדות (Releases)](https://github.com/Maplespe/DWMBlurGlass/releases).
2. חלץ את הקבצים למיקום כגון `C:\Program Files`.
<details><summary><b>3. הפעל את תוכנת הממשק הגרפי DWMBlurGlass.exe ולחץ על 'התקנה'.</b></summary>

![image](/Screenshot/012746.png)

> אם לא קורה כלום לאחר הלחיצה על 'התקנה', עליך לעבור לעמוד 'סמלים' וללחוץ על 'הורדה'.

> **ייתכן שתקבל הודעה על סמלים חסרים בעתיד, במיוחד לאחר עדכוני מערכת.**

![image](/Screenshot/012924.png)

</details>

### הסרה
1. הפעל את תוכנת הממשק הגרפי DWMBlurGlass.exe ולחץ על 'הסרה'.
2. מחק את הקבצים הרלוונטיים.

## קבצי שפה
התוכנה מציעה מספר תרגומים, כמו אנגלית, סינית מפושטת, ספרדית ופורטוגזית, בין היתר.
אם ברצונך לסייע בתרגום לשפות נוספות, עיין בהנחיות לפורמט קבצי השפה להלן.

1.  ראשית, עליך לבצע *fork* (פיצול) למאגר (repository) זה ולשכפל (clone) אותו באופן מקומי.
2.  פתח את תיקיית `Languagefiles`, בחר קובץ שפה קיים כגון `en-US.xml` וצור ממנו עותק.
3.  שנה את שם הקובץ לשם של [שפת היעד](https://learn.microsoft.com/en-us/windows/win32/intl/locale-names) ופתח אותו בעורך הטקסט המועדף עליך.
4.  בשורה השנייה, בשדה `local`, שנה את הערך לקוד שפת היעד, אשר אמור להיות זהה לשם הקובץ (ללא הסיומת .xml).
5.  באפשרותך להוסיף את שמך בשדה `author`.
6.  לאחר מכן, תרגם את הערכים שבתוך התגיות בקובץ ה-XML (היזהר לא לתרגם את שמות התגיות עצמן). הפורמט הנכון הוא: מ-`<config>Config</config>` ל-`<config>הגדרות</config>`.
7.  שמור את הקובץ בסיום והעתק אותו לספריית `data\lang` בתיקייה שבה נמצא קובץ ההפעלה DWMBlurGlass.exe.
8.  לאחר מכן, פתח את DWMBlurGlass.exe ובדוק אם קובץ השפה פועל כראוי. אם לא, ודא שהגדרת קוד השפה נכונה ושהקובץ עומד בתקן הפורמט של XML.
9.  לבסוף, בצע *commit* (שמירת שינויים) לקובץ במאגר המפוצל (forked) שלך ושלח *pull request* (בקשת משיכה) לענף הראשי (main) של הפרויקט.
10. לאחר שהבקשה תאושר, הקובץ שלך ייכלל בעדכון עתידי של התוכנה.


## תלויות
* [MiaoUI Lite interface library v2](https://github.com/Maplespe/MiaoUILite)
* [AcrylicEverywhere](https://github.com/ALTaleX531/AcrylicEverywhere) - יישום נפרד של שיטת CustomBlur, תודות ל-ALTaleX על המחקר והתמיכה.
* [minhook](https://github.com/m417z/minhook)
* [pugixml](https://github.com/zeux/pugixml)
* [VC_LTL](https://github.com/Chuyu-Team/VC-LTL5)
* [Windows Implementation Libraries (WIL)](https://github.com/Microsoft/wil)