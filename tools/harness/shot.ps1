param(
  [string]$Out = "C:\Users\forum\Desktop\cod\ghostbusters-modding\gbtvgr-playground\.harness\shot.png",
  [switch]$NoFocus
)
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
$sig = @'
[DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
[DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, System.Text.StringBuilder s, int n);
[DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
[DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
[DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, IntPtr p);
[DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
[DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
'@
$U = Add-Type -MemberDefinition $sig -Name U2 -Namespace W -PassThru

# Raise the game window: a desktop grab only ever shows whatever is on top.
if (-not $NoFocus) {
  $p = Get-Process ghost -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
  if ($p) {
    $h = $p.MainWindowHandle
    [void]$U::ShowWindow($h, 9)            # SW_RESTORE
    $me = $U::GetCurrentThreadId(); $them = $U::GetWindowThreadProcessId($h, [IntPtr]::Zero)
    [void]$U::AttachThreadInput($me, $them, $true)
    [void]$U::BringWindowToTop($h); [void]$U::SetForegroundWindow($h)
    [void]$U::AttachThreadInput($me, $them, $false)
    Start-Sleep -Milliseconds 700
  }
}

$b = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bmp = New-Object System.Drawing.Bitmap $b.Width, $b.Height
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($b.Location, [System.Drawing.Point]::Empty, $b.Size)
# downscale to keep the PNG small for reading
$w = [int]($b.Width / 2); $h2 = [int]($b.Height / 2)
$small = New-Object System.Drawing.Bitmap $w, $h2
$g2 = [System.Drawing.Graphics]::FromImage($small); $g2.DrawImage($bmp, 0, 0, $w, $h2)
$small.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$sb = New-Object System.Text.StringBuilder 256; [void]$U::GetWindowText($U::GetForegroundWindow(), $sb, 256)
"saved $Out  screen=$($b.Width)x$($b.Height)  foreground='$($sb.ToString())'"
