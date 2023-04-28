using DLSSTweaks.ConfigTool.Properties;
using System;
using System.Collections.Generic;
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
        struct IniChange
        {
            public string Section;
            public string Key;
            public string NewKey;

            public bool IsDeletion => string.IsNullOrEmpty(NewKey);
        }

        Dictionary<string, string> dllOverrides = new Dictionary<string, string>();

        string DefaultFormTitle = "DLSSTweaks"; // will be updated to actual text after InitializeComponent

        static string IniFilename = "dlsstweaks.ini";

        static string DefaultDescText = "Welcome to the DLSSTweaks ConfigTool!\r\n\r\nSelect any setting to view a description of it here, or click on the value for the setting to edit it.\r\n\r\nIf you just want to force DLAA, simply edit the ForceDLAA value above and then save the file.";
        static string UltraQualityText = "UltraQuality: allows setting the ratio for the 'UltraQuality' level.\r\n\r\nNot every game allows using this level, some may only expose it as an option once this has been set to non-zero.\r\nA very small number might also already show an UltraQuality level, which this setting should be able to customize.\r\n(the number of games that work with this is very small unfortunately)\r\n\r\nSet to 0 to leave this as DLSS default.";
        static string HoverLoadText = "Reload the DLSSTweaks.ini from the same folder as ConfigTool.";
        static string HoverSaveText = "Writes out the changed settings to DLSSTweaks.ini.";
        static string HoverAddDLLOverrideText = "DLL override: allows overriding the path that a game will load a DLL from, simply pick the new DLL you wish to override with.\r\n\r\nThis can be useful if you're prevented from editing the game files for some reason.\r\n\r\neg. with Rockstar Game Launcher, you can't easily update nvngx_dlss.dll without RGL reverting it, but by using this you can make the game load DLSS from a completely different path which RGL can't override.";
        static string HoverInstallDllText = "Allows copying this DLSSTweaks config & DLL to a chosen folder.\r\n\r\nCan be used to both install freshly extracted DLSSTweaks into a game, and also to copy existing config + DLL across to other titles.";
        static string HoverInstallDllTextUnavailable = "DLSSTweaks DLL not found in current folder, install not available.";

        static string HoverNvSigOverrideText = "Toggles the NVIDIA Signature Override registry key.\r\n\r\nWith the override enabled DLSSTweaks can be used in most DLSS2+ games by naming it as nvngx.dll.\r\n\r\n(this override only affects Nvidia related signature checks, not anything Windows related)\r\n\r\nChanging this requires Administrator privileges, a prompt will appear if necessary.";

        static string DllPathOverrideText = "DLLPathOverrides: allows overriding the path that a DLL will be loaded from based on the filename of it\r\n\r\nRight click on the override for options to rename/delete it.";

        static string[] BooleanKeys = new[] { "ForceDLAA", "DisableDevWatermark", "VerboseLogging", "Enable", "DisableIniMonitoring", "OverrideAppId", "EnableNvidiaSigOverride" };
        static string[] OverrideKeys = new[] { "OverrideAutoExposure", "OverrideDlssHud" };
        static string[] OverrideValues = new[] { "Default", "Force disable", "Force enable" }; // 0, -1, 1

        string DlssTweaksDll = "";
        static string[] DlssTweaksDllFilenames = new[] { "nvngx.dll", "xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "dxgi.dll", "XAPOFX1_5.dll", "x3daudio1_7.dll", "winmm.dll" };

        List<IniChange> IniChanges = new List<IniChange>();

        bool IsChangeUnsaved = false;

        bool CheckDlssTweaksDllAvailable()
        {
            DlssTweaksDll = string.Empty;
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
            lvSettings.Unfocus();
            lvSettings.Items.Clear();
            lvSettings.Groups.Clear();
            lvSettings.ClearEditCells();

            IsChangeUnsaved = false;

            IniChanges = new List<IniChange>();

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

            var userIni = new HackyIniParser();
            userIni.Parse(lines);

            var defaultIni = new HackyIniParser();
            defaultIni.Parse(Resources.DefaultINI.Split(new string[] { "\r\n" }, StringSplitOptions.None));

            // Copy across all comments from defaultIni to our userIni
            foreach(var section in defaultIni.Entries)
            {
                if (!userIni.Entries.ContainsKey(section.Key))
                    continue;
                foreach(var entry in section.Value)
                {
                    if (!userIni.Entries[section.Key].ContainsKey(entry.Key))
                        continue;
                    var iniEntry = userIni.Entries[section.Key][entry.Key];
                    iniEntry.Comment = entry.Value.Comment;
                    userIni.Entries[section.Key][entry.Key] = iniEntry;
                }
            }

            // Fetch default DLSSQualityLevels comment from our built-in INI
            string comment_DLSSQualityLevels = "";
            foreach(var section in defaultIni.Entries)
            {
                if (section.Key.ToLower() != "dlssqualitylevels")
                    continue;

                foreach(var entry in section.Value)
                {
                    if (entry.Key.ToLower() == "enable")
                        continue;
                    if (!string.IsNullOrEmpty(entry.Value.Comment))
                    {
                        comment_DLSSQualityLevels = entry.Value.Comment;
                        break;
                    }
                }
                break;
            }

            if (userIni.Entries.ContainsKey("DLSSQualityLevels"))
            {
                bool hasUltraQuality = false;
                var keys = userIni.Entries["DLSSQualityLevels"].Keys.ToArray();
                foreach (var key in keys)
                {
                    if (key.ToLower() == "enable")
                        continue;

                    var entry = userIni.Entries["DLSSQualityLevels"][key];
                    entry.Comment = comment_DLSSQualityLevels;
                    userIni.Entries["DLSSQualityLevels"][key] = entry;
                    if (key.ToLower() == "ultraquality")
                        hasUltraQuality = true;
                }

                // Add new UltraQuality entry if it doesn't exist...
                if (!hasUltraQuality)
                {
                    var entry = new HackyIniParser.IniEntry();
                    entry.Section = "DLSSQualityLevels";
                    entry.Key = "UltraQuality";
                    entry.Value = "0";
                    userIni.Entries["DLSSQualityLevels"]["UltraQuality"] = entry;
                }

                // Set UltraQuality comment to our stored text
                if (userIni.Entries["DLSSQualityLevels"].ContainsKey("UltraQuality"))
                {
                    var ultraQuality = userIni.Entries["DLSSQualityLevels"]["UltraQuality"];
                    ultraQuality.Comment = UltraQualityText;
                    userIni.Entries["DLSSQualityLevels"]["UltraQuality"] = ultraQuality;
                }
            }

            // Fetch default DLSSPresets comment from built-in INI
            var comment_DLSSPresets = "";
            foreach (var section in defaultIni.Entries)
            {
                if (section.Key.ToLower() != "dlsspresets")
                    continue;

                foreach (var entry in section.Value)
                {
                    if (!string.IsNullOrEmpty(entry.Value.Comment))
                    {
                        comment_DLSSPresets = entry.Value.Comment;
                        break;
                    }
                }
                break;
            }

            if (userIni.Entries.ContainsKey("DLSSPresets"))
            {
                var keys = userIni.Entries["DLSSPresets"].Keys.ToArray();
                foreach (var key in keys)
                {
                    var entry = userIni.Entries["DLSSPresets"][key];
                    entry.Comment = comment_DLSSPresets;
                    userIni.Entries["DLSSPresets"][key] = entry;
                }
            }

            if (userIni.Entries.ContainsKey("DLLPathOverrides"))
            {
                var keys = userIni.Entries["DLLPathOverrides"].Keys.ToArray();
                foreach(var key in keys)
                {
                    var entry = userIni.Entries["DLLPathOverrides"][key];
                    entry.Comment = DllPathOverrideText;
                    userIni.Entries["DLLPathOverrides"][key] = entry;
                }
            }

            var groups = new Dictionary<string, ListViewGroup>();

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
                    value = value.FirstCharToUpper();
                else
                {
                    if (isOverrideKey)
                    {
                        if (value == "-1")
                            value = OverrideValues[1];
                        else if (value == "0")
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

            // Add EnableNvidiaSigOverride psuedo-setting to list
            if (userIni.Entries.ContainsKey("DLSS"))
            {
                var entry = new HackyIniParser.IniEntry()
                {
                    Section = "DLSS",
                    Key = "EnableNvidiaSigOverride",
                    Comment = HoverNvSigOverrideText,
                    Value = NvSigOverride.IsOverride() ? "True" : "False"
                };
                userIni.Entries["DLSS"].Add("EnableNvidiaSigOverride", entry);
            }

            // AddSetting must be called in the exact order of items to add
            // Can't AddSetting to an earlier section since that would break the hacky ListViewEx input boxes
            // So all entries should be prepared inside userIni.Entries first
            foreach(var section in userIni.Entries)
            {
                foreach(var entry in section.Value)
                {
                    AddSetting(entry.Value.Section, entry.Value.Key, entry.Value.Value, entry.Value.Comment);
                }
            }
        }

        void IniWrite()
        {
            lvSettings.Unfocus();

            PeanutButter.INI.INIFile file = new PeanutButter.INI.INIFile(IniFilename);
            file.WrapValueInQuotes = false;

            foreach (ListViewItem item in lvSettings.Items)
            {
                var section = item.Group.Header;
                var key = item.Text;
                var value = item.SubItems[1].Text;

                if (section == "DLSS" && key == "EnableNvidiaSigOverride")
                    continue;

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

            foreach(var change in IniChanges)
            {
                var value = file[change.Section][change.Key];
                file.RemoveValue(change.Section, change.Key);
                if (!change.IsDeletion)
                    file[change.Section][change.NewKey] = value;
            }

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

            if (!CheckDlssTweaksDllAvailable())
            {
                installToGameFolderToolStripMenuItem.Enabled = false;
                installToGameFolderToolStripMenuItem.ToolTipText = HoverInstallDllTextUnavailable;
            }

            // Alert user if this is being ran with no game EXE in current folder
            var exePath = SearchForGameExe(Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName));
            if (string.IsNullOrEmpty(exePath))
            {
                MessageBox.Show("It appears the DLSSTweaks ConfigTool has been launched outside of a game directory.\n\n" +
                    "It's recommended to copy DLSSTweaks into a game folder first before configuring it.\n\n" +
                    "You can let ConfigTool copy the necessary files for you via the \"Copy to game folder...\" command on top right.", "Game EXE not found!");
            }
            else
            {
                // We have a game EXE in this folder
                // If user is setting up DLSSTweaks as nvngx.dll, check whether registry override is active, and alert them if not
                if (CheckDlssTweaksDllAvailable() && Path.GetFileName(DlssTweaksDll).ToLower() == "nvngx.dll")
                {
                    if (!NvSigOverride.IsOverride())
                    {
                        if (MessageBox.Show("It appears that DLSSTweaks is loading in via the nvngx.dll wrapper method.\n\n" +
                            "This method requires an Nvidia registry override to be set in order for the DLL to be loaded in.\n\n" +
                            "This override currently doesn't seem to be active, do you want ConfigTool to apply it for you?\n" +
                            "(this will require administrator permissions to apply, you will be prompted when continuing.)\n\n" +
                            "You can also add/remove the override from the tool via the \"EnableNvidiaSigOverride\" setting.", "Registry override not active", MessageBoxButtons.YesNo) == DialogResult.Yes)
                        {
                            NvSigOverride.SetOverride(true);
                        }
                    }
                }
            }

            IniRead();
        }

        private void lvSettings_ValueChanged(object sender, EventArgs e)
        {
            var lvi = sender as ListViewItem;
            if (lvi.Text == "EnableNvidiaSigOverride")
            {
                lvSettings.Unfocus();
                lvSettings.Focus();
                bool enableOverride = false;
                if (bool.TryParse(lvi.SubItems[1].Text, out enableOverride))
                    NvSigOverride.SetOverride(enableOverride);
                lvi.SubItems[1].Text = NvSigOverride.IsOverride().ToString().FirstCharToUpper();
            }
            else
            {
                this.Text = $"{DefaultFormTitle} - {IniFilename}*";
                IsChangeUnsaved = true;
            }
            lvSettings.Focus();
        }

        private void lvSettings_ItemMouseHover(object sender, ListViewItemMouseHoverEventArgs e)
        {
            var item = e.Item as ListViewItem;
            if (item == null || item.Tag == null || item.Tag.GetType() != typeof(string))
                return;
            txtDesc.Text = (string)item.Tag;
        }

        private bool IsDllOverrideSelected()
        {
            if (lvSettings.SelectedItems == null || lvSettings.SelectedItems.Count <= 0)
                return false;
            var item = lvSettings.SelectedItems[0];
            if (item.Group == null)
                return false;
            return item.Group.Header == "DLLPathOverrides";
        }

        private void lvSettings_SelectedIndexChanged(object sender, EventArgs e)
        {
            ctxDelete.Enabled = ctxRename.Enabled = IsDllOverrideSelected();

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

        private void ctxDelete_Click(object sender, EventArgs e)
        {
            if (!IsDllOverrideSelected())
                return;

            var item = lvSettings.SelectedItems[0];

            var entryText = $"{item.Text} = {item.SubItems[1].Text}";
            var result = MessageBox.Show($"Are you sure you want to delete this setting?\r\n\r\n{entryText}\r\n\r\nEntry will be deleted & settings saved, is this OK?", "Confirm", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
                return;

            var change = new IniChange() { Section = item.Group.Header, Key = item.Text, NewKey = string.Empty };
            IniChanges.Add(change);
            IniWrite();
        }

        private void ctxRename_Click(object sender, EventArgs e)
        {
            if (!IsDllOverrideSelected())
                return;

            var item = lvSettings.SelectedItems[0];

            string value = item.Text;
            var result = Utility.InputBox("Setting name", "Enter new name for setting", ref value);
            if (result != DialogResult.OK || value == item.Text)
                return;

            var entryText = $"{item.Text} -> {value}";
            result = MessageBox.Show($"Are you sure you want to rename this setting?\r\n\r\n{entryText}\r\n\r\nEntry will be renamed & settings saved, is this OK?", "Confirm", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
                return;

            var change = new IniChange() { Section = item.Group.Header, Key = item.Text, NewKey = value };

            IniChanges.Add(change);
            IniWrite();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            IniWrite();
        }

        private void loadToolStripMenuItem_Click(object sender, EventArgs e)
        {
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

            var result = MessageBox.Show($"Setting DLL override\r\n\r\n{filename} -> {filepath}\r\n\r\nOverride will be added & settings saved, is this OK?", "Confirm", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
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

        private string SearchForGameExe(string path)
        {
            // Return largest EXE we find in the specified folder, most likely to be the game EXE
            // TODO: search subdirectories too and recommend user change folder if larger EXE was found?
            FileInfo largest = null;
            foreach (var file in Directory.GetFiles(path, "*.exe"))
            {
                if (Path.GetFileName(file).ToLower().Contains("dlsstweak"))
                    continue;

                var info = new FileInfo(file);
                if (largest == null || info.Length > largest.Length)
                    largest = info;
            }
            if (largest == null)
                return null;

            return largest.FullName;
        }

        private void installToGameFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (!CheckDlssTweaksDllAvailable())
                MessageBox.Show("Failed to locate DLSSTweaks DLL, install aborted.");

            DialogResult result = DialogResult.No;
            if (IsChangeUnsaved)
            {
                result = MessageBox.Show("You have unsaved changes, save those first?", "Save?", MessageBoxButtons.YesNo);
                if (result == DialogResult.Yes)
                    IniWrite();
            }

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
                infoText += $"{file} -> {dest}\r\n\r\n";
            }

            result = MessageBox.Show($"Copying DLSSTweaks files\r\n\r\n{infoText}Is this OK?", "Confirm", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
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

            result = MessageBox.Show("Files copied to game folder, note that the DLL may need to be renamed for game to load it.\r\n\r\nDo you want to configure this install?", "Confirm", MessageBoxButtons.YesNo);
            if (result != DialogResult.Yes)
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

        private void Main_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!IsChangeUnsaved)
                return;

            DialogResult result = MessageBox.Show("You have unsaved changes, save those before exiting?", "Save?", MessageBoxButtons.YesNo);
            if (result == DialogResult.Yes)
                IniWrite();
        }
    }
}