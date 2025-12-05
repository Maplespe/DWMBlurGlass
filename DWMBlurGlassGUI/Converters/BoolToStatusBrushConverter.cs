using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace DWMBlurGlassGUI.Converters
{
    /// <summary>
    /// Converts a boolean to a status brush (green for installed, red for not installed)
    /// </summary>
    public class BoolToStatusBrushConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool isInstalled)
            {
                // Green for installed, red for not installed
                if (isInstalled)
                {
                    return Application.Current.FindResource("SystemFillColorSuccessBrush") as Brush 
                           ?? new SolidColorBrush(Color.FromRgb(108, 203, 95));
                }
                else
                {
                    return Application.Current.FindResource("SystemFillColorCriticalBrush") as Brush 
                           ?? new SolidColorBrush(Color.FromRgb(255, 99, 71));
                }
            }
            return Application.Current.FindResource("SystemFillColorCriticalBrush") as Brush 
                   ?? new SolidColorBrush(Color.FromRgb(255, 99, 71));
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
