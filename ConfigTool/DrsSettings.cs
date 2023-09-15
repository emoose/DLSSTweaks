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

        public uint OverrideRenderPreset = 0;
        uint OverrideRenderPresetOrig = 0;
        public bool OverrideForceDLAA = false;
        bool OverrideForceDLAAOrig = false;
        public float OverrideScaleRatio = 0;
        float OverrideScaleRatioOrig = 0;

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
            catch (SecurityException ex)
            {
                throw new UnauthorizedAccessException();
            }
        }

        public void Read()
        {
            ReadHudStatus();

            NvAPI_Status res = NvAPI_Status.NVAPI_OK;

            if (hSession == IntPtr.Zero)
            {
                if ((res = nvapi.DRS_CreateSession(ref hSession)) == NvAPI_Status.NVAPI_OK)
                    if ((res = nvapi.DRS_LoadSettings(hSession)) == NvAPI_Status.NVAPI_OK)
                        res = nvapi.DRS_DecryptSession(hSession);
            }

            if (hProfile == IntPtr.Zero && res == NvAPI_Status.NVAPI_OK)
            {
                res = nvapi.DRS_GetBaseProfile(hSession, ref hProfile);
            }

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
                (HudStatus != HudStatusOrig);
        }

        public void Write()
        {
            if (!HasChanges())
                return;

            WriteHudStatus();

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
        }
    }
}
