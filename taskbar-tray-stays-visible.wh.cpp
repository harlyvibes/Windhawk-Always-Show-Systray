// ==WindhawkMod==
// @id              taskbar-tray-stays-visible
// @name            Keep System Tray Visible on Auto-Hide Taskbar
// @description     When the auto-hide taskbar slides away, the notification area (tray icons, clock, notification button) stays on screen. Windows 11 and Windows 10.
// @version         1.19
// @author          Local
// @include         explorer.exe
// @architecture    x86-64
// @compilerOptions -luser32 -lgdi32 -lcomctl32 -lole32 -loleaut32
// ==/WindhawkMod==

// ==WindhawkModReadme==
/*
# Keep System Tray Visible on Auto-Hide Taskbar

With "Automatically hide the taskbar" turned on, this mod keeps the
notification area (tray icons, clock and notification button) on screen while
the rest of the taskbar is hidden. The tray icons stay clickable, and
right-click menus, tooltips and flyouts open next to them as usual.

Moving the mouse to the screen edge still brings the taskbar back, anywhere
except right below the tray, which belongs to the tray.

## Settings

- **Only show date/time**: keep just the clock on screen instead of the whole
  notification area.
- **Only date/time on the shown taskbar too**: with "Only show date/time" on,
  the rest of the tray stays hidden when the taskbar shows as well. Where it
  would be, the taskbar is see-through. With "Edge below the tray shows only
  the tray" on, touching the screen edge below it still shows it. Windows 11
  taskbar only.
- **Offset from the right edge**: the taskbar's content ends this many pixels
  before the right edge, so the tray (or the clock) sits that far from it,
  whether the taskbar is shown or hidden. Nothing is drawn in the gap, so it
  shows what's behind the taskbar. Windows 11 taskbar only.
- **Overflow button keeps the taskbar hidden**: when on, opening the hidden
  icons (^) while the taskbar is hidden leaves the rest of the taskbar hidden.
  Moving the mouse to the screen edge still shows it.
- **Edge below the tray shows only the tray**: with "Only show date/time" on,
  touching the screen edge below the tray shows the whole tray instead of the
  whole taskbar. It goes back to just the clock once the mouse leaves the
  tray. Windows 11 taskbar only.
- **Bottom right corner hides the tray**: pushing the mouse into the bottom
  right corner of the screen hides the tray until the mouse moves away from
  where it was, so you can reach what's underneath. The corner itself then
  belongs to the window underneath: it doesn't bring up the taskbar or click
  "Show desktop" while the taskbar is hidden. If the mouse brought up the
  taskbar by touching the screen edge on its way there, the taskbar hides
  again too.
- **Corner delay**: how long, in milliseconds, the mouse has to be away from
  where the tray was before the tray shows again.
- **Shortcut to hide/show the tray**: a shortcut such as `Ctrl+Alt+T` that
  hides or shows the tray while the taskbar is hidden. With the tray hidden,
  the auto-hide taskbar works as it does without the mod.

## How it works

Explorer still runs its own auto-hide logic, and the mod keeps that logic
working. When Explorer moves the taskbar off screen, the mod leaves the window
in its docked position and sets a window region that clips it down to:

- the thin strip Explorer would normally leave visible at the screen edge, and
- the notification area (or the clock).

Explorer is told that the window moved to the hidden position, so its state
machine is unchanged. Clicks outside the region go to the windows underneath.

On Windows 11 the tray is drawn by XAML inside the taskbar window, but
Explorer still keeps a `TrayNotifyWnd` child window sized to cover it. The mod
uses that window to find the tray area, on both Windows 11 and Windows 10.
Explorer also hides the window hosting the XAML content once the taskbar is
fully hidden. The mod keeps it shown but clipped to the tray, so the mouse on
the edge strip still reaches Explorer and brings the taskbar back. With the
"Edge below the tray" setting, the edge below the tray goes to the XAML
content instead, and the mod widens the clip to the whole tray when the mouse
touches it. The XAML content gets its input without window messages the mod
could watch, so for that setting and the corner setting a timer checks the
mouse position while the taskbar is hidden. The shortcut is registered for a
thread of the mod, which handles it in a message loop of its own.

The Windows 11 clock and overflow button have no windows of their own, so the
mod finds them with UI Automation: the clock is the tray button holding the
`TimeInnerTextBlock` text, and the overflow button the one showing the chevron
glyph (U+E971), since its class name varies between machines and its name is
localized. The offset makes the window hosting the XAML content narrower, so
the content lays itself out again with the tray further left. The taskbar
window keeps its full width, so the edge strip still spans the whole screen
edge.

Opening the hidden icons makes Explorer show the taskbar. With the overflow
setting on, the mod notices the taskbar starting to show while the mouse is
on the overflow button, and keeps it looking hidden until Explorer hides it
again. Explorer considers the taskbar shown meanwhile, so the mod shows it
itself if the mouse goes to the screen edge. The corner setting does the same
when the mouse reaches the corner on a taskbar that Explorer shows because
the mouse touched the edge strip just before.

For "Only date/time on the shown taskbar too", the mod clips the rest of the
tray out of the XAML content. The taskbar window draws nothing of its own
there, so that part is see-through, but it still gets the mouse.

## Notes

- Windows 11 taskbar, Windows 10 taskbar, and the Windows 10 taskbar restored
  on Windows 11 by tools like ExplorerPatcher.
- Only the primary taskbar is affected. Secondary taskbars have no
  notification area (on Windows 11 they can show a clock, which is hidden
  along with the rest of that taskbar).
- The slide animation becomes a wipe: the taskbar is revealed in place rather
  than sliding in.
- Maximized windows use the full screen under an auto-hide taskbar, so the
  tray sits on top of their bottom corner. Explorer takes a maximized window
  without a standard frame (many Electron and Chromium apps) for a full screen
  app and puts the taskbar behind it; the mod keeps the taskbar on top of
  maximized windows. Full screen windows, such as a browser playing a video
  full screen, have no frame at all and still cover it.
- Windows Animations, another Windhawk mod, draws its effects in windows above
  all others. The mod keeps them below the taskbar from their first frame, so
  they don't cover the taskbar or the tray, as with Windows' own minimize
  animation. The effect of a maximized window covers the screen, which
  Explorer takes for a full screen app; the mod keeps the taskbar on top
  meanwhile.
*/
// ==/WindhawkModReadme==

// ==WindhawkModSettings==
/*
- onlyDateTime: false
  $name: Only show date/time
  $description: Keep just the clock on screen while the taskbar is hidden
- clockOnlyWhenShown: false
  $name: Only date/time on the shown taskbar too
  $description: >-
    With "Only show date/time" on, keep the rest of the tray hidden when the
    taskbar shows as well. With "Edge below the tray shows only the tray" on,
    touching the screen edge below it still shows it. Windows 11 taskbar only.
- offset: 0
  $name: Offset from the right edge
  $description: >-
    End the taskbar's content this many pixels before the right edge, which
    moves the tray (or the clock) left, shown or hidden. Pixels at 100% display
    scaling. Windows 11 taskbar only.
- overflowKeepsTaskbarHidden: false
  $name: Overflow button keeps the taskbar hidden
  $description: >-
    When on, opening the hidden icons (^) while the taskbar is hidden leaves
    the rest of the taskbar hidden
- revealTrayAtEdge: false
  $name: Edge below the tray shows only the tray
  $description: >-
    With "Only show date/time" on, touching the screen edge below the tray
    shows the whole tray instead of the whole taskbar. Windows 11 taskbar only.
- cornerHidesTray: false
  $name: Bottom right corner hides the tray
  $description: >-
    Pushing the mouse into the bottom right corner of the screen hides the tray
    until the mouse moves away from where it was, so you can reach what's
    underneath
- cornerDelay: 400
  $name: Corner delay
  $description: >-
    How long the mouse has to be away from where the tray was before the tray
    shows again, in milliseconds
- hotkey: ""
  $name: Shortcut to hide/show the tray
  $description: >-
    For example Ctrl+Alt+T. Hides or shows the tray while the taskbar is
    hidden. Leave empty for none.
*/
// ==/WindhawkModSettings==

#include <windhawk_utils.h>

#include <windows.h>
#include <commctrl.h>
#include <uiautomation.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct {
    bool onlyDateTime;
    bool clockOnlyWhenShown;
    int offset;
    bool overflowKeepsTaskbarHidden;
    bool revealTrayAtEdge;
    bool cornerHidesTray;
    int cornerDelay;
    // The shortcut, for RegisterHotKey. hotkeyVk is 0 for none.
    UINT hotkeyModifiers;
    UINT hotkeyVk;
} g_settings;

// Unless noted, the state below is owned by the taskbar thread. The hooks
// only act on calls made for the taskbar window.
HWND g_taskbar;
DWORD g_taskbarThreadId;
HWND g_tray;

// Windows 11: the window hosting the XAML content, tray included. Explorer
// hides it once the taskbar is fully hidden; the mod keeps it visible, clipped
// to the tray.
HWND g_bridge;
bool g_bridgeHideSuppressed;
bool g_bridgeRegionSet;
RECT g_bridgeRegion;
RECT g_bridgeRegionEdge;
RECT g_bridgeRegionCut;

// The size Explorer gave the XAML host window, and how much narrower the mod
// made it for the offset setting.
SIZE g_bridgeSize;
int g_bridgeTrim;

// The rect Explorer asked for. Differs from the real window rect while the
// taskbar is (partly) hidden, since the real window stays docked.
RECT g_logicalRect;
bool g_offset;
bool g_reportLogicalMove;

// The overflow setting: Explorer shows the taskbar for the hidden icons, but
// the mod keeps it looking hidden, as it was at g_heldLogicalRect. The corner
// setting can hold it too, see g_shownByEdge.
bool g_holdHidden;
bool g_holdForCorner;
RECT g_heldLogicalRect;

// The reveal and corner settings, in screen coordinates. They react to the
// mouse on the XAML content, which gets its input without window messages the
// mod could watch, so a timer polls the cursor meanwhile.
//
// Reveal: touching g_revealEdgeRect (empty when the setting doesn't apply)
// shows the whole tray, g_revealedTrayRect, until the mouse leaves it for
// kWatchTicks timer ticks.
RECT g_revealEdgeRect;
RECT g_revealedTrayRect;
bool g_trayRevealed;
int g_revealOutsideTicks;

// Corner: touching g_cornerRect (empty when the setting doesn't apply) hides
// the tray until the mouse leaves g_cornerHoldRect for the corner delay.
RECT g_cornerRect;
RECT g_cornerHoldRect;
bool g_trayHiddenByCorner;
int g_cornerOutsideTicks;

// The corner also applies while Explorer shows the taskbar because the mouse
// touched its edge strip (at g_edgeTouchTime), e.g. on the way along the edge
// to the corner: g_shownByEdge, until the mouse leaves the taskbar or
// Explorer hides it. Reaching the corner then holds the taskbar looking
// hidden at g_hiddenLogicalRect, where Explorer last hid it.
ULONGLONG g_edgeTouchTime;
bool g_shownByEdge;
RECT g_hiddenLogicalRect;
// Explorer shows the taskbar about 100 ms after the touch.
constexpr ULONGLONG kEdgeTouchMs = 500;

bool g_watchTimerRunning;
constexpr UINT kWatchTimerMs = 100;
constexpr int kWatchTicks = 4;

// The watch timer's id. Unique to this copy of the mod, since Windhawk might
// briefly run the previous version alongside it while updating the mod. Also
// marks this copy's kRefreshUnload request.
UINT_PTR WatchTimerId() {
    return reinterpret_cast<UINT_PTR>(&g_watchTimerRunning);
}

// The corner's size at 100% display scaling.
constexpr int kCornerSize = 3;

// The shortcut setting hides the tray until it's pressed again.
bool g_trayHiddenByUser;

// The region the mod last applied, in window coordinates.
bool g_regionSet;
RECT g_regionVisible;
RECT g_regionKeep;
RECT g_regionExclude;
bool g_applyingRegion;
bool g_regionUpdatePending;

// Posted to the taskbar to update the region. wParam: kRefresh* flags.
UINT g_refreshMsg;
bool g_refreshPosted;

// Also run the current position through WM_WINDOWPOSCHANGING.
constexpr WPARAM kRefreshPosition = 1;
// The settings changed, or the mod just attached: also find the tray windows
// and ask the worker thread for fresh tray button rects.
constexpr WPARAM kRefreshSettings = 2;
// Sent by Wh_ModUninit, with lParam WatchTimerId(): clean up what only the
// taskbar thread can.
constexpr WPARAM kRefreshUnload = 4;
// Posted by the worker thread: the shortcut was pressed.
constexpr WPARAM kRefreshToggleTray = 8;
// Sent by the effect watcher thread, alone, with lParam an effect window that
// was just created or shown: see OnEffectWindowEvent.
constexpr WPARAM kRefreshEffect = 16;

// A worker thread does what can't be done on the taskbar thread:
// - Windows 11: finding the clock and the overflow button, which have no
//   windows, with UI Automation, which mustn't run on the taskbar thread.
// - The shortcut, registered for this thread so that the mod handles it in a
//   message loop of its own rather than through Explorer's.
// The state below is shared with it under g_workerLock.
HANDLE g_workerThread;
HANDLE g_workerWakeEvent;
HANDLE g_workerStopEvent;
SRWLOCK g_workerLock = SRWLOCK_INIT;
HWND g_workerTaskbar;
// Null when no rect is needed.
HWND g_uiaBridge;
bool g_uiaWantClock;
bool g_uiaWantOverflow;
// Refresh the rects every few seconds while they're in use.
bool g_uiaPoll;
// Relative to the XAML host window.
bool g_clockRectValid;
RECT g_clockRect;
bool g_overflowRectValid;
RECT g_overflowRect;

// The shortcut's id, an atom as Microsoft advises for code in DLLs.
ATOM g_hotkeyId;

// Watches for the effect windows of Windows Animations, see
// OnEffectWindowEvent.
HANDLE g_effectWatcherThread;
DWORD g_effectWatcherThreadId;

using GetWindowRect_t = decltype(&GetWindowRect);
GetWindowRect_t GetWindowRect_Original;

using SetWindowPos_t = decltype(&SetWindowPos);
SetWindowPos_t SetWindowPos_Original;

using SetWindowRgn_t = decltype(&SetWindowRgn);
SetWindowRgn_t SetWindowRgn_Original;

using CreateWindowExW_t = decltype(&CreateWindowExW);
CreateWindowExW_t CreateWindowExW_Original;

LRESULT CALLBACK TraySubclassProc(HWND hWnd,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam,
                                  DWORD_PTR dwRefData);

LRESULT CALLBACK BridgeSubclassProc(HWND hWnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    DWORD_PTR dwRefData);

// Returns the rect the taskbar has when shown: rc moved back inside its
// monitor, along the axes on which it fits.
RECT GetDockedRect(const RECT& rc, const RECT& actual) {
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);

    // Prefer the monitor the taskbar is on now, so that a taskbar hidden
    // towards a neighboring monitor isn't docked onto that monitor.
    RECT intersection;
    HMONITOR monitor = MonitorFromRect(&actual, MONITOR_DEFAULTTONULL);
    if (!monitor || !GetMonitorInfo(monitor, &mi) ||
        !IntersectRect(&intersection, &mi.rcMonitor, &rc)) {
        monitor = MonitorFromRect(&rc, MONITOR_DEFAULTTONEAREST);
        if (!GetMonitorInfo(monitor, &mi)) {
            return rc;
        }
    }

    const RECT& mon = mi.rcMonitor;
    int dx = 0;
    int dy = 0;

    if (rc.right - rc.left <= mon.right - mon.left) {
        if (rc.left < mon.left) {
            dx = mon.left - rc.left;
        } else if (rc.right > mon.right) {
            dx = mon.right - rc.right;
        }
    }

    if (rc.bottom - rc.top <= mon.bottom - mon.top) {
        if (rc.top < mon.top) {
            dy = mon.top - rc.top;
        } else if (rc.bottom > mon.bottom) {
            dy = mon.bottom - rc.bottom;
        }
    }

    RECT docked = rc;
    OffsetRect(&docked, dx, dy);
    return docked;
}

// The area of rc that's on screen, given the rect the taskbar has when shown.
int VisibleArea(const RECT& rc, const RECT& docked) {
    RECT visible;
    if (!IntersectRect(&visible, &rc, &docked)) {
        return 0;
    }

    return (visible.right - visible.left) * (visible.bottom - visible.top);
}

// Whether inserting the taskbar after insertAfter takes it out of the topmost
// windows.
bool LosesTopmost(HWND insertAfter) {
    if (insertAfter == HWND_TOPMOST || insertAfter == HWND_TOP) {
        return false;
    }

    if (insertAfter == HWND_NOTOPMOST || insertAfter == HWND_BOTTOM) {
        return true;
    }

    return !(GetWindowLongPtr(insertAfter, GWL_EXSTYLE) & WS_EX_TOPMOST);
}

// On Windows 10 TrayNotifyWnd draws the tray. On Windows 11 the tray is XAML
// content of the taskbar window, and TrayNotifyWnd is a transparent window
// that Explorer keeps over it, so its rect is still the tray's rect.
HWND FindTray(HWND taskbar) {
    HWND tray = FindWindowEx(taskbar, nullptr, L"TrayNotifyWnd", nullptr);
    if (tray != g_tray) {
        if (g_tray) {
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_tray,
                                                             TraySubclassProc);
        }

        g_tray = nullptr;
        if (tray && WindhawkUtils::SetWindowSubclassFromAnyThread(
                        tray, TraySubclassProc, 0)) {
            g_tray = tray;
        }
    }

    return tray;
}

// Makes the XAML content trim pixels narrower, for the offset setting. The
// content lays out again, so the tray ends that far from the right edge. The
// window hosting it is a child of the taskbar, so the taskbar's edge strip
// keeps its full width.
void TrimBridge(HWND bridge, int trim) {
    if (trim == g_bridgeTrim) {
        return;
    }

    // BridgeSubclassProc applies the trim.
    g_bridgeTrim = trim;
    SetWindowPos(bridge, nullptr, 0, 0, g_bridgeSize.cx, g_bridgeSize.cy,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                     SWP_NOACTIVATE);
}

// Returns null on Windows 10, which has no XAML content.
HWND FindBridge(HWND taskbar) {
    HWND bridge = FindWindowEx(taskbar, nullptr,
                               L"Windows.UI.Composition.DesktopWindowContentBridge",
                               nullptr);
    if (bridge != g_bridge) {
        if (g_bridge) {
            if (g_bridgeRegionSet) {
                SetWindowRgn_Original(g_bridge, nullptr, TRUE);
            }
            TrimBridge(g_bridge, 0);
            WindhawkUtils::RemoveWindowSubclassFromAnyThread(
                g_bridge, BridgeSubclassProc);
        }

        g_bridge = nullptr;
        g_bridgeHideSuppressed = false;
        g_bridgeRegionSet = false;
        g_bridgeTrim = 0;

        if (bridge) {
            RECT bridgeRect;
            GetWindowRect_Original(bridge, &bridgeRect);
            g_bridgeSize = {bridgeRect.right - bridgeRect.left,
                            bridgeRect.bottom - bridgeRect.top};
        }

        if (bridge && WindhawkUtils::SetWindowSubclassFromAnyThread(
                          bridge, BridgeSubclassProc, 0)) {
            g_bridge = bridge;
        }
    }

    return bridge;
}

// For ClipBridge: all of the XAML content.
constexpr RECT kEverywhere = {-0x8000, -0x8000, 0x7FFF, 0x7FFF};

// While Explorer has the XAML content hidden, the mouse on the edge strip must
// reach the taskbar window, as it does when the content is really hidden:
// that's how Explorer notices the mouse at the screen edge. So the content is
// clipped to the tray, plus edgeRc for the reveal setting. cutRc is cut out
// of it, for "Only date/time on the shown taskbar too". In screen
// coordinates; edgeRc and cutRc can be empty, and null rc removes the clip.
void ClipBridge(HWND bridge,
                const RECT* rc,
                const RECT* edgeRc,
                const RECT* cutRc) {
    if (!rc) {
        if (g_bridgeRegionSet) {
            g_bridgeRegionSet = false;
            SetWindowRgn_Original(bridge, nullptr, TRUE);
        }
        return;
    }

    RECT bridgeRect;
    GetWindowRect_Original(bridge, &bridgeRect);

    RECT clip = *rc;
    OffsetRect(&clip, -bridgeRect.left, -bridgeRect.top);

    RECT edge{};
    if (edgeRc && !IsRectEmpty(edgeRc)) {
        edge = *edgeRc;
        OffsetRect(&edge, -bridgeRect.left, -bridgeRect.top);
    }

    RECT cut{};
    if (cutRc && !IsRectEmpty(cutRc)) {
        cut = *cutRc;
        OffsetRect(&cut, -bridgeRect.left, -bridgeRect.top);
    }

    if (g_bridgeRegionSet && EqualRect(&clip, &g_bridgeRegion) &&
        EqualRect(&edge, &g_bridgeRegionEdge) &&
        EqualRect(&cut, &g_bridgeRegionCut)) {
        return;
    }

    HRGN region = CreateRectRgnIndirect(&clip);
    HRGN edgeRegion = CreateRectRgnIndirect(&edge);
    CombineRgn(region, region, edgeRegion, RGN_OR);
    DeleteObject(edgeRegion);
    if (!IsRectEmpty(&cut)) {
        HRGN cutRegion = CreateRectRgnIndirect(&cut);
        CombineRgn(region, region, cutRegion, RGN_DIFF);
        DeleteObject(cutRegion);
    }

    g_bridgeRegionSet = true;
    g_bridgeRegion = clip;
    g_bridgeRegionEdge = edge;
    g_bridgeRegionCut = cut;

    // The system owns the region after this call.
    if (!SetWindowRgn_Original(bridge, region, TRUE)) {
        DeleteObject(region);
        g_bridgeRegionSet = false;
    }
}

struct TrayButtons {
    ComPtr<IUIAutomationElement> clock;
    ComPtr<IUIAutomationElement> overflow;
};

// Whether any of a tray button's text blocks matches. They are only in the raw
// view.
template <typename Predicate>
bool AnyRawChild(IUIAutomationTreeWalker* walker,
                 IUIAutomationElement* button,
                 Predicate predicate) {
    ComPtr<IUIAutomationElement> child;
    walker->GetFirstChildElement(button, &child);
    while (child) {
        if (predicate(child.Get())) {
            return true;
        }

        ComPtr<IUIAutomationElement> next;
        walker->GetNextSiblingElement(child.Get(), &next);
        child = next;
    }

    return false;
}

// The clock holds the time, or the date when the time is hidden.
bool IsClockButton(IUIAutomationTreeWalker* walker,
                   IUIAutomationElement* button) {
    return AnyRawChild(walker, button, [](IUIAutomationElement* child) {
        BSTR childId = nullptr;
        bool isClock = SUCCEEDED(child->get_CurrentAutomationId(&childId)) &&
                       childId &&
                       (wcscmp(childId, L"TimeInnerTextBlock") == 0 ||
                        wcscmp(childId, L"DateInnerTextBlock") == 0);
        SysFreeString(childId);
        return isClock;
    });
}

// The overflow ("Show hidden icons") button shows the chevron glyph U+E971,
// also while the overflow is open. Its name is localized, and its class
// differs between machines: SystemTray.NormalButton, or
// SystemTray.AccentButton like the other buttons.
bool IsOverflowButton(IUIAutomationTreeWalker* walker,
                      IUIAutomationElement* button) {
    return AnyRawChild(walker, button, [](IUIAutomationElement* child) {
        BSTR name = nullptr;
        bool isChevron = SUCCEEDED(child->get_CurrentName(&name)) && name &&
                         wcscmp(name, L"") == 0;
        SysFreeString(name);
        return isChevron;
    });
}

// Finds the clock and the overflow ("Show hidden icons") button among the
// system tray buttons.
TrayButtons FindTrayButtons(IUIAutomation* automation, HWND taskbar) {
    TrayButtons found;

    ComPtr<IUIAutomationElement> root;
    if (FAILED(automation->ElementFromHandle(taskbar, &root)) || !root) {
        return found;
    }

    VARIANT id{};
    id.vt = VT_BSTR;
    id.bstrVal = SysAllocString(L"SystemTrayIcon");
    ComPtr<IUIAutomationCondition> condition;
    HRESULT hr = automation->CreatePropertyCondition(UIA_AutomationIdPropertyId,
                                                     id, &condition);
    VariantClear(&id);
    if (FAILED(hr)) {
        return found;
    }

    ComPtr<IUIAutomationElementArray> buttons;
    ComPtr<IUIAutomationTreeWalker> walker;
    int count = 0;
    if (FAILED(root->FindAll(TreeScope_Descendants, condition.Get(),
                             &buttons)) ||
        !buttons || FAILED(buttons->get_Length(&count)) ||
        FAILED(automation->get_RawViewWalker(&walker))) {
        return found;
    }

    for (int i = 0; i < count; i++) {
        ComPtr<IUIAutomationElement> button;
        if (FAILED(buttons->GetElement(i, &button)) || !button) {
            continue;
        }

        if (!found.overflow && IsOverflowButton(walker.Get(), button.Get())) {
            found.overflow = button;
        } else if (!found.clock && IsClockButton(walker.Get(), button.Get())) {
            found.clock = button;
        }
    }

    return found;
}

// Gets a tray button's rect relative to the XAML host window.
bool GetButtonRect(IUIAutomationElement* button, HWND bridge, RECT* rc) {
    for (int i = 0; i < 3; i++) {
        // The taskbar thread can resize the XAML host window meanwhile (see
        // TrimBridge); retry if it did.
        RECT before;
        RECT after;
        RECT bounds;
        GetWindowRect_Original(bridge, &before);
        HRESULT hr = button->get_CurrentBoundingRectangle(&bounds);
        GetWindowRect_Original(bridge, &after);
        if (FAILED(hr) || IsRectEmpty(&bounds)) {
            return false;
        }

        if (EqualRect(&before, &after)) {
            *rc = bounds;
            OffsetRect(rc, -before.left, -before.top);
            return true;
        }
    }

    return false;
}

DWORD WINAPI WorkerThreadProc(LPVOID) {
    // Physical pixels, like the taskbar windows.
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    bool comInitialized =
        SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    ComPtr<IUIAutomation> automation;
    TrayButtons buttons;

    // The shortcut, registered for this thread.
    UINT hotkeyModifiers = 0;
    UINT hotkeyVk = 0;
    bool hotkeyRegistered = false;

    HANDLE events[] = {g_workerStopEvent, g_workerWakeEvent};
    bool lookAgain = false;
    while (true) {
        // The shortcut setting changed, or registering it failed, e.g. while
        // the previous version of the mod still had it. Retried below.
        bool hotkeyChanged = g_settings.hotkeyModifiers != hotkeyModifiers ||
                             g_settings.hotkeyVk != hotkeyVk;
        if (hotkeyChanged || (hotkeyVk && !hotkeyRegistered)) {
            if (hotkeyRegistered) {
                UnregisterHotKey(nullptr, g_hotkeyId);
            }

            hotkeyModifiers = g_settings.hotkeyModifiers;
            hotkeyVk = g_settings.hotkeyVk;
            hotkeyRegistered =
                hotkeyVk && g_hotkeyId &&
                RegisterHotKey(nullptr, g_hotkeyId,
                               hotkeyModifiers | MOD_NOREPEAT, hotkeyVk);
            if (hotkeyVk && !hotkeyRegistered && hotkeyChanged) {
                // E.g. another program has this shortcut.
                Wh_Log(L"Can't register the shortcut, error %u",
                       GetLastError());
            }
        }

        AcquireSRWLockShared(&g_workerLock);
        bool poll = g_uiaPoll;
        ReleaseSRWLockShared(&g_workerLock);

        // After a request, look again shortly: the XAML layout may not have
        // caught up yet, e.g. right after TrimBridge.
        DWORD timeout = lookAgain ? 500 : poll ? 2000 : INFINITE;
        if (hotkeyVk && !hotkeyRegistered && timeout > 2000) {
            timeout = 2000;
        }

        DWORD wait = MsgWaitForMultipleObjects(ARRAYSIZE(events), events,
                                               FALSE, timeout, QS_ALLINPUT);
        if (wait == WAIT_OBJECT_0 + ARRAYSIZE(events)) {
            MSG msg;
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_HOTKEY && msg.wParam == g_hotkeyId) {
                    AcquireSRWLockShared(&g_workerLock);
                    HWND taskbar = g_workerTaskbar;
                    ReleaseSRWLockShared(&g_workerLock);
                    if (taskbar) {
                        PostMessage(taskbar, g_refreshMsg, kRefreshToggleTray,
                                    0);
                    }
                }
            }
            continue;
        }

        if (wait != WAIT_OBJECT_0 + 1 && wait != WAIT_TIMEOUT) {
            break;
        }

        lookAgain = wait == WAIT_OBJECT_0 + 1;

        AcquireSRWLockShared(&g_workerLock);
        HWND taskbar = g_workerTaskbar;
        HWND bridge = g_uiaBridge;
        bool wantClock = g_uiaWantClock;
        bool wantOverflow = g_uiaWantOverflow;
        ReleaseSRWLockShared(&g_workerLock);

        RECT clockRect{};
        RECT overflowRect{};
        bool clockValid = false;
        bool overflowValid = false;
        if (bridge && comInitialized && !automation) {
            CoCreateInstance(__uuidof(CUIAutomation), nullptr,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS(automation.GetAddressOf()));
        }

        for (int i = 0; bridge && automation && i < 2; i++) {
            if (!buttons.clock && !buttons.overflow) {
                buttons = FindTrayButtons(automation.Get(), taskbar);
            }

            clockValid = wantClock && buttons.clock &&
                         GetButtonRect(buttons.clock.Get(), bridge, &clockRect);
            overflowValid =
                wantOverflow && buttons.overflow &&
                GetButtonRect(buttons.overflow.Get(), bridge, &overflowRect);
            if (clockValid == wantClock && overflowValid == wantOverflow) {
                break;
            }

            // A button is missing or gone, e.g. the tray was rebuilt. Find
            // them again.
            buttons = {};
        }

        AcquireSRWLockExclusive(&g_workerLock);
        bool clockChanged = clockValid != g_clockRectValid ||
                            (clockValid && !EqualRect(&clockRect, &g_clockRect));
        g_clockRectValid = clockValid;
        g_clockRect = clockRect;
        g_overflowRectValid = overflowValid;
        g_overflowRect = overflowRect;
        ReleaseSRWLockExclusive(&g_workerLock);

        // Only the clock rect affects the region.
        if (clockChanged && taskbar) {
            PostMessage(taskbar, g_refreshMsg, 0, 0);
        }
    }

    if (hotkeyRegistered) {
        UnregisterHotKey(nullptr, g_hotkeyId);
    }

    buttons = {};
    automation = nullptr;
    if (comInitialized) {
        CoUninitialize();
    }

    return 0;
}

// Tells the worker thread which tray button rects are needed. wake:
// also ask for fresh rects now.
void UpdateUiaRequest(bool wake) {
    // On Windows 10 the buttons are windows, see GetClockRect and
    // IsCursorOnOverflowButton.
    bool wantClock = g_bridge && g_settings.onlyDateTime;
    bool wantOverflow = g_bridge && g_settings.overflowKeepsTaskbarHidden;
    HWND bridge = wantClock || wantOverflow ? g_bridge : nullptr;
    bool poll = bridge && (g_offset || g_holdHidden ||
                           (wantClock && g_settings.clockOnlyWhenShown));

    AcquireSRWLockExclusive(&g_workerLock);
    bool changed = bridge != g_uiaBridge || wantClock != g_uiaWantClock ||
                   wantOverflow != g_uiaWantOverflow || poll != g_uiaPoll;
    g_workerTaskbar = g_taskbar;
    g_uiaBridge = bridge;
    g_uiaWantClock = wantClock;
    g_uiaWantOverflow = wantOverflow;
    g_uiaPoll = poll;
    ReleaseSRWLockExclusive(&g_workerLock);

    if (bridge && (wake || changed)) {
        SetEvent(g_workerWakeEvent);
    }
}

// The tray's rect in window coordinates. contentRight: see ApplyRegion.
RECT GetTrayRect(HWND tray, const RECT& actual, int contentRight) {
    RECT rc{};
    if (tray && IsWindowVisible(tray)) {
        GetWindowRect_Original(tray, &rc);
        OffsetRect(&rc, -actual.left, -actual.top);

        // TrayNotifyWnd stays at the end of the taskbar when the XAML content
        // is trimmed.
        if (rc.right > contentRight) {
            OffsetRect(&rc, contentRight - rc.right, 0);
        }
    }

    return rc;
}

// The clock's rect in window coordinates, for "Only show date/time".
bool GetClockRect(HWND tray, HWND bridge, const RECT& actual, RECT* rc) {
    if (bridge) {
        AcquireSRWLockShared(&g_workerLock);
        bool valid = g_clockRectValid;
        RECT clock = g_clockRect;
        ReleaseSRWLockShared(&g_workerLock);
        if (!valid) {
            return false;
        }

        RECT bridgeRect;
        GetWindowRect_Original(bridge, &bridgeRect);
        OffsetRect(&clock, bridgeRect.left - actual.left,
                   bridgeRect.top - actual.top);
        *rc = clock;
        return true;
    }

    // Windows 10: the clock is a window in the tray.
    HWND clock =
        tray ? FindWindowEx(tray, nullptr, L"TrayClockWClass", nullptr)
             : nullptr;
    if (!clock || !IsWindowVisible(clock)) {
        return false;
    }

    GetWindowRect_Original(clock, rc);
    OffsetRect(rc, -actual.left, -actual.top);
    return true;
}

bool IsCursorOnOverflowButton(HWND tray, HWND bridge) {
    POINT pt;
    if (!GetCursorPos(&pt)) {
        return false;
    }

    RECT rc;
    if (bridge) {
        AcquireSRWLockShared(&g_workerLock);
        bool valid = g_overflowRectValid;
        rc = g_overflowRect;
        ReleaseSRWLockShared(&g_workerLock);
        if (!valid) {
            return false;
        }

        RECT bridgeRect;
        GetWindowRect_Original(bridge, &bridgeRect);
        OffsetRect(&rc, bridgeRect.left, bridgeRect.top);
    } else {
        // Windows 10: the button is a window in the tray.
        HWND button =
            tray ? FindWindowEx(tray, nullptr, L"Button", nullptr) : nullptr;
        if (!button || !IsWindowVisible(button)) {
            return false;
        }

        GetWindowRect_Original(button, &rc);
    }

    return PtInRect(&rc, pt);
}

// Runs the watch timer while it has something to watch.
void UpdateWatchTimer(HWND hWnd) {
    bool needed = g_trayRevealed || g_trayHiddenByCorner ||
                  !IsRectEmpty(&g_revealEdgeRect) || !IsRectEmpty(&g_cornerRect);
    if (needed == g_watchTimerRunning) {
        return;
    }

    g_watchTimerRunning = needed;
    if (needed) {
        SetTimer(hWnd, WatchTimerId(), kWatchTimerMs, nullptr);
    } else {
        KillTimer(hWnd, WatchTimerId());
    }
}

// Parses a shortcut like "Ctrl+Alt+T" for RegisterHotKey: modifiers, then
// one key (a letter, a digit, F1-F24 or one of kNamedKeys).
bool ParseHotkey(PCWSTR text, UINT* modifiers, UINT* vk) {
    static const struct {
        PCWSTR name;
        UINT vk;
    } kNamedKeys[] = {
        {L"Space", VK_SPACE},     {L"Enter", VK_RETURN},  {L"Tab", VK_TAB},
        {L"Esc", VK_ESCAPE},      {L"Insert", VK_INSERT}, {L"Delete", VK_DELETE},
        {L"Home", VK_HOME},       {L"End", VK_END},       {L"PageUp", VK_PRIOR},
        {L"PageDown", VK_NEXT},   {L"Up", VK_UP},         {L"Down", VK_DOWN},
        {L"Left", VK_LEFT},       {L"Right", VK_RIGHT},   {L"Pause", VK_PAUSE},
    };

    *modifiers = 0;
    *vk = 0;

    PCWSTR p = text;
    while (*p) {
        // The next part, without the spaces around it.
        while (*p == L' ') {
            p++;
        }
        PCWSTR end = wcschr(p, L'+');
        if (!end) {
            end = p + wcslen(p);
        }
        size_t length = end - p;
        while (length && p[length - 1] == L' ') {
            length--;
        }

        WCHAR part[16];
        if (!length || length >= ARRAYSIZE(part)) {
            return false;
        }
        wmemcpy(part, p, length);
        part[length] = L'\0';
        p = *end ? end + 1 : end;

        if (_wcsicmp(part, L"Ctrl") == 0 || _wcsicmp(part, L"Control") == 0) {
            *modifiers |= MOD_CONTROL;
            continue;
        }
        if (_wcsicmp(part, L"Alt") == 0) {
            *modifiers |= MOD_ALT;
            continue;
        }
        if (_wcsicmp(part, L"Shift") == 0) {
            *modifiers |= MOD_SHIFT;
            continue;
        }
        if (_wcsicmp(part, L"Win") == 0) {
            *modifiers |= MOD_WIN;
            continue;
        }

        UINT key = 0;
        WCHAR c = towupper(part[0]);
        if (length == 1 && ((c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9'))) {
            // These virtual keys are the characters themselves.
            key = c;
        } else if (c == L'F') {
            PWSTR numberEnd;
            long number = wcstol(part + 1, &numberEnd, 10);
            if (numberEnd != part + 1 && !*numberEnd && number >= 1 &&
                number <= 24) {
                key = VK_F1 + number - 1;
            }
        }

        for (const auto& namedKey : kNamedKeys) {
            if (!key && _wcsicmp(part, namedKey.name) == 0) {
                key = namedKey.vk;
            }
        }

        // Unknown, or a second key.
        if (!key || *vk) {
            return false;
        }
        *vk = key;
    }

    return *vk != 0;
}


// Whether g_hiddenLogicalRect is where Explorer hides the taskbar as it is
// now, e.g. not from before it moved to another monitor.
bool IsHiddenRectCurrent(const RECT& actual) {
    if (IsRectEmpty(&g_hiddenLogicalRect)) {
        return false;
    }

    RECT docked = GetDockedRect(g_hiddenLogicalRect, actual);
    return EqualRect(&docked, &actual);
}

void ApplyRegion(HWND hWnd) {
    // The setting the hold is for was turned off.
    if (g_holdHidden && !(g_holdForCorner
                              ? g_settings.cornerHidesTray
                              : g_settings.overflowKeepsTaskbarHidden)) {
        g_holdHidden = false;
    }

    bool looksHidden = g_offset || g_holdHidden;

    UpdateUiaRequest(false);

    RECT actual;
    GetWindowRect_Original(hWnd, &actual);

    RECT window = actual;
    OffsetRect(&window, -actual.left, -actual.top);

    HWND tray = FindTray(hWnd);
    HWND bridge = FindBridge(hWnd);

    // "Only date/time on the shown taskbar too" clips the XAML content.
    bool clockOnlyWhenShown =
        bridge && g_settings.onlyDateTime && g_settings.clockOnlyWhenShown;

    // The whole taskbar shows anyway, the shortcut hid the tray, or the
    // settings no longer apply.
    if (!looksHidden || g_trayHiddenByUser) {
        g_trayHiddenByCorner = false;
    }
    if ((!looksHidden && !clockOnlyWhenShown) || g_trayHiddenByUser ||
        !g_settings.onlyDateTime || !g_settings.revealTrayAtEdge) {
        g_trayRevealed = false;
    }
    if (!g_settings.cornerHidesTray) {
        g_trayHiddenByCorner = false;
    }

    // The XAML content's right end, shortened for the offset setting (see
    // TrimBridge), in window coordinates.
    int contentRight = window.right;
    if (bridge) {
        int trim = 0;
        if (g_settings.offset > 0) {
            trim = MulDiv(g_settings.offset, GetDpiForWindow(hWnd), 96);
            // Leave the taskbar content some room.
            if (trim > g_bridgeSize.cx / 2) {
                trim = g_bridgeSize.cx / 2;
            }
        }

        TrimBridge(bridge, trim);

        RECT bridgeRect;
        GetWindowRect_Original(bridge, &bridgeRect);
        contentRight = bridgeRect.right - actual.left;
    }

    SetRectEmpty(&g_revealEdgeRect);
    SetRectEmpty(&g_cornerRect);

    RECT trayRect = GetTrayRect(tray, actual, contentRight);

    // The part of the tray the mod keeps on screen: all of it, or just the
    // clock.
    bool clockOnly = g_settings.onlyDateTime && !g_trayRevealed;
    RECT clockRect{};
    bool clockKnown =
        clockOnly && GetClockRect(tray, bridge, actual, &clockRect);

    // For "Only date/time on the shown taskbar too": the rest of the tray, in
    // screen coordinates. It's cut out of the XAML content while more of that
    // than the tray shows.
    RECT restOfTray{};
    if (clockOnlyWhenShown && clockKnown && clockRect.left > trayRect.left) {
        restOfTray = trayRect;
        restOfTray.right = clockRect.left;
        OffsetRect(&restOfTray, actual.left, actual.top);
    }

    RECT visible;
    RECT keep;
    // Cut out of the region, so the mouse there reaches the windows
    // underneath.
    RECT exclude{};
    if (!looksHidden) {
        if (bridge) {
            ClipBridge(bridge,
                       IsRectEmpty(&restOfTray) ? nullptr : &kEverywhere,
                       nullptr, &restOfTray);
        }

        // The reveal setting, for the rest of the tray: the edge below it is
        // the strip Explorer leaves on screen when it hides the taskbar.
        RECT strip;
        if (clockOnlyWhenShown && g_settings.revealTrayAtEdge &&
            !g_trayRevealed && IsHiddenRectCurrent(actual) &&
            IntersectRect(&strip, &g_hiddenLogicalRect, &actual)) {
            RECT trayScreenRect = trayRect;
            OffsetRect(&trayScreenRect, actual.left, actual.top);
            IntersectRect(&g_revealEdgeRect, &trayScreenRect, &strip);
        }
        g_revealedTrayRect = trayRect;
        OffsetRect(&g_revealedTrayRect, actual.left, actual.top);

        // The corner setting, on a taskbar that the mouse brought up on its
        // way to the corner, see g_shownByEdge.
        if (g_shownByEdge && g_settings.cornerHidesTray &&
            !g_trayHiddenByUser) {
            int size = MulDiv(kCornerSize, GetDpiForWindow(hWnd), 96);
            g_cornerRect = {actual.right - size, actual.bottom - size,
                            actual.right, actual.bottom};
            g_cornerHoldRect = g_cornerRect;
        }

        UpdateWatchTimer(hWnd);

        // Nothing is drawn past the trimmed content, so let clicks through
        // there too.
        visible = window;
        if (contentRight < visible.right) {
            visible.right = contentRight;
        }

        if (EqualRect(&visible, &window)) {
            if (g_regionSet) {
                g_regionSet = false;
                SetWindowRgn_Original(hWnd, nullptr, TRUE);
            }
            return;
        }

        SetRectEmpty(&keep);
    } else {
        // The part Explorer means to keep on screen: the edge strip, or more
        // while sliding.
        const RECT& logical = g_holdHidden ? g_heldLogicalRect : g_logicalRect;
        if (IntersectRect(&visible, &logical, &actual)) {
            OffsetRect(&visible, -actual.left, -actual.top);
        } else {
            SetRectEmpty(&visible);
        }

        keep = clockKnown ? clockRect : trayRect;

        // Windows 11 lays out the XAML tray and TrayNotifyWnd separately, so
        // the latter can briefly extend past the taskbar.
        if (!IntersectRect(&keep, &keep, &window)) {
            SetRectEmpty(&keep);
        }

        // The corner setting: the corner goes to the window underneath, so
        // reaching it neither shows the taskbar nor clicks the tray. While
        // the tray is hidden for it, so does the area from the tray to the
        // corner.
        if (g_settings.cornerHidesTray && !g_trayHiddenByUser) {
            int size = MulDiv(kCornerSize, GetDpiForWindow(hWnd), 96);
            RECT corner = {window.right - size, window.bottom - size,
                           window.right, window.bottom};
            if (g_trayHiddenByCorner) {
                UnionRect(&exclude, &keep, &corner);
            } else {
                exclude = corner;
            }

            g_cornerRect = corner;
            OffsetRect(&g_cornerRect, actual.left, actual.top);
            g_cornerHoldRect = exclude;
            OffsetRect(&g_cornerHoldRect, actual.left, actual.top);
        }

        if (g_trayHiddenByUser || g_trayHiddenByCorner) {
            SetRectEmpty(&keep);
        }

        if (bridge) {
            // The XAML content was hidden before the mod saw it (e.g. the mod
            // was enabled while the taskbar was hidden).
            if (!IsWindowVisible(bridge)) {
                ShowWindow(bridge, SW_SHOWNA);
                g_bridgeHideSuppressed = true;
            }

            if (g_bridgeHideSuppressed || g_holdHidden) {
                RECT keepScreenRect = keep;
                OffsetRect(&keepScreenRect, actual.left, actual.top);

                // The reveal setting: the edge below the tray goes to the
                // XAML content rather than to Explorer, which would show the
                // whole taskbar. The watch timer watches it.
                if (clockOnly && g_settings.revealTrayAtEdge &&
                    !g_trayHiddenByUser && !g_trayHiddenByCorner &&
                    IntersectRect(&g_revealEdgeRect, &trayRect, &visible)) {
                    OffsetRect(&g_revealEdgeRect, actual.left, actual.top);
                }

                ClipBridge(bridge, &keepScreenRect, &g_revealEdgeRect,
                           nullptr);
            } else {
                // Sliding in or out.
                ClipBridge(bridge,
                           IsRectEmpty(&restOfTray) ? nullptr : &kEverywhere,
                           nullptr, &restOfTray);
            }

            g_revealedTrayRect = trayRect;
            OffsetRect(&g_revealedTrayRect, actual.left, actual.top);
        }

        UpdateWatchTimer(hWnd);
    }

    if (g_regionSet && EqualRect(&visible, &g_regionVisible) &&
        EqualRect(&keep, &g_regionKeep) &&
        EqualRect(&exclude, &g_regionExclude)) {
        return;
    }

    HRGN region = CreateRectRgnIndirect(&visible);
    HRGN keepRegion = CreateRectRgnIndirect(&keep);
    CombineRgn(region, region, keepRegion, RGN_OR);
    DeleteObject(keepRegion);
    if (!IsRectEmpty(&exclude)) {
        HRGN excludeRegion = CreateRectRgnIndirect(&exclude);
        CombineRgn(region, region, excludeRegion, RGN_DIFF);
        DeleteObject(excludeRegion);
    }

    g_regionSet = true;
    g_regionVisible = visible;
    g_regionKeep = keep;
    g_regionExclude = exclude;

    // The system owns the region after this call.
    if (!SetWindowRgn_Original(hWnd, region, TRUE)) {
        DeleteObject(region);
        g_regionSet = false;
    }
}

void UpdateRegion(HWND hWnd) {
    // Setting the region sends messages that can lead back here. Finish the
    // current update, then run it again.
    if (g_applyingRegion) {
        g_regionUpdatePending = true;
        return;
    }

    g_applyingRegion = true;
    for (int i = 0; i < 3; i++) {
        g_regionUpdatePending = false;
        ApplyRegion(hWnd);
        if (!g_regionUpdatePending) {
            break;
        }
    }
    g_applyingRegion = false;
}

// While the mod holds the taskbar looking hidden, Explorer considers it shown
// and doesn't react to the mouse at the screen edge, so the mod shows it.
void EndHoldAtEdge(HWND hWnd) {
    if (!g_holdHidden) {
        return;
    }

    RECT actual;
    GetWindowRect_Original(hWnd, &actual);

    RECT strip;
    POINT pt;
    if (IntersectRect(&strip, &g_heldLogicalRect, &actual) &&
        GetCursorPos(&pt) && PtInRect(&strip, pt)) {
        g_holdHidden = false;
        // As Explorer would have shown it.
        g_shownByEdge = true;
        UpdateRegion(hWnd);
    }
}

// Explorer shows the hidden taskbar shortly after the mouse touches its edge
// strip, see g_shownByEdge.
void NoteEdgeTouch(HWND hWnd) {
    if (!g_offset || g_holdHidden) {
        return;
    }

    RECT actual;
    GetWindowRect_Original(hWnd, &actual);

    RECT strip;
    POINT pt;
    if (IntersectRect(&strip, &g_logicalRect, &actual) && GetCursorPos(&pt) &&
        PtInRect(&strip, pt)) {
        g_edgeTouchTime = GetTickCount64();
    }
}

// Whether the mouse is where the corner setting hides the tray: in the
// corner, or while the tray is hidden for it, where it was.
bool IsCursorInCorner() {
    POINT pt;
    return GetCursorPos(&pt) &&
           (PtInRect(&g_cornerRect, pt) ||
            (g_trayHiddenByCorner && PtInRect(&g_cornerHoldRect, pt)));
}

// The mouse reached the corner on a taskbar that it brought up on its way
// (see g_shownByEdge): keeps the taskbar looking hidden, as for the overflow
// setting, until Explorer hides it.
bool HoldHiddenForCorner(const RECT& actual) {
    if (!IsHiddenRectCurrent(actual)) {
        return false;
    }

    g_holdHidden = true;
    g_holdForCorner = true;
    g_heldLogicalRect = g_hiddenLogicalRect;
    g_shownByEdge = false;
    return true;
}

void HideTrayForCorner() {
    g_trayHiddenByCorner = true;
    g_trayRevealed = false;
    g_cornerOutsideTicks = 0;
}

void OnWatchTimer(HWND hWnd) {
    POINT pt;
    if (!GetCursorPos(&pt)) {
        return;
    }

    RECT actual;
    GetWindowRect_Original(hWnd, &actual);

    bool changed = false;

    // The mouse left the taskbar it brought up.
    if (g_shownByEdge && !PtInRect(&actual, pt)) {
        g_shownByEdge = false;
        changed = true;
    }

    if (!g_trayHiddenByCorner) {
        // The mouse was pushed into the corner. A taskbar that it brought up
        // on its way looks hidden again too.
        if (PtInRect(&g_cornerRect, pt) &&
            (g_offset || g_holdHidden || HoldHiddenForCorner(actual))) {
            HideTrayForCorner();
            changed = true;
        }
    } else if (PtInRect(&g_cornerHoldRect, pt)) {
        g_cornerOutsideTicks = 0;
    } else if (++g_cornerOutsideTicks * (int)kWatchTimerMs >=
               g_settings.cornerDelay) {
        // Show the tray again once the mouse has been away for the delay.
        g_trayHiddenByCorner = false;
        changed = true;
    }

    if (g_trayHiddenByCorner) {
        // Nothing to reveal.
    } else if (!g_trayRevealed) {
        // The mouse touched the screen edge below the tray.
        if (PtInRect(&g_revealEdgeRect, pt)) {
            g_trayRevealed = true;
            g_revealOutsideTicks = 0;
            changed = true;
        }
    } else if (PtInRect(&g_revealedTrayRect, pt)) {
        g_revealOutsideTicks = 0;
    } else if (++g_revealOutsideTicks >= kWatchTicks) {
        // Back to just the clock once the mouse has been off the tray for a
        // moment.
        g_trayRevealed = false;
        changed = true;
    }

    if (changed) {
        UpdateRegion(hWnd);
    }
}

// Windows Animations, another Windhawk mod, draws its effects in windows that
// it shows above all others, so they would cover the taskbar, or the tray
// while the taskbar is hidden. It marks them with these properties.
bool IsAnimationEffectWindow(HWND hWnd) {
    return GetProp(hWnd, L"windows-animations.AnimationGhostV1") ||
           GetProp(hWnd, L"windows-animations.SwitchAnimationGhostV1");
}

// Whether an effect covers the taskbar's whole monitor, as it does while a
// maximized window minimizes or restores.
bool IsEffectCoveringMonitor(HWND taskbar) {
    MONITORINFO mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfo(MonitorFromWindow(taskbar, MONITOR_DEFAULTTONEAREST),
                        &mi)) {
        return false;
    }

    // Effect windows are topmost.
    for (HWND hWnd = GetTopWindow(nullptr);
         hWnd && (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST);
         hWnd = GetWindow(hWnd, GW_HWNDNEXT)) {
        RECT rc;
        if (IsWindowVisible(hWnd) && IsAnimationEffectWindow(hWnd) &&
            GetWindowRect_Original(hWnd, &rc) && rc.left <= mi.rcMonitor.left &&
            rc.top <= mi.rcMonitor.top && rc.right >= mi.rcMonitor.right &&
            rc.bottom >= mi.rcMonitor.bottom) {
            return true;
        }
    }

    return false;
}

void OnWindowPosChanging(HWND hWnd, WINDOWPOS* wp) {
    RECT actual;
    GetWindowRect_Original(hWnd, &actual);

    // Explorer takes a window that covers the screen for a full screen app
    // and puts the taskbar behind it. That includes maximized windows without
    // a standard frame, which shouldn't cover the tray, and an effect of
    // Windows Animations while a maximized window minimizes or restores (see
    // OnEffectWindowEvent). A full screen window, e.g. a browser playing a
    // video, can be maximized too, but it has no frame at all: no caption and
    // no sizing border.
    if (!(wp->flags & SWP_NOZORDER) && LosesTopmost(wp->hwndInsertAfter)) {
        HWND foreground = GetForegroundWindow();
        if ((foreground && IsZoomed(foreground) &&
             (GetWindowLongPtr(foreground, GWL_STYLE) &
              (WS_CAPTION | WS_THICKFRAME))) ||
            IsEffectCoveringMonitor(hWnd)) {
            wp->hwndInsertAfter = HWND_TOPMOST;
        }
    }

    RECT prevLogical = g_offset ? g_logicalRect : actual;
    bool prevOffset = g_offset;

    RECT logical = prevLogical;
    if (!(wp->flags & SWP_NOMOVE)) {
        OffsetRect(&logical, wp->x - logical.left, wp->y - logical.top);
    }

    if (!(wp->flags & SWP_NOSIZE)) {
        logical.right = logical.left + wp->cx;
        logical.bottom = logical.top + wp->cy;
    }

    RECT docked = GetDockedRect(logical, actual);
    bool unhiding = prevOffset && VisibleArea(logical, docked) >
                                      VisibleArea(prevLogical, docked);

    // Explorer shows the XAML content again as it starts to unhide the
    // taskbar. The content window was never really hidden, so that may not
    // reach BridgeSubclassProc; the taskbar moving back on screen does.
    if (g_bridgeHideSuppressed && unhiding) {
        g_bridgeHideSuppressed = false;
    }

    // Where Explorer hides the taskbar, and whether it shows it again because
    // the mouse touched the edge strip, see g_shownByEdge.
    bool offset = !EqualRect(&docked, &logical);
    if (offset && !unhiding &&
        (VisibleArea(logical, docked) < VisibleArea(prevLogical, docked) ||
         !IsHiddenRectCurrent(actual))) {
        g_hiddenLogicalRect = logical;
        g_shownByEdge = false;
    } else if (unhiding && g_edgeTouchTime &&
               GetTickCount64() - g_edgeTouchTime < kEdgeTouchMs) {
        g_shownByEdge = true;
    }

    // Opening the hidden icons makes Explorer show the taskbar. With the
    // overflow setting on, keep it looking hidden until Explorer hides it
    // again (or see EndHoldAtEdge). The corner setting does the same when the
    // mouse is already in the corner.
    if (unhiding && !g_holdHidden && g_settings.overflowKeepsTaskbarHidden &&
        IsCursorOnOverflowButton(g_tray, g_bridge)) {
        g_holdHidden = true;
        g_holdForCorner = false;
        g_heldLogicalRect = prevLogical;
    } else if (unhiding && !g_holdHidden && g_shownByEdge &&
               IsCursorInCorner() && HoldHiddenForCorner(actual)) {
        HideTrayForCorner();
    } else if (g_holdHidden && VisibleArea(logical, docked) <=
                                   VisibleArea(g_heldLogicalRect, docked)) {
        g_holdHidden = false;
    }

    g_logicalRect = logical;
    g_offset = offset;
    g_reportLogicalMove =
        logical.left != prevLogical.left || logical.top != prevLogical.top;

    // Keep the real window docked; the region decides what's visible.
    if (!(wp->flags & SWP_NOMOVE) || docked.left != actual.left ||
        docked.top != actual.top) {
        wp->x = docked.left;
        wp->y = docked.top;
        wp->flags &= ~SWP_NOMOVE;
    }

    // The region is normally updated right after the move (see
    // SetWindowPos_Hook). This covers moves made some other way.
    if ((!EqualRect(&logical, &prevLogical) || g_offset != prevOffset) &&
        !g_refreshPosted) {
        g_refreshPosted = PostMessage(hWnd, g_refreshMsg, 0, 0);
    }
}

// Behind a full screen app the taskbar isn't topmost (see
// OnWindowPosChanging), and windows above it stay there.
bool IsAboveTopmostTaskbar(HWND hWnd, HWND taskbar) {
    if (!(GetWindowLongPtr(taskbar, GWL_EXSTYLE) & WS_EX_TOPMOST)) {
        return false;
    }

    // Windows Animations keeps some effects below the taskbar itself.
    for (HWND above = GetWindow(taskbar, GW_HWNDPREV); above;
         above = GetWindow(above, GW_HWNDPREV)) {
        if (above == hWnd) {
            return true;
        }
    }

    return false;
}

// Windows Animations marks an effect window right after creating it, so the
// mark can come a moment after the creation is reported. Wait up to 2 ms for
// it, for windows that look like effects: topmost STATIC tool windows that
// don't activate.
bool WaitForEffectMark(HWND hWnd) {
    constexpr LONG_PTR kEffectExStyle =
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
    WCHAR className[16];
    if ((GetWindowLongPtr(hWnd, GWL_EXSTYLE) & kEffectExStyle) !=
            kEffectExStyle ||
        !GetClassName(hWnd, className, ARRAYSIZE(className)) ||
        _wcsicmp(className, L"Static") != 0) {
        return false;
    }

    LARGE_INTEGER frequency, start, now;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    do {
        if (IsAnimationEffectWindow(hWnd)) {
            return true;
        }

        SwitchToThread();
        QueryPerformanceCounter(&now);
    } while (now.QuadPart - start.QuadPart < frequency.QuadPart / 500);

    return false;
}

// An effect window was created or shown. The hook is out of context, so this
// runs on the effect watcher thread, which set it.
void CALLBACK OnEffectWindowEvent(HWINEVENTHOOK hWinEventHook,
                                  DWORD event,
                                  HWND hWnd,
                                  LONG idObject,
                                  LONG idChild,
                                  DWORD idEventThread,
                                  DWORD dwmsEventTime) {
    // The hook's range includes EVENT_OBJECT_DESTROY.
    if (event == EVENT_OBJECT_DESTROY || idObject != OBJID_WINDOW ||
        idChild != CHILDID_SELF || !hWnd ||
        !(event == EVENT_OBJECT_CREATE ? WaitForEffectMark(hWnd)
                                       : IsAnimationEffectWindow(hWnd))) {
        return;
    }

    HWND taskbar = FindWindow(L"Shell_TrayWnd", nullptr);
    DWORD processId = 0;
    if (!taskbar || !GetWindowThreadProcessId(taskbar, &processId) ||
        processId != GetCurrentProcessId() ||
        !IsAboveTopmostTaskbar(hWnd, taskbar)) {
        return;
    }

    // Right below the taskbar, where Windows Animations puts its Genie
    // effect. That keeps the effect topmost, since the taskbar stays topmost
    // even for an effect that covers the screen (see OnWindowPosChanging).
    // Asynchronously: the thread drawing the effect moves it between two
    // frames, too late for the first one (see RaiseAboveEffect).
    SetWindowPos(hWnd, taskbar, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE |
                     SWP_NOOWNERZORDER | SWP_ASYNCWINDOWPOS);

    // Sent rather than posted: Windows Animations makes the thread that
    // minimizes a window, the taskbar thread after a click on a taskbar
    // button, wait until the effect's first frame is on screen, and that
    // thread handles only sent messages meanwhile.
    DWORD_PTR result;
    SendMessageTimeout(taskbar, g_refreshMsg, kRefreshEffect, (LPARAM)hWnd,
                       SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, &result);
}

DWORD WINAPI EffectWatcherThreadProc(LPVOID) {
    HWINEVENTHOOK hook =
        SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_SHOW, nullptr,
                        OnEffectWindowEvent, 0, 0, WINEVENT_OUTOFCONTEXT);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        DispatchMessage(&msg);
    }

    if (hook) {
        UnhookWinEvent(hook);
    }

    return 0;
}

// Windows Animations draws an effect's first frame before it shows the
// window, and only then does the thread drawing it move it below the taskbar
// (see OnEffectWindowEvent). Moving the taskbar above the effect takes effect
// at once, so done as the effect is created, the first frame is below the
// taskbar too. That also works for effects of apps running as administrator,
// whose windows Explorer can't move.
void RaiseAboveEffect(HWND taskbar, HWND effect) {
    if (!IsAnimationEffectWindow(effect) ||
        !IsAboveTopmostTaskbar(effect, taskbar)) {
        return;
    }

    // The part of the taskbar on screen: all of it, or while it looks
    // hidden, the tray (or the clock) that the region keeps, see ApplyRegion.
    RECT onScreen;
    GetWindowRect_Original(taskbar, &onScreen);
    if (g_offset || g_holdHidden) {
        if (!g_regionSet || IsRectEmpty(&g_regionKeep)) {
            return;
        }

        RECT keep = g_regionKeep;
        OffsetRect(&keep, onScreen.left, onScreen.top);
        onScreen = keep;
    }

    RECT effectRect;
    if (!GetWindowRect_Original(effect, &effectRect) ||
        !IntersectRect(&effectRect, &effectRect, &onScreen)) {
        return;
    }

    SetWindowPos(taskbar, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

LRESULT CALLBACK TaskbarSubclassProc(HWND hWnd,
                                     UINT uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam,
                                     DWORD_PTR dwRefData) {
    if (uMsg == g_refreshMsg && g_refreshMsg) {
        if (wParam & kRefreshUnload) {
            // Maybe for another copy of the mod, see WatchTimerId.
            if ((UINT_PTR)lParam != WatchTimerId()) {
                return DefSubclassProc(hWnd, uMsg, wParam, lParam);
            }

            KillTimer(hWnd, WatchTimerId());
            g_watchTimerRunning = false;
            return 0;
        }

        if (wParam == kRefreshEffect) {
            RaiseAboveEffect(hWnd, (HWND)lParam);
            return 0;
        }

        g_refreshPosted = false;
        if (wParam & kRefreshToggleTray) {
            g_trayHiddenByUser = !g_trayHiddenByUser;
        }
        if (wParam & kRefreshPosition) {
            // Explorer keeps the window where it wants it, so this is a no-op
            // for Explorer, but it lets the mod dock an already hidden
            // taskbar.
            SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                             SWP_NOOWNERZORDER | SWP_NOACTIVATE);
        }
        if (wParam & kRefreshSettings) {
            // Find the windows now rather than when the taskbar first hides,
            // so that the button rects are known by then.
            FindTray(hWnd);
            FindBridge(hWnd);
            UpdateUiaRequest(true);
        }
        UpdateRegion(hWnd);
        return 0;
    }

    switch (uMsg) {
        case WM_WINDOWPOSCHANGING: {
            // Let Explorer pick its position first, then adjust it.
            LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);
            OnWindowPosChanging(hWnd, (WINDOWPOS*)lParam);
            return result;
        }

        case WM_WINDOWPOSCHANGED: {
            UpdateRegion(hWnd);

            if (g_offset || g_reportLogicalMove) {
                // Report the position Explorer asked for, so its auto-hide
                // state stays consistent.
                WINDOWPOS wp = *(const WINDOWPOS*)lParam;
                wp.x = g_logicalRect.left;
                wp.y = g_logicalRect.top;
                if (g_reportLogicalMove) {
                    wp.flags &= ~SWP_NOMOVE;
                    g_reportLogicalMove = false;
                }
                return DefSubclassProc(hWnd, uMsg, wParam, (LPARAM)&wp);
            }
            break;
        }

        // Children that don't handle the cursor pass WM_SETCURSOR up here.
        case WM_SETCURSOR:
        case WM_MOUSEMOVE:
            NoteEdgeTouch(hWnd);
            EndHoldAtEdge(hWnd);
            break;

        case WM_TIMER:
            if (wParam == WatchTimerId()) {
                OnWatchTimer(hWnd);
                return 0;
            }
            break;

        case WM_NCDESTROY:
            g_taskbar = nullptr;
            g_tray = nullptr;
            g_bridge = nullptr;
            g_bridgeHideSuppressed = false;
            g_bridgeRegionSet = false;
            g_bridgeTrim = 0;
            g_offset = false;
            g_holdHidden = false;
            g_trayRevealed = false;
            g_trayHiddenByCorner = false;
            g_shownByEdge = false;
            SetRectEmpty(&g_hiddenLogicalRect);
            g_watchTimerRunning = false;
            g_regionSet = false;
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TraySubclassProc(HWND hWnd,
                                  UINT uMsg,
                                  WPARAM wParam,
                                  LPARAM lParam,
                                  DWORD_PTR dwRefData) {
    LRESULT result = DefSubclassProc(hWnd, uMsg, wParam, lParam);

    switch (uMsg) {
        case WM_WINDOWPOSCHANGED:
            // The tray grew or shrank (icons added/removed, the clock got
            // wider), which can also move the clock and the overflow button.
            if (g_taskbar) {
                UpdateUiaRequest(true);
                if (g_offset || g_holdHidden ||
                    (g_settings.onlyDateTime &&
                     g_settings.clockOnlyWhenShown)) {
                    UpdateRegion(g_taskbar);
                }
            }
            break;

        case WM_NCDESTROY:
            if (hWnd == g_tray) {
                g_tray = nullptr;
            }
            break;
    }

    return result;
}

LRESULT CALLBACK BridgeSubclassProc(HWND hWnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam,
                                    DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_WINDOWPOSCHANGING: {
            WINDOWPOS* wp = (WINDOWPOS*)lParam;

            if (!(wp->flags & SWP_NOSIZE)) {
                // A caller that keeps the window's current size, rather than
                // the one Explorer gave it, already has the trim.
                bool keepsTrim = g_bridgeTrim &&
                                 wp->cx == g_bridgeSize.cx - g_bridgeTrim &&
                                 wp->cy == g_bridgeSize.cy;
                if (!keepsTrim) {
                    g_bridgeSize = {wp->cx, wp->cy};
                    wp->cx -= g_bridgeTrim;
                }
            }

            bool wasSuppressed = g_bridgeHideSuppressed;
            if (wp->flags & SWP_SHOWWINDOW) {
                g_bridgeHideSuppressed = false;
            } else if ((wp->flags & SWP_HIDEWINDOW) && g_offset &&
                       g_taskbar) {
                // Explorer hides the taskbar content once it's fully
                // hidden. Keep it; it gets clipped to the tray instead.
                wp->flags &= ~SWP_HIDEWINDOW;
                g_bridgeHideSuppressed = true;
            }

            // Update the clip once this change is done.
            if (g_bridgeHideSuppressed != wasSuppressed && g_taskbar &&
                !g_refreshPosted) {
                g_refreshPosted = PostMessage(g_taskbar, g_refreshMsg, 0, 0);
            }
            break;
        }

        case WM_NCDESTROY:
            if (hWnd == g_bridge) {
                g_bridge = nullptr;
                g_bridgeHideSuppressed = false;
                g_bridgeRegionSet = false;
                g_bridgeTrim = 0;
            }
            break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

void AttachToTaskbar(HWND hWnd) {
    if (g_taskbar) {
        return;
    }

    DWORD threadId = GetWindowThreadProcessId(hWnd, nullptr);
    if (!threadId) {
        return;
    }

    g_offset = false;
    g_holdHidden = false;
    g_trayRevealed = false;
    g_trayHiddenByCorner = false;
    g_shownByEdge = false;
    SetRectEmpty(&g_hiddenLogicalRect);
    g_edgeTouchTime = 0;
    g_watchTimerRunning = false;
    g_regionSet = false;
    g_reportLogicalMove = false;
    g_refreshPosted = false;
    g_taskbarThreadId = threadId;
    g_taskbar = hWnd;

    if (!WindhawkUtils::SetWindowSubclassFromAnyThread(
            hWnd, TaskbarSubclassProc, 0)) {
        Wh_Log(L"Failed to subclass the taskbar");
        g_taskbar = nullptr;
        return;
    }

    // Apply the current state once Explorer is idle, in case the taskbar is
    // already hidden.
    g_refreshPosted = PostMessage(hWnd, g_refreshMsg,
                                  kRefreshPosition | kRefreshSettings, 0);
}

BOOL WINAPI GetWindowRect_Hook(HWND hWnd, LPRECT lpRect) {
    // Explorer's auto-hide logic reads the taskbar rect (e.g. to decide whether
    // the cursor is on the hidden strip). Give it the rect it asked for.
    if (hWnd && hWnd == g_taskbar && g_offset && lpRect &&
        GetCurrentThreadId() == g_taskbarThreadId) {
        *lpRect = g_logicalRect;
        return TRUE;
    }

    return GetWindowRect_Original(hWnd, lpRect);
}

BOOL WINAPI SetWindowPos_Hook(HWND hWnd,
                              HWND hWndInsertAfter,
                              int X,
                              int Y,
                              int cx,
                              int cy,
                              UINT uFlags) {
    BOOL result =
        SetWindowPos_Original(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);

    // Explorer slides the taskbar with SetWindowPos. The real window doesn't
    // move, so no WM_WINDOWPOSCHANGED is sent; update the region here.
    if (hWnd && hWnd == g_taskbar &&
        GetCurrentThreadId() == g_taskbarThreadId) {
        UpdateRegion(hWnd);
    }

    return result;
}

int WINAPI SetWindowRgn_Hook(HWND hWnd, HRGN hRgn, BOOL bRedraw) {
    if (hWnd && hWnd == g_taskbar) {
        if (g_offset) {
            // With several monitors, Explorer clips the sliding taskbar to its
            // monitor. The docked window doesn't need that, and the mod's
            // region must stay.
            if (hRgn) {
                DeleteObject(hRgn);
            }
            return 1;
        }

        g_regionSet = false;
    }

    return SetWindowRgn_Original(hWnd, hRgn, bRedraw);
}

bool IsTaskbarClass(HWND hWnd, LPCWSTR lpClassName) {
    if (lpClassName && !IS_INTRESOURCE(lpClassName)) {
        return _wcsicmp(lpClassName, L"Shell_TrayWnd") == 0;
    }

    WCHAR className[32];
    return GetClassName(hWnd, className, ARRAYSIZE(className)) &&
           _wcsicmp(className, L"Shell_TrayWnd") == 0;
}

HWND WINAPI CreateWindowExW_Hook(DWORD dwExStyle,
                                 LPCWSTR lpClassName,
                                 LPCWSTR lpWindowName,
                                 DWORD dwStyle,
                                 int X,
                                 int Y,
                                 int nWidth,
                                 int nHeight,
                                 HWND hWndParent,
                                 HMENU hMenu,
                                 HINSTANCE hInstance,
                                 LPVOID lpParam) {
    HWND hWnd = CreateWindowExW_Original(dwExStyle, lpClassName, lpWindowName,
                                         dwStyle, X, Y, nWidth, nHeight,
                                         hWndParent, hMenu, hInstance, lpParam);

    if (hWnd && !g_taskbar && IsTaskbarClass(hWnd, lpClassName)) {
        Wh_Log(L"Taskbar created");
        AttachToTaskbar(hWnd);
    }

    return hWnd;
}

void LoadSettings() {
    g_settings.onlyDateTime = Wh_GetIntSetting(L"onlyDateTime");
    g_settings.clockOnlyWhenShown = Wh_GetIntSetting(L"clockOnlyWhenShown");
    g_settings.offset = Wh_GetIntSetting(L"offset");
    g_settings.overflowKeepsTaskbarHidden =
        Wh_GetIntSetting(L"overflowKeepsTaskbarHidden");
    g_settings.revealTrayAtEdge = Wh_GetIntSetting(L"revealTrayAtEdge");
    g_settings.cornerHidesTray = Wh_GetIntSetting(L"cornerHidesTray");
    g_settings.cornerDelay = Wh_GetIntSetting(L"cornerDelay");

    auto hotkey = WindhawkUtils::StringSetting::make(L"hotkey");
    if (!ParseHotkey(hotkey.get(), &g_settings.hotkeyModifiers,
                     &g_settings.hotkeyVk)) {
        if (*hotkey.get()) {
            Wh_Log(L"Can't read the shortcut \"%s\"", hotkey.get());
        }
        g_settings.hotkeyModifiers = 0;
        g_settings.hotkeyVk = 0;
    }
}

BOOL Wh_ModInit() {
    Wh_Log(L">");

    LoadSettings();

    g_refreshMsg = RegisterWindowMessage(L"Windhawk_Refresh_" WH_MOD_ID);
    g_hotkeyId = GlobalAddAtom(L"Windhawk_Hotkey_" WH_MOD_ID);

    if (!WindhawkUtils::SetFunctionHook(GetWindowRect, GetWindowRect_Hook,
                                        &GetWindowRect_Original) ||
        !WindhawkUtils::SetFunctionHook(SetWindowPos, SetWindowPos_Hook,
                                        &SetWindowPos_Original) ||
        !WindhawkUtils::SetFunctionHook(SetWindowRgn, SetWindowRgn_Hook,
                                        &SetWindowRgn_Original) ||
        !WindhawkUtils::SetFunctionHook(CreateWindowExW, CreateWindowExW_Hook,
                                        &CreateWindowExW_Original)) {
        Wh_Log(L"Failed to set hooks");
        return FALSE;
    }

    // Without the thread, the shortcut does nothing, and on Windows 11 "Only
    // show date/time" falls back to the whole tray and the overflow button
    // always shows the taskbar.
    g_workerWakeEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    g_workerStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (g_workerWakeEvent && g_workerStopEvent) {
        g_workerThread =
            CreateThread(nullptr, 0, WorkerThreadProc, nullptr, 0, nullptr);
    }

    g_effectWatcherThread = CreateThread(nullptr, 0, EffectWatcherThreadProc,
                                         nullptr, 0, &g_effectWatcherThreadId);

    return TRUE;
}

void Wh_ModAfterInit() {
    Wh_Log(L">");

    HWND hWnd = FindWindow(L"Shell_TrayWnd", nullptr);
    DWORD processId = 0;
    if (hWnd && GetWindowThreadProcessId(hWnd, &processId) &&
        processId == GetCurrentProcessId()) {
        AttachToTaskbar(hWnd);
    }
}

void Wh_ModSettingsChanged() {
    Wh_Log(L">");

    LoadSettings();

    // For the shortcut.
    if (g_workerWakeEvent) {
        SetEvent(g_workerWakeEvent);
    }

    HWND hWnd = g_taskbar;
    if (hWnd) {
        PostMessage(hWnd, g_refreshMsg, kRefreshSettings, 0);
    }
}

void Wh_ModUninit() {
    Wh_Log(L">");

    if (g_workerThread) {
        SetEvent(g_workerStopEvent);
        WaitForSingleObject(g_workerThread, INFINITE);
        CloseHandle(g_workerThread);
        g_workerThread = nullptr;
    }
    if (g_workerWakeEvent) {
        CloseHandle(g_workerWakeEvent);
        g_workerWakeEvent = nullptr;
    }
    if (g_workerStopEvent) {
        CloseHandle(g_workerStopEvent);
        g_workerStopEvent = nullptr;
    }

    if (g_effectWatcherThread) {
        // Posting fails until the thread has a message queue.
        while (!PostThreadMessage(g_effectWatcherThreadId, WM_QUIT, 0, 0) &&
               WaitForSingleObject(g_effectWatcherThread, 10) ==
                   WAIT_TIMEOUT) {
        }
        WaitForSingleObject(g_effectWatcherThread, INFINITE);
        CloseHandle(g_effectWatcherThread);
        g_effectWatcherThread = nullptr;
    }

    HWND hWnd = g_taskbar;
    if (hWnd) {
        SendMessage(hWnd, g_refreshMsg, kRefreshUnload, WatchTimerId());
    }

    if (g_hotkeyId) {
        GlobalDeleteAtom(g_hotkeyId);
        g_hotkeyId = 0;
    }

    if (!hWnd) {
        return;
    }

    if (g_tray) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(g_tray,
                                                         TraySubclassProc);
    }
    HWND bridge = g_bridge;
    bool hideBridge = g_bridgeHideSuppressed;
    bool unclipBridge = g_bridgeRegionSet;
    int bridgeTrim = g_bridgeTrim;
    SIZE bridgeSize = g_bridgeSize;
    if (bridge) {
        WindhawkUtils::RemoveWindowSubclassFromAnyThread(bridge,
                                                         BridgeSubclassProc);
    }
    WindhawkUtils::RemoveWindowSubclassFromAnyThread(hWnd,
                                                     TaskbarSubclassProc);

    bool wasOffset = g_offset;
    RECT logical = g_logicalRect;
    g_taskbar = nullptr;
    g_tray = nullptr;
    g_bridge = nullptr;
    g_bridgeHideSuppressed = false;
    g_bridgeRegionSet = false;
    g_bridgeTrim = 0;
    g_offset = false;
    g_holdHidden = false;
    g_trayRevealed = false;
    g_trayHiddenByCorner = false;

    if (bridge && bridgeTrim) {
        SetWindowPos(bridge, nullptr, 0, 0, bridgeSize.cx, bridgeSize.cy,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                         SWP_NOACTIVATE);
    }

    if (bridge && unclipBridge) {
        SetWindowRgn(bridge, nullptr, TRUE);
    }

    // Leave the content hidden, as Explorer left it.
    if (bridge && hideBridge && wasOffset) {
        ShowWindow(bridge, SW_HIDE);
    }

    SetWindowRgn(hWnd, nullptr, TRUE);

    // Put the window where Explorer thinks it is.
    if (wasOffset) {
        SetWindowPos(hWnd, nullptr, logical.left, logical.top, 0, 0,
                     SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER |
                         SWP_NOACTIVATE);
    }
}
