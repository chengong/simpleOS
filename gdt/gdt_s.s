[GLOBAL gdt_flush]

gdt_flush:
	mov eax, [esp+4]   ;save the parameter to eax
	lgdt [eax]         

	mov ax, 0x10       ;load our data segment descriptor
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:.flush    ;

.flush:
	ret

