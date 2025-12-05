using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.IO;
using System.Runtime.CompilerServices;
using System.Xml.Linq;

namespace DWMBlurGlassGUI.Services
{
    public class LanguageService : INotifyPropertyChanged
    {
        private const string FallbackLanguage = "en-US";
        
        private static LanguageService? _instance;
        public static LanguageService Instance => _instance ??= new LanguageService();

        private readonly Dictionary<string, string> _strings = new();
        private readonly Dictionary<string, string> _fallbackStrings = new();
        private string _currentLanguage = FallbackLanguage;
        private string _languageAuthor = "";

        public event PropertyChangedEventHandler? PropertyChanged;
        public event EventHandler? LanguageChanged;

        public string CurrentLanguage
        {
            get => _currentLanguage;
            private set
            {
                if (_currentLanguage != value)
                {
                    _currentLanguage = value;
                    OnPropertyChanged();
                }
            }
        }

        public string LanguageAuthor
        {
            get => _languageAuthor;
            private set
            {
                if (_languageAuthor != value)
                {
                    _languageAuthor = value;
                    OnPropertyChanged();
                }
            }
        }

        public string LanguageInfo => $"[{CurrentLanguage}] {this["langauthor"]} {LanguageAuthor}";

        private LanguageService()
        {
            LoadFallbackLanguage();
            LoadLanguage();
        }

        private string GetLangDirectory() => Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data", "lang");

        /// <summary>
        /// Load fallback language (en-US) for missing keys
        /// </summary>
        private void LoadFallbackLanguage()
        {
            var langDir = GetLangDirectory();
            var fallbackFile = Path.Combine(langDir, $"{FallbackLanguage}.xml");
            
            if (!File.Exists(fallbackFile))
                return;

            try
            {
                var doc = XDocument.Load(fallbackFile);
                var root = doc.Root;
                if (root == null) return;

                _fallbackStrings.Clear();
                foreach (var element in root.Elements())
                {
                    _fallbackStrings[element.Name.LocalName] = element.Value.Replace("\\n", "\n");
                }
            }
            catch
            {
                // Ignore errors loading fallback
            }
        }

        public void LoadLanguage(string? languageCode = null)
        {
            languageCode ??= CultureInfo.CurrentUICulture.Name;
            
            var langDir = GetLangDirectory();
            var langFile = Path.Combine(langDir, $"{languageCode}.xml");

            // Try to find language file
            if (!File.Exists(langFile))
            {
                // Try base language (e.g., "zh" from "zh-CN")
                var baseLang = languageCode.Split('-')[0];
                var files = Directory.Exists(langDir) ? Directory.GetFiles(langDir, $"{baseLang}*.xml") : [];
                if (files.Length > 0)
                {
                    langFile = files[0];
                }
                else
                {
                    // Fallback to en-US
                    langFile = Path.Combine(langDir, $"{FallbackLanguage}.xml");
                }
            }

            if (!File.Exists(langFile))
            {
                // No language files available, use fallback dictionary
                CurrentLanguage = FallbackLanguage;
                LanguageAuthor = "";
                _strings.Clear();
                OnPropertyChanged(string.Empty);
                LanguageChanged?.Invoke(this, EventArgs.Empty);
                return;
            }

            try
            {
                var doc = XDocument.Load(langFile);
                var root = doc.Root;
                
                if (root == null)
                {
                    CurrentLanguage = FallbackLanguage;
                    _strings.Clear();
                    return;
                }

                _strings.Clear();
                
                CurrentLanguage = root.Attribute("local")?.Value ?? languageCode;
                LanguageAuthor = root.Attribute("author")?.Value ?? "";

                foreach (var element in root.Elements())
                {
                    _strings[element.Name.LocalName] = element.Value.Replace("\\n", "\n");
                }

                // Notify all bindings to refresh
                OnPropertyChanged(string.Empty);
                LanguageChanged?.Invoke(this, EventArgs.Empty);
            }
            catch
            {
                CurrentLanguage = FallbackLanguage;
                _strings.Clear();
                OnPropertyChanged(string.Empty);
                LanguageChanged?.Invoke(this, EventArgs.Empty);
            }
        }

        public string this[string key]
        {
            get
            {
                // First try current language
                if (_strings.TryGetValue(key, out var value))
                    return value;
                
                // Then try fallback language
                if (_fallbackStrings.TryGetValue(key, out var fallbackValue))
                    return fallbackValue;
                
                // Return key placeholder if not found
                return $"[{key}]";
            }
        }

        public string GetString(string key, string defaultValue = "")
        {
            // First try current language
            if (_strings.TryGetValue(key, out var value))
                return value;
            
            // Then try fallback language
            if (_fallbackStrings.TryGetValue(key, out var fallbackValue))
                return fallbackValue;
            
            // Return default or key placeholder
            return string.IsNullOrEmpty(defaultValue) ? $"[{key}]" : defaultValue;
        }

        protected virtual void OnPropertyChanged([CallerMemberName] string? propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
