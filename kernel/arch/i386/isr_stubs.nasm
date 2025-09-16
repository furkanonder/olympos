; Declare the external C handler
extern isr_handler

; === Auto-generate global isr0 to isr31 ===
%assign i 0
%rep 32
    global isr%+i
%assign i i+1
%endrep

; === Macros to define ISR handlers ===
;
; The CPU automatically pushes different amounts of data depending on the exception:
; - Most exceptions (0-7, 9, 15, 16, 18-31): CPU pushes EFLAGS, CS, EIP (3 values)
; - Some exceptions (8, 10, 11, 12, 13, 14, 17): CPU pushes EFLAGS, CS, EIP, ErrorCode (4 values)
; 
; Our macros standardize this by ensuring ALL interrupts have the same stack layout:
; - ISR_NOERR: Adds dummy error code (0) for exceptions that don't push one
; - ISR_ERR: Uses the real error code that the CPU already pushed
; 
; This creates a uniform interface where every interrupt handler receives:
; [ESP + 0] = ErrorCode (real or dummy), [ESP + 4] = InterruptNumber

%macro ISR_NOERR 1
isr%1:
    push dword 0         ; Dummy error code (exceptions 0-7, 9, 15, 16, 18-31 don't push error codes)
    push dword %1        ; Interrupt number for identification
    jmp isr_common_stub  ; Jump to common handler
%endmacro

%macro ISR_ERR 1
isr%1:
    push dword [esp]     ; Real error code already on stack (exceptions 8, 10, 11, 12, 13, 14, 17)
    push dword %1        ; Interrupt number for identification
    jmp isr_common_stub  ; Jump to common handler
%endmacro

; === Common ISR handler stub ===
isr_common_stub:
    ; This is the common handler that ALL interrupt service routines jump to.
    ; It performs the complete context saving and restoration sequence.
    ;
    ; CURRENT STACK STATE (when we arrive here):
    ; ==========================================
    ; ESP ──► [ErrorCode] [IntNo] [EFLAGS] [CS] [EIP] [user data...]
    ;         0x0FE8      ↑
    ;                    This ESP value will be saved as ESP_DUMMY by pushad
    ;
    ; STEP-BY-STEP PROCESS:
    ; ====================
    ; 1. Save all general registers with pushad (creates esp_dummy)
    ; 2. Save data segment selector (DS)
    ; 3. Switch to kernel data segments for safe memory access
    ; 4. Prepare argument for C handler (pointer to saved state)
    ; 5. Call C interrupt handler
    ; 6. Restore data segment selector
    ; 7. Restore all general registers with popad
    ; 8. Clean up error code and interrupt number from stack
    ; 9. Return from interrupt (restores EIP, CS, EFLAGS, ESP, SS)
    ;
    ; DETAILED STACK STATE VISUALIZATION:
    ; ===================================
    ;
    ; BEFORE pushad (current state):
    ; ESP ──► [ErrorCode] [IntNo] [EFLAGS] [CS] [EIP] [user data...]
    ;         0x0FE8      ↑
    ;                    This ESP value (0x0FE8) gets saved as ESP_DUMMY
    ;
    ; AFTER pushad (what happens next):
    ; ESP ──► [EAX] [ECX] [EDX] [EBX] [ESP_DUMMY] [EBP] [ESI] [EDI] [ErrorCode] [IntNo] [EFLAGS] [CS] [EIP] [user data...]
    ;         0x0FC8  ↑
    ;                 Current ESP (useful for interrupt handling)
    ;                 ↑
    ;                 ESP_DUMMY contains 0x0FE8 (the ESP from BEFORE pushad - not useful!)
    ;
    ; AFTER additional pushes (DS, regs_ptr):
    ; ESP ──► [DS] [regs_ptr] [EAX] [ECX] [EDX] [EBX] [ESP_DUMMY] [EBP] [ESI] [EDI] [ErrorCode] [IntNo] [EFLAGS] [CS] [EIP] [user data...]
    ;         0x0FC0  ↑
    ;                 This is the REAL current stack pointer (what you need for interrupt handling)
    ;
    ; WHY ESP_DUMMY IS "DUMMY":
    ; ========================
    ; 1. ESP_DUMMY = 0x0FE8 (ESP before pushad)
    ; 2. Current ESP = 0x0FC0 (ESP after all pushes)
    ; 3. For interrupt handling, you need the CURRENT ESP (0x0FC0), not the old one (0x0FE8)
    ; 4. ESP_DUMMY is just a placeholder to maintain the exact register order from pushad
    ;
    ; The pushad instruction saves registers in this order:
    ; EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    ;
    ; The ESP it saves is the value BEFORE pushad started, which is why
    ; it's called "dummy" - it's not the current stack pointer you need.
    ;
    ; =============================================================================

    ; STEP 1: Save all general-purpose registers with pushad
    ; This instruction pushes: EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    ; The ESP saved here is the value BEFORE pushad, not the current ESP
    ; This creates the esp_dummy field in our regs_t structure
    pushad

    ; STEP 2: Save current data segment selector
    ; =========================================
    ; User-space might have different DS, so we need to save it before switching
    ; to kernel segments. This ensures we can restore the original DS later.
    push ds

    ; STEP 3: Switch to kernel data segments for safe memory access
    ; =============================================================
    ; Why this is necessary:
    ; - Interrupts can occur in user-space with user DS
    ; - Kernel code needs kernel data segments to access kernel data structures safely
    ; - This ensures all data segment registers point to kernel data (GDT index 2)
    ; - Prevents segmentation faults when accessing kernel memory from user context
    mov ax, 0x10         ; Kernel data segment selector (GDT index 2 << 3 = 0x10)
    mov ds, ax           ; Data segment - for accessing kernel data
    mov es, ax           ; Extra segment - for string operations
    mov fs, ax           ; FS segment - for thread-local storage (kernel context)
    mov gs, ax           ; GS segment - for additional kernel data access

    ; STEP 4: Prepare argument for C handler
    ; =====================================
    ; ESP now points to the saved register structure (regs_t)
    ; We push this pointer as the argument to our C handler
    mov eax, esp         ; ESP points to the saved register structure
    push eax             ; Push pointer as argument: (regs_t *regs)
    
    ; STEP 5: Call C interrupt handler
    ; ===============================
    ; This calls our C function that will handle the actual interrupt processing
    ; The C handler receives a pointer to the complete saved CPU state
    call isr_handler     ; Call C interrupt handler
    
    ; STEP 6: Clean up argument from stack
    ; ===================================
    ; Remove the regs_t pointer we pushed as an argument
    add esp, 4           ; Clean up argument from stack

    ; STEP 7: Restore original data segment selector
    ; =============================================
    ; Restore the DS that was in use before the interrupt
    pop ds
    
    ; STEP 8: Restore all general-purpose registers
    ; ============================================
    ; This restores: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    ; Note: The ESP restored here is the esp_dummy value (not useful)
    ; The real ESP will be restored by the iret instruction
    popad
    
    ; STEP 9: Clean up error code and interrupt number from stack
    ; ==========================================================
    ; Remove the error code and interrupt number that were pushed by our macros
    ; This restores the stack to the state it was in when the CPU first pushed
    ; EFLAGS, CS, EIP (and ErrorCode if applicable)
    add esp, 8           ; Remove error code + interrupt number from stack
    
    ; STEP 10: Return from interrupt
    ; =============================
    ; This instruction restores the CPU state that was automatically saved:
    ; - Pops SS and ESP (if privilege change occurred)
    ; - Pops EFLAGS, CS, EIP (always)
    ; - Resumes execution at the saved EIP
    iret

; === Define ISR stubs 0–31 (with correct error code usage) ===
; Exceptions that push error codes: 8, 10, 11, 12, 13, 14, 17
; - 8:  Double Fault
; - 10: Invalid TSS
; - 11: Segment Not Present
; - 12: Stack Fault
; - 13: General Protection Fault
; - 14: Page Fault
; - 17: Alignment Check
; All other exceptions (0-7, 9, 15, 16, 18-31) do not push error codes
%assign i 0
%rep 32
    ; Exceptions that push an error code
    %if i = 8 || i = 10 || i = 11 || i = 12 || i = 13 || i = 14 || i = 17
        ISR_ERR i
    %else
        ISR_NOERR i
    %endif
%assign i i+1
%endrep
