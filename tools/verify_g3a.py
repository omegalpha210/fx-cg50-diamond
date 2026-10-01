#!/usr/bin/env python3
"""Check DIAMOND packaging against installed fxSDK's g3a.h/util.c layout.
Packaging only: calculator execution and performance require hardware.
"""
from pathlib import Path
import hashlib
import struct
import sys

def verify(path):
    raw=Path(path).read_bytes()
    assert len(raw)>=0x7114, 'truncated G3A'
    header=bytes(x^255 for x in raw[:32])
    word=lambda data,offset:struct.unpack_from('>I',data,offset)[0]
    checks={
        'magic':header[:8]==b'USBPower',
        'type':header[8]==0x2c,
        'signature':header[9:14]==bytes((0,1,0,1,0)),
        'name':raw[0x40:0x50].split(b'\0')[0]==b'DIAMOND',
        'internal ID':raw[0x60:0x6b].split(b'\0')[0]==b'@DIAMOND',
        'size1':word(header,0x10)==len(raw),
        'size2':word(raw,0x2e)==len(raw)-0x7004,
        'size3':word(raw,0x5c)==len(raw),
        'control1':header[0x0e]==(len(raw)+0x41)&255,
        'control2':header[0x14]==(len(raw)+0xb8)&255,
        'SH entry checksum':struct.unpack_from('>H',header,0x16)[0]==sum(struct.unpack_from('>8H',raw,0x7100))&65535,
        'container checksum':word(raw,0x20)==(sum(raw[:32])+sum(raw[0x24:-4]))&0xffffffff,
        'footer':raw[0x20:0x24]==raw[-4:],
        'unselected icon':len(set(raw[0x1000:0x1000+92*64*2]))>1,
        'selected icon':len(set(raw[0x4000:0x4000+92*64*2]))>1,
        'public path hygiene':(b'/' + b'Users' + b'/') not in raw,
    }
    failed=[name for name,ok in checks.items() if not ok]
    if failed:raise ValueError('G3A failed: '+', '.join(failed))
    print(f'{path}: {len(raw)} bytes, {len(checks)} package checks PASS')
    print('SHA256 '+hashlib.sha256(raw).hexdigest())
    print('HARDWARE TEST REQUIRED')

if __name__=='__main__':
    for item in sys.argv[1:]:verify(item)
