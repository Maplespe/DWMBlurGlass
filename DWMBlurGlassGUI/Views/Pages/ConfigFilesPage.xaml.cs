using DWMBlurGlassGUI.ViewModels.Pages;
using System.Windows.Controls;

namespace DWMBlurGlassGUI.Views.Pages
{
    public partial class ConfigFilesPage : Page
    {
        public ConfigFilesViewModel ViewModel { get; }

        public ConfigFilesPage(ConfigFilesViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            InitializeComponent();
        }
    }
}
