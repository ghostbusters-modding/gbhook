param([string[]]$Keys)
$sig = '[DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);'
$U = Add-Type -MemberDefinition $sig -Name PK -Namespace W -PassThru
$p = Get-Process ghost -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { Write-Output "no ghost window"; exit 1 }
$h = $p.MainWindowHandle
$VKMAP = @{ UP=0x26; DOWN=0x28; LEFT=0x25; RIGHT=0x27; ENTER=0x0D; ESCAPE=0x1B; SPACE=0x20; INSERT=0x2D }
$SCMAP = @{ UP=0x48; DOWN=0x50; LEFT=0x4B; RIGHT=0x4D; ENTER=0x1C; ESCAPE=0x01; SPACE=0x39; INSERT=0x52 }
$EXT = @{ UP=1; DOWN=1; LEFT=1; RIGHT=1; INSERT=1 }
# -File hands the whole -Keys value over as one string; split it ourselves.
$list = @(); foreach ($k in $Keys) { $list += ($k -split ',') }
foreach ($k in $list) {
  if ($k -match '^wait (\d+)$') { Start-Sleep -Milliseconds ([int]$Matches[1]); continue }
  if ($k.Length -eq 1 -and $k -match '[A-Za-z0-9]') {
    $up = $k.ToUpper(); $vk = [int][char]$up
    $letters = 'QWERTYUIOPASDFGHJKLZXCVBNM'; $scs = 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32
    $i = $letters.IndexOf($up); if ($i -ge 0) { $sc = $scs[$i] } else { $sc = 0x02 + (([int][char]$up - 49 + 10) % 10) }
    $ext = 0
    $ldown = [IntPtr](1 -bor ($sc -shl 16)); $lup = [IntPtr](1 -bor ($sc -shl 16) -bor (1 -shl 30) -bor (1 -shl 31))
    [void]$U::PostMessage($h, 0x100, [IntPtr]$vk, $ldown); Start-Sleep -Milliseconds 60
    [void]$U::PostMessage($h, 0x101, [IntPtr]$vk, $lup); Write-Output "posted $k"; Start-Sleep -Milliseconds 120; continue
  }
  $vk = $VKMAP[$k]; $sc = $SCMAP[$k]; if ($null -eq $vk) { Write-Output ("unknown [" + $k + "] len=" + $k.Length); continue }
  $ext = 0; if ($EXT[$k]) { $ext = 1 }
  $ldown = [IntPtr](1 -bor ($sc -shl 16) -bor ($ext -shl 24))
  $lup   = [IntPtr](1 -bor ($sc -shl 16) -bor ($ext -shl 24) -bor (1 -shl 30) -bor (1 -shl 31))
  [void]$U::PostMessage($h, 0x100, [IntPtr]$vk, $ldown)
  Start-Sleep -Milliseconds 80
  [void]$U::PostMessage($h, 0x101, [IntPtr]$vk, $lup)
  Write-Output "posted $k"
  Start-Sleep -Milliseconds 250
}
