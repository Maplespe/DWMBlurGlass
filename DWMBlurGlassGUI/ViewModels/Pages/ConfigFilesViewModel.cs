using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DWMBlurGlassGUI.Services;
using Microsoft.Win32;
using System.Threading.Tasks;
using Wpf.Ui;
using Wpf.Ui.Controls;

namespace DWMBlurGlassGUI.ViewModels.Pages
{
    public partial class ConfigFilesViewModel : ObservableObject
    {
        private readonly ConfigManager _configManager;
        private readonly IContentDialogService _contentDialogService;

        public LanguageService Lang => LanguageService.Instance;

        public ConfigFilesViewModel(ConfigManager configManager, IContentDialogService contentDialogService)
        {
            _configManager = configManager;
            _contentDialogService = contentDialogService;
        }

        [RelayCommand]
        private async Task ImportConfig()
        {
            var openFileDialog = new OpenFileDialog
            {
                Filter = "Config files (*.ini)|*.ini|All files (*.*)|*.*",
                FileName = "config.ini"
            };

            if (openFileDialog.ShowDialog() == true)
            {
                _configManager.ImportConfig(openFileDialog.FileName);
            }
            await Task.CompletedTask;
        }

        [RelayCommand]
        private async Task ExportConfig()
        {
            var saveFileDialog = new SaveFileDialog
            {
                Filter = "Config files (*.ini)|*.ini|All files (*.*)|*.*",
                FileName = "config.ini"
            };

            if (saveFileDialog.ShowDialog() == true)
            {
                _configManager.ExportConfig(saveFileDialog.FileName);
            }
            await Task.CompletedTask;
        }

        [RelayCommand]
        private async Task RestoreDefaultConfig()
        {
            var dialogHost = _contentDialogService.GetDialogHost();
            if (dialogHost == null) return;

            var dialog = new ContentDialog
            {
                Title = Lang["restore"],
                Content = Lang["restorecfg"],
                PrimaryButtonText = Lang["msgdlg_yes"],
                CloseButtonText = Lang["msgdlg_no"],
                DialogHost = dialogHost
            };

            var result = await dialog.ShowAsync();
            if (result == ContentDialogResult.Primary)
            {
                _configManager.RestoreDefaultConfig();
            }
        }
    }
}
