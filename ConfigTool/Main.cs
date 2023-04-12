using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace DLSSTweaks.ConfigTool
{
    public partial class Main : Form
    {
        public static string FirstCharToUpper(string input)
        {
            switch (input)
            {
                case null: throw new ArgumentNullException(nameof(input));
                case "": return "";
                default: return input[0].ToString().ToUpper() + input.Substring(1);
            }
        }

        Dictionary<string, string> dllOverrides = new Dictionary<string, string>();

        string DefaultFormTitle = "DLSSTweaks"; // will be updated to actual text after InitializeComponent

        static string IniFilename = "dlsstweaks.ini";

        static string DefaultDescText = "Welcome to DLSSTweaks ConfigTool!\r\n\r\nSelect any setting to view a description of it here, or click on the value for the setting to edit it.\r\n\r\nIf you just want to force DLAA, simply edit the ForceDLAA value above and then save the file.";
        static string UltraQualityText = "UltraQuality: allows setting the ratio for the 'UltraQuality' level.\r\n\r\nNot every game allows using this level, some may only expose it as an option once this has been set to non-zero.\r\nA very small number might also already show an UltraQuality level, which this setting should be able to customize.\r\n(the number of games that work with this is very small unfortunately)\r\n\r\nSet to 0 to leave this as DLSS default.";
        static string HoverLoadText = "Reload the DLSSTweaks.ini from the same folder as ConfigTool.";
        static string HoverSaveText = "Writes out the changed settings to DLSSTweaks.ini.";
        static string HoverAddDLLOverrideText = "DLL override: allows overriding the path that a game will load a DLL from, simply pick the new DLL you wish to override with.\r\n\r\nThis can be useful if you're prevented from editing the game files for some reason.\r\n\r\neg. with Rockstar Game Launcher, you can't easily update nvngx_dlss.dll without RGL reverting it, but by using this you can make the game load DLSS from a completely different path which RGL can't override.";
        static string HoverInstallDllText = "Allows copying this DLSSTweaks config & DLL to a chosen folder.\r\n\r\nCan be used to both install freshly extracted DLSSTweaks into a game, and also to copy existing config + DLL across to other titles.";
        static string HoverInstallDllTextUnavailable = "DLSSTweaks DLL not found in current folder, install not available.";

        static string[] BooleanKeys = new[] { "ForceDLAA", "DisableDevWatermark", "VerboseLogging", "Enable", "DisableIniMonitoring", "OverrideAppId" };
        static string[] OverrideKeys = new[] { "OverrideAutoExposure", "OverrideDlssHud" };
        static string[] OverrideValues = new[] { "Default", "Force disable", "Force enable" }; // 0, -1, 1

        string DlssTweaksDll = "";
        static string[] DlssTweaksDllFilenames = new[] { "nvngx.dll", "xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "dxgi.dll", "XAPOFX1_5.dll", "x3daudio1_7.dll", "winmm.dll" };

        bool CheckDlssTweaksDllAvailable()
        {
            foreach (var filename in DlssTweaksDllFilenames)
            {
                try
                {
                    if (!File.Exists(filename))
                        continue;

                    FileVersionInfo fileInfo = FileVersionInfo.GetVersionInfo(filename);
                    if (fileInfo.ProductName != "DLSSTweaks")
                        continue;

                    DlssTweaksDll = filename;
                    return true;
                }
                catch
                {
                    continue;
                }
            }
            return false;
        }

        public void IniRead()
        {
            lvSettings.Items.Clear();
            lvSettings.Groups.Clear();
            lvSettings.ClearEditCells();

            string[] lines = null;

            if (File.Exists(IniFilename))
                lines = File.ReadAllLines(IniFilename);

            if (lines == null || lines.Length == 0)
            {
                MessageBox.Show("Failed to load dlsstweaks.ini, or INI file is empty.\r\n\r\nPlease extract dlsstweaks.ini from the ZIP you downloaded next to this tool first before running.");
                Application.Exit();
                System.Diagnostics.Process.GetCurrentProcess().Kill();
                return;
            }

            this.Text = $"{DefaultFormTitle} - {IniFilename}";

            var state_comment = "";
            var state_section = "";

            var comment_DLSSQualityLevels = "";
            var comment_DLSSPresets = "";

            var groups = new Dictionary<string, ListViewGroup>();

            bool isUltraQualityAdded = false;

            void AddSetting(string section, string key, string value, string comment)
            {
                ListViewGroup group = null;
                if (!groups.TryGetValue(section, out group))
                {
                    group = new ListViewGroup(section);
                    groups.Add(section, group);
                    lvSettings.Groups.Add(group);
                }

                bool isBooleanKey = BooleanKeys.Contains(key);
                bool isOverrideKey = OverrideKeys.Contains(key);
                if (isBooleanKey)
                    value = FirstCharToUpper(value);
                else
                {
                    if (isOverrideKey)
                    {
                        if (value == "-1")
                            value = OverrideValues[1];
                        else if(value == "0")
                            value = OverrideValues[0];
                        else
                            value = OverrideValues[2];
                    }
                }

                var listViewItem = new ListViewItem();
                listViewItem.Text = key;
                listViewItem.Tag = comment;
                listViewItem.Group = group;
                listViewItem.SubItems.Add(value);
                lvSettings.Items.Add(listViewItem);

                var row = lvSettings.Items.Count - 1;

                if (isBooleanKey)
                    lvSettings.AddComboBoxCell(row, 1, new string[] { "False", "True" });
                else
                {
                    if (isOverrideKey)
                        lvSettings.AddComboBoxCell(row, 1, OverrideValues);
                    else
                    {
                        if (section == "DLSSPresets")
                            lvSettings.AddComboBoxCell(row, 1, new string[] { "Default", "A", "B", "C", "D", "F" });
                        else
                            lvSettings.AddEditableCell(row, 1);
                    }
                }
            }

            foreach (var line in lines)
            {
                var trimmed = line.TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t');
                if (trimmed.Length <= 0)
                    continue;

                if (trimmed.StartsWith(";"))
                {
                    bool isCommentStart = state_comment.Length <= 0;
                    state_comment += trimmed.Substring(1).TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t') + "\r\n";
                    if (isCommentStart)
                        state_comment += "\r\n";
                    continue;
                }
                if (trimmed.StartsWith("[") && trimmed.EndsWith("]"))
                {
                    state_comment = "";

                    // If we're changing from DLSSQualityLevels section to new section, and haven't added UltraQuality...
                    if (state_section == "DLSSQualityLevels" && !isUltraQualityAdded)
                    {
                        AddSetting("DLSSQualityLevels", "UltraQuality", "0", UltraQualityText);
                    }

                    state_section = trimmed.Substring(1, trimmed.Length - 2);
                    continue;
                }

                var seperator = trimmed.IndexOf('=');
                if (seperator < 0)
                    continue;
                var key = trimmed.Substring(0, seperator).TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t');
                var value = trimmed.Substring(seperator + 1).TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t');

                if (state_section == "DLLPathOverrides")
                    state_comment = "DLLPathOverrides: allows overriding the path that a DLL will be loaded from based on the filename of it\r\n\r\nDelete/clear the path to remove this override.";

                if (state_section == "DLSSQualityLevels" && key != "Enable")
                {
                    if (!string.IsNullOrEmpty(state_comment))
                        comment_DLSSQualityLevels = state_comment;
                    else
                        state_comment = comment_DLSSQualityLevels;

                    if (key == "UltraQuality")
                        state_comment = UltraQualityText;
                }

                if (state_section == "DLSSPresets")
                {
                    if (!string.IsNullOrEmpty(state_comment))
                        comment_DLSSPresets = state_comment;
                    else
                        state_comment = comment_DLSSPresets;
                }

                AddSetting(state_section, key, value, state_comment);

                if (state_section == "DLSSQualityLevels" && key == "UltraQuality")
                    isUltraQualityAdded = true;

                state_comment = "";
            }
        }

        void IniWrite()
        {
            PeanutButter.INI.INIFile file = new PeanutButter.INI.INIFile(IniFilename);
            file.WrapValueInQuotes = false;

            foreach (ListViewItem item in lvSettings.Items)
            {
                var section = item.Group.Header;
                var key = item.Text;
                var value = item.SubItems[1].Text;

                if (OverrideKeys.Contains(key))
                {
                    if (value == OverrideValues[1])
                        value = "-1";
                    else if (value == OverrideValues[0])
                        value = "0";
                    else
                        value = "1";
                }
                else if (BooleanKeys.Contains(key))
                    value = value.ToLower();

                file[section][key] = value;
            }

            foreach (var kvp in dllOverrides)
            {
                file["DLLPathOverrides"][kvp.Key] = kvp.Value;
            }
            dllOverrides.Clear();

            file.Persist();

            IniRead();
        }

        public Main()
        {
            InitializeComponent();

            lblIniPath.Text = Path.GetFullPath(IniFilename);

            AutoScaleMode = AutoScaleMode.Dpi;
            lvSettings.ValueChanged += lvSettings_ValueChanged;

            this.Text += $" v{Assembly.GetExecutingAssembly().GetName().Version}";
            DefaultFormTitle = this.Text;

            txtDesc.Text = DefaultDescText;

            saveToolStripMenuItem.ToolTipText = HoverSaveText;
            loadToolStripMenuItem.ToolTipText = HoverLoadText;
            addDLLOverrideToolStripMenuItem.ToolTipText = HoverAddDLLOverrideText;
            installToGameFolderToolStripMenuItem.ToolTipText = HoverInstallDllText;

            if(!CheckDlssTweaksDllAvailable())
            {
                installToGameFolderToolStripMenuItem.Enabled = false;
                installToGameFolderToolStripMenuItem.ToolTipText = HoverInstallDllTextUnavailable;
            }

            IniRead();
        }

        private void lvSettings_ValueChanged(object sender, EventArgs e)
        {
            this.Text = $"{DefaultFormTitle} - {IniFilename}*";
        }

        private void lvSettings_ItemMouseHover(object sender, ListViewItemMouseHoverEventArgs e)
        {
            var item = e.Item as ListViewItem;
            if (item == null || item.Tag == null || item.Tag.GetType() != typeof(string))
                return;
            txtDesc.Text = (string)item.Tag;
        }

        private void lvSettings_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lvSettings.SelectedItems.Count <= 0)
            {
                txtDesc.Text = DefaultDescText;
                return;
            }
            var item = lvSettings.SelectedItems[0];
            if (item == null || item.Tag == null || item.Tag.GetType() != typeof(string))
                return;
            txtDesc.Text = (string)item.Tag;
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            lvSettings.Unfocus();
            IniWrite();
        }

        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
            lvSettings.Unfocus();
            IniRead();
        }

        private void addDLLOverrideToolStripMenuItem_Click(object sender, EventArgs e)
        {
            lvSettings.Unfocus();
            var ofd = new OpenFileDialog();
            ofd.Title = "Select the new DLL that you want to override with";
            ofd.Filter = "DLL Files (*.dll)|*.dll|All Files (*.*)|*.*";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            var filepath = ofd.FileName;
            var filename = Path.GetFileNameWithoutExtension(filepath);
            if (MessageBox.Show($"Setting DLL override\r\n\r\n  {filename} -> {filepath}\r\n\r\nOverride will be added & settings saved, is this OK?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes)
                return;

            dllOverrides[filename] = filepath;

            IniWrite();
        }

        private void addDLLOverrideToolStripMenuItem_MouseHover(object sender, EventArgs e)
        {
            txtDesc.Text = HoverAddDLLOverrideText;
        }

        private void loadToolStripMenuItem_MouseHover(object sender, EventArgs e)
        {
            txtDesc.Text = HoverLoadText;
        }

        private void saveToolStripMenuItem_MouseHover(object sender, EventArgs e)
        {
            txtDesc.Text = HoverSaveText;
        }

        private void installToGameFolderToolStripMenuItem_MouseHover(object sender, EventArgs e)
        {
            if (CheckDlssTweaksDllAvailable())
                txtDesc.Text = HoverInstallDllText;
            else
                txtDesc.Text = HoverInstallDllTextUnavailable;
        }

        private void installToGameFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if(!CheckDlssTweaksDllAvailable())
                MessageBox.Show("Failed to locate DLSSTweaks DLL, install aborted.");

            var configToolName = Path.GetFileName(Process.GetCurrentProcess().MainModule.FileName);

            var filesToCopy = new [] { 
                DlssTweaksDll, 
                "dlsstweaks.ini",
                configToolName
            };

            foreach(var file in filesToCopy)
            {
                if (!File.Exists(file))
                {
                    MessageBox.Show($"Failed to find file {file} for copy, install aborted.");
                    return;
                }
            }

            var dialog = new FolderSelectDialog();
            dialog.Title = "Select game folder to copy DLSSTweaks files into";

            if (!dialog.Show(Handle))
                return;

            var infoText = "";
            foreach (var file in filesToCopy)
            {
                var dest = Path.Combine(dialog.FileName, file);
                infoText += $"  {file} -> {dest}\r\n\r\n";
            }

            if (MessageBox.Show($"Copying DLSSTweaks files\r\n\r\n{infoText}Is this OK?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes)
                return;

            try
            {
                FileOperations.CopyFiles(filesToCopy, dialog.FileName);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Install failed, exception {ex.Message} during file copy...");
                return;
            }

            if (MessageBox.Show("Files copied to game folder, note that the DLL may need to be renamed for game to load it.\r\n\r\nDo you want to configure this install?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes)
                return;

            var startInfo = new ProcessStartInfo() { WorkingDirectory = dialog.FileName, FileName = Path.Combine(dialog.FileName, configToolName) };

            var proc = Process.Start(startInfo);

            // Wait up to 3000ms for MainWindowHandle to be set...
            int tries = 3;
            while(tries >= 0)
            {
                tries--;
                if (proc.MainWindowHandle != IntPtr.Zero)
                    break;

                System.Threading.Thread.Sleep(1000);
            }

            SetForegroundWindow(proc.MainWindowHandle);
            Process.GetCurrentProcess().Kill();
        }

        [DllImport("user32.dll")]
        static extern bool SetForegroundWindow(IntPtr hWnd);
    }
}