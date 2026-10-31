# Prints how the taskbar's notification area is built on this machine, for
# fixing the "Overflow button keeps the taskbar hidden" setting of the
# "Keep System Tray Visible on Auto-Hide Taskbar" Windhawk mod.
#
# Read-only: it doesn't click, type or change anything. Run it in PowerShell
# with the taskbar shown (or hidden, either works):
#   powershell -ExecutionPolicy Bypass -File diagnose-tray.ps1
# and send the output.

Add-Type @'
using System; using System.Text; using System.Runtime.InteropServices;
public static class Diag {
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr v);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindow(string c, string n);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern IntPtr FindWindowEx(IntPtr p, IntPtr a, string c, string n);
  [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc f, IntPtr l);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern IntPtr GetParent(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern uint GetDpiForWindow(IntPtr h);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
  public static string Rect(IntPtr h) { RECT r; GetWindowRect(h, out r); return "[" + r.L + "," + r.T + "," + r.R + "," + r.B + "]"; }
  public static string Children(IntPtr parent) {
    var sb = new StringBuilder();
    EnumChildWindows(parent, (h, l) => {
      var c = new StringBuilder(256); GetClassName(h, c, 256);
      var p = new StringBuilder(256); GetClassName(GetParent(h), p, 256);
      sb.AppendFormat("  {0,-52} parent={1,-30} visible={2,-5} {3}\n", c, p, IsWindowVisible(h), Rect(h));
      return true;
    }, IntPtr.Zero);
    return sb.ToString();
  }
}
'@
[void][Diag]::SetProcessDpiAwarenessContext([IntPtr]-4)
Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes
$nul = [NullString]::Value

$os = Get-ItemProperty 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion'
"Windows: $($os.ProductName) $($os.DisplayVersion), build $($os.CurrentBuild).$($os.UBR)"

$taskbar = [Diag]::FindWindow('Shell_TrayWnd', $nul)
if ($taskbar -eq [IntPtr]::Zero) { "No Shell_TrayWnd found."; return }
"Taskbar: $([Diag]::Rect($taskbar)), DPI $([Diag]::GetDpiForWindow($taskbar))"
""
"== Taskbar child windows =="
[Diag]::Children($taskbar)

"== Overflow popup windows =="
foreach ($class in 'TopLevelWindowForOverflowXamlIsland', 'NotifyIconOverflowWindow') {
    $h = [Diag]::FindWindow($class, $nul)
    if ($h -ne [IntPtr]::Zero) { "  $class visible=$([Diag]::IsWindowVisible($h)) $([Diag]::Rect($h))" } else { "  $class not found" }
}
""

"== UI Automation elements in the notification area =="
$A = [System.Windows.Automation.AutomationElement]
$tray = [Diag]::FindWindowEx($taskbar, [IntPtr]::Zero, 'TrayNotifyWnd', $nul)
$trayRect = New-Object Diag+RECT
if ($tray -ne [IntPtr]::Zero) { [void][Diag]::GetWindowRect($tray, [ref]$trayRect) }
$walker = [System.Windows.Automation.TreeWalker]::RawViewWalker
$root = $A::FromHandle($taskbar)
$cache = New-Object System.Windows.Automation.CacheRequest
$cache.TreeFilter = [System.Windows.Automation.Automation]::RawViewCondition
$cache.Push()
try {
    $all = $root.FindAll([System.Windows.Automation.TreeScope]::Descendants, [System.Windows.Automation.Condition]::TrueCondition)
} finally {
    $cache.Pop()
}
foreach ($e in $all) {
    try {
        $c = $e.Current
        $r = $c.BoundingRectangle
        $inTray = $tray -ne [IntPtr]::Zero -and -not $r.IsEmpty -and ($r.Left + $r.Width / 2) -ge $trayRect.L -and ($r.Left + $r.Width / 2) -le $trayRect.R
        $named = $c.ClassName -match 'SystemTray|Chevron|Overflow|NotifyIcon|Notification' -or $c.AutomationId -match 'SystemTray|Chevron|Overflow|NotifyItem'
        if (-not ($inTray -or $named)) { continue }
        $rect = if ($r.IsEmpty) { '[empty]' } else { "[{0},{1},{2},{3}]" -f [int]$r.Left, [int]$r.Top, [int]$r.Right, [int]$r.Bottom }
        "  {0,-36} id={1,-24} {2,-12} {3} '{4}'" -f $c.ClassName, $c.AutomationId, ($c.ControlType.ProgrammaticName -replace 'ControlType\.', ''), $rect, ($c.Name -replace "`r?`n", ' | ')
    } catch {}
}
""

"== Windhawk mods =="
$mods = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
if (Test-Path $mods) {
    foreach ($m in Get-ChildItem $mods) {
        $p = Get-ItemProperty $m.PSPath
        "  {0,-45} version={1,-6} disabled={2}" -f $m.PSChildName, $p.Version, $p.Disabled
    }
    $settings = Get-ChildItem $mods | Where-Object { $_.PSChildName -like '*taskbar-tray-stays-visible*' } | Select-Object -First 1
    if ($settings) {
        $s = Get-ItemProperty (Join-Path $settings.PSPath 'Settings') -ErrorAction SilentlyContinue
        "  settings: onlyDateTime=$($s.onlyDateTime) overflowKeepsTaskbarHidden=$($s.overflowKeepsTaskbarHidden) revealTrayAtEdge=$($s.revealTrayAtEdge) cornerHidesTray=$($s.cornerHidesTray)"
    }
} else {
    "  (no Windhawk registry key; portable install?)"
}
$loaded = Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object { $_.Modules } | Where-Object { $_.FileName -like '*taskbar-tray-stays-visible*' } | Select-Object -ExpandProperty FileName
"  loaded in Explorer: $(if ($loaded) { Split-Path $loaded -Leaf } else { 'no' })"
