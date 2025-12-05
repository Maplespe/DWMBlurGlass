using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace DWMBlurGlassGUI.Views.Controls
{
    public partial class ColorSettingItem : UserControl
    {
        public ColorSettingItem()
        {
            InitializeComponent();
        }

        public static readonly DependencyProperty ColorProperty =
            DependencyProperty.Register(nameof(Color), typeof(Color), typeof(ColorSettingItem), new PropertyMetadata(Colors.Transparent, OnColorChanged));

        public Color Color
        {
            get => (Color)GetValue(ColorProperty);
            set => SetValue(ColorProperty, value);
        }

        public static readonly DependencyProperty PickColorCommandProperty =
            DependencyProperty.Register(nameof(PickColorCommand), typeof(ICommand), typeof(ColorSettingItem), new PropertyMetadata(null));

        public ICommand PickColorCommand
        {
            get => (ICommand)GetValue(PickColorCommandProperty);
            set => SetValue(PickColorCommandProperty, value);
        }

        public static readonly DependencyProperty CommandParameterProperty =
            DependencyProperty.Register(nameof(CommandParameter), typeof(object), typeof(ColorSettingItem), new PropertyMetadata(null));

        public object CommandParameter
        {
            get => GetValue(CommandParameterProperty);
            set => SetValue(CommandParameterProperty, value);
        }

        public Brush ColorBrush => new SolidColorBrush(Color);

        public string ColorText
        {
            get
            {
                if (IsRgb)
                {
                    return $"RGB({Color.R},{Color.G},{Color.B})";
                }
                return $"RGBA({Color.R},{Color.G},{Color.B},{Color.A})";
            }
        }

        private static void OnColorChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is ColorSettingItem item)
            {
                item.OnPropertyChanged(nameof(ColorBrush));
                item.OnPropertyChanged(nameof(ColorText));
            }
        }

        public static readonly DependencyProperty IsRgbProperty =
            DependencyProperty.Register(nameof(IsRgb), typeof(bool), typeof(ColorSettingItem), new PropertyMetadata(false, OnIsRgbChanged));

        public bool IsRgb
        {
            get => (bool)GetValue(IsRgbProperty);
            set => SetValue(IsRgbProperty, value);
        }

        private static void OnIsRgbChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            if (d is ColorSettingItem item)
            {
                item.OnPropertyChanged(nameof(ColorText));
            }
        }

        // Implement INotifyPropertyChanged logic for ColorBrush and ColorText updates binding
        // Since UserControl is a DependencyObject, we can't implement INotifyPropertyChanged easily for properties that are not DPs if we want to bind to them in XAML using ElementName.
        // However, we can use DependencyProperties for ColorBrush and ColorText as well, with private setters (read-only effectively).
        
        private void OnPropertyChanged(string propertyName)
        {
            if (propertyName == nameof(ColorBrush))
            {
                SetValue(ColorBrushPropertyKey, ColorBrush);
            }
            else if (propertyName == nameof(ColorText))
            {
                SetValue(ColorTextPropertyKey, ColorText);
            }
        }

        private static readonly DependencyPropertyKey ColorBrushPropertyKey =
            DependencyProperty.RegisterReadOnly(nameof(ColorBrush), typeof(Brush), typeof(ColorSettingItem), new PropertyMetadata(Brushes.Transparent));

        public static readonly DependencyProperty ColorBrushProperty = ColorBrushPropertyKey.DependencyProperty;

        private static readonly DependencyPropertyKey ColorTextPropertyKey =
            DependencyProperty.RegisterReadOnly(nameof(ColorText), typeof(string), typeof(ColorSettingItem), new PropertyMetadata("RGBA(0,0,0,0)"));

        public static readonly DependencyProperty ColorTextProperty = ColorTextPropertyKey.DependencyProperty;
    }
}
