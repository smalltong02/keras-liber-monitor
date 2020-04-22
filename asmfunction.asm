title  "amd64 function"

_hookImplementFunction_x64 PROTO C

wrtext SEGMENT READ WRITE EXECUTE
	BreakInt3    proc
        int 3
        ret
    BreakInt3    endp

	_hookImplementFunction    proc
		push rbx
		push rcx
		push rdx
		push rsi
		push rdi
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		push r14
		push r15
		push rbp
		push rsp
		pushfq
		mov  rbp, rsp
		sub  rsp, 200h
		mov  rcx, rbp
		call _hookImplementFunction_x64
		add  rsp, 200h
		popfq
		add  rsp, 8h
		pop  rbp
		pop  r15
		pop  r14
		pop  r13
		pop  r12
		pop  r11
		pop  r10
		pop  r9
		pop  r8
		pop  rdi
		pop  rsi
		pop  rdx
		pop  rcx
		pop  rbx
		add  rsp, 8h
        ret
    _hookImplementFunction    endp

	_callPreProcessing_x64    proc
		; shadow spaces
		mov  qword ptr [rsp + 8],rcx		;__pnode
		mov  qword ptr [rsp + 10h],rdx		;__params
		mov  qword ptr [rsp + 18h],r8		;__psize
		mov  qword ptr [rsp + 20h],r9		;__post_func
		push rbx
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		sub  rsp, 100h
		mov  rax, qword ptr [rsp + 160h]
		cdq
		and  rdx, 3
		add  rax, rdx
		sar  rax, 3
		mov  r11, rax
		xor  r12, r12
		test rax, rax
		jle  for_exit
loop_start:									;Backward propagation argments
		mov  rax, r12
		imul rbx, rax, 8
		mov  r10, qword ptr [rsp + 158h]
		cmp  rax, 0
		jnz  loop_1
		mov  rdx, qword ptr [r10]
		jmp  loop_4
loop_1:
		cmp  rax, 1
		jnz  loop_2
		mov  r8, qword ptr [r10 + rbx]
		jmp  loop_4
loop_2:
		cmp  rax, 2
		jnz  loop_3
		mov  r9, qword ptr [r10 + rbx]
		jmp  loop_4
loop_3:
		mov  r13, qword ptr [r10 + rbx]
		mov  qword ptr [rsp + rbx + 8h], r13
loop_4:
		inc  r12
		cmp  r12, r11
		jl   loop_start
for_exit:
		mov  rax, qword ptr [rsp + 168h]
		call rax
		add  rsp, 100h
		pop  r13
		pop  r12
		pop  r11
		pop  r10
		pop  r9
		pop  r8
		pop  rdx
		pop  rcx
		pop  rbx
		ret 
	_callPreProcessing_x64    endp

	_callOrginApiFunction_x64 proc
		; shadow spaces
		mov  qword ptr [rsp + 8],rcx		;__params
		mov  qword ptr [rsp + 10h],rdx		;__psize
		mov  qword ptr [rsp + 18h],r8		;__call_conv, only support __fastcall x64 now
		mov  qword ptr [rsp + 20h],r9		;__orgin_func
											;qword ptr [rsp + 28h] __return
		push rbx
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11
		push r12
		push r13
		sub  rsp, 100h
		mov  rax, qword ptr [rsp + 158h]
		cdq
		and  rdx, 3
		add  rax, rdx
		sar  rax, 3
		mov  r11, rax
		xor  r12, r12
		test rax, rax
		jle  for_exit
loop_start:									;Backward propagation argments
		mov  rax, r12
		imul rbx, rax, 8
		mov  r10, qword ptr [rsp + 150h]
		cmp  rax, 0
		jnz  loop_1
		mov  rcx, qword ptr [r10]
		jmp  loop_5
loop_1:
		cmp  rax, 1
		jnz  loop_2
		mov  rdx, qword ptr [r10 + rbx]
		jmp  loop_5
loop_2:
		cmp  rax, 2
		jnz  loop_3
		mov  r8, qword ptr [r10 + rbx]
		jmp  loop_5
loop_3:
		cmp  rax, 3
		jnz  loop_4
		mov  r9, qword ptr [r10 + rbx]
		jmp  loop_5
loop_4:
		mov  r13, qword ptr [r10 + rbx]
		mov  qword ptr [rsp + rbx], r13
loop_5:
		inc  r12
		cmp  r12, r11
		jl   loop_start
for_exit:
		mov  rax, qword ptr [rsp + 168h]
		call rax
		mov  rbx, qword ptr [rsp + 170h]
		mov  qword ptr [rbx], rax
		add  rsp, 100h
		pop  r13
		pop  r12
		pop  r11
		pop  r10
		pop  r9
		pop  r8
		pop  rdx
		pop  rcx
		pop  rbx
		ret 

	_callOrginApiFunction_x64 endp

wrtext ENDS

END
