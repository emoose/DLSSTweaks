using System.ComponentModel;
using System.Reflection;

namespace DLSSTweaks.ConfigTool
{
    public partial class Main : Form
    {
        Dictionary<string, string> dllOverrides = new Dictionary<string, string>();

        string DefaultFormTitle = "DLSSTweaks"; // will be updated to actual text after InitializeComponent

        static string IniFilename = "dlsstweaks.ini";

        static string DefaultDescText = "Select a setting to view a description of it here, click on the value for the setting to edit it.";

        static string HoverLoadText = "Reload the DLSSTweaks.ini from the same folder as ConfigTool.";
        static string HoverSaveText = "Writes out the changed settings to DLSSTweaks.ini.";
        static string HoverAddDLLOverrideText = "DLL override: allows overriding the path that a game will load a DLL from.\r\nThis can be useful if you're prevented from editing the game files for some reason.\r\neg. with Rockstar Game Launcher, you can't easily update nvngx_dlss.dll without RGL reverting it, but by using this you can make the game load DLSS from a completely different path which RGL can't override.";

        public void IniRead()
        {
            lvSettings.Items.Clear();
            lvSettings.Groups.Clear();

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
                    state_section = trimmed.Substring(1, trimmed.Length - 2);
                    continue;
                }

                var seperator = trimmed.IndexOf('=');
                if (seperator < 0)
                    continue;
                var key = trimmed.Substring(0, seperator).TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t');
                var value = trimmed.Substring(seperator + 1).TrimStart(' ').TrimEnd(' ').TrimStart('\t').TrimEnd('\t');

                bool isBooleanValue = value.ToLower() == "true" || value.ToLower() == "false";
                if (isBooleanValue)
                    value = value.ToLower();

                ListViewGroup group = null;
                if (!groups.TryGetValue(state_section, out group))
                {
                    group = new ListViewGroup(state_section);
                    groups.Add(state_section, group);
                    lvSettings.Groups.Add(group);
                }

                if (state_section == "DLLPathOverrides")
                    state_comment = "DLLPathOverrides: allows overriding the path that a DLL will be loaded from based on the filename of it\r\n\r\nDelete/clear the path to remove this override.";

                if (state_section == "DLSSQualityLevels" && key != "Enable")
                {
                    if (!string.IsNullOrEmpty(state_comment))
                        comment_DLSSQualityLevels = state_comment;
                    else
                        state_comment = comment_DLSSQualityLevels;
                }

                if (state_section == "DLSSPresets")
                {
                    if (!string.IsNullOrEmpty(state_comment))
                        comment_DLSSPresets = state_comment;
                    else
                        state_comment = comment_DLSSPresets;
                }

                var listViewItem = new ListViewItem();
                listViewItem.Text = key;
                listViewItem.Tag = state_comment;
                listViewItem.Group = group;
                listViewItem.SubItems.Add(value);
                lvSettings.Items.Add(listViewItem);

                var row = lvSettings.Items.Count - 1;

                if (isBooleanValue)
                    lvSettings.AddComboBoxCell(row, 1, new string[] { "true", "false" });
                else
                    lvSettings.AddEditableCell(row, 1);

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
            lvSettings.ValueChanged += lvSettings_ValueChanged;

            this.Text += $" v{Assembly.GetExecutingAssembly().GetName().Version}";
            DefaultFormTitle = this.Text;

            txtDesc.Text = DefaultDescText;

            IniRead();
        }

        private void lvSettings_ValueChanged(object? sender, EventArgs e)
        {
            this.Text = $"{DefaultFormTitle} - {IniFilename}*";
        }

        private void TxtDesc_TextChanged(object? sender, EventArgs e)
        {
            throw new NotImplementedException();
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
            IniWrite();
        }

        private void addDLLOverrideToolStripMenuItem_Click(object sender, EventArgs e)
        {
            var ofd = new OpenFileDialog();
            ofd.Title = "Select the DLL that you want to override with";
            ofd.Filter = "DLL Files (*.dll)|*.dll|All Files (*.*)|*.*";
            if (ofd.ShowDialog() != DialogResult.OK)
                return;

            var filepath = ofd.FileName;
            var filename = Path.GetFileNameWithoutExtension(filepath);
            if (MessageBox.Show($"Setting DLL override\r\n\r\n  {filename} -> {filepath}\r\n\r\nIs this OK?", "Confirm", MessageBoxButtons.YesNo) != DialogResult.Yes)
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
    }
}