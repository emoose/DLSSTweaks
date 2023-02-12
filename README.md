# DLSSTweaks

Issue tracking github for DLSSTweaks, please feel free to post on the [issues page](https://github.com/emoose/DLSSTweaks/issues).

DLSSTweaks binaries can be found in the [releases section](https://github.com/emoose/DLSSTweaks/releases).

Source code is currently unpublished, but will be posted sometime soon.

```
; DLSSTweaks by emoose - https://github.com/emoose/DLSSTweaks
; Wrapper DLL that can force DLAA onto DLSS-supported games, along with tweaking the presets used by them

; The default dxgi.dll filename should hopefully work for the majority of games, otherwise you can try renaming it to any of the following:
; - XInput1_3.dll
; - XInput9_1_0.dll
; - XAPOFX1_5.dll
; - X3DAudio1_7.dll
; - dxgi.dll
; If you have a game that doesn't work with any of these please let me know!

; DLSSTweaks was built for DLSS 3.1.1, but should work on earlier versions too
; The preset overrides will only work on 3.1.1 and newer however

; It's recommended to test the game with the dev DLSS DLL + ngx_driver_onscreenindicator.reg to make sure tweaks here are being applied
; If it works with dev DLL you can then switch to the regular release DLL fine
; dev DLL is available at https://github.com/NVIDIA/DLSS/tree/main/lib/Windows_x86_64/dev
; reg file at https://github.com/NVIDIA/DLSS/blob/main/utils/ngx_driver_onscreenindicator.reg
```
