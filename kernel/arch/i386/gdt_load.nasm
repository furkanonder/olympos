global gdt_load    ; make the label gdt_load visible outside this file
global tss_flush   ; make the label tss_flush visible outside this file

; gdt_load - Load the global descriptor table (GDT).
; stack: [esp + 4] the address of the first entry in the GDT
;        [esp    ] the return address
gdt_load:
    mov eax, [esp + 4]  ; load the address of the GDT into register eax
    lgdt [eax]          ; load the GDT

    ; 0x10 is our data segment register in the GDT.
    mov ax, 0x10
    ; The processor has six 16-bit segment registers: cs, ss, ds, es, gs and fs. The register cs is the code segment
    ; register and specifies the segment to use when fetching instructions. The register ss is used whenever accessing
    ; the stack (through the stack pointer esp), and ds is used for other data accesses. The OS is free to use the
    ; registers es, gs and fs however it want. Load all data segment registers.
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax

    ; To load cs we have to do a "far jump". A far jump is a jump where we explicitly specify the full 48-bit logical
    ; address: the segment selector to use and the absolute address to jump to. It will first set cs to 0x08 (0x08 is
    ; the code segment register in the GDT) and then jump to flush_cs using its absolute address.
    jmp 0x08:flush_cs
flush_cs:
    ; now we've changed cs to 0x08
    ret     ; return to the calling function

; tss_flush - Load the Task State Segment (TSS) into the Task Register.
; The TSS is at index 5 in the GDT.
;
; x86 Selector Format (16-bit):
;   Bits 15 - 3:    Index (13 bits) - GDT/LDT entry number
;   Bit  2:         TI (1 bit)      - Table Indicator (0=GDT, 1=LDT)
;   Bits 1 - 0:     RPL (2 bits)    - Requested Privilege Level (0 - 3)
;
; Selector = (index << 3) | (TI << 2) | RPL
;   - index << 3: Shift index to bits 3 - 15 (skip 3 bits: 2 for RPL + 1 for TI)
;   - TI << 2:    Shift TI to bit 2 (skip 2 bits for RPL)
;   - RPL:        Already in bits 0 - 1, no shift needed
;
; For TSS: (5 << 3) | (0 << 2) | 0 = 0x28
tss_flush:
    mov ax, 0x28        ; Load TSS selector (index=5, GDT, Ring 0)
    ltr ax              ; Load Task Register with TSS selector
    ret
