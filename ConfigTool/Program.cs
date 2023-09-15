using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace DLSSTweaks.ConfigTool
{
    internal static class Program
    {
        [DllImport("user32.dll")]
        private static extern bool SetProcessDPIAware();

        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            var args = Environment.GetCommandLineArgs();
            if (args != null && args.Length > 0)
            {
                if (NvSigOverride.ProcessArgs())
                    return;
                try
                {
                    if (ConfigTool.Main.Drs.ProcessArgs())
                        return;
                }
                catch (UnauthorizedAccessException)
                {
                    var argStr = "";
                    for (int i = 1; i < args.Length; i++)
                        argStr += $"\"{args[i]}\" ";

                    NvSigOverride.Elevate(argStr, false);
                    return;
                }
            }

            AppDomain.CurrentDomain.AssemblyResolve += (sender, arg) => { 
                if (arg.Name.StartsWith("PeanutButter.INI")) 
                    return Assembly.Load(Properties.Resources.PeanutButter_INI); 
                return null;
            };
            if (Environment.OSVersion.Version.Major >= 6)
                SetProcessDPIAware();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(true);
            Application.Run(new Main());
        }
    }
}