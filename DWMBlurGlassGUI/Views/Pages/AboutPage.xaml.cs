using DWMBlurGlassGUI.ViewModels.Pages;
using System.Windows.Controls;

namespace DWMBlurGlassGUI.Views.Pages
{
    public partial class AboutPage : Page
    {
        public AboutViewModel ViewModel { get; }

        public AboutPage(AboutViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            InitializeComponent();
        }
    }
}
