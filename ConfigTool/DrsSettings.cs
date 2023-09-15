using Microsoft.Win32;
using nspector.Native.NVAPI2;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;
using nvapi = nspector.Native.NVAPI2.NvapiDrsWrapper;

namespace DLSSTweaks.ConfigTool
{
    public class DrsSettings
    {
        static IntPtr hSession = IntPtr.Zero;
        static IntPtr hProfile = IntPtr.Zero;

        const uint DRS_NGX_OVERRIDE_RENDER_PRESET_SELECTION = 0x10E41DF3;
        const uint DRS_NGX_FORCE_DLAA = 0x10E41DF4;
        const uint DRS_NGX_SCALE_RATIO = 0x10E41DF5;

        const uint VIDEO_STATE_VIDEOSUPERRESOLUTION = 29;

        public uint OverrideRenderPreset = 0;
        uint OverrideRenderPresetOrig = 0;
        public bool OverrideForceDLAA = false;
        bool OverrideForceDLAAOrig = false;
        public float OverrideScaleRatio = 0;
        float OverrideScaleRatioOrig = 0;

        public Dictionary<uint, uint> VideoSuperResolutionQuality = new Dictionary<uint, uint>();
        Dictionary<uint, uint> VideoSuperResolutionQualityOrig = new Dictionary<uint, uint>();

        public enum NvHudStatus : uint
        {
            Disabled = 0,
            Enabled = 1,
            EnabledAllDlls = 0x400
        }

        public NvHudStatus HudStatus = NvHudStatus.Disabled;
        NvHudStatus HudStatusOrig = NvHudStatus.Disabled;

        void ReadHudStatus()
        {
            // Read in DLSS HUD indicator value
            try
            {
                RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NVIDIA Corporation\\Global\\NGXCore", false);
                var value = (uint)(int)key.GetValue("ShowDlssIndicator");
                HudStatusOrig = HudStatus = (NvHudStatus)value;
            }
            catch
            {
                HudStatusOrig = HudStatus = NvHudStatus.Disabled;
            }
        }

        void WriteHudStatus()
        {
            try
            {
                if (HudStatus == HudStatusOrig)
                    return; // nothing to do
                RegistryKey key = Registry.LocalMachine.OpenSubKey("SOFTWARE\\NVIDIA Corporation\\Global\\NGXCore", true);
                key.SetValue("ShowDlssIndicator", (uint)HudStatus, RegistryValueKind.DWord);
            }
            catch (SecurityException)
            {
                throw new UnauthorizedAccessException();
            }
        }

        public void ReadVideoSettings()
        {
            uint displayIdx = 0;
            IntPtr displayHandle = IntPtr.Zero;

            VideoSuperResolutionQuality = new Dictionary<uint, uint>();
            VideoSuperResolutionQualityOrig = new Dictionary<uint, uint>();

            while (nvapi.EnumNvidiaDisplayHandle(displayIdx, ref displayHandle) == NvAPI_Status.NVAPI_OK)
            {
                NVAPI_GetVideoStateEx state = new NVAPI_GetVideoStateEx();
                state.version = 0x10080;
                state.deviceNum = 0;
                state.settingId = VIDEO_STATE_VIDEOSUPERRESOLUTION;

                if (nvapi.GetVideoStateEx(displayHandle, ref state) == NvAPI_Status.NVAPI_OK)
                {
                    if (state.enabled == 0)
                        state.value = 0;
                }
                else
                    state.value = 0;

                VideoSuperResolutionQualityOrig[displayIdx] = VideoSuperResolutionQuality[displayIdx] = state.value;

                displayIdx++;
            }
        }

        public void WriteVideoSettings()
        {
            foreach (var kvp in VideoSuperResolutionQuality)
            {
                bool keyChanged = !VideoSuperResolutionQualityOrig.ContainsKey(kvp.Key) || VideoSuperResolutionQualityOrig[kvp.Key] != kvp.Value;
                if (!keyChanged)
                    continue;

                IntPtr displayHandle = IntPtr.Zero;
                if (nvapi.EnumNvidiaDisplayHandle(kvp.Key, ref displayHandle) != NvAPI_Status.NVAPI_OK)
                    continue;

                NVAPI_SetVideoStateEx state = new NVAPI_SetVideoStateEx();
                state.version = 0x10040;
                state.deviceNum = 0;
                state.settingId = VIDEO_STATE_VIDEOSUPERRESOLUTION;
                state.enable = kvp.Value != 0 ? 1u : 0u;
                state.value = kvp.Value;

                nvapi.SetVideoStateEx(displayHandle, ref state);
            }
        }

        bool HasVideoChanges()
        {
            foreach (var kvp in VideoSuperResolutionQuality)
            {
                bool keyChanged = !VideoSuperResolutionQualityOrig.ContainsKey(kvp.Key) || VideoSuperResolutionQualityOrig[kvp.Key] != kvp.Value;
                if (keyChanged)
                    return true;
            }
            return false;
        }

        public void Read()
        {
            ReadHudStatus();
            ReadVideoSettings();

            NvAPI_Status res = NvAPI_Status.NVAPI_OK;

            // Always create a new session when we want to read settings
            // Otherwise we could be reading stale data...
            if (hSession != IntPtr.Zero)
            {
                nvapi.DRS_DestroySession(hSession);
                hSession = IntPtr.Zero;
                hProfile = IntPtr.Zero;
            }

            if ((res = nvapi.DRS_CreateSession(ref hSession)) == NvAPI_Status.NVAPI_OK)
                if ((res = nvapi.DRS_LoadSettings(hSession)) == NvAPI_Status.NVAPI_OK)
                    if ((res = nvapi.DRS_DecryptSession(hSession)) == NvAPI_Status.NVAPI_OK)
                        res = nvapi.DRS_GetBaseProfile(hSession, ref hProfile);

            if (res != NvAPI_Status.NVAPI_OK)
                return;

            NVDRS_SETTING setting;

            setting = new NVDRS_SETTING();
            setting.version = nvapi.NVDRS_SETTING_VER;
            if (nvapi.DRS_GetSetting(hSession, hProfile, DRS_NGX_OVERRIDE_RENDER_PRESET_SELECTION, ref setting) == NvAPI_Status.NVAPI_OK)
                OverrideRenderPresetOrig = OverrideRenderPreset = setting.currentValue.dwordValue;

            setting = new NVDRS_SETTING();
            setting.version = nvapi.NVDRS_SETTING_VER;
            if (nvapi.DRS_GetSetting(hSession, hProfile, DRS_NGX_FORCE_DLAA, ref setting) == NvAPI_Status.NVAPI_OK)
                OverrideForceDLAAOrig = OverrideForceDLAA = setting.currentValue.dwordValue != 0;

            setting = new NVDRS_SETTING();
            setting.version = nvapi.NVDRS_SETTING_VER;
            if (nvapi.DRS_GetSetting(hSession, hProfile, DRS_NGX_SCALE_RATIO, ref setting) == NvAPI_Status.NVAPI_OK)
                OverrideScaleRatioOrig = OverrideScaleRatio = BitConverter.ToSingle(BitConverter.GetBytes(setting.currentValue.dwordValue), 0);
        }

        public bool HasChanges()
        {
            return
                (OverrideRenderPreset != OverrideRenderPresetOrig) ||
                (OverrideForceDLAA != OverrideForceDLAAOrig) ||
                (OverrideScaleRatio != OverrideScaleRatioOrig) ||
                (HudStatus != HudStatusOrig) ||
                HasVideoChanges();
        }

        public void Write()
        {
            if (!HasChanges())
                return;

            WriteHudStatus();
            WriteVideoSettings();

            if (hSession == IntPtr.Zero)
            {
                nvapi.DRS_CreateSession(ref hSession);
                nvapi.DRS_LoadSettings(hSession);
                nvapi.DRS_DecryptSession(hSession);
            }

            if (hProfile == IntPtr.Zero)
            {
                nvapi.DRS_GetBaseProfile(hSession, ref hProfile);
            }

            NVDRS_SETTING setting;
            NvAPI_Status res;
            bool changesMade = false;
            if (OverrideRenderPreset != OverrideRenderPresetOrig)
            {
                setting = new NVDRS_SETTING();
                setting.version = nvapi.NVDRS_SETTING_VER;
                setting.settingId = DRS_NGX_OVERRIDE_RENDER_PRESET_SELECTION;
                setting.currentValue.dwordValue = OverrideRenderPreset;
                res = nvapi.DRS_SetSetting(hSession, hProfile, ref setting);
                if (res == NvAPI_Status.NVAPI_OK)
                    changesMade = true;
                else
                {
                    if (res == NvAPI_Status.NVAPI_INVALID_USER_PRIVILEGE)
                        throw new UnauthorizedAccessException();
                    else
                        throw new Exception($"NVAPI Result: {res}");
                }
            }

            if (OverrideForceDLAA != OverrideForceDLAAOrig)
            {
                setting = new NVDRS_SETTING();
                setting.version = nvapi.NVDRS_SETTING_VER;
                setting.settingId = DRS_NGX_FORCE_DLAA;
                setting.currentValue.dwordValue = OverrideForceDLAA ? 1u : 0u;
                res = nvapi.DRS_SetSetting(hSession, hProfile, ref setting);
                if (res == NvAPI_Status.NVAPI_OK)
                    changesMade = true;
                else
                {
                    if (res == NvAPI_Status.NVAPI_INVALID_USER_PRIVILEGE)
                        throw new UnauthorizedAccessException();
                    else
                        throw new Exception($"NVAPI Result: {res}");
                }
            }

            if (OverrideScaleRatio != OverrideScaleRatioOrig)
            {
                setting = new NVDRS_SETTING();
                setting.version = nvapi.NVDRS_SETTING_VER;
                setting.settingId = DRS_NGX_SCALE_RATIO;
                setting.currentValue.dwordValue = BitConverter.ToUInt32(BitConverter.GetBytes(OverrideScaleRatio), 0);
                res = nvapi.DRS_SetSetting(hSession, hProfile, ref setting);
                if (res == NvAPI_Status.NVAPI_OK)
                    changesMade = true;
                else
                {
                    if (res == NvAPI_Status.NVAPI_INVALID_USER_PRIVILEGE)
                        throw new UnauthorizedAccessException();
                    else
                        throw new Exception($"NVAPI Result: {res}");
                }
            }

            if (!changesMade)
                return;

            res = nvapi.DRS_SaveSettings(hSession);
            if (res != NvAPI_Status.NVAPI_OK)
            {
                if (res == NvAPI_Status.NVAPI_INVALID_USER_PRIVILEGE)
                    throw new UnauthorizedAccessException();
                else
                    throw new Exception($"NVAPI Result: {res}");
            }

            // Run Read again to update our Orig values
            Read();
        }


        // Updates DRS settings based on command-line args
        public bool ProcessArgs()
        {
            var args = Environment.GetCommandLineArgs();
            if (args == null || args.Length < 2)
                return false;

            Read();

            var argGlobalForceDLAA = Main.NvidiaGlobalsForceDLAAKey.ToLower();
            var argGlobalForcedScaleKey = Main.NvidiaGlobalsForcedScaleKey.ToLower();
            var argGlobalsForcedPresetKey = Main.NvidiaGlobalsForcedPresetKey.ToLower();
            var argGlobalsHudOverrideKey = Main.NvidiaGlobalsHudOverrideKey.ToLower();
            var argGlobalsVideoSuperResKey = Main.NvidiaGlobalsVideoSuperResKey.ToLower();

            bool hasDrsArg = false;

            for (int i = 1; i < args.Length; i++)
            {
                var arg = args[i];
                if (arg.Length <= 0)
                    continue;

                var isArg = arg[0] == '-' || arg[0] == '/';
                if (!isArg)
                    continue;

                string argValue = "";
                if (i + 1 < args.Length)
                {
                    argValue = args[i + 1];

                    // don't allow any arg-looking strings as the value
                    if (argValue.Length > 0)
                        if (argValue[0] == '-' || argValue[0] == '/')
                            argValue = string.Empty;
                }

                bool hasValue = !string.IsNullOrEmpty(argValue);

                arg = arg.Substring(1).ToLower();

                if (arg == argGlobalForceDLAA)
                {
                    hasDrsArg = true;

                    bool value = true; // if GlobalForceDLAA arg is specified we treat it as enabling, unless user provided a value
                    if (hasValue)
                    {
                        value = Utility.ParseBool(argValue);
                        i++;
                    }

                    OverrideForceDLAA = value;
                }
                else if (arg == argGlobalForcedScaleKey)
                {
                    hasDrsArg = true;
                    if (hasValue)
                    {
                        float value = float.Parse(argValue);
                        OverrideScaleRatio = value;
                        i++;
                    }
                }
                else if (arg == argGlobalsForcedPresetKey)
                {
                    hasDrsArg = true;
                    if (hasValue)
                    {
                        if (argValue.ToLower() == "default" || argValue == "0")
                            OverrideRenderPreset = 0;
                        else
                        {
                            var ch = argValue.ToUpper()[0] - 'A';
                            OverrideRenderPreset = (uint)(ch + 1);
                        }
                        i++;
                    }
                }
                else if (arg == argGlobalsHudOverrideKey)
                {
                    hasDrsArg = true;
                    HudStatus = NvHudStatus.EnabledAllDlls;

                    if (hasValue)
                    {
                        i++;
                        argValue = argValue.ToLower();
                        if (argValue == "0" || argValue == "false" || argValue == "disabled" || argValue == "default")
                            HudStatus = NvHudStatus.Disabled;
                        else if (argValue == "dev")
                            HudStatus = NvHudStatus.Enabled;
                    }
                }
                else if (arg == argGlobalsVideoSuperResKey || arg == "vsr")
                {
                    hasDrsArg = true;
                    VideoSuperResolutionQuality[0] = 4; // if VSR arg is specified we treat it as enabling, unless user provided a value

                    if (hasValue)
                    {
                        i++;
                        argValue = argValue.ToLower();
                        if (argValue == "0" || argValue == "false" || argValue == "disabled" || argValue == "default")
                            VideoSuperResolutionQuality[0] = 0;
                        else
                        {
                            try
                            {
                                VideoSuperResolutionQuality[0] = uint.Parse(argValue);
                            }
                            catch { }
                        }
                    }
                }
            }

            Write();

            return hasDrsArg;
        }
    }
}
