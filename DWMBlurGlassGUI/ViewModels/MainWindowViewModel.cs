using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using System.Collections.ObjectModel;
using Wpf.Ui.Controls;
using DWMBlurGlassGUI.Services;

namespace DWMBlurGlassGUI.ViewModels
{
    public partial class MainWindowViewModel : ObservableObject
    {
        private readonly ConfigManager _configManager;
        private readonly LanguageService _langService;

        public MainWindowViewModel(ConfigManager configManager, LanguageService langService)
        {
            _configManager = configManager;
            _langService = langService;
            
            _configManager.PropertyChanged += (s, e) =>
            {
                if (e.PropertyName == nameof(ConfigManager.IsModified))
                {
                    SaveCommand.NotifyCanExecuteChanged();
                }
            };

            _langService.LanguageChanged += (s, e) => UpdateMenuItemsLanguage();
            InitializeMenuItems();
        }

        private void InitializeMenuItems()
        {
            MenuItems = new ObservableCollection<object>
            {
                new NavigationViewItem()
                {
                    Content = _langService["general"],
                    Icon = new SymbolIcon { Symbol = SymbolRegular.Settings24 },
                    TargetPageType = typeof(Views.Pages.GeneralSettingsPage)
                },
                new NavigationViewItem()
                {
                    Content = _langService["advanced"],
                    Icon = new SymbolIcon { Symbol = SymbolRegular.Wrench24 },
                    TargetPageType = typeof(Views.Pages.AdvancedSettingsPage)
                },
                new NavigationViewItem()
                {
                    Content = _langService["symbol"],
                    Icon = new SymbolIcon { Symbol = SymbolRegular.Bug24 },
                    TargetPageType = typeof(Views.Pages.SymbolFilesPage)
                },
                new NavigationViewItem()
                {
                    Content = _langService["config"],
                    Icon = new SymbolIcon { Symbol = SymbolRegular.Document24 },
                    TargetPageType = typeof(Views.Pages.ConfigFilesPage)
                }
            };

            FooterMenuItems = new ObservableCollection<object>
            {
                new NavigationViewItem()
                {
                    Content = _langService["about"],
                    Icon = new SymbolIcon { Symbol = SymbolRegular.Info24 },
                    TargetPageType = typeof(Views.Pages.AboutPage)
                }
            };
        }

        private void UpdateMenuItemsLanguage()
        {
            if (MenuItems.Count >= 4)
            {
                ((NavigationViewItem)MenuItems[0]).Content = _langService["general"];
                ((NavigationViewItem)MenuItems[1]).Content = _langService["advanced"];
                ((NavigationViewItem)MenuItems[2]).Content = _langService["symbol"];
                ((NavigationViewItem)MenuItems[3]).Content = _langService["config"];
            }
            
            if (FooterMenuItems.Count >= 1)
            {
                ((NavigationViewItem)FooterMenuItems[0]).Content = _langService["about"];
            }
        }

        [RelayCommand(CanExecute = nameof(CanSave))]
        private void Save()
        {
            _configManager.SaveConfig();
            if (TaskSchedulerService.IsTaskInstalled())
            {
                TaskSchedulerService.RefreshDwmConfig();
            }
        }

        private bool CanSave() => _configManager.IsModified;

        [ObservableProperty]
        private string _applicationTitle = "DWMBlurGlass";

        [ObservableProperty]
        private ObservableCollection<object> _menuItems = new();

        [ObservableProperty]
        private ObservableCollection<object> _footerMenuItems = new();
    }
}
