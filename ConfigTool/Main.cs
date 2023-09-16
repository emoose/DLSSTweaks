// NOTE: this code is pretty much freshly microwaved spaghetti
// It does the job it needs to, but probably could have been handled much better if I'd known where I was going with it from the start
//
// So far this code has had several stages:
// going from a quick test project to write out an INI ->
//   something that can also read our INI settings ->
//     reading INI settings & tracking a default INI file alongside it, copying over comments/missing settings as needed ->
//       also supporting NvProfile/DRS settings, and NvAPI VideoState settings which aren't kept in DRS for some reason ->
//           and finally, being able to work without reading/writing any INIs at all
//
// If you're reading this to try and learn more about NvProfile things the NvidiaProfileInspector codebase is much cleaner & better to learn from
//   https://github.com/Orbmu2k/nvidiaProfileInspector
// NvapiDrsWrapper.cs in this project originates there, though some extra NvAPI functions have been added here which may be useful

using DLSSTweaks.ConfigTool.Properties;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
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

        public static DrsSettings Drs = new DrsSettings();

        Dictionary<string, string> dllOverrides = new Dictionary<string, string>();

        string DefaultFormTitle = "DLSSTweaks"; // will be updated to actual text after InitializeComponent

        static string IniFilename = "dlsstweaks.ini";
        static string IniFilenameFriendly = "DLSSTweaks.ini";

        static string DefaultDescText = "Welcome to the DLSSTweaks ConfigTool!\r\n\r\nSelect or hover over any setting name to view a description of it here, or click on the value for the setting to edit it.\r\n\r\nIf you just want to force DLAA, simply edit the ForceDLAA value above and then save the changes.";
        static string UltraQualityText = "UltraQuality: allows setting the ratio for the 'UltraQuality' level.\r\n\r\nNot every game allows using this level, some may only expose it as an option once this has been set to non-zero.\r\nA very small number might also already show an UltraQuality level, which this setting should be able to customize.\r\n(the number of games that work with this is very small unfortunately)\r\n\r\nSet to 0 to leave this as DLSS default.";
        static string HoverLoadText = $"Reloads settings from INI & Nvidia profiles.";
        static string HoverSaveText = $"Writes out the changed settings to INI / Nvidia profiles.";
        static string HoverAddDLLOverrideText = "DLL override: allows overriding the path that a game will load a DLL from, simply pick the new DLL you wish to override with.\r\n\r\nThis can be useful if you're prevented from editing the game files for some reason.\r\n\r\neg. with Rockstar Game Launcher, you can't easily update nvngx_dlss.dll without RGL reverting it, but by using this you can make the game load DLSS from a completely different path which RGL can't override.";
        static string HoverInstallDllText = "Allows copying this DLSSTweaks config & DLL into a chosen folder.\r\n\r\nCan be used to either install freshly extracted DLSSTweaks into a game, or to copy existing config + DLL across to other titles.";
        static string HoverInstallDllTextUnavailable = "DLSSTweaks DLL not found in current folder, install not available.";

        static string NvidiaGlobalsSectionName = "NvGlobalProfileSettings (non-DLSSTweaks settings, DLSS 3.1.11+ only)";
        static string NvidiaGlobalsSectionNameShort = "NvGlobalProfileSettings";

        static string HoverNvSigOverrideText = "EnableNvidiaSigOverride: allows toggling the NVIDIA Signature Override registry key.\r\n\r\nWith the override enabled the DLSSTweaks DLL can be used in most DLSS2+ games by naming it as nvngx.dll.\r\n\r\n" +
            $"(note that the settings listed under {NvidiaGlobalsSectionNameShort} don't require this, this setting is only needed if you're using the DLSSTweaks DLL in a game - this override also only affects Nvidia related signature checks, not anything Windows related)\r\n\r\nChanging this requires Administrator privileges, an elevation prompt will appear if necessary.";

        static string HoverNvidiaGlobalsDisclaimer = $"{NvidiaGlobalsSectionNameShort} settings are checked by any games using DLSS 3.1.11+, and don't require the DLSSTweaks DLL to be setup for DLSS to use them.\r\n\r\nHowever, changes to the \"{NvidiaGlobalsSectionNameShort}\" section require ConfigTool to be ran as admin, if necessary you will be prompted to relaunch when saving.\r\n\r\n";

        static string HoverGlobalForceDLAAText = "GlobalForceDLAA: if set to true, all DLSS quality levels will be forced as DLAA instead, on all DLSS 3.1.11+ games (without DLSSTweaks needing to be setup on them).\r\n\r\nMay have compatibility issues with certain titles as this setting is handled by DLSS itself, not DLSSTweaks, so the compatibility workarounds used by DLSSTweaks can't be applied to it.";
        static string HoverGlobalForcedScaleText = "GlobalForcedScale: if set, forces all DLSS quality levels to the specified render scale, on all DLSS 3.1.11+ games (without DLSSTweaks needing to be setup on them).\r\n\r\nValid range is 0.33 to 1.00, set to 0 to disable.";
        static string HoverGlobalForcedPresetText = "GlobalForcedPreset: if set, forces all DLSS quality levels to use the specified preset, on all DLSS 3.1.11+ games (without DLSSTweaks needing to be setup on them).\r\n\r\nMay come in useful if DLSSTweaks was unable to force a preset for some reason.";
        static string HoverGlobalHudOverrideText = "GlobalHudOverride: updates the registry key used by all DLSS games to know if the DLSS debug HUD should be activated.\r\n\r\nThe OverrideDlssHud setting included in DLSSTweaks can already enable the HUD for you, but this may still be useful if DLSSTweaks is unable to show the HUD for some reason.\r\n\r\n" +
            "The (dev DLLs only) option will only enable the DLSS HUD on certain 'dev' versions of DLSS, while (all DLLs) will let it show on all versions of DLSS.";

        static string HoverGlobalVideoSuperResText = "VSRQuality ##(NUM): changes quality level for Nvidia VideoSuperResolution on monitor number #(NUM).\r\n\r\n" + 
            "When enabled VSR can improve the scaling applied to internet videos & certain video-playing apps.\r\n\r\n" +
            "\"With RTX VSR, GeForce RTX 40 and 30 Series GPU users can tap AI to upscale lower-resolution content up to 4K, matching their display resolution. The AI removes blocky compression artifacts and improves the video’s sharpness and clarity.\" - https://blogs.nvidia.com/blog/2023/02/28/rtx-video-super-resolution/\r\n\r\n" + 
            "Nvidia Control Panel can let you pick from quality 1 - 4, but the video setting itself allows choosing up to quality 5 - it's unknown whether this gives any image quality improvement (power draw seems the same as quality 4 at least).";

        static string DllPathOverrideText = "DLLPathOverrides: allows overriding the path that a DLL will be loaded from based on the filename of it\r\n\r\nRight click on the override for options to rename/delete it.";

        public static string NvidiaGlobalsForceDLAAKey = "GlobalForceDLAA";
        public static string NvidiaGlobalsForcedScaleKey = "GlobalForcedScale";
        public static string NvidiaGlobalsForcedPresetKey = "GlobalForcedPreset";
        public static string NvidiaGlobalsHudOverrideKey = "GlobalHudOverride";
        public static string NvidiaGlobalsSigOverrideKey = "EnableNvidiaSigOverride";
        public static string NvidiaGlobalsVideoSuperResKey = "VSRQuality";

        public static string[] ElevationRequiredKeys = new[]
        {
            NvidiaGlobalsForceDLAAKey, NvidiaGlobalsForcedScaleKey, NvidiaGlobalsForcedPresetKey, NvidiaGlobalsHudOverrideKey
        };

        static string NvidiaHudOverrideEnabledDevValue = "Enabled (dev DLLs only)";
        static string NvidiaHudOverrideEnabledAllValue = "Enabled (all DLLs)";
        static string NvidiaHudOverrideDisabledValue = "Disabled";

        static string[] BooleanKeys = new[] { "GlobalForceDLAA", "ForceDLAA", "DisableDevWatermark", "VerboseLogging", "Enable", "DisableIniMonitoring", "OverrideAppId", "EnableNvidiaSigOverride", "DynamicResolutionOverride" };
        
        static string[] OverrideKeys = new[] { "OverrideAutoExposure", "OverrideDlssHud" };
        static string[] OverrideValues = new[] { "Default", "Force disable", "Force enable" }; // 0, -1, 1

        string DlssTweaksDll = "";

        // Supported DLL filenames, in order of least -> most preferred (least preferred = most likely to be used by other wrappers)
        static string[] DlssTweaksDllFilenames = new[] { "nvngx.dll", "xinput1_3.dll", "xinput1_4.dll", "xinput9_1_0.dll", "dxgi.dll", "xapofx1_5.dll", "x3daudio1_7.dll", "winmm.dll" };

        List<IniChange> IniChanges = new List<IniChange>(); // Changes to apply to INI in next IniWrite, stuff like renamed/removed entries, etc

        bool IsChangeUnsaved = false;

        string[] SearchDlssTweaksDlls(string path)
        {
            var dlls = new List<string>();
            foreach (var filename in DlssTweaksDllFilenames)
            {
                try
                {
                    var filepath = Path.Combine(path, filename);
                    if (!File.Exists(filepath))
                        continue;

                    FileVersionInfo fileInfo = FileVersionInfo.GetVersionInfo(filepath);
                    if (fileInfo.ProductName != "DLSSTweaks")
                        continue;

                    dlls.Add(filepath);
                }
                catch
                {
                    continue;
                }
            }
            return dlls.ToArray();
        }

        string SearchForGameExe(string dirPath)
        {
            List<string> filesToCheck = new List<string>();

            // Try searching for UE4/UE5 specific EXE name
            var allExeFiles = Directory.GetFiles(dirPath, "*.exe", SearchOption.AllDirectories);
            foreach (var filePath in allExeFiles)
            {
                var fileName = Path.GetFileNameWithoutExtension(filePath).ToLower();
                if (fileName.Contains("-win") && fileName.EndsWith("-shipping"))
                    filesToCheck.Add(filePath);
            }

            // blacklist some DLLs that are known to not be relevant
            var blacklistDlls = new string[]
            {
                    "dlsstweak",
                    "igxess",
                    "libxess",
                    "nvngx",
                    "EOSSDK-",
                    "steam_api",
                    "sl.",
                    "D3D12",
                    "dstorage",
                    "PhysX",
                    "NvBlast",
                    "amd_",
                    "PeanutButter."
            };

            if (filesToCheck.Count <= 0)
            {
                // Fetch all EXE/DLL files in the chosen dir
                filesToCheck.AddRange(Directory.GetFiles(dirPath, "*.exe"));
                filesToCheck.AddRange(Directory.GetFiles(dirPath, "*.dll"));
            }

            long dllSizeMinimum = 2_097_152; // 2MB

            // Return largest EXE we find in the specified folder, most likely to be the game EXE
            // TODO: search subdirectories too and recommend user change folder if larger EXE was found?
            FileInfo largest = null;
            foreach (var file in filesToCheck)
            {
                var lowerName = Path.GetFileName(file).ToLower();

                // Check against blacklistDlls array
                if (blacklistDlls.Any(blacklistDll => lowerName.StartsWith(blacklistDll.ToLower())))
                    continue;

                var info = new FileInfo(file);

                if (info.Extension.ToLower() == "dll" && info.Length < dllSizeMinimum)
                    continue;

                if (largest == null || info.Length > largest.Length)
                    largest = info;
            }
            if (largest == null)
                return null;

            return largest.FullName;
        }

        bool IniIsLikelyValid()
        {
            if (!File.Exists(IniFilename))
                return false;

            try
            {
                var info = new FileInfo(IniFilename);
                if (info.Length > 0)
                    return true;
            }
            catch { }

            return false;
        }

        string RunDlssTweaksDllCleanup(string prompt, string[] dllsToCleanup, string dllBasePath)
        {
            var choices = new List<string>();
            var keepFileName = "";
            foreach (var dll in dllsToCleanup)
            {
                var dllFilename = Path.GetFileName(dll);
                var fileVersion = "";
                try
                {
                    var fileVersionInfo = FileVersionInfo.GetVersionInfo(dll);
                    fileVersion = $" (v{fileVersionInfo.FileVersion})";
                }
                catch { }
                choices.Add(dllFilename + fileVersion);
                keepFileName = dllFilename; // use last DLL as the default keeper
            }

            // User has multiple DLSSTweaks DLLs in the current folder, le sigh
            var result = Utility.MultipleChoice(prompt + "\r\n" +
                "To let ConfigTool clear these DLLs, enter the filename of the DLL you wish to keep.\r\n" +
                "The rest will then be removed, alternatively press Cancel to ignore this.\r\n\r\n" +
                "Detected DLLs:", "Multiple DLSSTweaks DLLs!", choices.ToArray(), "\r\n  ", ref keepFileName);
            if (result != DialogResult.OK || string.IsNullOrEmpty(keepFileName))
                return null;

            var keepFilePath = Path.Combine(dllBasePath, keepFileName);

            if (!File.Exists(keepFilePath))
            {
                MessageBox.Show($"File {keepFileName} not found, aborting cleanup.", "File cleanup failed");
                return null;
            }

            var filesToDelete = new List<string>();
            foreach (var dll in dllsToCleanup)
                if (!string.Equals(Path.GetFileName(dll), keepFileName, StringComparison.OrdinalIgnoreCase))
                    filesToDelete.Add(dll);

            if (MessageBox.Show("The following files will be deleted:\n\n" +
                string.Join("\n", filesToDelete) + "\n\n" +
                "The following file will be kept:\n\n" +
                keepFilePath + "\n\n" +
                "Is this OK?", "Confirm File Deletion", MessageBoxButtons.OKCancel) != DialogResult.OK)
            {
                if (File.Exists(keepFilePath))
                    return keepFileName;

                return null;
            }

            foreach (var dll in filesToDelete)
            {
                if (!string.Equals(Path.GetFileName(dll), keepFileName, StringComparison.OrdinalIgnoreCase))
                    try
                    {
                        File.Delete(dll);
                    }
                    catch
                    {
                    }
            }

            if (File.Exists(keepFilePath))
                return keepFileName;

            return null;
        }

        bool SkipLoadWarnings = false;

        void ProcessArgs()
        {
            var args = Environment.GetCommandLineArgs();
            if (args == null || args.Length < 2)
                return;

            foreach (var arg in args)
                if (arg.ToLower() == "-SkipLoadWarnings".ToLower())
                    SkipLoadWarnings = true;
        }

        void UpdateTitle()
        {
            var formTitle = $"{DefaultFormTitle}";
            if (!IniIsEmpty)
                formTitle += $" - {IniFilename}";
            else
                formTitle += $" - Nvidia Settings";

            if (IsChangeUnsaved)
                formTitle += "*";

            if (!string.IsNullOrEmpty(DlssTweaksDll))
                formTitle += $" ({DlssTweaksDll})";
            if (NvSigOverride.IsElevated())
                formTitle += " (admin)";

            this.Text = formTitle;
        }

        bool IniIsEmpty = false;

        public void IniRead()
        {
            Drs.Read();

            lvSettings.Unfocus();
            lvSettings.Items.Clear();
            lvSettings.Groups.Clear();
            lvSettings.ClearEditCells();

            IsChangeUnsaved = false;

            IniChanges = new List<IniChange>();

            string[] lines = null;

            IniIsEmpty = !IniIsLikelyValid();

            if (!IniIsEmpty)
            {
                lblIniPath.Text = Path.GetFullPath(IniFilename);
                lines = File.ReadAllLines(IniFilename);
            }
            else
            {
                lblIniPath.Text = $"DLSSTweaks settings INI not found, only displaying Nvidia profile globals.";
            }

            addDLLOverrideToolStripMenuItem.Enabled = !IniIsEmpty;

            UpdateTitle();

            var userIni = new HackyIniParser();
            userIni.Parse(lines);

            var defaultIni = new HackyIniParser();
            defaultIni.Parse(Resources.DefaultINI.Split(new string[] { "\r\n" }, StringSplitOptions.None));

            // Copy across all comments from defaultIni to our userIni
            foreach (var section in defaultIni.Entries)
            {
                if (!userIni.Entries.ContainsKey(section.Key))
                    continue;
                foreach (var entry in section.Value)
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
            foreach (var section in defaultIni.Entries)
            {
                if (section.Key.ToLower() != "dlssqualitylevels")
                    continue;

                foreach (var entry in section.Value)
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
                foreach (var key in keys)
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

                // If this is a scale value calculate the resolution & display it in tooltip
                if ((group.Header.ToLower() == "dlssqualitylevels" && key.ToLower() != "enable") ||
                    (group.Header == NvidiaGlobalsSectionName && key == NvidiaGlobalsForcedScaleKey))
                {
                    listViewItem.ToolTipText = CalculateResForScale(value);
                }

                var row = lvSettings.Items.Count - 1;

                if (isBooleanKey)
                    lvSettings.AddComboBoxCell(row, 1, new string[] { "False", "True" });
                else
                {
                    if (isOverrideKey)
                        lvSettings.AddComboBoxCell(row, 1, OverrideValues);
                    else
                    {
                        if (section == "DLSSPresets" || key == NvidiaGlobalsForcedPresetKey)
                            lvSettings.AddComboBoxCell(row, 1, new string[] { "Default", "A", "B", "C", "D", "E", "F", "G" });
                        else if (key.StartsWith(NvidiaGlobalsVideoSuperResKey))
                            lvSettings.AddComboBoxCell(row, 1, new string[] { "Disabled", "1", "2", "3", "4", "5" });
                        else if (key == NvidiaGlobalsHudOverrideKey)
                            lvSettings.AddComboBoxCell(row, 1, new string[] { NvidiaHudOverrideDisabledValue, NvidiaHudOverrideEnabledDevValue, NvidiaHudOverrideEnabledAllValue });
                        else
                            lvSettings.AddEditableCell(row, 1);
                    }
                }
            }

            // Add Global pseudo-settings to list
            if (!userIni.Entries.ContainsKey(NvidiaGlobalsSectionName))
            {
                var entries = new Dictionary<string, HackyIniParser.IniEntry>()
                {
                    {
                        NvidiaGlobalsForceDLAAKey,
                        new HackyIniParser.IniEntry()
                        {
                            Section = NvidiaGlobalsSectionName,
                            Key = NvidiaGlobalsForceDLAAKey,
                            Comment = HoverNvidiaGlobalsDisclaimer + HoverGlobalForceDLAAText,
                            Value = Drs.OverrideForceDLAA ? "True" : "False"
                        }
                    },
                    {
                        NvidiaGlobalsForcedScaleKey,
                        new HackyIniParser.IniEntry()
                        {
                            Section = NvidiaGlobalsSectionName,
                            Key = NvidiaGlobalsForcedScaleKey,
                            Comment = HoverNvidiaGlobalsDisclaimer + HoverGlobalForcedScaleText,
                            Value = Drs.OverrideScaleRatio.ToString()
                        }
                    },
                    {
                        NvidiaGlobalsForcedPresetKey,
                        new HackyIniParser.IniEntry()
                        {
                            Section = NvidiaGlobalsSectionName,
                            Key = NvidiaGlobalsForcedPresetKey,
                            Comment = HoverNvidiaGlobalsDisclaimer + HoverGlobalForcedPresetText,
                            Value = Drs.OverrideRenderPreset == 0 ? "Default" : new string((char)('A' + (Drs.OverrideRenderPreset - 1)), 1)
                        }
                    },
                    {
                        NvidiaGlobalsHudOverrideKey,
                        new HackyIniParser.IniEntry()
                        {
                            Section = NvidiaGlobalsSectionName,
                            Key = NvidiaGlobalsHudOverrideKey,
                            Comment = HoverNvidiaGlobalsDisclaimer + HoverGlobalHudOverrideText,
                            Value = Drs.HudStatus == DrsSettings.NvHudStatus.Enabled ? NvidiaHudOverrideEnabledDevValue :
                                (Drs.HudStatus == DrsSettings.NvHudStatus.EnabledAllDlls ? NvidiaHudOverrideEnabledAllValue : NvidiaHudOverrideDisabledValue)
                        }
                    },
                    {
                        NvidiaGlobalsSigOverrideKey,
                        new HackyIniParser.IniEntry()
                        {
                            Section = NvidiaGlobalsSectionName,
                            Key = NvidiaGlobalsSigOverrideKey,
                            Comment = HoverNvSigOverrideText,
                            Value = NvSigOverride.IsOverride() ? "True" : "False"
                        }
                    },
                };

                foreach (var kvp in Drs.VideoSuperResolutionQuality)
                {
                    var key = $"{NvidiaGlobalsVideoSuperResKey} #{kvp.Key}";
                    entries.Add(key, new HackyIniParser.IniEntry()
                    {
                        Section = NvidiaGlobalsSectionName,
                        Key = key,
                        Comment = HoverGlobalVideoSuperResText.Replace("#(NUM)", $"{kvp.Key}"),
                        Value = kvp.Value == 0 ? "Disabled" : kvp.Value.ToString()
                    });
                }
                userIni.Entries.Add(NvidiaGlobalsSectionName, entries);
            }

            // AddSetting must be called in the exact order of items to add
            // Can't AddSetting to an earlier section since that would break the hacky ListViewEx input boxes
            // So all entries should be prepared inside userIni.Entries first

            var orderedSections = new Dictionary<string, Dictionary<string, HackyIniParser.IniEntry>>();

            if (userIni.Entries.ContainsKey("DLSS"))
                orderedSections.Add("DLSS", userIni.Entries["DLSS"]);
            if (userIni.Entries.ContainsKey(NvidiaGlobalsSectionName))
                orderedSections.Add(NvidiaGlobalsSectionName, userIni.Entries[NvidiaGlobalsSectionName]);

            foreach (var section in userIni.Entries)
            {
                if (section.Key == "DLSS" || section.Key == NvidiaGlobalsSectionName)
                    continue;
                orderedSections.Add(section.Key, section.Value);
            }

            foreach (var section in orderedSections)
            {
                foreach (var entry in section.Value)
                {
                    AddSetting(entry.Value.Section, entry.Value.Key, entry.Value.Value, entry.Value.Comment);
                }
            }
        }

        bool IniCheckPermissions()
        {
            bool deleteAfter = !File.Exists(IniFilename);
            bool hasPermission = false;

            try
            {
                using (FileStream fileStream = File.Open(IniFilename, FileMode.OpenOrCreate, FileAccess.Write))
                {
                    hasPermission = true;
                }
            }
            catch
            {
                hasPermission = false;
            }

            if (File.Exists(IniFilename) && deleteAfter)
                try
                {
                    File.Delete(IniFilename);
                }
                catch { }

            return hasPermission;
        }

        void IniWrite()
        {
            try
            {
                IniWriteMain();
            }
            catch (UnauthorizedAccessException)
            {
                MessageBox.Show($"UnauthorizedAccessException: DLSSTweaks failed to write INI file to the following path:\r\n\r\n" + 
                    $"  {Path.GetFullPath(IniFilename)}\r\n\r\nYou may need to relaunch ConfigTool as administrator.", "Failed to write INI", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        void IniWriteMain()
        {
            lvSettings.Unfocus();

            PeanutButter.INI.INIFile file = new PeanutButter.INI.INIFile();

            // Only let INIParser read from INI if it already exists, we don't handle creating INI yet
            if (!IniIsEmpty)
                file = new PeanutButter.INI.INIFile(IniFilename);

            file.WrapValueInQuotes = false;

            foreach (ListViewItem item in lvSettings.Items)
            {
                var section = item.Group.Header;
                var key = item.Text;
                var value = item.SubItems[1].Text;

                if (section == NvidiaGlobalsSectionName)
                {
                    if (key == NvidiaGlobalsForceDLAAKey)
                        Drs.OverrideForceDLAA = Utility.ParseBool(value);
                    else if (key == NvidiaGlobalsForcedPresetKey)
                    {
                        Drs.OverrideRenderPreset = 0;
                        if (value != "Default")
                        {
                            int presetNum = value[0] - 'A';
                            Drs.OverrideRenderPreset = (uint)(presetNum + 1);
                        }
                    }
                    else if (key == NvidiaGlobalsForcedScaleKey)
                    {
                        Drs.OverrideScaleRatio = float.Parse(value);
                    }
                    else if (key == NvidiaGlobalsHudOverrideKey)
                    {
                        Drs.HudStatus = DrsSettings.NvHudStatus.Disabled;
                        if (value == NvidiaHudOverrideEnabledDevValue)
                            Drs.HudStatus = DrsSettings.NvHudStatus.Enabled;
                        else if (value == NvidiaHudOverrideEnabledAllValue)
                            Drs.HudStatus = DrsSettings.NvHudStatus.EnabledAllDlls;
                    }
                    else if (key.StartsWith(NvidiaGlobalsVideoSuperResKey))
                    {
                        var numBegin = key.IndexOf("#");
                        if (numBegin > -1)
                        {
                            var indexStr = key.Substring(numBegin + 1);
                            var index = uint.Parse(indexStr);

                            Drs.VideoSuperResolutionQuality[index] = 0;
                            if (value != "Disabled")
                            {
                                Drs.VideoSuperResolutionQuality[index] = uint.Parse(value);
                            }
                        }
                    }

                    continue;
                }

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

            foreach (var change in IniChanges)
            {
                var value = file[change.Section][change.Key];
                file.RemoveValue(change.Section, change.Key);
                if (!change.IsDeletion)
                    file[change.Section][change.NewKey] = value;
            }

            // Write out changed INI -- but only if INI already exists, ConfigTool currently doesn't handle creating new one
            if (!IniIsEmpty)
                file.Persist();

            // INI has been written, try writing any DRS settings if they've been changed
            if (Drs.HasChanges())
            {
                try
                {
                    Drs.Write();
                }
                catch (UnauthorizedAccessException)
                {
                    if (MessageBox.Show($"UnauthorizedAccessException: ConfigTool failed to write {NvidiaGlobalsSectionNameShort}.\r\n\r\n" +
                        $"Any DLSSTweaks INI changes have been saved, but {NvidiaGlobalsSectionNameShort} changes failed to apply.\r\n\r\n" +
                        "ConfigTool may need to be launched as administrator, do you want to relaunch as admin? (requires UAC prompt)", "Failed to write DRS settings", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes)
                    {
                        NvSigOverride.Elevate("", false);
                        Process.GetCurrentProcess().Kill();
                    }
                }
            }

            IniRead();
        }

        public Main()
        {
            InitializeComponent();
            ProcessArgs();

            // Column header sizing seems to act weird at different DPIs, ugh
            {
                float dpi = 96;
                Graphics g = CreateGraphics();
                try
                {
                    dpi = Math.Max(g.DpiX, g.DpiY);
                }
                finally
                {
                    g.Dispose();
                }

                if (dpi < 96)
                    dpi = 96;

                float origDpi = 168; // DPI used during dev

                float scale = dpi / origDpi;
                chSetting.Width = (int)Math.Ceiling(scale * chSetting.Width);
                chValue.Width = (int)Math.Ceiling(scale * chValue.Width);
            }

            statusStrip1.LayoutStyle = ToolStripLayoutStyle.Flow;

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

            var dlssTweaksDlls = SearchDlssTweaksDlls("");
            DlssTweaksDll = null;

            if (dlssTweaksDlls.Length == 1)
                DlssTweaksDll = dlssTweaksDlls[0];
            else if (dlssTweaksDlls.Length > 1)
            {
                DlssTweaksDll = RunDlssTweaksDllCleanup(
                        "Multiple DLSSTweaks DLLs have been detected in the ConfigTool directory, this is likely to result in unstability.", dlssTweaksDlls, "");
            }

            if (string.IsNullOrEmpty(DlssTweaksDll))
            {
                installToGameFolderToolStripMenuItem.Enabled = false;
                installToGameFolderToolStripMenuItem.ToolTipText = HoverInstallDllTextUnavailable;
            }

            if (!IniCheckPermissions())
            {
                if (MessageBox.Show($"DLSSTweaks ConfigTool is unable to write to {IniFilenameFriendly}, the file may be protected, or ConfigTool may need to run as administrator." +
                    "\r\n\r\nDo you want to relaunch ConfigTool as admin? (requires UAC prompt)", "INI access denied", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.Yes)
                {
                    NvSigOverride.Elevate("", false);
                    Process.GetCurrentProcess().Kill();
                }
            }

            IniRead();

            // Alert user if this is being ran with no game EXE in current folder
            if (!IniIsEmpty)
            {
                var exePath = SearchForGameExe(Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName));
                if (string.IsNullOrEmpty(exePath))
                {
                    if (!SkipLoadWarnings)
                        MessageBox.Show("It looks like DLSSTweaks ConfigTool has been launched outside of a game directory (no game EXE found).\n\n" +
                            "It's recommended to copy DLSSTweaks into a game folder first before configuring it.\n\n" +
                            "You can let ConfigTool copy the necessary files for you via the \"Copy to game folder...\" command on top right.", "Game EXE not found!");
                }
                else
                {
                    // We have a game EXE in this folder
                    // If user is setting up DLSSTweaks as nvngx.dll, check whether registry override is active, and alert them if not
                    if (!string.IsNullOrEmpty(DlssTweaksDll) && Path.GetFileName(DlssTweaksDll).ToLower() == "nvngx.dll")
                    {
                        if (!NvSigOverride.IsOverride())
                        {
                            if (!SkipLoadWarnings && MessageBox.Show("It appears that DLSSTweaks is loading in via the nvngx.dll wrapper method.\n\n" +
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
            }
        }

        private string CalculateResForScale(string scaleVal)
        {
            var screen = Screen.FromControl(this);
            int screenWidth = screen.Bounds.Width;
            int screenHeight = screen.Bounds.Height;

            var resSeperator = scaleVal.IndexOf('x');
            if (resSeperator >= 0 && scaleVal.Length > resSeperator + 1)
            {
                // Calc ratio from users resolution choice
                var widthStr = scaleVal.Substring(0, resSeperator);
                var heightStr = scaleVal.Substring(resSeperator + 1);
                float width = 0;
                float height = 0;
                if (float.TryParse(widthStr, out width) && float.TryParse(heightStr, out height))
                {
                    if(width > 0 && height > 0)
                    {
                        float widthRatio = width / screenWidth;
                        float heightRatio = height / screenHeight;

                        string ratioStr = $"{widthRatio}x";
                        if (widthRatio != heightRatio)
                            ratioStr = $"{widthRatio}x (width), {heightRatio}x (height)";

                        return $"     {screenWidth}x{screenHeight} / {width}x{height}\r\n      = {ratioStr}";
                    }
                }
            }

            // Calc resolution from users ratio choice
            float scale = 0;
            if (!float.TryParse(scaleVal, out scale) || scale < 0)
                return string.Empty;

            int newWidth = (int)Math.Ceiling(scale * screenWidth);
            int newHeight = (int)Math.Ceiling(scale * screenHeight);

            return $"     {screenWidth}x{screenHeight} @ {scaleVal}x\r\n      = {newWidth}x{newHeight}";
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
                if ((lvi.Text == NvidiaGlobalsForcedScaleKey) ||
                    (lvi.Group.Header.ToLower() == "dlssqualitylevels" && lvi.Text.ToLower() != "enable"))
                {
                    // If user added x to end of scale then remove it...
                    if (lvi.SubItems[1].Text.EndsWith("x"))
                        lvi.SubItems[1].Text = lvi.SubItems[1].Text.Substring(0, lvi.SubItems[1].Text.Length - 1);

                    lvi.ToolTipText = CalculateResForScale(lvi.SubItems[1].Text);
                }

                IsChangeUnsaved = true;
                UpdateTitle();
            }
            lvSettings.Focus();
        }

        // Returns whether tooltip should be shown or hidden
        ListViewItem.ListViewSubItem prevSubItem = null;
        private bool lvSettings_MouseMove_Main(object sender, MouseEventArgs e)
        {
            ListViewHitTestInfo hitTest = null;
            try
            {
                hitTest = lvSettings.HitTest(e.X, e.Y);
            }
            catch
            {
                hitTest = null;
            }

            if (hitTest == null)
                return false;

            var item = hitTest.Item;
            if (item == null || item.Tag == null || item.Tag.GetType() != typeof(string) || hitTest.SubItem == null)
                return false;

            if (prevSubItem != hitTest.SubItem)
                toolTip.Hide(lvSettings);

            prevSubItem = hitTest.SubItem;

            if (hitTest.SubItem == item.SubItems[0])
                txtDesc.Text = (string)item.Tag;
            else if (hitTest.SubItem == item.SubItems[1])
            {
                // Display the tooltip for the item under the mouse cursor
                toolTip.Show(hitTest.Item.ToolTipText, lvSettings, new Point(e.Location.X + 5, e.Location.Y));
                return true;
            }

            return false;
        }
        private void lvSettings_MouseMove(object sender, MouseEventArgs e)
        {
            var res = lvSettings_MouseMove_Main(sender, e);
            if (!res)
                toolTip.Hide(lvSettings);
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
            var result = Utility.InputBox("Enter new name for setting", "Setting name", ref value);
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

            var result = MessageBox.Show($"Setting DLL override\r\n\r\n{filename} -> {filepath}\r\n\r\nOverride will be added & settings saved, is this OK?\r\n\r\n(after adding this you can set the name of the DLL to override by right clicking entry and picking Rename)", "Confirm", MessageBoxButtons.YesNo);
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
            if (!string.IsNullOrEmpty(DlssTweaksDll))
                txtDesc.Text = HoverInstallDllText;
            else
                txtDesc.Text = HoverInstallDllTextUnavailable;
        }

        private void installToGameFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(DlssTweaksDll))
                MessageBox.Show("Failed to locate DLSSTweaks DLL, install aborted.");

            DialogResult result = DialogResult.No;
            if (IsChangeUnsaved)
            {
                result = MessageBox.Show("You have unsaved changes, save those first?", "Save?", MessageBoxButtons.YesNoCancel);
                if (result == DialogResult.Cancel)
                    return;
                else if (result == DialogResult.Yes)
                    IniWrite();
            }

            var configToolName = Path.GetFileName(Process.GetCurrentProcess().MainModule.FileName);

            var filesToCopy = new[] {
                DlssTweaksDll,
                IniFilename,
                configToolName
            };

            foreach (var file in filesToCopy)
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

            var preferredDllName = "";
            bool isReplacingDll = false;

            var destFolderPath = dialog.FileName;

            // If we have an existing DLSSTweaks DLL in the dest folder, we'll replace that instead of prompting user for filename to use
            var existingDlls = SearchDlssTweaksDlls(destFolderPath);
            if (existingDlls.Length == 1)
            {
                preferredDllName = Path.GetFileName(existingDlls[0]);
            }
            else if (existingDlls.Length > 1)
            {
                preferredDllName =
                    RunDlssTweaksDllCleanup("Multiple DLSSTweaks DLLs have been detected in the target directory, this is likely to result in unstability.", existingDlls, dialog.FileName);
            }
            else
            {
                var gameExe = SearchForGameExe(destFolderPath);
                if (!string.IsNullOrEmpty(gameExe))
                {
                    string[] importedModules = new string[0];

                    try
                    {
                        // Found a game EXE, try checking the imports of it
                        using (var peStream = new FileStream(gameExe, FileMode.Open, FileAccess.Read, FileShare.Read))
                        using (var reader = new BinaryReader(peStream))
                        {
                            var modulePe = new QuickPEReader();
                            if (modulePe.Read(reader))
                            {
                                importedModules = modulePe.ReadImportedModules(reader);
                                if (importedModules == null)
                                    importedModules = new string[0];
                            }
                        }
                    }
                    catch
                    {
                    }

                    if (importedModules.Length > 0)
                    {
                        var availableNames = "nvngx.dll";
                        preferredDllName = "nvngx.dll";
                        string supportedDllText = "only supports the following";
                        foreach (var module in DlssTweaksDllFilenames)
                        {
                            foreach (var importedModule in importedModules)
                            {
                                var importedModLower = importedModule.ToLower();
                                if (importedModLower == module.ToLower())
                                {
                                    availableNames += ", " + importedModLower;
                                    preferredDllName = importedModLower;
                                    supportedDllText = "supports multiple";
                                }
                            }
                        }

                        // If they have the NV sig override set we'll use nvngx.dll as preferred filename
                        if (NvSigOverride.IsOverride())
                            preferredDllName = "nvngx.dll";

                        var gameFileType = "executable";
                        if (Path.GetExtension(gameExe).ToLower() == ".dll")
                            gameFileType = "file";

                        if (Utility.InputBox($"The game {gameFileType} \"{Path.GetFileName(gameExe)}\" {supportedDllText} DLSSTweaks DLL filenames.\n\n" +
                            $"Please enter the filename you want to use:\r\n  {availableNames}", "Enter a DLL wrapper filename", ref preferredDllName)
                            != DialogResult.OK)
                        {
                            return;
                        }

                        destFolderPath = Path.GetDirectoryName(gameExe);
                    }
                }
            }

            using (var tempFolder = new TempFolder())
            {
                if (!string.IsNullOrEmpty(preferredDllName))
                {
                    // Copy the DLL into a temp folder using the users specified filename
                    // so that the win32 copy later on can also copy it & handle any existing file issues etc for us
                    tempFolder.New();

                    var tempFileName = Path.Combine(tempFolder.GetPath(), preferredDllName);
                    File.Copy(DlssTweaksDll, tempFileName, true);
                    if (File.Exists(tempFileName))
                        filesToCopy[0] = tempFileName;
                }

                var infoText = "";
                foreach (var file in filesToCopy)
                {
                    var fileName = Path.GetFileName(file);
                    var dest = Path.Combine(destFolderPath, fileName);
                    infoText += $"{fileName} -> {dest}\r\n\r\n";
                }

                var infoTextPost = "";
                if (isReplacingDll)
                    infoTextPost = $"\r\n\r\n({preferredDllName} is being replaced with the DLL from the ConfigTool folder)";

                result = MessageBox.Show($"Copying the following DLSSTweaks files:\r\n\r\n{infoText}Is this OK?{infoTextPost}", "Confirm", MessageBoxButtons.YesNo);
                if (result != DialogResult.Yes)
                    return;

                try
                {
                    FileOperations.CopyFiles(filesToCopy, destFolderPath);

                    // Wait a sec for the little copy animation
                    System.Threading.Thread.Sleep(1000);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Install failed, exception {ex.Message} during file copy...");
                    return;
                }

                var renameNote = !tempFolder.IsSet ? ", note that the DLL may need to be renamed for game to load it" : "";

                result = MessageBox.Show($"Files copied to game folder{renameNote}.\r\n\r\nDo you want to close down this ConfigTool & configure the new install?", "Confirm", MessageBoxButtons.YesNo);
                if (result != DialogResult.Yes)
                    return;
            }

            var startInfo = new ProcessStartInfo() { WorkingDirectory = destFolderPath, FileName = Path.Combine(destFolderPath, configToolName) };

            var proc = Process.Start(startInfo);

            // Wait up to 3000ms for MainWindowHandle to be set...
            int tries = 3;
            while (tries >= 0)
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

            DialogResult result = MessageBox.Show("You have unsaved changes, save those before exiting?", "Save?", MessageBoxButtons.YesNoCancel);
            if (result == DialogResult.Cancel)
                e.Cancel = true;
            else if (result == DialogResult.Yes)
                IniWrite();
        }
    }
}
