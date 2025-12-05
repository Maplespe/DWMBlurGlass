using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DWMBlurGlassGUI.Models;
using DWMBlurGlassGUI.Services;
using DWMBlurGlassGUI.Views.Controls;
using DWMBlurGlassGUI.Views.Pages;
using Microsoft.Win32;
using System;
using System.Threading.Tasks;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Wpf.Ui;
using Wpf.Ui.Controls;

namespace DWMBlurGlassGUI.ViewModels.Pages
{
    public partial class GeneralSettingsViewModel : ObservableObject
    {
        private readonly IContentDialogService _contentDialogService;
        private readonly INavigationService _navigationService;
        private readonly ConfigManager _configManager;
        private readonly SymbolDownloader _symbolDownloader;

        public LanguageService Lang => LanguageService.Instance;

        public GeneralSettingsViewModel(IContentDialogService contentDialogService, INavigationService navigationService, ConfigManager configManager)
        {
            _navigationService = navigationService;
            _contentDialogService = contentDialogService;
            _symbolDownloader = new SymbolDownloader();
            _configManager = configManager;
            _configManager.ConfigReloaded += (s, e) => LoadFromConfig();
            _configManager.Config.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(AppConfig.BlurMethod))
                    UpdateEnabledStates();
                if (e.PropertyName == nameof(AppConfig.CustomTitleBtnSize) ||
                    e.PropertyName == nameof(AppConfig.TitleBtnSizePreset))
                    OnPropertyChanged(nameof(TitleBtnSizePresetName));
            };
            _configManager.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(ConfigManager.IsModified))
                    SaveConfigCommand.NotifyCanExecuteChanged();
            };
            LoadWallpaper();
            LoadFromConfig();
            CheckInstallStatus();
        }

        private void CheckInstallStatus()
        {
            IsInstalled = TaskSchedulerService.IsTaskInstalled();
            StatusText = IsInstalled ? Lang["status1"] : Lang["status0"];
        }

        private void LoadFromConfig()
        {
            var config = _configManager.Config;
            OverrideDwmApi = config.ApplyGlobal;
            ExtendToBorder = config.ExtendBorder;
            EnableAeroReflection = config.Reflection;
            RestoreWin7ButtonSize = config.OldBtnHeight;
            EnableWin7ButtonGlow = config.TitleBtnGlow;
            CustomAmount = config.CustomAmount;
            BlurRadius = (int)config.BlurAmount;

            _lightActiveTitleBarColor = UIntToColor(config.ActiveBlendColor);
            _lightInactiveTitleBarColor = UIntToColor(config.InactiveBlendColor);
            _lightActiveTextColor = UIntToColor(config.ActiveTextColor);
            _lightInactiveTextColor = UIntToColor(config.InactiveTextColor);

            _darkActiveTitleBarColor = UIntToColor(config.ActiveBlendColorDark);
            _darkInactiveTitleBarColor = UIntToColor(config.InactiveBlendColorDark);
            _darkActiveTextColor = UIntToColor(config.ActiveTextColorDark);
            _darkInactiveTextColor = UIntToColor(config.InactiveTextColorDark);

            OnPropertyChanged(nameof(ActiveTitleBarColor));
            OnPropertyChanged(nameof(InactiveTitleBarColor));
            OnPropertyChanged(nameof(ActiveTextColor));
            OnPropertyChanged(nameof(InactiveTextColor));
        }

        private void SaveToConfig()
        {
            var config = _configManager.Config;
            config.ApplyGlobal = OverrideDwmApi;
            config.ExtendBorder = ExtendToBorder;
            config.Reflection = EnableAeroReflection;
            config.OldBtnHeight = RestoreWin7ButtonSize;
            config.TitleBtnGlow = EnableWin7ButtonGlow;
            config.CustomAmount = CustomAmount;
            config.BlurAmount = BlurRadius;

            config.ActiveBlendColor = ColorToUInt(_lightActiveTitleBarColor);
            config.InactiveBlendColor = ColorToUInt(_lightInactiveTitleBarColor);
            config.ActiveTextColor = ColorToUInt(_lightActiveTextColor);
            config.InactiveTextColor = ColorToUInt(_lightInactiveTextColor);

            config.ActiveBlendColorDark = ColorToUInt(_darkActiveTitleBarColor);
            config.InactiveBlendColorDark = ColorToUInt(_darkInactiveTitleBarColor);
            config.ActiveTextColorDark = ColorToUInt(_darkActiveTextColor);
            config.InactiveTextColorDark = ColorToUInt(_darkInactiveTextColor);

            _configManager.SaveConfig();
        }

        private Color UIntToColor(uint color)
        {
            // Format: 0xAARRGGBB
            return Color.FromArgb((byte)(color >> 24), (byte)color, (byte)(color >> 8), (byte)(color >> 16));
        }

        private uint ColorToUInt(Color color)
        {
            // Format: 0xAARRGGBB
            return ((uint)color.A << 24) | ((uint)color.B << 16) | ((uint)color.G << 8) | color.R;
        }

        [ObservableProperty]
        private ImageSource? _wallpaperSource;

        private void LoadWallpaper()
        {
            try
            {
                var registryKey = Registry.CurrentUser.OpenSubKey(@"Control Panel\Desktop");
                if (registryKey != null)
                {
                    var wallpaperPath = registryKey.GetValue("Wallpaper")?.ToString();
                    if (!string.IsNullOrEmpty(wallpaperPath) && System.IO.File.Exists(wallpaperPath))
                    {
                        var bitmap = new BitmapImage();
                        bitmap.BeginInit();
                        bitmap.CacheOption = BitmapCacheOption.OnLoad;
                        bitmap.UriSource = new Uri(wallpaperPath);
                        bitmap.EndInit();
                        bitmap.Freeze();
                        WallpaperSource = bitmap;
                    }
                }
            }
            catch
            {
                // Ignore errors
            }
        }

        [ObservableProperty]
        private string _pageTitle = "通用设置";

        [ObservableProperty]
        private string _statusText = "未安装";

        [ObservableProperty]
        [NotifyPropertyChangedFor(nameof(CanInstall))]
        private bool _isInstalled = false;

        public bool CanInstall => !IsInstalled;

        [ObservableProperty]
        private bool _overrideDwmApi = true;

        partial void OnOverrideDwmApiChanged(bool value)
        {
            _configManager.Config.ApplyGlobal = value;
        }

        [ObservableProperty]
        private bool _extendToBorder = false;

        partial void OnExtendToBorderChanged(bool value)
        {
            _configManager.Config.ExtendBorder = value;
        }

        [ObservableProperty]
        private bool _enableAeroReflection = true;

        partial void OnEnableAeroReflectionChanged(bool value)
        {
            _configManager.Config.Reflection = value;
        }

        [ObservableProperty]
        private bool _restoreWin7ButtonSize = true;

        partial void OnRestoreWin7ButtonSizeChanged(bool value)
        {
            _configManager.Config.OldBtnHeight = value;
        }

        public string TitleBtnSizePresetName
        {
            get
            {
                var config = _configManager.Config;
                if (!config.CustomTitleBtnSize)
                    return "Windows 7";
                
                // Use saved preset index
                return config.TitleBtnSizePreset switch
                {
                    0 => "Windows 7",
                    1 => "Windows Vista",
                    _ => Lang["custom"]
                };
            }
        }

        [RelayCommand]
        private void NavigateToTitleBtnSizeSettings()
        {
            AdvancedSettingsPage.ScrollToTitleBtnSizeOnLoad = true;
            _navigationService.Navigate(typeof(AdvancedSettingsPage));
        }

        [ObservableProperty]
        private bool _enableWin7ButtonGlow = false;

        partial void OnEnableWin7ButtonGlowChanged(bool value)
        {
            _configManager.Config.TitleBtnGlow = value;
        }

        [ObservableProperty]
        private bool _customAmount = false;

        partial void OnCustomAmountChanged(bool value)
        {
            _configManager.Config.CustomAmount = value;
        }

        [ObservableProperty]
        private int _blurRadius = 15;

        partial void OnBlurRadiusChanged(int value)
        {
            _configManager.Config.BlurAmount = value;
        }

        [ObservableProperty]
        private int _selectedThemeIndex = 0;

        // Light Theme Colors Storage
        private Color _lightActiveTitleBarColor = Color.FromArgb(100, 255, 66, 220);
        private Color _lightInactiveTitleBarColor = Color.FromArgb(100, 62, 239, 59);
        private Color _lightActiveTextColor = Color.FromRgb(0, 0, 0);
        private Color _lightInactiveTextColor = Color.FromRgb(180, 180, 180);

        // Dark Theme Colors Storage
        private Color _darkActiveTitleBarColor = Color.FromArgb(100, 0, 0, 0);
        private Color _darkInactiveTitleBarColor = Color.FromArgb(100, 30, 30, 30);
        private Color _darkActiveTextColor = Color.FromRgb(255, 255, 255);
        private Color _darkInactiveTextColor = Color.FromRgb(150, 150, 150);

        // Proxy Properties
        public Color ActiveTitleBarColor
        {
            get => SelectedThemeIndex == 0 ? _lightActiveTitleBarColor : _darkActiveTitleBarColor;
            set
            {
                if (SelectedThemeIndex == 0)
                {
                    if (_lightActiveTitleBarColor != value)
                    {
                        _lightActiveTitleBarColor = value;
                        _configManager.Config.ActiveBlendColor = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
                else
                {
                    if (_darkActiveTitleBarColor != value)
                    {
                        _darkActiveTitleBarColor = value;
                        _configManager.Config.ActiveBlendColorDark = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
            }
        }

        public Color InactiveTitleBarColor
        {
            get => SelectedThemeIndex == 0 ? _lightInactiveTitleBarColor : _darkInactiveTitleBarColor;
            set
            {
                if (SelectedThemeIndex == 0)
                {
                    if (_lightInactiveTitleBarColor != value)
                    {
                        _lightInactiveTitleBarColor = value;
                        _configManager.Config.InactiveBlendColor = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
                else
                {
                    if (_darkInactiveTitleBarColor != value)
                    {
                        _darkInactiveTitleBarColor = value;
                        _configManager.Config.InactiveBlendColorDark = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
            }
        }

        public Color ActiveTextColor
        {
            get => SelectedThemeIndex == 0 ? _lightActiveTextColor : _darkActiveTextColor;
            set
            {
                if (SelectedThemeIndex == 0)
                {
                    if (_lightActiveTextColor != value)
                    {
                        _lightActiveTextColor = value;
                        _configManager.Config.ActiveTextColor = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
                else
                {
                    if (_darkActiveTextColor != value)
                    {
                        _darkActiveTextColor = value;
                        _configManager.Config.ActiveTextColorDark = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
            }
        }

        public Color InactiveTextColor
        {
            get => SelectedThemeIndex == 0 ? _lightInactiveTextColor : _darkInactiveTextColor;
            set
            {
                if (SelectedThemeIndex == 0)
                {
                    if (_lightInactiveTextColor != value)
                    {
                        _lightInactiveTextColor = value;
                        _configManager.Config.InactiveTextColor = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
                else
                {
                    if (_darkInactiveTextColor != value)
                    {
                        _darkInactiveTextColor = value;
                        _configManager.Config.InactiveTextColorDark = ColorToUInt(value);
                        OnPropertyChanged();
                    }
                }
            }
        }

        public Brush ContentAreaBrush => SelectedThemeIndex == 0 
            ? new SolidColorBrush(Color.FromArgb(100, 255, 255, 255)) 
            : new SolidColorBrush(Color.FromArgb(100, 0, 0, 0));

        partial void OnSelectedThemeIndexChanged(int value)
        {
            OnPropertyChanged(nameof(ActiveTitleBarColor));
            OnPropertyChanged(nameof(InactiveTitleBarColor));
            OnPropertyChanged(nameof(ActiveTextColor));
            OnPropertyChanged(nameof(InactiveTextColor));
            OnPropertyChanged(nameof(ContentAreaBrush));
        }

        [RelayCommand]
        private async Task OnInstall()
        {
            // First save current config
            SaveToConfig();
            _configManager.SaveConfig();

            var (success, errorMessage) = TaskSchedulerService.InstallTask();
            if (success)
            {
                IsInstalled = true;
                StatusText = Lang["status1"];

                // Check if symbols are valid, if not show warning
                if (!_symbolDownloader.CheckSymbolsValid())
                {
                    await ShowMessageDialog("Info", Lang["installsucs1"]);
                }
                else
                {
                    // Start the host process
                    TaskSchedulerService.RunHostProcess();
                }
            }
            else
            {
                await ShowMessageDialog("Error", Lang["installfail"] + (errorMessage != null ? $"{errorMessage}" : ""));
            }
        }

        [RelayCommand]
        private void OnUninstall()
        {
            // Stop the running process first
            TaskSchedulerService.StopHostProcess();

            var (success, errorMessage) = TaskSchedulerService.UninstallTask();
            if (success)
            {
                IsInstalled = false;
                StatusText = Lang["status0"];
            }
            else
            {
                // Show error only on failure
                _ = ShowMessageDialog("Error", Lang["uninstallfail"] + (errorMessage != null ? $"{errorMessage}" : ""));
            }
        }

        private async Task ShowMessageDialog(string title, string message)
        {
            var dialogHost = _contentDialogService.GetDialogHost();
            if (dialogHost == null) return;

            var dialog = new ContentDialog
            {
                Title = title,
                Content = message,
                CloseButtonText = "OK",
                DialogHost = dialogHost
            };

            await dialog.ShowAsync();
        }

        [RelayCommand(CanExecute = nameof(CanSaveConfig))]
        private void OnSaveConfig()
        {
            SaveToConfig();
            
            if (IsInstalled)
            {
                TaskSchedulerService.RefreshDwmConfig();
            }
        }

        private bool CanSaveConfig() => _configManager.IsModified;

        [RelayCommand]
        private async Task OnPickColor(string colorType)
        {
            Color currentColor = colorType switch
            {
                "ActiveTitleBar" => ActiveTitleBarColor,
                "InactiveTitleBar" => InactiveTitleBarColor,
                "ActiveText" => ActiveTextColor,
                "InactiveText" => InactiveTextColor,
                _ => Colors.White
            };

            bool isRgbMode = colorType == "ActiveText" || colorType == "InactiveText";

            var dialogHost = _contentDialogService.GetDialogHost();
            var dialog = new ColorPickerDialog(dialogHost, currentColor, isRgbMode);

            var result = await dialog.ShowAsync();

            if (result == ContentDialogResult.Primary)
            {
                switch (colorType)
                {
                    case "ActiveTitleBar":
                        ActiveTitleBarColor = dialog.SelectedColor;
                        break;
                    case "InactiveTitleBar":
                        InactiveTitleBarColor = dialog.SelectedColor;
                        break;
                    case "ActiveText":
                        ActiveTextColor = dialog.SelectedColor;
                        break;
                    case "InactiveText":
                        InactiveTextColor = dialog.SelectedColor;
                        break;
                }
            }
        }

        // Enabled state properties based on BlurMethod
        // CustomBlur (0): All enabled
        // OldBlur/AccentBlur (1): All enabled except EnableWin7ButtonGlow
        // DWMAPI (2): All disabled except install module
        private BlurMethod CurrentBlurMethod => _configManager.Config.BlurMethod;
        
        public bool IsEffectsSettingsEnabled => CurrentBlurMethod != BlurMethod.DWMAPIBlur;
        public bool IsColorsSettingsEnabled => CurrentBlurMethod != BlurMethod.DWMAPIBlur;
        public bool IsPreviewEnabled => CurrentBlurMethod != BlurMethod.DWMAPIBlur;
        public bool IsWin7ButtonGlowEnabled => CurrentBlurMethod == BlurMethod.CustomBlur;

        private void UpdateEnabledStates()
        {
            OnPropertyChanged(nameof(IsEffectsSettingsEnabled));
            OnPropertyChanged(nameof(IsColorsSettingsEnabled));
            OnPropertyChanged(nameof(IsPreviewEnabled));
            OnPropertyChanged(nameof(IsWin7ButtonGlowEnabled));
        }
    }
}
