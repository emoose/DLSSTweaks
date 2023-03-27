# DLSSTweaks

Wrapper DLL that can force DLAA onto DLSS-supported titles, along with tweaking scaling ratios & DLSS 3.1 presets.

Most titles that support DLSS2+ should hopefully work fine with this, but if you find any that don't, or have any other issues, feel free to post in the [issue tracker](https://github.com/emoose/DLSSTweaks/issues).

DLSS 3.1 is required for DLSSPresets overrides to be applied, but DLAA forcing / scaling ratio tweaks should work fine across 2.x too - DLSS framegen is unaffected by this DLL.

This has been tested with Nvidia driver series **528** & **531** - earlier driver versions are known to have issues, recommend updating if DLSSTweaks doesn't seem to apply to any games for you.

Check [releases section](https://github.com/emoose/DLSSTweaks/releases) for information about releases.

---

Many hours have gone into developing/testing/rewriting, along with debugging issues with different games.  
If the tweaks have helped improve your experience, please consider buying a coffee to support future development & help to obtain more games to test with. Thank you!

<a href='https://ko-fi.com/emoose' target='_blank'><img src='https://i.imgur.com/I3zDqrO.png' border='0' alt='Buy Me a Coffee at ko-fi.com' />

---

DLSSTweaks now offers two different ways to setup the wrapper, which should help let it work across different system configs:

#### nvngx.dll
The easiest way to install is via nvngx.dll wrapping, this should work fine for the majority of games without needing to rename any files first.

However this method requires a small registry tweak to be applied first to stop DLSS from checking the nvngx.dll signature.

The included `EnableNvidiaSigOverride.reg` can install this tweak for you (the tweak can also be removed via the `DisableNvidiaSigOverride.reg`)

After setting up the registry tweak you should be able to just copy the `nvngx.dll` & `dlsstweaks.ini` files next to your game EXE, and it should hopefully load into the game fine.

(this is the same registry tweak used by [CyberFSR2](https://github.com/PotatoOfDoom/CyberFSR2) to load in their custom nvngx.dll, haven't seen any reports of issues caused by it, but if you play games that use anti-cheat software you may want to use the method below, instead of installing this global registry tweak)

#### dxgi.dll/etc
Alternatively if you don't wish to use the registry tweak, the older methods of loading in via dxgi.dll etc wrapping are also still supported.  
(this older method requires more code hooks to be applied though, which may have issues on certain systems, the nvngx.dll method is believed to be more compatible)

You can switch to these wrappers by renaming the included nvngx.dll file to one of the supported filenames, the [dlsstweaks.ini](https://github.com/emoose/DLSSTweaks/blob/master/dlsstweaks.ini) includes a list of filenames you can try.

---

If the DLL loaded in fine there should be a dlsstweaks.log file created next to the EXE, if that shows up then hopefully the tweaks should be active. 
(you can use the DLSS dev DLL to verify that they're actually active, the INI file has more info about that)

Note that **the default dlsstweaks.ini won't apply any tweaks**, it must be edited first - the included INI explains each of the available tweaks, along with alternate filenames you can rename the DLSSTweaks DLL to.

**NOT RECOMMENDED FOR ONLINE GAMES**  
The way the hook works is similar to how some game cheats modify games, it's very likely to be picked up by most anti-cheats.

**Please don't reupload this DLL elsewhere**, linking to the [releases page](https://github.com/emoose/DLSSTweaks/releases) would be appreciated.

---
### Game Compatibility
A list of games tested against DLSSTweaks can be found here: https://github.com/emoose/DLSSTweaks/wiki/Games

If you try out any game that isn't mentioned there please let us know how it went on the [issue tracker](https://github.com/emoose/DLSSTweaks/issues)!

Many thanks to [DoktorSleepless](https://www.reddit.com/user/DoktorSleepless) & [OrganizationOk4516](https://www.reddit.com/user/OrganizationOk4516) for helping test the DLL with most of these games before release!

---
### Thanks
DLSSTweaks is built on top of several open-source projects, many thanks to the following:

- praydog for the project template (https://github.com/praydog/AutomataMP)
- cursey for safetyhook (https://github.com/cursey/safetyhook)
- Silent for ModUtils (https://github.com/CookiePLMonster/ModUtils)
- PotatoOfDoom for CyberFSR2 & the nvngx.dll loading method/signature override (https://github.com/PotatoOfDoom/CyberFSR2)
- This software contains source code provided by NVIDIA Corporation.
