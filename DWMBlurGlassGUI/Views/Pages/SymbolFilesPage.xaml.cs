using DWMBlurGlassGUI.ViewModels.Pages;
using System.Windows.Controls;

namespace DWMBlurGlassGUI.Views.Pages
{
    public partial class SymbolFilesPage : Page
    {
        public SymbolFilesViewModel ViewModel { get; }

        public SymbolFilesPage(SymbolFilesViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            InitializeComponent();
        }
    }
}
