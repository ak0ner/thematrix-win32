;
; Load the embedded binary
;	Args: None
;	Ret: The binary on success, NULL otherwise
;
load_embedded_binary proc frame
	leb_hMod EQU LOCALS.Local1
	leb_hRes EQU LOCALS.Local2
	leb_hGlobal EQU LOCALS.Local3
	leb_res_buffer EQU LOCALS.Local4

	_CreateFrame leb_,LOCALS_SIZE,0,rsi,rdi,r14,r15
	_EndProlog

	_AllocHomeArea

	call get_module_base
	test rax, rax
	jz @fail
	mov qword ptr leb_hMod[rbp], rax

	; load the binary from the resource
	mov r8, RT_VERSION
	mov rdx, VS_VERSION_INFO
	mov rcx, qword ptr leb_hMod[rbp]
	call FindResourceA
	test rax, rax
	jz @exit
	mov qword ptr leb_hRes[rbp], rax
		
	mov rdx, qword ptr leb_hRes[rbp]
	mov rcx, qword ptr leb_hMod[rbp]
	call LoadResource
	test rax, rax
	jz @fail
	mov qword ptr leb_hGlobal[rbp], rax

	mov rcx, qword ptr leb_hGlobal[rbp]
	call LockResource
	test rax, rax
	jz @fail
	mov qword ptr leb_res_buffer[rbp], rax

	; get resource size
	mov rdx, qword ptr leb_hRes[rbp]
	mov rcx, qword ptr leb_hMod[rbp]
	call SizeofResource
	test rax, rax
	jz @fail

	mov r14, rax ; save value to non-volatile register

	; allocate memory and copy the resource content
	mov r9, PAGE_READWRITE
	mov r8, MEM_COMMIT
	mov rdx, rax
	xor rcx, rcx
	call VirtualAlloc
	test rax, rax
	jz @fail
	mov r15, rax

	; copy the resource content to the newly allocated memory
	mov rcx, r14
	mov rsi, qword ptr leb_res_buffer[rbp]
	mov rdi, r15
	rep movsb

	; free resources
	mov rcx, qword ptr leb_hRes[rbp]
	call FreeResource

	mov rax, r15

@exit:
	_DeleteFrame rsi,rdi,r14,r15
	ret

@fail:
	xor rax, rax
	jmp @exit
load_embedded_binary endp