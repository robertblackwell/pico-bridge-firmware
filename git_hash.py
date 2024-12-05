#!/usr/bin/python3
import subprocess
import sys
import os
result = subprocess.run(["git", "branch"], stdout=subprocess.PIPE)
r = [s for s in result.stdout.decode("utf-8").split("\n") if s != "" and s[0] == "*"]
if len(r) != 1:
    raise RuntimeError("did not find active branch")

branch = r[0][2:]
hashbin = subprocess.run(["git", "rev-parse", branch], stdout=subprocess.PIPE).stdout
hash = hashbin.replace(b"\n", b"").decode("utf-8")
ghash = f"{branch}:{hash}"

version_file = open(os.path.join(os.path.dirname(__file__),"src/common/version.h"), "w")
version_file.write(f"#define VERSION_NUMBER \"{ghash}\"")
version_file.close()