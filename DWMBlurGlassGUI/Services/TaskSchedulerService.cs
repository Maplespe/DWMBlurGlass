using Microsoft.Win32.TaskScheduler;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace DWMBlurGlassGUI.Services
{
    /// <summary>
    /// Service for managing Windows Task Scheduler tasks using COM interface
    /// </summary>
    public static class TaskSchedulerService
    {
        private const string TaskName = "DWMBlurGlass_Extend";
        private static readonly IntPtr HWND_MESSAGE = new IntPtr(-3);
        private const int WM_APP = 0x8000;

        /// <summary>
        /// Check if the DWMBlurGlass task is installed
        /// </summary>
        public static bool IsTaskInstalled()
        {
            try
            {
                using var ts = new TaskService();
                var task = ts.GetTask(TaskName);
                return task != null;
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Install the DWMBlurGlass scheduled task
        /// </summary>
        public static (bool Success, string? ErrorMessage) InstallTask()
        {
            try
            {
                string dllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "DWMBlurGlassHost.dll");
                
                if (!File.Exists(dllPath))
                {
                    return (false, "DWMBlurGlassHost.dll not found");
                }

                using var ts = new TaskService();

                // Create a new task definition
                var td = ts.NewTask();

                // Set task properties
                td.RegistrationInfo.Author = "DWMBlurGlass";
                td.RegistrationInfo.Description = "DWMBlurGlass Extension Host";

                // Set principal (run with highest privileges)
                td.Principal.LogonType = TaskLogonType.InteractiveToken;
                td.Principal.RunLevel = TaskRunLevel.Highest;

                // Set settings
                td.Settings.MultipleInstances = TaskInstancesPolicy.Parallel;
                td.Settings.DisallowStartIfOnBatteries = false;
                td.Settings.StopIfGoingOnBatteries = false;
                td.Settings.AllowHardTerminate = true;
                td.Settings.StartWhenAvailable = false;
                td.Settings.RunOnlyIfNetworkAvailable = false;
                td.Settings.AllowDemandStart = true;
                td.Settings.Enabled = true;
                td.Settings.Hidden = true;
                td.Settings.RunOnlyIfIdle = false;
                td.Settings.WakeToRun = false;
                td.Settings.ExecutionTimeLimit = TimeSpan.Zero;
                td.Settings.Priority = ProcessPriorityClass.Normal;

                // Add logon trigger
                td.Triggers.Add(new LogonTrigger { Enabled = true });

                // Add action - use cmd to run the dll (which is actually an exe with .dll extension)
                td.Actions.Add(new ExecAction(dllPath, "runhost", null));

                // Register the task (will overwrite if exists)
                ts.RootFolder.RegisterTaskDefinition(TaskName, td);

                return (true, null);
            }
            catch (Exception ex)
            {
                return (false, ex.Message);
            }
        }

        /// <summary>
        /// Uninstall the DWMBlurGlass scheduled task
        /// </summary>
        public static (bool Success, string? ErrorMessage) UninstallTask()
        {
            try
            {
                using var ts = new TaskService();
                
                // Check if task exists
                var task = ts.GetTask(TaskName);
                if (task == null)
                {
                    return (true, null); // Task doesn't exist, consider it success
                }

                // Delete the task
                ts.RootFolder.DeleteTask(TaskName, false);

                return (true, null);
            }
            catch (Exception ex)
            {
                return (false, ex.Message);
            }
        }

        /// <summary>
        /// Run the host process
        /// </summary>
        public static void RunHostProcess()
        {
            try
            {
                string dllPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "DWMBlurGlassHost.dll");
                if (!File.Exists(dllPath)) return;
                // DWMBlurGlassHost.dll is actually an EXE with .dll extension
                // Use cmd to run it directly
                var startInfo = new ProcessStartInfo
                {
                    FileName = "cmd.exe",
                    Arguments = $"/c \"\"{dllPath}\" runhost\"",
                    UseShellExecute = false,
                    WindowStyle = ProcessWindowStyle.Hidden,
                    CreateNoWindow = true
                };
                Process.Start(startInfo);
            }
            catch
            {
                // Ignore errors
            }
        }

        /// <summary>
        /// Stop the host process
        /// </summary>
        public static void StopHostProcess()
        {
            try
            {
                // Send notification to MHost to stop (MClientNotifyType::Shutdown = 1)
                // This will trigger the host to unload the DWM extension first
                // MDWMBlurGlassHostNotify is a message-only window, so we need FindWindowEx with HWND_MESSAGE
                IntPtr hwnd = FindWindowEx(HWND_MESSAGE, IntPtr.Zero, "MDWMBlurGlassHostNotify", null);
                if (hwnd != IntPtr.Zero)
                {
                    // MClientNotifyType::Shutdown = 1
                    SendMessage(hwnd, WM_APP + 20, (IntPtr)1, IntPtr.Zero);
                }

                // Kill the host process - need to find it by process name
                // Since it's a .dll file (renamed exe), we need to find it differently
                foreach (var process in Process.GetProcesses())
                {
                    try
                    {
                        string processName = process.ProcessName.ToLowerInvariant();
                        // The process might show up as "DWMBlurGlassHost" or with full path
                        if (!processName.Contains("dwmblurglasshost")) continue;
                        if (process.HasExited) continue;

                        process.Kill();
                        process.WaitForExit(3000);
                    }
                    catch
                    {
                        // Ignore - process may have already exited or access denied
                    }
                    finally
                    {
                        process.Dispose();
                    }
                }
            }
            catch
            {
                // Ignore errors
            }
        }

        /// <summary>
        /// Refresh DWM configuration
        /// </summary>
        public static void RefreshDwmConfig()
        {
            try
            {
                // Notify DWM Extension
                // Send notification to DWM extension to refresh (MHostNotifyType::Refresh = 0)
                // MDWMBlurGlassExtNotify is a message-only window
                IntPtr hwndExt = FindWindowEx(HWND_MESSAGE, IntPtr.Zero, "MDWMBlurGlassExtNotify", null);
                if (hwndExt != IntPtr.Zero)
                {
                    // MHostNotifyType::Refresh = 0
                    SendMessage(hwndExt, WM_APP + 20, (IntPtr)0, IntPtr.Zero);
                }

                // Refresh System Config (DWM)
                IntPtr hwndDwm = FindWindow("Dwm", null);
                if (hwndDwm != IntPtr.Zero)
                {
                    const int WM_THEMECHANGED = 0x031A;
                    PostMessage(hwndDwm, WM_THEMECHANGED, IntPtr.Zero, IntPtr.Zero);
                }

                // Invalidate all windows
                InvalidateRect(IntPtr.Zero, IntPtr.Zero, false);
            }
            catch
            {
                // Ignore errors
            }
        }

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr FindWindow(string lpClassName, string? lpWindowName);

        [DllImport("user32.dll", SetLastError = true, CharSet = CharSet.Unicode)]
        private static extern IntPtr FindWindowEx(IntPtr parentHandle, IntPtr childAfter, string lpszClass, string? lpszWindow);

        [DllImport("user32.dll")]
        private static extern bool PostMessage(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern IntPtr SendMessage(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);

        [DllImport("user32.dll")]
        private static extern bool InvalidateRect(IntPtr hWnd, IntPtr lpRect, bool bErase);
    }
}
