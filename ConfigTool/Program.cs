using System;
using System.Reflection;
using System.Windows.Forms;

namespace DLSSTweaks.ConfigTool
{
    internal static class Program
    {
        /// <summary>
        ///  The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            AppDomain.CurrentDomain.AssemblyResolve += (sender, arg) => { 
                if (arg.Name.StartsWith("PeanutButter.INI")) 
                    return Assembly.Load(Properties.Resources.PeanutButter_INI); 
                return null;
            };

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Main());
        }
    }
}