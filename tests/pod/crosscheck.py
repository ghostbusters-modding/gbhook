#!/usr/bin/env python3
# gbhook: SPDX-License-Identifier: GPL-2.0-only
# The double-confirm for pod/Pod: our C++ writer's output, read by gbtvgr-py's engine-verified reader.
import os, sys
pod_file, gbpy = sys.argv[1], sys.argv[2]
sys.path.insert(0, os.path.join(gbpy, "src"))
from gbtvgr.pod import Pod

expect = {
    "world\\haunt1.lvl": b"level bytes here",
    "art\\logo.tex": b"",
    "data\\scripts\\boot.dante": b"0123456789ABCDEF0123",
}
p = Pod(pod_file)
assert p.revision == 1, p.revision
assert p.next_pod == "MODS.POD", repr(p.next_pod)
assert p.names() == list(expect), p.names()
for n, want in expect.items():
    assert p.read(p.entry(n)) == want, n
    assert p.entry(n)["method"] == 0, n      # stored, never a compression level
p.close()
print("podcheck OK: engine-verified reader accepts our writer (%d entries)" % len(expect))
