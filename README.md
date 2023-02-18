# DLSSTweaks

Wrapper DLL that can force DLAA onto DLSS-supported titles, along with tweaking scaling ratios & DLSS 3.1 presets.

Most titles that support DLSS2+ should hopefully work fine with this, but if you find any that don't, or have any other issues, feel free to post in the [issue tracker](https://github.com/emoose/DLSSTweaks/issues).

DLSS 3.1 is required for DLSSPresets overrides to be applied, but DLAA forcing / scaling ratio tweaks should work fine across 2.x too - DLSS framegen is unaffected by this DLL.

Binaries can be found in the [releases section](https://github.com/emoose/DLSSTweaks/releases).

---

To install just extract dxgi.dll & dlsstweaks.ini next to your game executable, then edit the dlsstweaks.ini file with the tweaks you want to apply.
(note that **the default dlsstweaks.ini won't apply any tweaks**, it must be edited first)

If the DLL loaded in fine there should be a dlsstweaks.log file created next to the EXE, if that shows up then hopefully the tweaks should be active. 
(you can use the DLSS dev DLL to verify that they're actually active, the INI file has more info about that)

The included INI explains each of the tweaks available, along with alternate filenames you can try for the DLL if it doesn't seem to load in properly, or if you want to use it alongside another DLL wrapper such as ReShade.

**NOT RECOMMENDED FOR ONLINE GAMES**  
The way the hook works is similar to how some game cheats modify games, it's very likely to be picked up by most anti-cheats.

**Please don't reupload this DLL elsewhere**, linking to the [releases page](https://github.com/emoose/DLSSTweaks/releases) would be appreciated.

If anyone would like to support future development my ko-fi page is https://ko-fi.com/emoose, any help is appreciated :)

---
### Titles known to hopefully work with it:
- Dying Light 2
- Shadow of the Tomb Raider
- Judgment
- Lost Judgment
- Death Stranding
- Cyberpunk 2077
- Red Dead Redemption 2 (rename to `XInput9_1_0.dll`)
- Plague Tale Requiem
- Chernobylite (UE4)
- Ghostwire Tokyo (UE4)
- God of War
- Uncharted 4
- Horizon Zero Dawn
- Deathloop
- Ready Or Not (UE4)
- HITMAN 3
- Nioh 2 (`OverrideAutoExposure = 1` can be used to fix the graphics issues it has with DLSS3.1)
- FIST: Forged in Shadow Torch (UE4)
- Metro Exodus Enhanced Edition (may need rename to `XInput1_3.dll` or `XInput9_1_0.dll`)
- Assetto Corsa Competizione (UE4)
- Returnal (UE4)

UE4 games usually include two EXEs, one for the main game, and one that simply launches the main game - the DLL usually needs to be placed next to main game EXE (normally found inside a `[gameName]\Binaries\Win64\` folder) - most UE4 titles need the DLL renamed to `XInput1_3.dll` or `XInput9_1_0.dll` to work.

Some titles listed here might have native DLAA support already, but you can still tweak scaling ratios/DLSS3.1 presets for those using DLSSTweaks.

### Titles that may have issues with DLAA:
- Mortal Shell
- Minecraft Bedrock Edition (DLAA causes screen cut in half)
- Crysis 3 Remastered

DLSS3.1 preset overrides may still work fine in the titles above.

Many thanks to [DoktorSleepless](https://www.reddit.com/user/DoktorSleepless) & [OrganizationOk4516](https://www.reddit.com/user/OrganizationOk4516) for helping test the DLL with most of these games before release!

If you try out any game that isn't mentioned here please let us know how it went on the [issue tracker](https://github.com/emoose/DLSSTweaks/issues)!
