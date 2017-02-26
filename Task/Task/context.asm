.code

Task STRUCT
	user		QWORD	?
	stack_ptr	QWORD	?
	parent		QWORD	?
	stack_base	QWORD	?
	stack_size	QWORD	?
Task ENDS

; (SIZEOF Saved) % 16 == 8 because of stack alignment
Saved STRUCT
	_xmm6	XMMWORD	?
	_xmm7	XMMWORD	?
	_xmm8	XMMWORD	?
	_xmm9	XMMWORD	?
	_xmm10	XMMWORD	?
	_xmm11	XMMWORD	?
	_xmm12	XMMWORD	?
	_xmm13	XMMWORD	?
	_xmm14	XMMWORD	?
	_xmm15	XMMWORD	?
	_mxcsr	DWORD	?
	_x87cw	DWORD	?
	_r12	QWORD	?
	_r13	QWORD	?
	_r14	QWORD	?
	_r15	QWORD	?
	_rsi	QWORD	?
	_rdi	QWORD	?
	_rbx	QWORD	?
	_rbp	QWORD	?
Saved ENDS

makecontext PROC FRAME
	.endprolog

	;RCX contains the task to initialize
	;RDX contains the function pointer to execute when the task is switched to

	;align stack
	mov		RAX, [RCX].Task.stack_base
	add		RAX, [RCX].Task.stack_size
	and		RAX, -16

	;allocate stack for Saved structure + 8 bytes for the trampoline address
	sub		RAX, 8 + SIZEOF Saved

	;initialize the Saved structure at the top of the stack
	;set MMX/SSE and x87 control word fields the same as the creator
	stmxcsr	[RAX].Saved._mxcsr
	fnstcw	WORD PTR [RAX].Saved._x87cw
	;set only RSI and RDI as they will be used in the trampoline, we don't care about the rest
	mov		[RAX].Saved._rsi, RCX
	mov		[RAX].Saved._rdi, RDX

	;push trampoline address at the bottom of the stack
	lea		R9, trampoline
	mov		[RAX + SIZEOF Saved], R9

	;save the stack pointer for the new task
	mov		[RCX].Task.stack_ptr, RAX

	ret

trampoline:
	;set task argument and call the task entry point
	mov		RCX, RSI
	;alloc space for argument registers as required by x64 ABI
	sub		RSP, 4 * SIZEOF QWORD
	call	RDI
	add		RSP, 4 * SIZEOF QWORD
	;set arguments and call swapcontext to return to the parent task
	mov		RCX, RSI
	mov		RDX, [RSI].Task.parent
	;zero out the task parent to indicate the task's finished
	xor		RAX, RAX
	mov		[RSI].Task.parent, RAX
	call	swapcontext
	;we should not return to a completed task
	int		3

makecontext ENDP

swapcontext PROC FRAME
	.endprolog

	;RCX contains the FROM task
	;RDX contains the TO task

	sub		RSP, SIZEOF Saved
	movaps	[RSP].Saved._xmm6, XMM6
	movaps	[RSP].Saved._xmm7, XMM7
	movaps	[RSP].Saved._xmm8, XMM8
	movaps	[RSP].Saved._xmm9, XMM9
	movaps	[RSP].Saved._xmm10, XMM10
	movaps	[RSP].Saved._xmm11, XMM11
	movaps	[RSP].Saved._xmm12, XMM12
	movaps	[RSP].Saved._xmm13, XMM13
	movaps	[RSP].Saved._xmm14, XMM14
	movaps	[RSP].Saved._xmm15, XMM15
	stmxcsr	[RSP].Saved._mxcsr
	fnstcw	WORD PTR [RSP].Saved._x87cw
	mov		[RSP].Saved._r12, R12
	mov		[RSP].Saved._r13, R13
	mov		[RSP].Saved._r14, R14
	mov		[RSP].Saved._r15, R15
	mov		[RSP].Saved._rsi, RSI
	mov		[RSP].Saved._rdi, RDI
	mov		[RSP].Saved._rbx, RBX
	mov		[RSP].Saved._rbp, RBP

	mov		[RCX].Task.stack_ptr, RSP
	mov		RSP, [RDX].Task.stack_ptr

	movaps	XMM6, [RSP].Saved._xmm6 
	movaps	XMM7, [RSP].Saved._xmm7 
	movaps	XMM8, [RSP].Saved._xmm8 
	movaps	XMM9, [RSP].Saved._xmm9 
	movaps	XMM10, [RSP].Saved._xmm10
	movaps	XMM11, [RSP].Saved._xmm11
	movaps	XMM12, [RSP].Saved._xmm12
	movaps	XMM13, [RSP].Saved._xmm13
	movaps	XMM14, [RSP].Saved._xmm14
	movaps	XMM15, [RSP].Saved._xmm15
	ldmxcsr	[RSP].Saved._mxcsr
	fldcw	WORD PTR [RSP].Saved._x87cw
	mov		R12, [RSP].Saved._r12
	mov		R13, [RSP].Saved._r13
	mov		R14, [RSP].Saved._r14
	mov		R15, [RSP].Saved._r15
	mov		RSI, [RSP].Saved._rsi
	mov		RDI, [RSP].Saved._rdi
	mov		RBX, [RSP].Saved._rbx
	mov		RBP, [RSP].Saved._rbp
	add		RSP, SIZEOF Saved

	ret

swapcontext ENDP

END