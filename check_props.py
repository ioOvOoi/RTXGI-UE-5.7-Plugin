import re

with open('Source/RTXGI/Public/DDGIVolumeComponent.h', encoding='utf-8') as f:
    lines = f.readlines()

for i, line in enumerate(lines):
    if 'UPROPERTY(EditAnywhere' in line and 'DisplayName' not in line and 'DeprecatedProperty' not in line:
        if i+1 < len(lines):
            m = re.search(r'(\w+)\s*(=|;)', lines[i+1])
            if m:
                print(f'L{i+2}: {m.group(1):30s} | {line.strip()[:90]}')
