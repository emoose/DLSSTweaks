using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace DLSSTweaks.ConfigTool
{
    public class TempFolder : IDisposable
    {
        private string tempFolder;
        public bool IsSet => string.IsNullOrEmpty(tempFolder) == false;

        public TempFolder()
        {
            tempFolder = string.Empty;
        }

        public void New()
        {
            tempFolder = Path.Combine(Path.GetTempPath(), Path.GetRandomFileName());
            int tries = 1;
            var baseFolder = tempFolder;
            while (Directory.Exists(tempFolder) && tries < 3)
            {
                tempFolder = $"{baseFolder}_{tries}";
                tries++;
            }
            if (tries >= 3)
                throw new Exception("Failed to create temporary folder");

            Directory.CreateDirectory(tempFolder);
        }

        public string GetPath()
        {
            return tempFolder;
        }

        public void Dispose()
        {
            if (IsSet)
                Directory.Delete(tempFolder, true);
        }
    }

    public static class Utility
    {
        public static string GetNewTempFolder()
        {
            string tempFolder = null;
            try
            {
                tempFolder = Path.GetTempFileName();
                if (File.Exists(tempFolder))
                    File.Delete(tempFolder);
                if (!Directory.Exists(tempFolder))
                    Directory.CreateDirectory(tempFolder);
                if (Directory.Exists(tempFolder))
                    return tempFolder;
            }
            catch { }
            return null;
        }
        public static string FirstCharToUpper(this string input)
        {
            switch (input)
            {
                case null: throw new ArgumentNullException(nameof(input));
                case "": return "";
                default: return input[0].ToString().ToUpper() + input.Substring(1);
            }
        }

        public static T BytesToStruct<T>(byte[] bytes)
        {
            // Pin the managed memory while, copy it out the data, then unpin it
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var theStructure = (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            handle.Free();

            return theStructure;
        }

        public static T ReadStruct<T>(this BinaryReader reader)
        {
            var size = Marshal.SizeOf(typeof(T));
            // Read in a byte array
            var bytes = reader.ReadBytes(size);

            return BytesToStruct<T>(bytes);
        }

        public static string ReadNullTerminatedString(this BinaryReader reader)
        {
            var bytes = new List<byte>();
            while (true)
            {
                var b = reader.ReadByte();
                if (b == 0)
                {
                    break;
                }
                bytes.Add(b);
            }
            return System.Text.Encoding.ASCII.GetString(bytes.ToArray());
        }

        public static DialogResult MultipleChoice(string prompt, string title, string[] choices, string choiceSeperator, ref string result)
        {
            var text = $"{prompt}\r\n  ";
            for (int i = 0; i < choices.Length; i++)
            {
                text += choices[i];
                if (i < choices.Length - 1)
                    text += choiceSeperator;
            }
            return Utility.InputBox(text, title, ref result);
        }

        public static DialogResult InputBox(string prompt, string title, ref string value)
        {
            Form form = new Form();
            Label label = new Label();
            TextBox textBox = new TextBox();
            Button buttonOk = new Button();
            Button buttonCancel = new Button();
            Panel panel = new Panel();

            form.Text = title;
            form.BackColor = SystemColors.Window;
            label.Text = prompt;
            textBox.Text = value;

            buttonOk.Text = "OK";
            buttonCancel.Text = "Cancel";
            buttonOk.DialogResult = DialogResult.OK;
            buttonCancel.DialogResult = DialogResult.Cancel;

            label.AutoSize = true;
            label.SetBounds(9, 20, 372, label.Height);

            panel.SetBounds(9, label.Bottom + 10, 372, 90);
            panel.Anchor = AnchorStyles.Bottom | AnchorStyles.Right | AnchorStyles.Left;
            panel.BackColor = SystemColors.ButtonFace;

            textBox.Anchor = AnchorStyles.Left | AnchorStyles.Bottom | AnchorStyles.Right;
            textBox.SetBounds(16, 16, 340, 20);
            textBox.BorderStyle = BorderStyle.FixedSingle;

            buttonOk.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            buttonOk.SetBounds(panel.Width - 132, 48, 100, 33);
            buttonOk.BackColor = SystemColors.ControlLightLight;
            buttonOk.FlatStyle = FlatStyle.System;

            buttonCancel.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
            buttonCancel.SetBounds(panel.Width - 244, 48, 100, 33);
            buttonCancel.BackColor = SystemColors.ControlLightLight;
            buttonCancel.FlatStyle = FlatStyle.System;

            panel.Controls.Add(textBox);
            panel.Controls.Add(buttonOk);
            panel.Controls.Add(buttonCancel);

            form.ClientSize = new Size(396, panel.Bottom);
            form.Controls.Add(label);
            form.Controls.Add(panel);

            form.SuspendLayout();

            form.ClientSize = new Size(Math.Max(300, label.Right + 10), form.ClientSize.Height);

            panel.SetBounds(0, label.Bottom + 10, form.ClientSize.Width, 90);
            form.ClientSize = new Size(form.ClientSize.Width, panel.Bottom);
            form.MinimumSize = form.Size;

            form.ResumeLayout(false);

            form.AutoScaleMode = AutoScaleMode.None;
            form.BackColor = SystemColors.Window;
            form.FormBorderStyle = FormBorderStyle.FixedDialog;
            form.StartPosition = FormStartPosition.CenterScreen;
            form.MinimizeBox = false;
            form.MaximizeBox = false;
            form.AcceptButton = buttonOk;
            form.CancelButton = buttonCancel;
            form.AcceptButton = buttonOk;
            form.CancelButton = buttonCancel;

            DialogResult dialogResult = form.ShowDialog();
            value = textBox.Text;
            return dialogResult;
        }
    }

    // Quick and dirty INI parser that preserves comments
    public class HackyIniParser
    {
        public struct IniEntry
        {
            public string Section;
            public string Key;
            public string Value;
            public string Comment;
        }

        public HackyIniParser() { }

        public Dictionary<string, Dictionary<string, IniEntry>> Entries;

        public void Parse(string[] lines)
        {
            var state_comment = "";
            var state_section = "";

            Entries = new Dictionary<string, Dictionary<string, IniEntry>>();

            if (lines == null)
                return;

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

                var item = new IniEntry()
                {
                    Comment = state_comment,
                    Section = state_section,
                    Key = key,
                    Value = value
                };

                if (!Entries.ContainsKey(state_section))
                    Entries[state_section] = new Dictionary<string, IniEntry>();

                Entries[state_section][key] = item;
                state_comment = "";
            }
        }
    }

    public static class FileOperations
    {
        [DllImport("shell32.dll", CharSet = CharSet.Unicode)]
        private static extern int SHFileOperation(ref SHFILEOPSTRUCT FileOp);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        private struct SHFILEOPSTRUCT
        {
            public IntPtr hwnd;
            public uint wFunc;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pFrom;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string pTo;
            public ushort fFlags;
            public bool fAnyOperationsAborted;
            public IntPtr hNameMappings;
            [MarshalAs(UnmanagedType.LPWStr)]
            public string lpszProgressTitle;

        }

        public static void CopyFiles(string[] sourceFiles, string destinationFolder)
        {
            string sourceFilesString = string.Join("\0", sourceFiles) + '\0';

            SHFILEOPSTRUCT fileOp = new SHFILEOPSTRUCT();
            fileOp.hwnd = IntPtr.Zero;
            fileOp.wFunc = FO_COPY;
            fileOp.pFrom = sourceFilesString;
            fileOp.pTo = destinationFolder;
            fileOp.fFlags = FOF_NOCONFIRMMKDIR;

            int result = SHFileOperation(ref fileOp);

            if (result != 0)
            {
                throw new Exception("File operation failed with error code " + result);
            }
        }

        private const uint FO_COPY = 0x0002;
        private const ushort FOF_NOCONFIRMATION = 0x0010;
        private const ushort FOF_NOCONFIRMMKDIR = 0x0200;
    }

    // FolderSelectDialog from https://stackoverflow.com/a/33835934
    /// <summary>
    /// Present the Windows Vista-style open file dialog to select a folder. Fall back for older Windows Versions
    /// </summary>
    public class FolderSelectDialog
    {
        private string _initialDirectory;
        private string _title;
        private string _fileName = "";

        public string InitialDirectory
        {
            get { return string.IsNullOrEmpty(_initialDirectory) ? Environment.CurrentDirectory : _initialDirectory; }
            set { _initialDirectory = value; }
        }
        public string Title
        {
            get { return _title ?? "Select a folder"; }
            set { _title = value; }
        }
        public string FileName { get { return _fileName; } }

        public bool Show() { return Show(IntPtr.Zero); }

        /// <param name="hWndOwner">Handle of the control or window to be the parent of the file dialog</param>
        /// <returns>true if the user clicks OK</returns>
        public bool Show(IntPtr hWndOwner)
        {
            var result = Environment.OSVersion.Version.Major >= 6
                ? VistaDialog.Show(hWndOwner, InitialDirectory, Title)
                : ShowXpDialog(hWndOwner, InitialDirectory, Title);
            _fileName = result.FileName;
            return result.Result;
        }

        private struct ShowDialogResult
        {
            public bool Result { get; set; }
            public string FileName { get; set; }
        }

        private static ShowDialogResult ShowXpDialog(IntPtr ownerHandle, string initialDirectory, string title)
        {
            var folderBrowserDialog = new FolderBrowserDialog
            {
                Description = title,
                SelectedPath = initialDirectory,
                ShowNewFolderButton = false
            };
            var dialogResult = new ShowDialogResult();
            if (folderBrowserDialog.ShowDialog(new WindowWrapper(ownerHandle)) == DialogResult.OK)
            {
                dialogResult.Result = true;
                dialogResult.FileName = folderBrowserDialog.SelectedPath;
            }
            return dialogResult;
        }

        private static class VistaDialog
        {
            private const string c_foldersFilter = "Folders|\n";

            private const BindingFlags c_flags = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic;
            private readonly static Assembly s_windowsFormsAssembly = typeof(FileDialog).Assembly;
            private readonly static Type s_iFileDialogType = s_windowsFormsAssembly.GetType("System.Windows.Forms.FileDialogNative+IFileDialog");
            private readonly static MethodInfo s_createVistaDialogMethodInfo = typeof(OpenFileDialog).GetMethod("CreateVistaDialog", c_flags);
            private readonly static MethodInfo s_onBeforeVistaDialogMethodInfo = typeof(OpenFileDialog).GetMethod("OnBeforeVistaDialog", c_flags);
            private readonly static MethodInfo s_getOptionsMethodInfo = typeof(FileDialog).GetMethod("GetOptions", c_flags);
            private readonly static MethodInfo s_setOptionsMethodInfo = s_iFileDialogType.GetMethod("SetOptions", c_flags);
            private readonly static uint s_fosPickFoldersBitFlag = (uint)s_windowsFormsAssembly
                .GetType("System.Windows.Forms.FileDialogNative+FOS")
                .GetField("FOS_PICKFOLDERS")
                .GetValue(null);
            private readonly static ConstructorInfo s_vistaDialogEventsConstructorInfo = s_windowsFormsAssembly
                .GetType("System.Windows.Forms.FileDialog+VistaDialogEvents")
                .GetConstructor(c_flags, null, new[] { typeof(FileDialog) }, null);
            private readonly static MethodInfo s_adviseMethodInfo = s_iFileDialogType.GetMethod("Advise");
            private readonly static MethodInfo s_unAdviseMethodInfo = s_iFileDialogType.GetMethod("Unadvise");
            private readonly static MethodInfo s_showMethodInfo = s_iFileDialogType.GetMethod("Show");

            public static ShowDialogResult Show(IntPtr ownerHandle, string initialDirectory, string title)
            {
                var openFileDialog = new OpenFileDialog
                {
                    AddExtension = false,
                    CheckFileExists = false,
                    DereferenceLinks = true,
                    Filter = c_foldersFilter,
                    InitialDirectory = initialDirectory,
                    Multiselect = false,
                    Title = title
                };

                var iFileDialog = s_createVistaDialogMethodInfo.Invoke(openFileDialog, new object[] { });
                s_onBeforeVistaDialogMethodInfo.Invoke(openFileDialog, new[] { iFileDialog });
                s_setOptionsMethodInfo.Invoke(iFileDialog, new object[] { (uint)s_getOptionsMethodInfo.Invoke(openFileDialog, new object[] { }) | s_fosPickFoldersBitFlag });
                var adviseParametersWithOutputConnectionToken = new[] { s_vistaDialogEventsConstructorInfo.Invoke(new object[] { openFileDialog }), 0U };
                s_adviseMethodInfo.Invoke(iFileDialog, adviseParametersWithOutputConnectionToken);

                try
                {
                    int retVal = (int)s_showMethodInfo.Invoke(iFileDialog, new object[] { ownerHandle });
                    return new ShowDialogResult
                    {
                        Result = retVal == 0,
                        FileName = openFileDialog.FileName
                    };
                }
                finally
                {
                    s_unAdviseMethodInfo.Invoke(iFileDialog, new[] { adviseParametersWithOutputConnectionToken[1] });
                }
            }
        }

        // Wrap an IWin32Window around an IntPtr
        private class WindowWrapper : IWin32Window
        {
            private readonly IntPtr _handle;
            public WindowWrapper(IntPtr handle) { _handle = handle; }
            public IntPtr Handle { get { return _handle; } }
        }
    }
}
