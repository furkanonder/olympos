global idt_load    ; make the label idt_load visible outside this file

; idt_load - Load the interrupt descriptor table (IDT).
; stack: [esp + 4] the address of the IDTR structure
;        [esp    ] the return address
idt_load:
	mov eax, [esp + 4]  ; load the address of the IDTR into eax
	lidt [eax]          ; load the IDT
	ret                 ; return to caller
