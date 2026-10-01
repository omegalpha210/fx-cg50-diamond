/* Read-only scalar OS queries, following the verified NUM GAME pattern.
   libfxcg syscall IDs: char backlight duration 0x12d9 (30-second units);
   int APO minutes 0x1e91. main.c preserves these return types.
   No OS settings writes or hard-coded OS data addresses. */
.text
.align 2
.global _dg_os_backlight_duration
_dg_os_backlight_duration:
 mov.l .Lbl,r0
 mov.l .Lsys,r2
 jmp @r2
 nop
.global _dg_os_apo_minutes
_dg_os_apo_minutes:
 mov.l .Lapo,r0
 mov.l .Lsys,r2
 jmp @r2
 nop
.align 2
.Lbl: .long 0x12d9
.Lapo: .long 0x1e91
.Lsys: .long 0x80020070
