using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using DWMBlurGlassGUI.Services;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace DWMBlurGlassGUI.ViewModels.Pages
{
    public partial class SymbolFilesViewModel : ObservableObject
    {
        private readonly SymbolDownloader _symbolDownloader;
        private readonly ConfigManager _configManager;
        private CancellationTokenSource? _downloadCts;

        public LanguageService Lang => LanguageService.Instance;

        public SymbolFilesViewModel(ConfigManager configManager)
        {
            _symbolDownloader = new SymbolDownloader();
            _configManager = configManager;
            LoadFromConfig();
            CheckSymbolStatus();
        }

        private void LoadFromConfig()
        {
            AutoDownloadSymbols = _configManager.Config.AutoDownloadSymbols;
        }

        [ObservableProperty]
        private bool _autoDownloadSymbols = true;

        partial void OnAutoDownloadSymbolsChanged(bool value)
        {
            _configManager.Config.AutoDownloadSymbols = value;
        }

        [ObservableProperty]
        private string _symbolStatus = "...";

        [ObservableProperty]
        private bool _isSymbolValid = false;

        [ObservableProperty]
        private bool _isDownloading = false;

        [ObservableProperty]
        private int _downloadProgress = 0;

        [ObservableProperty]
        private string _downloadStatusText = "";

        [ObservableProperty]
        private bool _canDownload = true;

        private void CheckSymbolStatus()
        {
            try
            {
                IsSymbolValid = _symbolDownloader.CheckSymbolsValid();
                SymbolStatus = IsSymbolValid ? Lang["symstatus1"] : Lang["symstatus0"];
                CanDownload = !IsSymbolValid;
            }
            catch
            {
                IsSymbolValid = false;
                SymbolStatus = Lang["symstatus0"];
                CanDownload = true;
            }
        }

        [RelayCommand(CanExecute = nameof(CanDownload))]
        private async Task DownloadSymbols()
        {
            IsDownloading = true;
            CanDownload = false;
            DownloadProgress = 0;
            DownloadStatusText = Lang["symdowning"];

            try
            {
                _downloadCts = new CancellationTokenSource();
                var progress = new Progress<DownloadProgressEventArgs>(args =>
                {
                    DownloadProgress = args.ProgressPercentage;
                    if (args.TotalBytes > 0)
                    {
                        DownloadStatusText = $"{Lang["symdowning"]} {args.FileName} ({args.CurrentFile}/{args.TotalFiles}): {args.BytesReceived / 1024}KB / {args.TotalBytes / 1024}KB";
                    }
                    else
                    {
                        DownloadStatusText = $"{Lang["symdowning"]} {args.FileName} ({args.CurrentFile}/{args.TotalFiles})...";
                    }
                });

                bool success = await _symbolDownloader.DownloadSymbolsAsync(progress, _downloadCts.Token);

                if (success)
                {
                    DownloadStatusText = "";
                    DownloadProgress = 100;
                    CheckSymbolStatus();

                    // If task is installed and symbols are now valid, run host process
                    if (IsSymbolValid && TaskSchedulerService.IsTaskInstalled())
                    {
                        TaskSchedulerService.RunHostProcess();
                    }
                }
                else
                {
                    DownloadStatusText = Lang["symdownfail"];
                    DownloadProgress = 0;
                }
            }
            catch (OperationCanceledException)
            {
                DownloadStatusText = "";
                DownloadProgress = 0;
            }
            catch (Exception)
            {
                DownloadStatusText = Lang["symdownfail"];
                DownloadProgress = 0;
            }
            finally
            {
                IsDownloading = false;
                CanDownload = !IsSymbolValid;
                _downloadCts?.Dispose();
                _downloadCts = null;
            }
        }

        public void CancelDownload()
        {
            _downloadCts?.Cancel();
        }
    }
}
