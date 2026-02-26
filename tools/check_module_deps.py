#!/usr/bin/env python3
from pathlib import Path
import re, sys

root = Path(__file__).resolve().parents[1]
inc = root / 'include' / 'geo'
pat = re.compile(r'#include\s+"([^"]+)"')

errors = []
for h in (inc / 'core').glob('*.h'):
    for m in pat.findall(h.read_text()):
        if m.startswith('geo/crs/') or m.startswith('geo/index/') or m.startswith('geo/vector/'):
            errors.append(f'{h}: forbidden include {m}')

for mod in ['crs','index','vector']:
    for h in (inc / mod).glob('*.h'):
        for m in pat.findall(h.read_text()):
            if m.startswith('geo/crs/') or m.startswith('geo/index/') or m.startswith('geo/vector/'):
                if not (mod == 'vector' and m.startswith('geo/crs/')):
                    errors.append(f'{h}: cross-module include {m}')

if errors:
    print('\n'.join(errors))
    sys.exit(1)
print('module dependency check passed')
