#!/bin/bash
# Waits for a test program to end after its run is over (tools/ci_tests.sh): usage  ci_wait_exit.sh <pid> <part id>
# The run is over when the program has written <id>.done (JP_DONE_FILE, tests/testrunner.cpp). The program is given up (exit status 1, the evidence
# printed first) only when it has used NO CPU for CI_EXIT_IDLE_POLLS polls in a row (a program that is slow to end but working is waited for: a
# loaded machine stretches a wall-clock limit), or is still there CI_EXIT_CAP_POLLS polls after its run ended (10 minutes at the defaults).
# Exit status 0: the program ended.
pid=$1; id=$2
poll=${CI_EXIT_POLL:-2}; idle_limit=${CI_EXIT_IDLE_POLLS:-45}; cap=${CI_EXIT_CAP_POLLS:-300}
os=$(uname -s)

alive() {
    kill -0 "$pid" 2>/dev/null || return 1
    # (a program that has ended and is not yet collected shows as a zombie on Linux)
    if [ "$os" = Linux ]; then [ "$(awk '{print $3}' /proc/"$pid"/stat 2>/dev/null)" != Z ] || return 1; fi
    return 0
}

winpid() { cat /proc/"$pid"/winpid 2>/dev/null || echo "$pid"; }

# The CPU time the program has used, in hundredths of a second; nothing when it cannot be read.
cpu() {
    case $os in
    Linux) awk '{print $14 + $15}' /proc/"$pid"/stat 2>/dev/null ;;
    Darwin) ps -o time= -p "$pid" 2>/dev/null | awk '{ n = split($1, a, ":"); h = (n > 2) ? a[n-2] : 0; printf "%d\n", ((h * 60 + a[n-1]) * 60 + a[n]) * 100 }' ;;
    *) powershell -NoProfile -Command "[math]::Round((Get-Process -Id $(winpid)).TotalProcessorTime.TotalMilliseconds / 10)" 2>/dev/null | tr -d '\r' ;;
    esac
}

evidence() {
    echo "--- $id: what process $pid is doing ($os)"
    case $os in
    Darwin)
        sample "$pid" 5 -file "$id.sample.txt" > /dev/null 2>&1
        head -150 "$id.sample.txt" 2>/dev/null || echo "(sample gave nothing)" ;;
    Linux)
        if command -v gdb > /dev/null 2>&1; then
            timeout -k 5 60 gdb -q -batch -p "$pid" -ex "thread apply all bt 30" 2>&1 | grep -v '^\[' | head -200
        else
            for t in /proc/"$pid"/task/*; do echo "thread ${t##*/} $(cat "$t"/comm 2>/dev/null) state $(awk '{print $3}' "$t"/stat 2>/dev/null) wchan $(cat "$t"/wchan 2>/dev/null)"; done
        fi ;;
    *)
        w=$(winpid)
        powershell -NoProfile -Command "\$q = Get-Process -Id $w; 'threads=' + \$q.Threads.Count + ' handles=' + \$q.HandleCount + ' cpu=' + \$q.TotalProcessorTime; \$q.Threads | ForEach-Object { 'tid=' + \$_.Id + ' state=' + \$_.ThreadState + ' wait=' + \$_.WaitReason + ' start=0x' + \$_.StartAddress.ToString('x') }" 2>&1 | tr -d '\r' | head -60
        dump=$(cygpath -w "$PWD/$id.dmp" 2>/dev/null || echo "$id.dmp")
        powershell -NoProfile -Command "\$j = Start-Process -FilePath rundll32.exe -ArgumentList 'C:\\Windows\\System32\\comsvcs.dll, MiniDump $w $dump full' -PassThru -WindowStyle Hidden; if (-not \$j.WaitForExit(60000)) { Stop-Process -Id \$j.Id -Force }; if (Test-Path '$dump') { 'minidump: ' + (Get-Item '$dump').Length + ' bytes in $dump' } else { 'no minidump' }" 2>&1 | tr -d '\r' ;;
    esac
}

last=""; same=0; n=0; reason=""
while alive; do
    if [ -f "$id.done" ]; then
        n=$((n + 1))
        c=$(cpu)
        if [ -n "$c" ] && [ "$c" = "$last" ]; then same=$((same + 1)); else same=0; fi
        last=$c
        if [ "$same" -ge "$idle_limit" ]; then reason="used no CPU for $same polls in a row (${poll} s each)"; break; fi
        if [ "$n" -ge "$cap" ]; then reason="still there $n polls (${poll} s each) after its run ended"; break; fi
    fi
    sleep "$poll"
done
alive || exit 0
evidence
echo "$id: STUCK AT EXIT: the run finished (exit code $(cat "$id.done")) and process $pid $reason"
exit 1
