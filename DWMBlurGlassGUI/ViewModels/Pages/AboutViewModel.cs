using CommunityToolkit.Mvvm.ComponentModel;
using DWMBlurGlassGUI.Services;

namespace DWMBlurGlassGUI.ViewModels.Pages
{
    public partial class AboutViewModel : ObservableObject
    {
        public LanguageService Lang => LanguageService.Instance;

        [ObservableProperty]
        private string _appVersion = "2.3.2 Beta 2";
    }
}
