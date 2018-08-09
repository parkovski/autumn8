using System;
using Avalonia;
using Avalonia.Logging.Serilog;
using Autumn8.UI.ViewModels;
using Autumn8.UI.Views;

namespace Autumn8.UI
{
    class Program
    {
        static void Main(string[] args)
        {
            BuildAvaloniaApp().Start<MainWindow>(() => new MainWindowViewModel());
        }

        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .UseReactiveUI()
                .LogToDebug();
    }
}
