#!/usr/bin/env bash
# gbhook harness -- run from WSL. gbhook.log and gbhook.cmd next to ghost.exe are the channel.
#   gb.sh build | install | start | stop | status | cmd "<line>" | wait "<re>" [sec]
#         | log [n] | shot [name] | keys <step ...> | post <KEY|'wait N'> ...
#         | menuskip | unlock | clean
set -uo pipefail

REPO_WSL="${GBHOOK_REPO:-/mnt/x/ghostbusters-modding/new/gbhook}"
REPO_WIN='X:\ghostbusters-modding\new\gbhook'
TOOLS_WIN='X:\ghostbusters-modding\new\gbhook\tools\harness'
GAME_WSL="/mnt/x/SteamLibrary/steamapps/common/Ghostbusters The Video Game Remastered"
MSBUILD="/mnt/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/amd64/MSBuild.exe"
APPID=1449280

DLL_SRC="$REPO_WSL/build/x64/Release/dinput8.dll"
DLL_DST="$GAME_WSL/dinput8.dll"
LOG="$GAME_WSL/gbhook.log"
CMDF="$GAME_WSL/gbhook.cmd"
HDIR="$REPO_WSL/.harness"
MARK="$HDIR/logmark"
KEEPAWAKE="$HDIR/keepawake.ps1"
LOCK="$HDIR/game.lock"
OWNER="${GB_OWNER:-unknown}"
mkdir -p "$HDIR"

PS() { powershell.exe -NoProfile -NonInteractive -ExecutionPolicy Bypass "$@" 2>/dev/null | tr -d '\r'; }
loglines() { [ -f "$LOG" ] && wc -l < "$LOG" | tr -d ' ' || echo 0; }
running()  { tasklist.exe /FI "IMAGENAME eq ghost.exe" /NH 2>/dev/null | tr -d '\r' | grep -qi '^ghost.exe'; }
gpid()     { tasklist.exe /FI "IMAGENAME eq ghost.exe" /NH 2>/dev/null | tr -d '\r' | awk '/^ghost.exe/{print $2; exit}'; }
setmark()  { echo "${1:-$(loglines)}" > "$MARK"; }

# ---- mutual exclusion: only one game instance may exist at a time ----------
acquire_lock() {
  if [ -f "$LOCK" ]; then
    if running; then
      echo "LOCKED: the game is already owned by another harness user:"
      sed 's/^/  /' "$LOCK"
      echo "  (ghost.exe pid $(gpid)).  Use 'gb.sh stop' if it is yours, or 'gb.sh unlock' to clear a stale lock."
      return 1
    fi
    echo "note: stale lock from $(head -1 "$LOCK" 2>/dev/null) -- ghost.exe is not running, taking it over"
  fi
  { echo "owner=$OWNER"; echo "shellpid=$$"; echo "acquired=$(date -Iseconds)"; } > "$LOCK"
  return 0
}

# ---------------------------------------------------------------- subcommands
do_build() {
  "$MSBUILD" "$REPO_WIN\\GbHook.vcxproj" \
    -p:Configuration=Release -p:Platform=x64 -v:minimal -nologo
  rc=$?
  if [ $rc -ne 0 ]; then echo "BUILD FAILED (exit $rc)"; exit $rc; fi
  echo "build ok: $(md5sum "$DLL_SRC" | cut -c1-32)"
}

do_install() {
  [ -f "$DLL_SRC" ] || { echo "no built dll at $DLL_SRC"; exit 1; }
  if running; then
    echo "ghost.exe running (pid $(gpid)) -- renaming loaded dll"
    rm -f "$GAME_WSL/dinput8.dll.bak"
    mv "$DLL_DST" "$GAME_WSL/dinput8.dll.bak" || { echo "rename failed"; exit 1; }
  fi
  cp "$DLL_SRC" "$DLL_DST" || { echo "copy failed"; exit 1; }
  a=$(md5sum "$DLL_SRC" | cut -d' ' -f1); b=$(md5sum "$DLL_DST" | cut -d' ' -f1)
  [ "$a" = "$b" ] || { echo "MD5 MISMATCH $a != $b"; exit 1; }
  echo "installed, md5 $a"
}

start_keepawake() {
  cat > "$KEEPAWAKE" <<'PS1'
Add-Type -MemberDefinition '[DllImport("kernel32.dll")] public static extern uint SetThreadExecutionState(uint e);' -Name P -Namespace KA
while ($true) { [void][KA.P]::SetThreadExecutionState(0x80000000 -bor 0x00000002 -bor 0x00000001); Start-Sleep -Seconds 30 }
PS1
  # Start-Process returns immediately; `cmd /c start` blocks WSL interop forever.
  PS -Command "Start-Process -FilePath 'powershell.exe' -WindowStyle Hidden -ArgumentList '-NoProfile','-WindowStyle','Hidden','-ExecutionPolicy','Bypass','-File','$REPO_WIN\\.harness\\keepawake.ps1'" >/dev/null
  echo "keep-awake started"
}

launch_allowed() {
  if [ "${GB_ALLOW_LAUNCH:-0}" != "1" ]; then
    echo "launching is disabled (set GB_ALLOW_LAUNCH=1 to allow start/boot/resume)"
    return 1
  fi
  return 0
}

do_start() {
  launch_allowed || return 1
  if running; then
    echo "already running (pid $(gpid))"
    [ -f "$LOCK" ] && sed 's/^/  lock: /' "$LOCK"
    return 0
  fi
  acquire_lock || return 1
  start_keepawake
  PS -Command "Start-Process 'steam://rungameid/$APPID'" >/dev/null
  for i in $(seq 1 30); do sleep 2; running && { echo "pid=$(gpid)" >> "$LOCK"; echo "started via steam (pid $(gpid))"; return 0; }; done
  echo "steam:// did not start the game; launching ghost.exe directly"
  PS -Command "Start-Process -FilePath 'X:\\SteamLibrary\\steamapps\\common\\Ghostbusters The Video Game Remastered\\ghost.exe' -WorkingDirectory 'X:\\SteamLibrary\\steamapps\\common\\Ghostbusters The Video Game Remastered'" >/dev/null
  for i in $(seq 1 15); do sleep 2; running && { echo "pid=$(gpid)" >> "$LOCK"; echo "started directly (pid $(gpid))"; return 0; }; done
  rm -f "$LOCK"; echo "FAILED to start"; return 1
}

do_stop() {
  taskkill.exe /F /IM ghost.exe >/dev/null 2>&1 && echo "ghost.exe killed" || echo "ghost.exe not running"
  PS -Command "Get-CimInstance Win32_Process -Filter \"Name='powershell.exe'\" | Where-Object { \$_.CommandLine -like '*keepawake*' } | ForEach-Object { Stop-Process -Id \$_.ProcessId -Force }" >/dev/null
  echo "keep-awake stopped"
  rm -f "$LOCK"; echo "lock released"
}

do_status() {
  if running; then echo "ghost.exe RUNNING pid=$(gpid)"; else echo "ghost.exe not running"; fi
  if [ -f "$LOCK" ]; then sed 's/^/lock: /' "$LOCK"; else echo "lock: none"; fi
  echo "log: $(loglines) lines"
  [ -f "$LOG" ] && tail -3 "$LOG" | tr -d '\r'
}

# Sends one line and prints what the game wrote back, up to its OK or ERR line (10 s, or $2 seconds).
do_cmd() {
  [ -n "${1:-}" ] || { echo "usage: gb.sh cmd \"<line>\" [timeoutSec]"; exit 2; }
  local line="$1" to="${2:-10}"
  local from; from=$(loglines)
  setmark "$from"
  printf '%s\n' "$line" >> "$CMDF"
  running || { echo "sent: $line  (ghost.exe is not running; it runs at the next launch)"; return 0; }
  local deadline=$(( $(date +%s) + to )) shown=0
  while :; do
    if [ -f "$LOG" ]; then
      local n; n=$(loglines)
      if [ "$n" -gt "$((from + shown))" ]; then
        tail -n "+$((from + shown + 1))" "$LOG" | tr -d '\r' | while IFS= read -r l; do
          case "$l" in
            *"] CMD  $line"|*"] RES  "*|*"] ERR  $line"*|*"] OK   $line") echo "$l" ;;
          esac
        done
        if tail -n "+$((from + 1))" "$LOG" | tr -d '\r' | grep -q -E -- "\] (OK   |ERR  )$(printf '%s' "$line" | sed 's/[][\.*^$]/\\&/g')"; then
          setmark "$n"; return 0
        fi
        shown=$((n - from))
      fi
    fi
    [ "$(date +%s)" -ge "$deadline" ] && { echo "no OK/ERR for '$line' within ${to}s (see gb.sh log)"; return 1; }
    sleep 0.2
  done
}

do_wait() {
  local re="${1:?usage: gb.sh wait \"<regex>\" [timeoutSec]}" ; local to="${2:-60}"
  local from; from=$(tr -cd "0-9" < "$MARK" 2>/dev/null); [ -n "$from" ] || from=$(loglines)
  local now; now=$(loglines); [ "$from" -gt "$now" ] && from=$now
  local deadline=$(( $(date +%s) + to ))
  while :; do
    if [ -f "$LOG" ]; then
      local hit
      hit=$(tail -n "+$((from+1))" "$LOG" | tr -d '\r' | grep -n -E -- "$re" | head -1)
      if [ -n "$hit" ]; then
        local off=${hit%%:*}
        setmark $((from + off))
        echo "MATCH ${hit#*:}"
        return 0
      fi
    fi
    [ "$(date +%s)" -ge "$deadline" ] && { echo "TIMEOUT after ${to}s waiting for /$re/"; return 1; }
    sleep 1
  done
}

do_log() { local n="${1:-20}"; [ -f "$LOG" ] && tail -n "$n" "$LOG" | tr -d '\r' || echo "(no log)"; }

do_shot() {
  local name="${1:-shot}"
  PS -File "$TOOLS_WIN\\shot.ps1" -Out "$REPO_WIN\\.harness\\$name.png"
  echo "-> $HDIR/$name.png"
}

do_keys() {
  [ $# -gt 0 ] || { echo "usage: gb.sh keys <step ...>   step = 'wait N' | 'tap KEY' | 'spam KEY N' | 'hold KEY N'"; exit 2; }
  # Steps become `key` and `sleep` lines and go as ONE batch, so the sequence runs back-to-back on the loop
  # thread. No window focus is involved: the keys land in the engine's own scan-code table.
  local out="" step op k n
  for step in "$@"; do
    set -- $step; op="$1"; k="${2:-}"; n="${3:-}"
    case "$op" in
      wait)  out+="sleep $(( ${k%.*} * 1000 ))"$'\n' ;;
      tap)   out+="key tap $k"$'\n' ;;
      down)  out+="key down $k"$'\n' ;;
      up)    out+="key up $k"$'\n' ;;
      hold)  out+="key hold $k $(( ${n%.*} * 1000 ))"$'\n' ;;
      spam)  out+="key spam $k $(( ${n%.*} * 1000 ))"$'\n' ;;
      clear) out+="key clear"$'\n' ;;
      focus) : ;;   # no longer needed
      *) echo "unknown step '$step'"; return 2 ;;
    esac
  done
  setmark
  printf '%s' "$out" >> "$CMDF"
  printf '%s' "$out" | sed 's/^/sent: /'
}

# Front-end MENU navigation. The in-process scan-table injection (`keys`)
# drives ENTER, ESC and gameplay movement, but the title menus read the
# arrows through the window's own WM_KEYDOWN handler, so those must arrive
# as real window messages. PostMessage needs no focus. Steps: UP DOWN LEFT
# RIGHT ENTER ESCAPE SPACE | "wait <ms>".
do_post() {
  [ $# -gt 0 ] || { echo "usage: gb.sh post <KEY|'wait N'> ..."; exit 2; }
  # -File binds a string[] parameter from ONE comma-separated token.
  local joined; joined=$(IFS=,; echo "$*")
  PS -File "$TOOLS_WIN\\postkey.ps1" -Keys "$joined"
}

# Confirm through the "press ENTER" and continue prompts at the front end.
do_menuskip() {
  do_keys "tap ENTER" "tap ESC" "tap ENTER" "tap ESC" "tap ESC" "spam ENTER 5"
  echo "menu skip sequence sent"
}

do_clean() { rm -f "$CMDF" "$GAME_WSL/gbhook.cmd.run" "$MARK"; echo "cleaned cmd/mark"; }

case "${1:-}" in
  build)   shift; do_build "$@" ;;
  install) shift; do_install "$@" ;;
  start)   shift; do_start "$@" ;;
  stop)    shift; do_stop "$@" ;;
  status)  shift; do_status "$@" ;;
  cmd)     shift; do_cmd "$@" ;;
  wait)    shift; do_wait "$@" ;;
  log)     shift; do_log "$@" ;;
  shot)    shift; do_shot "$@" ;;
  keys)    shift; do_keys "$@" ;;
  post)    shift; do_post "$@" ;;
  clean)   shift; do_clean "$@" ;;
  menuskip) shift; do_menuskip "$@" ;;
  unlock)  rm -f "$LOCK"; echo "lock cleared" ;;
  mark)    setmark; echo "mark=$(cat "$MARK")" ;;
  *) sed -n '2,5p' "$0" | sed 's/^# \?//' ;;
esac
