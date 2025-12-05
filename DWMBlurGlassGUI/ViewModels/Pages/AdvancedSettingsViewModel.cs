using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DWMBlurGlassGUI.Models;
using DWMBlurGlassGUI.Services;
using Microsoft.Win32;
using System;
using System.Threading.Tasks;
using System.Windows;
using Wpf.Ui;
using Wpf.Ui.Controls;

namespace DWMBlurGlassGUI.ViewModels.Pages
{
    public partial class AdvancedSettingsViewModel : ObservableObject
    {
        private readonly ConfigManager _configManager;
        private readonly IContentDialogService _contentDialogService;

        public LanguageService Lang => LanguageService.Instance;

        public AdvancedSettingsViewModel(ConfigManager configManager, IContentDialogService contentDialogService)
        {
            _configManager = configManager;
            _contentDialogService = contentDialogService;
            _configManager.ConfigReloaded += (s, e) => LoadFromConfig();
            _configManager.Config.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(AppConfig.BlurMethod))
                    UpdateEnabledStates();
            };
            LoadFromConfig();
        }

        private void LoadFromConfig()
        {
            var config = _configManager.Config;
            SelectedBlurMethodIndex = (int)config.BlurMethod;
            SelectedEffectTypeIndex = (int)config.EffectType;
            TitleBarBlurRadius = config.CustomBlurAmount;
            MaterialOpacity = config.LuminosityOpacity * 100;
            EnableCrossFade = config.CrossFade;
            CrossFadeDuration = (int)config.CrossFadeTime;
            UseSystemThemeColor = config.UseAccentColor;
            OverrideAccentBlur = config.OverrideAccent;
            EnableBlurOptimization = config.ScaleOptimizer;
            DisableInPowerSaving = config.DisableOnBattery;
            DisableDcompRateLimit = config.DisableFramerateLimit;
            AeroReflectionOpacity = config.GlassIntensity * 100;
            ColorBalance = config.AeroColorBalance * 100;
            LuminosityBalance = config.AeroBlurBalance * 100;
            SaturationBalance = config.AeroAfterglowBalance * 100;
            UseCustomAeroTexture = config.CustomAeroTexture;
            CustomAeroTexturePath = config.CustomAeroTexturePath;
            CustomTitleBtnSize = config.CustomTitleBtnSize;
            CustomCloseBtnW = config.CustomCloseBtnW;
            CustomMaxBtnW = config.CustomMaxBtnW;
            CustomMinBtnW = config.CustomMinBtnW;
            CustomBtnFrameH = config.CustomBtnFrameH;
            TitleBtnSizePresetIndex = config.TitleBtnSizePreset;
        }

        [ObservableProperty]
        private string _pageTitle = "高级设置";

        // Blur Method
        [ObservableProperty]
        private int _selectedBlurMethodIndex = 0;

        partial void OnSelectedBlurMethodIndexChanged(int value)
        {
            _configManager.Config.BlurMethod = (BlurMethod)value;
            UpdateEnabledStates();
        }

        // Effect Settings
        [ObservableProperty]
        private int _selectedEffectTypeIndex = 0;

        [ObservableProperty]
        private double _titleBarBlurRadius = 8;

        partial void OnTitleBarBlurRadiusChanged(double value)
        {
            _configManager.Config.CustomBlurAmount = (float)value;
        }

        [ObservableProperty]
        private double _materialOpacity = 43;

        partial void OnMaterialOpacityChanged(double value)
        {
            _configManager.Config.LuminosityOpacity = (float)(value / 100.0);
        }

        // Aero Settings
        [ObservableProperty]
        private double _colorBalance = 50;

        partial void OnColorBalanceChanged(double value)
        {
            _configManager.Config.AeroColorBalance = (float)(value / 100.0);
        }

        [ObservableProperty]
        private double _luminosityBalance = 50;

        partial void OnLuminosityBalanceChanged(double value)
        {
            _configManager.Config.AeroBlurBalance = (float)(value / 100.0);
        }

        [ObservableProperty]
        private double _saturationBalance = 50;

        partial void OnSaturationBalanceChanged(double value)
        {
            _configManager.Config.AeroAfterglowBalance = (float)(value / 100.0);
        }

        // Visibility properties based on effect type
        // Blur: 0, Aero: 1, Acrylic: 2, Mica: 3
        public Visibility BlurRadiusVisibility => SelectedEffectTypeIndex is 0 or 1 or 2 ? Visibility.Visible : Visibility.Collapsed;
        public Visibility AeroSettingsVisibility => SelectedEffectTypeIndex == 1 ? Visibility.Visible : Visibility.Collapsed;
        public Visibility MaterialOpacityVisibility => SelectedEffectTypeIndex is 2 or 3 ? Visibility.Visible : Visibility.Collapsed;

        partial void OnSelectedEffectTypeIndexChanged(int value)
        {
            _configManager.Config.EffectType = (EffectType)value;
            OnPropertyChanged(nameof(BlurRadiusVisibility));
            OnPropertyChanged(nameof(AeroSettingsVisibility));
            OnPropertyChanged(nameof(MaterialOpacityVisibility));
        }

        // Misc Settings
        [ObservableProperty]
        private bool _enableCrossFade = true;

        partial void OnEnableCrossFadeChanged(bool value)
        {
            _configManager.Config.CrossFade = value;
            OnPropertyChanged(nameof(IsCrossFadeDurationEnabled));
        }

        [ObservableProperty]
        private int? _crossFadeDuration = 96;

        partial void OnCrossFadeDurationChanged(int? value)
        {
            if (value.HasValue)
                _configManager.Config.CrossFadeTime = (uint)value.Value;
        }

        [ObservableProperty]
        private bool _useSystemThemeColor = true;

        partial void OnUseSystemThemeColorChanged(bool value)
        {
            _configManager.Config.UseAccentColor = value;
        }

        [ObservableProperty]
        private bool _overrideAccentBlur = false;

        partial void OnOverrideAccentBlurChanged(bool value)
        {
            _configManager.Config.OverrideAccent = value;
        }

        [ObservableProperty]
        private bool _enableBlurOptimization = true;

        partial void OnEnableBlurOptimizationChanged(bool value)
        {
            _configManager.Config.ScaleOptimizer = value;
        }

        [ObservableProperty]
        private bool _disableInPowerSaving = true;

        partial void OnDisableInPowerSavingChanged(bool value)
        {
            _configManager.Config.DisableOnBattery = value;
        }

        [ObservableProperty]
        private bool _disableDcompRateLimit = false;

        partial void OnDisableDcompRateLimitChanged(bool value)
        {
            _configManager.Config.DisableFramerateLimit = value;
        }

        [ObservableProperty]
        private double _aeroReflectionOpacity = 100;

        partial void OnAeroReflectionOpacityChanged(double value)
        {
            _configManager.Config.GlassIntensity = (float)(value / 100.0);
        }

        // Custom Aero Texture
        [ObservableProperty]
        private bool _useCustomAeroTexture = false;

        partial void OnUseCustomAeroTextureChanged(bool value)
        {
            _configManager.Config.CustomAeroTexture = value;
        }

        [ObservableProperty]
        private string _customAeroTexturePath = "";

        partial void OnCustomAeroTexturePathChanged(string value)
        {
            _configManager.Config.CustomAeroTexturePath = value;
        }

        // Custom Title Button Size
        [ObservableProperty]
        private bool _customTitleBtnSize = false;

        partial void OnCustomTitleBtnSizeChanged(bool value)
        {
            _configManager.Config.CustomTitleBtnSize = value;
            OnPropertyChanged(nameof(CustomTitleBtnInputVisibility));
        }

        [ObservableProperty]
        private int _titleBtnSizePresetIndex = 0;

        partial void OnTitleBtnSizePresetIndexChanged(int value)
        {
            _configManager.Config.TitleBtnSizePreset = value;
            switch (value)
            {
                case 0: // Windows 7
                    CustomCloseBtnW = 49;
                    CustomMaxBtnW = 27;
                    CustomMinBtnW = 29;
                    CustomBtnFrameH = 20;
                    break;
                case 1: // Windows Vista
                    CustomCloseBtnW = 44;
                    CustomMaxBtnW = 26;
                    CustomMinBtnW = 25;
                    CustomBtnFrameH = 18;
                    break;
                case 2: // Custom
                    // Do nothing, allow user to input custom values
                    break;
            }
            OnPropertyChanged(nameof(CustomTitleBtnInputVisibility));
        }

        [ObservableProperty]
        private int? _customCloseBtnW = 49;

        partial void OnCustomCloseBtnWChanged(int? value)
        {
            if (value.HasValue)
                _configManager.Config.CustomCloseBtnW = value.Value;
        }

        [ObservableProperty]
        private int? _customMaxBtnW = 27;

        partial void OnCustomMaxBtnWChanged(int? value)
        {
            if (value.HasValue)
                _configManager.Config.CustomMaxBtnW = value.Value;
        }

        [ObservableProperty]
        private int? _customMinBtnW = 29;

        partial void OnCustomMinBtnWChanged(int? value)
        {
            if (value.HasValue)
                _configManager.Config.CustomMinBtnW = value.Value;
        }

        [ObservableProperty]
        private int? _customBtnFrameH = 20;

        partial void OnCustomBtnFrameHChanged(int? value)
        {
            if (value.HasValue)
                _configManager.Config.CustomBtnFrameH = value.Value;
        }

        public Visibility CustomTitleBtnInputVisibility => CustomTitleBtnSize && TitleBtnSizePresetIndex == 2 ? Visibility.Visible : Visibility.Collapsed;

        [RelayCommand]
        private async Task BrowseTexture()
        {
            var openFileDialog = new OpenFileDialog
            {
                Filter = "PNG files (*.png)|*.png",
                Title = Lang["customAeroTexture"]
            };

            if (openFileDialog.ShowDialog() == true)
            {
                string selectedPath = openFileDialog.FileName;
                
                // Check if the file is in user profile directory
                string userProfilePath = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                if (selectedPath.Length >= userProfilePath.Length &&
                    selectedPath.StartsWith(userProfilePath, StringComparison.OrdinalIgnoreCase))
                {
                    // Show error dialog
                    var dialogHost = _contentDialogService.GetDialogHost();
                    if (dialogHost != null)
                    {
                        var dialog = new ContentDialog
                        {
                            Title = Lang["customAeroTexture"],
                            Content = Lang["texturepath_error"],
                            CloseButtonText = Lang["msgdlg_yes"],
                            DialogHost = dialogHost
                        };
                        await dialog.ShowAsync();
                    }
                    return;
                }
                
                CustomAeroTexturePath = selectedPath;
            }
        }

        // Enabled state properties based on BlurMethod
        // CustomBlur (0): All enabled
        // OldBlur/AccentBlur (1): Disable effect type, most misc settings (except UseSystemThemeColor, EnableBlurOptimization)
        // DWMAPI (2): All disabled

        public bool IsEffectTypeEnabled => SelectedBlurMethodIndex == 0;
        public bool IsCrossFadeEnabled => SelectedBlurMethodIndex == 0;
        public bool IsCrossFadeDurationEnabled => SelectedBlurMethodIndex == 0 && EnableCrossFade;
        public bool IsOverrideAccentBlurEnabled => SelectedBlurMethodIndex == 0;
        public bool IsDisableInPowerSavingEnabled => SelectedBlurMethodIndex == 0;
        public bool IsDisableDcompRateLimitEnabled => SelectedBlurMethodIndex == 0;
        public bool IsAeroReflectionOpacityEnabled => SelectedBlurMethodIndex == 0;
        
        // These two are enabled for both CustomBlur and OldBlur
        public bool IsUseSystemThemeColorEnabled => SelectedBlurMethodIndex != 2;
        public bool IsEnableBlurOptimizationEnabled => SelectedBlurMethodIndex != 2;

        private void UpdateEnabledStates()
        {
            OnPropertyChanged(nameof(IsEffectTypeEnabled));
            OnPropertyChanged(nameof(IsCrossFadeEnabled));
            OnPropertyChanged(nameof(IsCrossFadeDurationEnabled));
            OnPropertyChanged(nameof(IsOverrideAccentBlurEnabled));
            OnPropertyChanged(nameof(IsDisableInPowerSavingEnabled));
            OnPropertyChanged(nameof(IsDisableDcompRateLimitEnabled));
            OnPropertyChanged(nameof(IsAeroReflectionOpacityEnabled));
            OnPropertyChanged(nameof(IsUseSystemThemeColorEnabled));
            OnPropertyChanged(nameof(IsEnableBlurOptimizationEnabled));
        }
    }
}
