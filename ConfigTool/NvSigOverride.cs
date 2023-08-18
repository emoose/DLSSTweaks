using System;
using System.Diagnostics;
using System.Security.Principal;
using Microsoft.Win32;

namespace DLSSTweaks.ConfigTool
{
    public static class NvSigOverride
    {
        public static bool ProcessArgs()
        {
            var args = Environment.GetCommandLineArgs();
            if (args == null || args.Length < 2)
                return false;

            if (args[1].ToLower() == "-enablesigoverride")
            {
                SetOverride(true, false);
                return true;
            }

            if (args[1].ToLower() == "-disablesigoverride")
            {
                SetOverride(false, false);
                return true;
            }

            return false;
        }

        public static bool IsOverride()
        {
            try
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NVIDIA Corporation\\Global", false);
                byte[] binaryValue = key.GetValue("{41FCC608-8496-4DEF-B43E-7D9BD675A6FF}") as byte[];
                if (binaryValue == null || binaryValue.Length < 1)
                    return false;
                return binaryValue[0] == 1;
            }
            catch
            {
                return false;
            }
        }
        public static bool SetOverride(bool enableOverride, bool allowElevate = true)
        {
            // Elevate the current process via UAC
            if (!IsElevated())
            {
                if (allowElevate)
                    Elevate(enableOverride ? "-enableSigOverride" : "-disableSigOverride");

                return IsOverride() == enableOverride;
            }
            try
            {
                // Update the registry entry
                RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NVIDIA Corporation\\Global", true);

                byte[] binaryValue = new byte[] { 0 };
                if (enableOverride)
                    binaryValue[0] = 1;

                key.SetValue("{41FCC608-8496-4DEF-B43E-7D9BD675A6FF}", binaryValue, RegistryValueKind.Binary);

                if (!enableOverride)
                {
                    try
                    {
                        // Try deleting key if we're disabling override, its fine if this part fails since above should have set to 0
                        key.DeleteValue("{41FCC608-8496-4DEF-B43E-7D9BD675A6FF}");
                    }
                    catch
                    {
                    }
                }

                return IsOverride() == enableOverride;
            }
            catch
            {
                return false;
            }
        }

        private static bool IsElevated()
        {
            using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            {
                return identity != null && new WindowsPrincipal(identity).IsInRole(WindowsBuiltInRole.Administrator);
            }
        }

        public static void Elevate(string args = "", bool waitForExit = true)
        {
            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.UseShellExecute = true;
            startInfo.WorkingDirectory = Environment.CurrentDirectory;
            startInfo.FileName = Process.GetCurrentProcess().MainModule.FileName;
            startInfo.Verb = "runas";
            startInfo.Arguments = args;

            try
            {
                var proc = Process.Start(startInfo);
                if (waitForExit)
                    proc.WaitForExit();
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: " + ex.Message);
            }
        }
    }
}
