#!/usr/bin/env python3
"""Compile .po files to UE .locres binary format.
Usage: python compile_locres.py
Generates .locres next to each .po file.
"""
import struct
import os
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
        # Header
        f.write(struct.pack('<B', 0x0E))        # Magic
        f.write(struct.pack('<I', 1))            # Version
        f.write(struct.pack('<I', 1))            # StringTableCount = 1
        # StringTable name
        write_fstring(f, 'RTXGI')
        # Entry count
        f.write(struct.pack('<I', len(sorted_entries)))
        # Entries
        for (ctx, src), dst in sorted_entries:
            key = f'{ctx}\x00{src}'
            write_fstring(f, key)
            f.write(struct.pack('<I', zlib.crc32(src.encode('utf-8')) & 0xFFFFFFFF))
            write_fstring(f, dst)

base = os.path.dirname(os.path.abspath(__file__))
for fname in os.listdir(base):
    if fname.endswith('.po'):
        po_path = os.path.join(base, fname)
        locres_path = po_path.rsplit('.', 1)[0] + '.locres'
        entries = read_po(po_path)
        if entries:
            write_locres(locres_path, entries)
            print(f'{fname} -> {os.path.basename(locres_path)} ({len(entries)} entries)')
