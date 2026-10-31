# Keep System Tray Visible on Auto-Hide Taskbar

A [Windhawk](https://windhawk.net) mod for Windows 11 and Windows 10. With
"Automatically hide the taskbar" turned on, it keeps the notification area
(tray icons, clock and notification button) on screen while the rest of the
taskbar is hidden. The tray icons stay clickable, and right-click menus,
tooltips and flyouts open next to them as usual.

Moving the mouse to the screen edge still brings the taskbar back, anywhere
except right below the tray, which belongs to the tray.

## Install

1. Install [Windhawk](https://windhawk.net).
2. In Windhawk, choose **Create a New Mod**.
3. Replace the editor's contents with
   [`taskbar-tray-stays-visible.wh.cpp`](taskbar-tray-stays-visible.wh.cpp)
   (or the file attached to the latest
   [release](../../releases/latest)).
4. Click **Compile Mod**, then **Exit Editing Mode**, and turn the mod on.
5. Make sure "Automatically hide the taskbar" is on in Windows' taskbar
   settings.

## Settings

- **Only show date/time**: keep just the clock on screen instead of the whole
  notification area.
- **Only date/time on the shown taskbar too**: with "Only show date/time" on,
  the rest of the tray stays hidden when the taskbar shows as well. Where it
  would be, the taskbar is see-through. With "Edge below the tray shows only
  the tray" on, touching the screen edge below it still shows it. Windows 11
  taskbar only.
- **Offset from the right edge**: the taskbar's content ends this many pixels
  before the right edge, so the tray (or the clock) sits that far from it.
  Windows 11 taskbar only.
- **Overflow button keeps the taskbar hidden**: opening the hidden icons (^)
  while the taskbar is hidden leaves the rest of the taskbar hidden.
- **Edge below the tray shows only the tray**: with "Only show date/time" on,
  touching the screen edge below the tray shows the whole tray instead of the
  whole taskbar. Windows 11 taskbar only.
- **Bottom right corner hides the tray**: pushing the mouse into the bottom
  right corner hides the tray until the mouse moves away, so you can reach
  what's underneath. This also works if the mouse touched the screen edge on
  its way to the corner.
- **Corner delay**: how long, in milliseconds, the mouse has to be away before
  the tray shows again.
- **Shortcut to hide/show the tray**: a shortcut such as `Ctrl+Alt+T` that
  hides or shows the tray while the taskbar is hidden.

The mod's readme in Windhawk (the top of the `.wh.cpp` file) explains how it
works and lists notes on compatibility, including with the Windows Animations
mod.

## Troubleshooting scripts

Both scripts only read state; they don't click, type or change anything.
Run them in PowerShell and paste the output into a bug report.

- [`diagnose-tray.ps1`](diagnose-tray.ps1): what the mod sees of the tray
  (buttons, clock, overflow button) through UI Automation.
- [`diagnose-blink.ps1`](diagnose-blink.ps1): records the taskbar, the tray
  and Windows Animations' effect windows for 20 seconds, for flicker and
  z-order problems.

```
powershell -ExecutionPolicy Bypass -File diagnose-blink.ps1
```
