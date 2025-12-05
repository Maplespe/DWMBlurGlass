using DWMBlurGlassGUI.ViewModels.Pages;
using System;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace DWMBlurGlassGUI.Views.Pages
{
    public partial class AdvancedSettingsPage : Page
    {
        public AdvancedSettingsViewModel ViewModel { get; }

        /// <summary>
        /// 静态标志，用于指示页面加载时是否需要滚动到标题栏按钮尺寸设置
        /// </summary>
        public static bool ScrollToTitleBtnSizeOnLoad { get; set; } = false;

        public AdvancedSettingsPage(AdvancedSettingsViewModel viewModel)
        {
            ViewModel = viewModel;
            DataContext = ViewModel;
            InitializeComponent();
        }

        private void Page_Loaded(object sender, RoutedEventArgs e)
        {
            if (ScrollToTitleBtnSizeOnLoad)
            {
                ScrollToTitleBtnSizeOnLoad = false;
                
                // 延迟执行以确保布局完成
                Dispatcher.BeginInvoke(DispatcherPriority.Loaded, new System.Action(() =>
                {
                    ScrollToAndHighlightTitleBtnSizeSection();
                }));
            }
        }

        private void ScrollToAndHighlightTitleBtnSizeSection()
        {
            // 滚动到目标元素
            TitleBtnSizeSection.BringIntoView();
            
            // 获取系统高亮颜色
            var highlightColor = (Color)FindResource("SystemAccentColor");
            
            // 创建高亮动画
            var borderBrush = new SolidColorBrush(highlightColor);
            TitleBtnSizeSection.BorderBrush = borderBrush;
            
            // 创建淡出动画
            var fadeAnimation = new ColorAnimation
            {
                From = highlightColor,
                To = Colors.Transparent,
                Duration = TimeSpan.FromSeconds(2),
                BeginTime = TimeSpan.FromSeconds(1) // 等待1秒后开始淡出
            };
            
            borderBrush.BeginAnimation(SolidColorBrush.ColorProperty, fadeAnimation);
        }

        private void NumericTextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
        {
            e.Handled = !Regex.IsMatch(e.Text, @"^[0-9]+$");
        }

        private void BtnSizeTextBox_LostFocus(object sender, RoutedEventArgs e)
        {
            if (sender is TextBox textBox)
            {
                if (int.TryParse(textBox.Text, out int value))
                {
                    if (value < 14)
                        textBox.Text = "14";
                    else if (value > 200)
                        textBox.Text = "200";
                }
                else if (string.IsNullOrEmpty(textBox.Text))
                {
                    // 如果为空，恢复默认值
                    textBox.Text = "14";
                }
            }
        }

        private void Page_MouseDown(object sender, MouseButtonEventArgs e)
        {
            // 点击空白处时将焦点移到页面，使TextBox失去焦点
            FocusManager.SetFocusedElement(FocusManager.GetFocusScope(this), this);
            Keyboard.ClearFocus();
        }
    }
}
