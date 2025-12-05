using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using DWMBlurGlassGUI.Services;
using Wpf.Ui.Controls;

namespace DWMBlurGlassGUI.Views.Controls
{
    public partial class ColorPickerDialog : ContentDialog
    {
        private bool _isUpdating = false;
        private bool _isDraggingSatVal = false;
        private bool _isDraggingHue = false;

        // HSV values (H: 0-360, S: 0-100, V: 0-100)
        private double _hue = 0;
        private double _saturation = 100;
        private double _value = 100;
        private byte _alpha = 255;

        private bool _isRgbMode = false;

        public Color SelectedColor { get; private set; }
        public Color OriginalColor { get; private set; }

        /// <summary>
        /// 是否为 RGB 模式（不显示不透明度滑块）
        /// </summary>
        public bool IsRgbMode
        {
            get => _isRgbMode;
            set
            {
                _isRgbMode = value;
                if (OpacityLabel != null)
                {
                    OpacityLabel.Visibility = value ? Visibility.Collapsed : Visibility.Visible;
                    OpacitySlider.Visibility = value ? Visibility.Collapsed : Visibility.Visible;
                    OpacityText.Visibility = value ? Visibility.Collapsed : Visibility.Visible;
                }
                if (value)
                {
                    _alpha = 255;
                }
            }
        }

        public ColorPickerDialog(ContentPresenter? contentPresenter) : base(contentPresenter)
        {
            InitializeComponent();
            Loaded += ColorPickerDialog_Loaded;
        }

        public ColorPickerDialog(ContentPresenter? contentPresenter, Color initialColor, bool isRgbMode = false) 
            : base(contentPresenter)
        {
            OriginalColor = initialColor;
            SelectedColor = initialColor;
            _alpha = isRgbMode ? (byte)255 : initialColor.A;
            _isRgbMode = isRgbMode;
            
            InitializeComponent();
            Loaded += ColorPickerDialog_Loaded;
        }

        private void ColorPickerDialog_Loaded(object sender, RoutedEventArgs e)
        {
            // Set localized strings
            var lang = LanguageService.Instance;
            Title = lang["colorpicker_dlg_sel"];
            OpacityLabel.Text = lang["colorpicker_dlg_alpha"];
            NewColorLabel.Text = lang["colorpicker_dlg_c"];
            PrimaryButtonText = lang["msgdlg_yes"];
            CloseButtonText = lang["msgdlg_no"];

            // Set opacity controls visibility
            if (_isRgbMode)
            {
                OpacityLabel.Visibility = Visibility.Collapsed;
                OpacitySlider.Visibility = Visibility.Collapsed;
                OpacityText.Visibility = Visibility.Collapsed;
            }

            // Set original color preview
            OldColorPreview.Background = new SolidColorBrush(OriginalColor);

            // Initialize from original color
            RgbToHsv(OriginalColor.R, OriginalColor.G, OriginalColor.B, out _hue, out _saturation, out _value);
            if (!_isRgbMode)
            {
                _alpha = OriginalColor.A;
            }

            UpdateAllFromHsv();
        }

        #region Color Conversion

        private void RgbToHsv(byte r, byte g, byte b, out double h, out double s, out double v)
        {
            double rd = r / 255.0;
            double gd = g / 255.0;
            double bd = b / 255.0;

            double max = Math.Max(rd, Math.Max(gd, bd));
            double min = Math.Min(rd, Math.Min(gd, bd));
            double delta = max - min;

            // Value
            v = max * 100;

            // Saturation
            if (max == 0)
                s = 0;
            else
                s = (delta / max) * 100;

            // Hue
            if (delta == 0)
            {
                h = 0;
            }
            else if (max == rd)
            {
                h = 60 * (((gd - bd) / delta) % 6);
            }
            else if (max == gd)
            {
                h = 60 * (((bd - rd) / delta) + 2);
            }
            else
            {
                h = 60 * (((rd - gd) / delta) + 4);
            }

            if (h < 0) h += 360;
        }

        private void HsvToRgb(double h, double s, double v, out byte r, out byte g, out byte b)
        {
            s = s / 100.0;
            v = v / 100.0;

            double c = v * s;
            double x = c * (1 - Math.Abs((h / 60.0) % 2 - 1));
            double m = v - c;

            double rd, gd, bd;

            if (h < 60)
            {
                rd = c; gd = x; bd = 0;
            }
            else if (h < 120)
            {
                rd = x; gd = c; bd = 0;
            }
            else if (h < 180)
            {
                rd = 0; gd = c; bd = x;
            }
            else if (h < 240)
            {
                rd = 0; gd = x; bd = c;
            }
            else if (h < 300)
            {
                rd = x; gd = 0; bd = c;
            }
            else
            {
                rd = c; gd = 0; bd = x;
            }

            r = (byte)Math.Round((rd + m) * 255);
            g = (byte)Math.Round((gd + m) * 255);
            b = (byte)Math.Round((bd + m) * 255);
        }

        private Color HueToColor(double hue)
        {
            HsvToRgb(hue, 100, 100, out byte r, out byte g, out byte b);
            return Color.FromRgb(r, g, b);
        }

        #endregion

        #region Update Methods

        private void UpdateAllFromHsv()
        {
            if (_isUpdating) return;
            _isUpdating = true;

            try
            {
                // Update RGB
                HsvToRgb(_hue, _saturation, _value, out byte r, out byte g, out byte b);
                SelectedColor = Color.FromArgb(_alpha, r, g, b);

                // Update text boxes
                RedTextBox.Text = r.ToString();
                GreenTextBox.Text = g.ToString();
                BlueTextBox.Text = b.ToString();

                HueTextBox.Text = ((int)Math.Round(_hue)).ToString();
                SaturationTextBox.Text = ((int)Math.Round(_saturation)).ToString();
                ValueTextBox.Text = ((int)Math.Round(_value)).ToString();

                if (_isRgbMode)
                {
                    HexTextBox.Text = $"# {r:X2}{g:X2}{b:X2}";
                }
                else
                {
                    HexTextBox.Text = $"# {_alpha:X2}{r:X2}{g:X2}{b:X2}";
                    OpacitySlider.Value = Math.Round(_alpha / 255.0 * 100);
                    OpacityText.Text = $"{(int)OpacitySlider.Value}%";
                }

                // Update visuals
                UpdateHueBackground();
                UpdateIndicators();
                UpdateColorPreviews();
            }
            finally
            {
                _isUpdating = false;
            }
        }

        private void UpdateFromRgb(byte r, byte g, byte b)
        {
            if (_isUpdating) return;

            RgbToHsv(r, g, b, out _hue, out _saturation, out _value);
            UpdateAllFromHsv();
        }

        private void UpdateHueBackground()
        {
            HueBackground.Background = new SolidColorBrush(HueToColor(_hue));
        }

        private void UpdateIndicators()
        {
            // Update saturation/value indicator
            double width = SaturationValuePicker.ActualWidth;
            double height = SaturationValuePicker.ActualHeight;

            if (width > 0 && height > 0)
            {
                double x = (_saturation / 100.0) * width - 7;
                double y = (1 - _value / 100.0) * height - 7;

                Canvas.SetLeft(SatValIndicator, Math.Max(-7, Math.Min(width - 7, x)));
                Canvas.SetTop(SatValIndicator, Math.Max(-7, Math.Min(height - 7, y)));
            }

            // Update hue indicator
            double hueHeight = HueSliderBorder.ActualHeight;
            if (hueHeight > 0)
            {
                double hueY = (_hue / 360.0) * hueHeight - 2;
                Canvas.SetTop(HueIndicator, Math.Max(-2, Math.Min(hueHeight - 2, hueY)));
            }
        }

        private void UpdateColorPreviews()
        {
            NewColorPreview.Background = new SolidColorBrush(SelectedColor);
        }

        #endregion

        #region Saturation/Value Picker Events

        private void SaturationValuePicker_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _isDraggingSatVal = true;
            SaturationValuePicker.CaptureMouse();
            UpdateSaturationValueFromMouse(e.GetPosition(SaturationValuePicker));
        }

        private void SaturationValuePicker_MouseMove(object sender, MouseEventArgs e)
        {
            if (_isDraggingSatVal)
            {
                UpdateSaturationValueFromMouse(e.GetPosition(SaturationValuePicker));
            }
        }

        private void SaturationValuePicker_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            _isDraggingSatVal = false;
            SaturationValuePicker.ReleaseMouseCapture();
        }

        private void UpdateSaturationValueFromMouse(Point pos)
        {
            double width = SaturationValuePicker.ActualWidth;
            double height = SaturationValuePicker.ActualHeight;

            if (width > 0 && height > 0)
            {
                _saturation = Math.Max(0, Math.Min(100, (pos.X / width) * 100));
                _value = Math.Max(0, Math.Min(100, (1 - pos.Y / height) * 100));
                UpdateAllFromHsv();
            }
        }

        #endregion

        #region Hue Slider Events

        private void HueSlider_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _isDraggingHue = true;
            var parent = (FrameworkElement)sender;
            parent.CaptureMouse();
            UpdateHueFromMouse(e.GetPosition(parent));
        }

        private void HueSlider_MouseMove(object sender, MouseEventArgs e)
        {
            if (_isDraggingHue)
            {
                UpdateHueFromMouse(e.GetPosition((FrameworkElement)sender));
            }
        }

        private void HueSlider_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            _isDraggingHue = false;
            ((FrameworkElement)sender).ReleaseMouseCapture();
        }

        private void UpdateHueFromMouse(Point pos)
        {
            double height = HueSliderBorder.ActualHeight;
            if (height > 0)
            {
                _hue = Math.Max(0, Math.Min(360, (pos.Y / height) * 360));
                UpdateAllFromHsv();
            }
        }

        #endregion

        #region TextBox Events

        private void RgbTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_isUpdating) return;

            if (byte.TryParse(RedTextBox.Text, out byte r) &&
                byte.TryParse(GreenTextBox.Text, out byte g) &&
                byte.TryParse(BlueTextBox.Text, out byte b))
            {
                UpdateFromRgb(r, g, b);
            }
        }

        private void HsvTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_isUpdating) return;

            if (double.TryParse(HueTextBox.Text, out double h) &&
                double.TryParse(SaturationTextBox.Text, out double s) &&
                double.TryParse(ValueTextBox.Text, out double v))
            {
                _hue = Math.Max(0, Math.Min(360, h));
                _saturation = Math.Max(0, Math.Min(100, s));
                _value = Math.Max(0, Math.Min(100, v));
                UpdateAllFromHsv();
            }
        }

        private void HexTextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (_isUpdating) return;

            string hex = HexTextBox.Text.Replace("#", "").Replace(" ", "").Trim();

            try
            {
                if (hex.Length == 6)
                {
                    byte r = Convert.ToByte(hex.Substring(0, 2), 16);
                    byte g = Convert.ToByte(hex.Substring(2, 2), 16);
                    byte b = Convert.ToByte(hex.Substring(4, 2), 16);
                    UpdateFromRgb(r, g, b);
                }
                else if (hex.Length == 8 && !_isRgbMode)
                {
                    _alpha = Convert.ToByte(hex.Substring(0, 2), 16);
                    byte r = Convert.ToByte(hex.Substring(2, 2), 16);
                    byte g = Convert.ToByte(hex.Substring(4, 2), 16);
                    byte b = Convert.ToByte(hex.Substring(6, 2), 16);
                    UpdateFromRgb(r, g, b);
                }
            }
            catch
            {
                // Invalid hex format, ignore
            }
        }

        private void OpacitySlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            if (_isUpdating || _isRgbMode || OpacityText == null || HexTextBox == null) return;

            _alpha = (byte)Math.Round(OpacitySlider.Value / 100.0 * 255);
            OpacityText.Text = $"{(int)OpacitySlider.Value}%";

            HsvToRgb(_hue, _saturation, _value, out byte r, out byte g, out byte b);
            SelectedColor = Color.FromArgb(_alpha, r, g, b);

            _isUpdating = true;
            HexTextBox.Text = $"# {_alpha:X2}{r:X2}{g:X2}{b:X2}";
            _isUpdating = false;

            UpdateColorPreviews();
        }

        #endregion

    }
}
