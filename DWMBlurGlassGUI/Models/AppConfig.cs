using CommunityToolkit.Mvvm.ComponentModel;
using System.Runtime.InteropServices;

namespace DWMBlurGlassGUI.Models
{
    public enum BlurMethod
    {
        CustomBlur,
        AccentBlur,
        DWMAPIBlur
    }

    public enum EffectType
    {
        None = -1,
        Blur,
        Aero,
        Acrylic,
        Mica
    }

    public partial class AppConfig : ObservableObject
    {
        [ObservableProperty]
        private bool _applyGlobal = false;

        [ObservableProperty]
        private bool _extendBorder = false;

        [ObservableProperty]
        private bool _reflection = false;

        [ObservableProperty]
        private bool _oldBtnHeight = false;

        [ObservableProperty]
        private bool _customAmount = false;

        [ObservableProperty]
        private bool _useAccentColor = false;

        [ObservableProperty]
        private bool _crossFade = true;

        [ObservableProperty]
        private bool _overrideAccent = false;

        [ObservableProperty]
        private bool _powerSavingMode = false;

        [ObservableProperty]
        private bool _disableOnBattery = true;

        [ObservableProperty]
        private bool _scaleOptimizer = false;

        [ObservableProperty]
        private bool _titleBtnGlow = false;

        [ObservableProperty]
        private bool _disableFramerateLimit = false;

        [ObservableProperty]
        private bool _autoDownloadSymbols = true;

        [ObservableProperty]
        private bool _customTitleBtnSize = false;

        [ObservableProperty]
        private int _titleBtnSizePreset = 0; // 0=Windows7, 1=Vista, 2=Custom

        // Options without GUI
        [ObservableProperty]
        private int _extendRound = 10;

        [ObservableProperty]
        private int _titleBtnOffsetX = -1;

        [ObservableProperty]
        private int _customCloseBtnW = 49;

        [ObservableProperty]
        private int _customMaxBtnW = 27;

        [ObservableProperty]
        private int _customMinBtnW = 29;

        [ObservableProperty]
        private int _customBtnFrameH = 20;
        //

        [ObservableProperty]
        private float _blurAmount = 20.0f;

        [ObservableProperty]
        private float _customBlurAmount = 20.0f;

        [ObservableProperty]
        private float _luminosityOpacity = 0.65f;

        [ObservableProperty]
        private float _glassIntensity = 1.0f;

        [ObservableProperty]
        private bool _customAeroTexture = false;

        [ObservableProperty]
        private string _customAeroTexturePath = "";

        [ObservableProperty]
        private uint _crossFadeTime = 160;

        [ObservableProperty]
        private float _aeroColorBalance = 0.08f;

        [ObservableProperty]
        private float _aeroAfterglowBalance = 0.43f;

        [ObservableProperty]
        private float _aeroBlurBalance = 0.49f;

        [ObservableProperty]
        private uint _activeTextColor = 0xFF000000;

        [ObservableProperty]
        private uint _inactiveTextColor = 0xFFB4B4B4;

        [ObservableProperty]
        private uint _activeBlendColor = 0x64FFFFFF;

        [ObservableProperty]
        private uint _inactiveBlendColor = 0x64FFFFFF;

        [ObservableProperty]
        private uint _activeTextColorDark = 0xFFFFFFFF;

        [ObservableProperty]
        private uint _inactiveTextColorDark = 0xFFB4B4B4;

        [ObservableProperty]
        private uint _activeBlendColorDark = 0x64000000;

        [ObservableProperty]
        private uint _inactiveBlendColorDark = 0x64000000;

        [ObservableProperty]
        private BlurMethod _blurMethod = BlurMethod.CustomBlur;

        [ObservableProperty]
        private EffectType _effectType = EffectType.Blur;
    }
}
