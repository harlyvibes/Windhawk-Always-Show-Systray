# Records what the taskbar, the tray and Windows Animations' effect windows do
# while a window minimizes, for finding why the taskbar and tray blink with the
# "Keep System Tray Visible on Auto-Hide Taskbar" Windhawk mod.
#
# Read-only: it doesn't click, type or change anything. Run it in PowerShell:
#   powershell -ExecutionPolicy Bypass -File diagnose-blink.ps1
# then, within 20 seconds, minimize a window the way that makes the taskbar
# blink (e.g. click the taskbar button of a maximized window), and send the
# output once it finishes.
param([int]$Seconds = 20)

Add-Type @'
using System; using System.Collections.Generic; using System.Diagnostics;
using System.Text; using System.Threading; using System.Runtime.InteropServices;
public static class BD {
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr v);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string c, string n);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindowEx(IntPtr p, IntPtr a, string c, string n);
  [DllImport("user32.dll")] public static extern IntPtr GetTopWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetWindow(IntPtr h, uint cmd);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsZoomed(IntPtr h);
  [DllImport("user32.dll")] public static extern long GetWindowLongPtr(IntPtr h, int i);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern int GetWindowRgnBox(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr GetProp(IntPtr h, string name);
  [DllImport("user32.dll")] public static extern bool GetCursorPos(out POINT p);
  [DllImport("user32.dll")] public static extern uint GetDpiForWindow(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern bool EnumDisplaySettings(string dev, int mode, ref DEVMODE dm);
  [DllImport("dwmapi.dll")] public static extern int DwmGetWindowAttribute(IntPtr h, int attr, out int value, int size);
  [DllImport("shell32.dll")] public static extern IntPtr SHAppBarMessage(uint msg, ref APPBARDATA d);
  [DllImport("winmm.dll")] public static extern uint timeBeginPeriod(uint p);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
  [StructLayout(LayoutKind.Sequential)] public struct APPBARDATA { public int cbSize; public IntPtr hWnd; public uint uCallbackMessage; public uint uEdge; public RECT rc; public IntPtr lParam; }
  [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)] public struct DEVMODE {
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst=32)] public string dmDeviceName;
    public short dmSpecVersion, dmDriverVersion, dmSize, dmDriverExtra; public int dmFields;
    public int dmPositionX, dmPositionY, dmDisplayOrientation, dmDisplayFixedOutput;
    public short dmColor, dmDuplex, dmYResolution, dmTTOption, dmCollate;
    [MarshalAs(UnmanagedType.ByValTStr, SizeConst=32)] public string dmFormName;
    public short dmLogPixels; public int dmBitsPerPel, dmPelsWidth, dmPelsHeight, dmDisplayFlags, dmDisplayFrequency;
  }

  public static int RefreshRate() {
    var dm = new DEVMODE(); dm.dmSize = (short)Marshal.SizeOf(typeof(DEVMODE));
    return EnumDisplaySettings(null, -1, ref dm) ? dm.dmDisplayFrequency : 0;
  }
  static string R(RECT r) { return "[" + r.L + "," + r.T + "," + r.R + "," + r.B + "]"; }
  static string Cls(IntPtr h) { var s = new StringBuilder(96); GetClassName(h, s, 96); return s.ToString(); }
  static Dictionary<uint, string> names = new Dictionary<uint, string>();
  static string Proc(IntPtr h) {
    uint pid; GetWindowThreadProcessId(h, out pid);
    string n;
    if (!names.TryGetValue(pid, out n)) {
      try { n = Process.GetProcessById((int)pid).ProcessName; } catch { n = "pid" + pid; }
      names[pid] = n;
    }
    return n;
  }
  static int Cloaked(IntPtr h) { int c = 0; DwmGetWindowAttribute(h, 14, out c, 4); return c; }
  static bool Topmost(IntPtr h) { return (GetWindowLongPtr(h, -20) & 8) != 0; }
  static bool Overlaps(RECT a, RECT b) { return a.L < b.R && b.L < a.R && a.T < b.B && b.T < a.B; }

  // One line per aspect; only changes are logged.
  static string[] Sample(IntPtr taskbar, IntPtr bridge) {
    RECT tr, rb, br, bb;
    GetWindowRect(taskbar, out tr);
    int rgn = GetWindowRgnBox(taskbar, out rb);
    string taskbarLine = "taskbar visible=" + IsWindowVisible(taskbar) + " cloaked=" + Cloaked(taskbar) + " topmost=" + Topmost(taskbar) +
      " rect=" + R(tr) + " region=" + (rgn == 0 ? "none" : rgn + R(rb));
    string bridgeLine = "";
    if (bridge != IntPtr.Zero) {
      GetWindowRect(bridge, out br);
      int brg = GetWindowRgnBox(bridge, out bb);
      bridgeLine = "tray content visible=" + IsWindowVisible(bridge) + " rect=" + R(br) + " region=" + (brg == 0 ? "none" : brg + R(bb));
    }

    // Topmost windows, top first, until the first window that isn't topmost.
    var above = new List<string>();
    var effects = new List<string>();
    bool aboveTaskbar = true;
    for (IntPtr h = GetTopWindow(IntPtr.Zero); h != IntPtr.Zero; h = GetWindow(h, 2)) {
      if (h == taskbar) { aboveTaskbar = false; continue; }
      long ex = GetWindowLongPtr(h, -20);
      if ((ex & 8) == 0) break;
      if (!IsWindowVisible(h)) continue;
      RECT r; GetWindowRect(h, out r);
      if (GetProp(h, "windows-animations.AnimationGhostV1") != IntPtr.Zero ||
          GetProp(h, "windows-animations.SwitchAnimationGhostV1") != IntPtr.Zero) {
        effects.Add("effect " + h.ToString("X") + " (" + Proc(h) + ") " + R(r) + (aboveTaskbar ? " ABOVE taskbar" : " below taskbar"));
      } else if (aboveTaskbar && Overlaps(r, tr) && r.R - r.L > 1 && r.B - r.T > 1) {
        above.Add(Cls(h) + " (" + Proc(h) + ") " + R(r));
      }
    }

    IntPtr fg = GetForegroundWindow();
    string fgLine = "foreground " + Cls(fg) + " (" + Proc(fg) + ") zoomed=" + IsZoomed(fg) + " minimized=" + IsIconic(fg) + " cloaked=" + Cloaked(fg);
    return new string[] {
      taskbarLine, bridgeLine,
      "windows over the taskbar: " + (above.Count == 0 ? "none" : string.Join("; ", above)),
      "effects: " + (effects.Count == 0 ? "none" : string.Join("; ", effects)),
      fgLine
    };
  }

  public static List<string> Watch(IntPtr taskbar, IntPtr bridge, int seconds) {
    timeBeginPeriod(1);
    var log = new List<string>();
    var clock = Stopwatch.StartNew();
    string[] last = new string[5];
    int lastAutoHide = -1;
    long nextAutoHide = 0;
    while (clock.ElapsedMilliseconds < seconds * 1000L) {
      string[] now = Sample(taskbar, bridge);
      double ms = clock.Elapsed.TotalMilliseconds;
      for (int i = 0; i < now.Length; i++) {
        if (now[i] != last[i]) { log.Add(ms.ToString("F1").PadLeft(8) + "ms " + now[i]); last[i] = now[i]; }
      }
      if (clock.ElapsedMilliseconds >= nextAutoHide) {
        var abd = new APPBARDATA(); abd.cbSize = Marshal.SizeOf(typeof(APPBARDATA));
        int autoHide = ((int)SHAppBarMessage(4, ref abd) & 1);  // ABM_GETSTATE, ABS_AUTOHIDE
        if (autoHide != lastAutoHide) { log.Add(ms.ToString("F1").PadLeft(8) + "ms auto-hide setting=" + (autoHide == 1 ? "on" : "off")); lastAutoHide = autoHide; }
        nextAutoHide = clock.ElapsedMilliseconds + 100;
      }
      Thread.Sleep(1);
    }
    return log;
  }
}
'@
[void][BD]::SetProcessDpiAwarenessContext([IntPtr]-4)
$nul = [NullString]::Value

$os = Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
"Windows build $($os.CurrentBuild).$($os.UBR)"
$taskbar = [BD]::FindWindow('Shell_TrayWnd', $nul)
if ($taskbar -eq [IntPtr]::Zero) { "No Shell_TrayWnd found."; return }
$bridge = [BD]::FindWindowEx($taskbar, [IntPtr]::Zero, 'Windows.UI.Composition.DesktopWindowContentBridge', $nul)
"Screen refresh rate $([BD]::RefreshRate()) Hz, taskbar DPI $([BD]::GetDpiForWindow($taskbar))"

$mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
if (Test-Path $mods) {
    $enabled = Get-ChildItem $mods | Where-Object { (Get-ItemProperty $_.PSPath).Disabled -ne 1 }
    "Enabled mods: " + (($enabled | ForEach-Object { "$($_.PSChildName) $((Get-ItemProperty $_.PSPath).Version)" }) -join ', ')
    foreach ($m in $enabled) {
        if ($m.PSChildName -match 'taskbar-tray-stays-visible|windows-animations|taskbar-auto-hide-when-maximized') {
            $s = Get-ItemProperty (Join-Path $m.PSPath 'Settings') -ErrorAction SilentlyContinue
            if ($s) {
                $pairs = $s.PSObject.Properties | Where-Object { $_.Name -notlike 'PS*' } | ForEach-Object { "$($_.Name)=$($_.Value)" }
                "  $($m.PSChildName) settings: $($pairs -join ' ')"
            }
        }
    }
}
$loaded = Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object { $_.Modules } | Where-Object { $_.FileName -like '*taskbar-tray-stays-visible*' } | Select-Object -ExpandProperty FileName
"Loaded in Explorer: $(if ($loaded) { Split-Path $loaded -Leaf } else { 'no' })"
""
"Recording for $Seconds seconds. Minimize a window the way that makes the taskbar blink now."
""
[BD]::Watch($taskbar, $bridge, $Seconds)
""
"Done."
