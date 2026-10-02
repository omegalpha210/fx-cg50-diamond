#!/usr/bin/env python3
"""Report exact SH ELF sections and compiler stack frames, without hardware claims."""
import hashlib
import json
from pathlib import Path
import subprocess
import re

ROOT=Path(__file__).resolve().parents[1]
ELF=ROOT/'build/target/diamond'
PACKAGE=ROOT/'dist/DIAMOND.g3a'

def run(*args):
    return subprocess.check_output(args,text=True)

def main():
    sizes=run('sh-elf-size','-A',str(ELF))
    sections={}
    for line in sizes.splitlines():
        match=re.match(r'^(\.\S+)\s+(\d+)\s+\d+$',line)
        if match:sections[match[1]]=int(match[2])
    nm=run('sh-elf-nm','-S',str(ELF))
    symbols={}
    for line in nm.splitlines():
        values=line.split()
        if len(values)==4:symbols[values[3]]=int(values[1],16)
    frames=[]
    for source in sorted((ROOT/'build/target').rglob('*.su')):
        for line in source.read_text().splitlines():
            location,size,kind=line.rsplit('\t',2)
            file,lineno,column,function=location.rsplit(':',3)
            path=str(Path(file).relative_to(ROOT))
            frames.append(dict(file=path,line=int(lineno),function=function,bytes=int(size),kind=kind))
    frames.sort(key=lambda f:(-f['bytes'],f['function']))
    report={
        'package_bytes':PACKAGE.stat().st_size,
        'package_sha256':hashlib.sha256(PACKAGE.read_bytes()).hexdigest(),
        'sections':{k:v for k,v in sections.items() if not k.startswith('.debug')},
        'ai_workspace_bytes':symbols['_ai']+symbols['_distances_ready']+symbols['_endgame_cache'],
        'ai_transposition_table_bytes':0,
        'controller_bytes':symbols['_app'],
        'ui_pre_move_board_bytes':73,
        'ui_trail_bytes':77,
        'ui_two_trails_bytes':154,
        'ui_new_visual_fields_bytes':231,
        'save_max_payload_bytes':208,
        'save_decoder_limit_bytes':256,
        'largest_own_stack_frame':frames[0],
        'own_stack_frames':frames,
        'native_framebuffers':1,
        'native_framebuffer_pixel_bytes':396*224*2,
        'gint_framebuffer_allocation_bytes':396*224*2+96,
        'limitations':'Compiler frame sizes are not total stack high water. gint/OS/libc frames and runtime heap are not fully measured; hardware retest required.'
    }
    destination=ROOT/'docs/validation/memory.json'
    destination.parent.mkdir(parents=True,exist_ok=True)
    destination.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k!='own_stack_frames'},indent=2))

if __name__=='__main__':main()
