using DWMBlurGlassGUI.ViewModels;
using DWMBlurGlassGUI.Views.Pages;
using Wpf.Ui;
using Wpf.Ui.Abstractions;
using Wpf.Ui.Controls;

namespace DWMBlurGlassGUI.Views
{
    public partial class MainWindow : FluentWindow
    {
        public MainWindowViewModel ViewModel { get; }

        public MainWindow(MainWindowViewModel viewModel, INavigationViewPageProvider pageService, INavigationService navigationService, IContentDialogService contentDialogService)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            
            InitializeComponent();
            
            NavigationView.SetPageProviderService(pageService);
            navigationService.SetNavigationControl(NavigationView);
            contentDialogService.SetDialogHost(RootContentDialog);

            // Navigate to the first page after loaded
            Loaded += (s, e) => NavigationView.Navigate(typeof(GeneralSettingsPage));
        }
    }
}
