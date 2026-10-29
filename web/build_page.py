# PlatformIO pre-build step: gzips web/setup.html into a C array that the
# firmware serves as the setup page (setup_page.h in the build directory).
import gzip
import os

Import("env")

source = os.path.join(env["PROJECT_DIR"], "web", "setup.html")
out_dir = os.path.join(env.subst("$BUILD_DIR"), "generated")
os.makedirs(out_dir, exist_ok=True)

with open(source, "rb") as f:
    data = gzip.compress(f.read(), compresslevel=9, mtime=0)

lines = []
for i in range(0, len(data), 16):
    lines.append("  " + ", ".join("0x%02x" % b for b in data[i:i + 16]) + ",")

header = (
    "#pragma once\n"
    "// Generated from web/setup.html by web/build_page.py. Do not edit.\n"
    "#include <stddef.h>\n"
    "#include <stdint.h>\n"
    "static const uint8_t SETUP_PAGE_GZ[] = {\n" + "\n".join(lines) + "\n};\n"
    "static const size_t SETUP_PAGE_GZ_LEN = sizeof(SETUP_PAGE_GZ);\n"
)

out = os.path.join(out_dir, "setup_page.h")
if not os.path.exists(out) or open(out).read() != header:
    with open(out, "w") as f:
        f.write(header)

env.Append(CPPPATH=[out_dir])
