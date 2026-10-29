# PlatformIO pre-build step: gzips each web/<name>.html into a C array that
# the firmware serves (<name>_page.h in the build directory, array
# <NAME>_PAGE_GZ), and stamps the build date as FIRMWARE_DATE.
import datetime
import glob
import gzip
import os

Import("env")

web_dir = os.path.join(env["PROJECT_DIR"], "web")
out_dir = os.path.join(env.subst("$BUILD_DIR"), "generated")
os.makedirs(out_dir, exist_ok=True)

for source in sorted(glob.glob(os.path.join(web_dir, "*.html"))):
    name = os.path.splitext(os.path.basename(source))[0]
    array = name.upper() + "_PAGE_GZ"
    with open(source, "rb") as f:
        data = gzip.compress(f.read(), compresslevel=9, mtime=0)

    lines = []
    for i in range(0, len(data), 16):
        lines.append("  " + ", ".join("0x%02x" % b for b in data[i:i + 16]) + ",")

    header = (
        "#pragma once\n"
        "// Generated from web/%s.html by web/build_page.py. Do not edit.\n"
        "#include <stddef.h>\n"
        "#include <stdint.h>\n"
        "static const uint8_t %s[] = {\n%s\n};\n"
        "static const size_t %s_LEN = sizeof(%s);\n"
    ) % (name, array, "\n".join(lines), array, array)

    out = os.path.join(out_dir, name + "_page.h")
    if not os.path.exists(out) or open(out).read() != header:
        with open(out, "w") as f:
            f.write(header)

env.Append(CPPPATH=[out_dir])
env.Append(CPPDEFINES=[("FIRMWARE_DATE", env.StringifyMacro(datetime.date.today().strftime("%Y.%m.%d")))])
