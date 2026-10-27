import os
from os.path import isfile
Import("env")

print("📦 Running env-extra.py...")


def mask_value(value):
    value = value.strip().strip('"').strip("'")
    if not value:
        return "(empty)"
    if len(value) <= 2:
        return "*" * len(value)
    return value[0] + ("*" * (len(value) - 2)) + value[-1]

if not isfile(".env"):
    print("⚠️  .env file not found")
else:
    with open(".env") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" not in line:
                print(f"⚠️  Skipping malformed .env line: {line}")
                continue
            key, val = line.split("=", 1)
            # Escape backslashes and double quotes properly
            escaped = val.replace("\\", "\\\\").replace('"', '\\"')
            flag = f'-D{key}="{escaped}"'
            env.Append(BUILD_FLAGS=[flag])
            print(f'✅ Injecting macro: -D{key}=<{mask_value(val)}>')
