using System;
using System.IO;
using System.Threading;
using System.Windows;
using System.Windows.Threading;
using Microsoft.Extensions.DependencyInjection;
using DWMBlurGlassGUI.Services;
using DWMBlurGlassGUI.ViewModels;
using DWMBlurGlassGUI.ViewModels.Pages;
using DWMBlurGlassGUI.Views;
using DWMBlurGlassGUI.Views.Pages;
using Wpf.Ui;
using Wpf.Ui.Abstractions;
using Wpf.Ui.Appearance;

namespace DWMBlurGlassGUI
{
    /// <summary>
    /// Interaction logic for App.xaml
    /// </summary>
    public partial class App : Application
    {
        private static readonly IServiceProvider _serviceProvider;
        private static Mutex? _mutex;

        static App()
        {
            var services = new ServiceCollection();
            ConfigureServices(services);
            _serviceProvider = services.BuildServiceProvider();
        }

        private static void ConfigureServices(IServiceCollection services)
        {
            // Services
            services.AddSingleton<INavigationViewPageProvider, PageService>();
            services.AddSingleton<INavigationService, NavigationService>();
            services.AddSingleton<IContentDialogService, ContentDialogService>();
            services.AddSingleton<ConfigManager>();
            services.AddSingleton(LanguageService.Instance);

            // Main Window
            services.AddSingleton<MainWindowViewModel>();
            services.AddSingleton<MainWindow>();

            // Pages
            services.AddSingleton<GeneralSettingsPage>();
            services.AddSingleton<GeneralSettingsViewModel>();

            services.AddSingleton<AdvancedSettingsPage>();
            services.AddSingleton<AdvancedSettingsViewModel>();

            services.AddSingleton<SymbolFilesPage>();
            services.AddSingleton<SymbolFilesViewModel>();

            services.AddSingleton<ConfigFilesPage>();
            services.AddSingleton<ConfigFilesViewModel>();

            services.AddSingleton<AboutPage>();
            services.AddSingleton<AboutViewModel>();
        }

        public static IServiceProvider Services => _serviceProvider;

        public static T GetService<T>() where T : class
        {
            return _serviceProvider.GetRequiredService<T>();
        }

        protected override void OnStartup(StartupEventArgs e)
        {
            // Check for duplicate instance using Mutex
            _mutex = new Mutex(true, "_DWMBlurGlassGUI_", out bool createdNew);
            if (!createdNew)
            {
                // Another instance is already running
                MessageBox.Show("DWMBlurGlass GUI is already running.", "Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                Shutdown();
                return;
            }

            // Check if running from user profile directory (forbidden)
            if (!CheckAllowedDirectory())
            {
                Shutdown();
                return;
            }

            base.OnStartup(e);

            ApplicationThemeManager.ApplySystemTheme();

            var mainWindow = GetService<MainWindow>();
            mainWindow.Show();
        }

        /// <summary>
        /// Check if the application is running from an allowed directory.
        /// The application must not be placed in the user profile directory.
        /// </summary>
        private bool CheckAllowedDirectory()
        {
            try
            {
                string userProfilePath = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
                string currentPath = AppDomain.CurrentDomain.BaseDirectory;

                if (currentPath.Length >= userProfilePath.Length &&
                    currentPath.StartsWith(userProfilePath, StringComparison.OrdinalIgnoreCase))
                {
                    var lang = LanguageService.Instance;
                    string message = lang["initfail0"].Replace("{path}", userProfilePath);
                    MessageBox.Show(message, "Warning", MessageBoxButton.OK, MessageBoxImage.Warning);
                    return false;
                }
            }
            catch
            {
                // Ignore errors in path checking
            }

            return true;
        }

        protected override void OnExit(ExitEventArgs e)
        {
            // Release the mutex when exiting
            _mutex?.ReleaseMutex();
            _mutex?.Dispose();
            _mutex = null;

            base.OnExit(e);
        }

        private void OnDispatcherUnhandledException(object sender, DispatcherUnhandledExceptionEventArgs e)
        {
            // Handle exceptions here
            MessageBox.Show($"发生未处理的异常: {e.Exception.Message}", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
            e.Handled = true;
        }
    }
}
