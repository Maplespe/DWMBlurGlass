using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using CommunityToolkit.Mvvm.ComponentModel;
using DWMBlurGlassGUI.Models;

namespace DWMBlurGlassGUI.Services
{
    public partial class ConfigManager : ObservableObject
    {
        private const string ConfigFileName = "config.ini";
        private const string DataFolderName = "data";
        private const string SectionName = "config";
        private readonly string _configPath;

        [ObservableProperty]
        private bool _isModified;

        public AppConfig Config { get; private set; }

        public event EventHandler? ConfigReloaded;

        public ConfigManager()
        {
            var dataDir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, DataFolderName);
            Directory.CreateDirectory(dataDir);
            _configPath = Path.Combine(dataDir, ConfigFileName);
            Config = new AppConfig();
            InitializeConfigTracking();
            LoadConfig();
        }

        private void InitializeConfigTracking()
        {
            Config.PropertyChanged -= OnConfigPropertyChanged;
            Config.PropertyChanged += OnConfigPropertyChanged;
        }

        private void OnConfigPropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            IsModified = true;
        }

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern int GetPrivateProfileString(string section, string key, string defaultValue, StringBuilder retVal, int size, string filePath);

        [DllImport("kernel32", CharSet = CharSet.Unicode)]
        private static extern long WritePrivateProfileString(string section, string key, string value, string filePath);

        private string ReadString(string key, string defaultValue = "")
        {
            var sb = new StringBuilder(255);
            GetPrivateProfileString(SectionName, key, defaultValue, sb, 255, _configPath);
            return sb.ToString();
        }

        private void WriteString(string key, string value)
        {
            WritePrivateProfileString(SectionName, key, value, _configPath);
        }

        private bool ReadBool(string key, bool defaultValue)
        {
            string val = ReadString(key, defaultValue ? "true" : "false");
            return val.ToLower() == "true";
        }

        private void WriteBool(string key, bool value)
        {
            WriteString(key, value ? "true" : "false");
        }

        private int ReadInt(string key, int defaultValue)
        {
            string val = ReadString(key, defaultValue.ToString());
            if (int.TryParse(val, out int result))
                return result;
            return defaultValue;
        }

        private void WriteInt(string key, int value)
        {
            WriteString(key, value.ToString());
        }

        private uint ReadUInt(string key, uint defaultValue)
        {
            string val = ReadString(key, defaultValue.ToString());
            if (uint.TryParse(val, out uint result))
                return result;
            return defaultValue;
        }

        private void WriteUInt(string key, uint value)
        {
            WriteString(key, value.ToString());
        }

        private float ReadFloat(string key, float defaultValue)
        {
            string val = ReadString(key, defaultValue.ToString(CultureInfo.InvariantCulture));
            if (float.TryParse(val, out float result))
                return result;
            return defaultValue;
        }

        private void WriteFloat(string key, float value)
        {
            WriteString(key, value.ToString(CultureInfo.InvariantCulture));
        }

        public void LoadConfig()
        {
            if (!File.Exists(_configPath))
            {
                Config = new AppConfig();
                InitializeConfigTracking();
                IsModified = false;
                return;
            }

            Config.ApplyGlobal = ReadBool("applyglobal", false);
            Config.ExtendBorder = ReadBool("extendBorder", false);
            Config.Reflection = ReadBool("reflection", false);
            Config.OldBtnHeight = ReadBool("oldBtnHeight", false);
            Config.CustomAmount = ReadBool("customAmount", false);
            Config.UseAccentColor = ReadBool("useAccentColor", false);
            Config.CrossFade = ReadBool("crossFade", true);
            Config.OverrideAccent = ReadBool("overrideAccent", false);
            Config.ScaleOptimizer = ReadBool("occlusionCulling", false); // Note: key is occlusionCulling
            Config.DisableOnBattery = ReadBool("disableOnBattery", true);
            Config.TitleBtnGlow = ReadBool("titlebtnGlow", false);
            Config.DisableFramerateLimit = ReadBool("disableFramerateLimit", false);
            Config.AutoDownloadSymbols = ReadBool("autoDownloadSymbols", true);

            Config.ExtendRound = ReadInt("extendRound", 10);
            Config.TitleBtnOffsetX = ReadInt("titlebtnOffsetX", -1);
            Config.CustomTitleBtnSize = ReadBool("customTitleBtnSize", false);
            Config.TitleBtnSizePreset = ReadInt("titleBtnSizePreset", 0);
            Config.CustomCloseBtnW = ReadInt("customCloseBtnW", 49);
            Config.CustomMaxBtnW = ReadInt("customMaxBtnW", 27);
            Config.CustomMinBtnW = ReadInt("customMinBtnW", 29);
            Config.CustomBtnFrameH = ReadInt("customBtnFrameH", 20);

            Config.BlurAmount = ReadFloat("blurAmount", 20.0f);
            Config.CustomBlurAmount = ReadFloat("customBlurAmount", 20.0f);
            Config.LuminosityOpacity = ReadFloat("luminosityOpacity", 0.65f);
            Config.GlassIntensity = ReadFloat("glassIntensity", 1.0f);
            Config.CustomAeroTexture = ReadBool("customAeroTexture", false);
            Config.CustomAeroTexturePath = ReadString("customAeroTexturePath", "");

            Config.CrossFadeTime = ReadUInt("crossfadeTime", 160);

            Config.AeroColorBalance = ReadFloat("aeroColorBalance", 0.08f);
            Config.AeroAfterglowBalance = ReadFloat("aeroAfterglowBalance", 0.43f);
            Config.AeroBlurBalance = ReadFloat("aeroBlurBalance", 0.49f);

            Config.ActiveTextColor = ReadUInt("activeTextColor", 0xFF000000);
            Config.InactiveTextColor = ReadUInt("inactiveTextColor", 0xFFB4B4B4);
            Config.ActiveBlendColor = ReadUInt("activeBlendColor", 0x64FFFFFF);
            Config.InactiveBlendColor = ReadUInt("inactiveBlendColor", 0x64FFFFFF);

            Config.ActiveTextColorDark = ReadUInt("activeTextColorDark", 0xFFFFFFFF);
            Config.InactiveTextColorDark = ReadUInt("inactiveTextColorDark", 0xFFB4B4B4);
            Config.ActiveBlendColorDark = ReadUInt("activeBlendColorDark", 0x64000000);
            Config.InactiveBlendColorDark = ReadUInt("inactiveBlendColorDark", 0x64000000);

            Config.BlurMethod = (BlurMethod)ReadInt("blurMethod", (int)BlurMethod.CustomBlur);
            Config.EffectType = (EffectType)ReadInt("effectType", (int)EffectType.Blur);

            IsModified = false;
            ConfigReloaded?.Invoke(this, EventArgs.Empty);
        }

        public void SaveConfig()
        {
            WriteBool("applyglobal", Config.ApplyGlobal);
            WriteBool("extendBorder", Config.ExtendBorder);
            WriteBool("reflection", Config.Reflection);
            WriteBool("oldBtnHeight", Config.OldBtnHeight);
            WriteBool("customAmount", Config.CustomAmount);
            WriteBool("crossFade", Config.CrossFade);
            WriteBool("useAccentColor", Config.UseAccentColor);
            WriteBool("overrideAccent", Config.OverrideAccent);
            WriteBool("occlusionCulling", Config.ScaleOptimizer); // Note: key is occlusionCulling
            WriteBool("disableOnBattery", Config.DisableOnBattery);
            WriteBool("titlebtnGlow", Config.TitleBtnGlow);
            WriteBool("disableFramerateLimit", Config.DisableFramerateLimit);
            WriteBool("autoDownloadSymbols", Config.AutoDownloadSymbols);

            WriteInt("extendRound", Config.ExtendRound);
            WriteInt("titlebtnOffsetX", Config.TitleBtnOffsetX);
            WriteBool("customTitleBtnSize", Config.CustomTitleBtnSize);
            WriteInt("titleBtnSizePreset", Config.TitleBtnSizePreset);
            WriteInt("customCloseBtnW", Config.CustomCloseBtnW);
            WriteInt("customMaxBtnW", Config.CustomMaxBtnW);
            WriteInt("customMinBtnW", Config.CustomMinBtnW);
            WriteInt("customBtnFrameH", Config.CustomBtnFrameH);

            WriteFloat("blurAmount", Config.BlurAmount);
            WriteFloat("customBlurAmount", Config.CustomBlurAmount);
            WriteFloat("luminosityOpacity", Config.LuminosityOpacity);
            WriteFloat("glassIntensity", Config.GlassIntensity);
            WriteBool("customAeroTexture", Config.CustomAeroTexture);
            WriteString("customAeroTexturePath", Config.CustomAeroTexturePath);

            WriteUInt("crossfadeTime", Config.CrossFadeTime);

            WriteFloat("aeroColorBalance", Config.AeroColorBalance);
            WriteFloat("aeroAfterglowBalance", Config.AeroAfterglowBalance);
            WriteFloat("aeroBlurBalance", Config.AeroBlurBalance);

            WriteUInt("activeTextColor", Config.ActiveTextColor);
            WriteUInt("inactiveTextColor", Config.InactiveTextColor);
            WriteUInt("activeBlendColor", Config.ActiveBlendColor);
            WriteUInt("inactiveBlendColor", Config.InactiveBlendColor);

            WriteUInt("activeTextColorDark", Config.ActiveTextColorDark);
            WriteUInt("inactiveTextColorDark", Config.InactiveTextColorDark);
            WriteUInt("activeBlendColorDark", Config.ActiveBlendColorDark);
            WriteUInt("inactiveBlendColorDark", Config.InactiveBlendColorDark);

            WriteInt("blurMethod", (int)Config.BlurMethod);
            WriteInt("effectType", (int)Config.EffectType);

            IsModified = false;
        }

        public void ImportConfig(string path)
        {
            if (File.Exists(path))
            {
                File.Copy(path, _configPath, true);
                LoadConfig();
            }
        }

        public void ExportConfig(string path)
        {
            SaveConfig(); // Ensure current memory state is saved to disk first
            File.Copy(_configPath, path, true);
        }

        public void RestoreDefaultConfig()
        {
            Config = new AppConfig();
            InitializeConfigTracking();
            SaveConfig();
            ConfigReloaded?.Invoke(this, EventArgs.Empty);
        }
    }
}
