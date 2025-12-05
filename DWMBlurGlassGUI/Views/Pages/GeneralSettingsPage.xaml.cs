using DWMBlurGlassGUI.ViewModels.Pages;
using System.Windows.Controls;

namespace DWMBlurGlassGUI.Views.Pages
{
    public partial class GeneralSettingsPage : Page
    {
        public GeneralSettingsViewModel ViewModel { get; }

        public GeneralSettingsPage(GeneralSettingsViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            InitializeComponent();
        }
    }
}
