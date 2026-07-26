#!/usr/bin/env python3
"""Compile .po files to UE .locres binary format.
Usage: python Scripts/compile_locres.py
Generates .locres under Content/Localization/<TargetName>/<locale>/
"""
import struct
import os
import re
import zlib


def read_po(path):
    entries = {}
    ctx = msgid = msgstr = None
    with open(path, encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if line.startswith('msgctxt'):
                ctx = line[9:-1]
            elif line.startswith('msgid '):
                msgid = line[7:-1]
            elif line.startswith('msgstr '):
                msgstr = line[8:-1]
                if ctx and msgid and msgstr and ctx != '""':
                    entries[(ctx, msgid)] = msgstr
                ctx = msgid = msgstr = None
    return entries


def write_fstring(f, s):
    """Write UE FString (int32 len, UTF-16LE chars, null terminator)."""
    encoded = s.encode('utf-16-le')
    f.write(struct.pack('<i', len(s)))
    f.write(encoded)
    f.write(b'\x00\x00')


def write_locres(path, entries):
    sorted_entries = sorted(entries.items(), key=lambda x: x[0][0] + x[0][1])
    with open(path, 'wb') as f:
        f.write(struct.pack('<B', 0x0E))        # Magic
        f.write(struct.pack('<I', 1))            # Version
        f.write(struct.pack('<I', 1))            # StringTableCount
        write_fstring(f, 'RTXGI')
        f.write(struct.pack('<I', len(sorted_entries)))
        for (ctx, src), dst in sorted_entries:
            key = f'{ctx}\x00{src}'
            write_fstring(f, key)
            f.write(struct.pack('<I', zlib.crc32(src.encode('utf-8')) & 0xFFFFFFFF))
            write_fstring(f, dst)


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    loc_dir = os.path.join(root, 'Localization')
    for fname in os.listdir(loc_dir):
        if not fname.endswith('.po'):
            continue
        m = re.match(r'^([\w]+)\.([\w-]+)\.po$', fname)
        if not m:
            continue
        target, locale = m.group(1), m.group(2)
        po_path = os.path.join(loc_dir, fname)
        out_dir = os.path.join(root, 'Content', 'Localization', target, locale)
        os.makedirs(out_dir, exist_ok=True)
        out_path = os.path.join(out_dir, f'{target}.locres')
        entries = read_po(po_path)
        if entries:
            write_locres(out_path, entries)
            print(f'{fname} -> Content/Localization/{target}/{locale}/{target}.locres ({len(entries)} entries)')


if __name__ == '__main__':
    main()
