import os
from os.path import isfile
Import("env")

print("📦 Running env-extra.py...")

if not isfile(".env"):
    print("⚠️  .env file not found")
else:
    with open(".env") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            key, val = line.split("=", 1)
            # Escape backslashes and double quotes properly
            escaped = val.replace("\\", "\\\\").replace('"', '\\"')
            flag = f'-D{key}="{escaped}"'
            env.Append(BUILD_FLAGS=[flag])
            print(f'✅ Injecting macro: {flag}')