using DWMBlurGlassGUI.Services;
using System;
using System.Windows.Data;
using System.Windows.Markup;

namespace DWMBlurGlassGUI.Helpers
{
    /// <summary>
    /// XAML Markup Extension for localized strings
    /// Usage: Text="{helpers:Localize Key=general}"
    /// </summary>
    [MarkupExtensionReturnType(typeof(string))]
    public class LocalizeExtension : MarkupExtension
    {
        public string Key { get; set; } = string.Empty;
        public string Default { get; set; } = string.Empty;

        public LocalizeExtension() { }

        public LocalizeExtension(string key)
        {
            Key = key;
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            if (string.IsNullOrEmpty(Key))
                return Default;

            var binding = new Binding($"[{Key}]")
            {
                Source = LanguageService.Instance,
                Mode = BindingMode.OneWay
            };

            return binding.ProvideValue(serviceProvider);
        }
    }
}
