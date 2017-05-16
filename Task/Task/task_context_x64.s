#.intel_syntax

	.struct	0
tc_start:
tc_user:
	.struct tc_user+8
tc_parent:
	.struct	tc_parent+8
tc_stack_ptr:
	.struct	tc_stack_ptr+8
tc_stack_base:
	.struct	tc_stack_base+8
tc_stack_size:
	.struct	tc_stack_size+8
tc_end:
tc_size = tc_end - tc_start


	.struct 0
sv_start:
sv_rbx:
	.struct sv_rbx+8
sv_rbp:
	.struct sv_rbp+8
sv_r12:
	.struct sv_r12+8
sv_r13:
	.struct sv_r13+8
sv_r14:
	.struct sv_r14+8
sv_r15:
	.struct sv_r15+8
sv_mxcsr:
	.struct sv_mxcsr+4
sv_x87cw:
	.struct sv_x87cw+4
sv_end:
sv_size = sv_end - sv_start
# (sv_size % 16) == 8 because of stack alignment


.text

.global task_associate
#.type	task_associate, %function

task_associate:

	#RDI contains the task to initialize
	#RSI contains the function pointer to execute when the task is switched to

	#align stack
	mov		tc_stack_base(%RDI), %RAX
	add		tc_stack_size(%RDI), %RAX
	and		$-16, %RAX

	#allocate stack for Saved structure + 8 bytes for the trampoline address
	sub		$sv_size+8, %RAX

	#initialize the Saved structure at the top of the stack
	#set MMX/SSE and x87 control word fields the same as the creator
	stmxcsr	sv_mxcsr(%RAX)
	fnstcw	sv_x87cw(%RAX)
	#set only R12 and R13 as they will be used in the trampoline, we don't care about the rest
	mov		%RDI, sv_r12(%RAX)
	mov		%RSI, sv_r13(%RAX)

	#push trampoline address at the bottom of the stack
	lea		trampoline, %R9
	mov		%R9, sv_size(%RAX)

	#save the stack pointer for the new task
	mov		%RAX, tc_stack_ptr(%RDI)

	ret

trampoline:
	#set task argument and call the task entry point
	mov		%R12, %RDI
	call	*%R13
	#set arguments and call task_swap to return to the parent task
	xor		%RDI, %RDI
	mov		tc_parent(%R12), %RSI
	#zero out the task stack_ptr to indicate the task's finished
	mov		%RDI, tc_stack_ptr(%R12)
	call	task_swap
	#we should not return to a completed task
	int3


.global	task_swap
#.type	task_swap, %function

task_swap:

	#RDI contains the FROM task
	#RSI contains the TO task

	or		%RDI, %RDI
	jz		skip_save
	sub		$sv_size, %RSP
	mov		%RBX, sv_rbx(%RSP)
	mov		%RBP, sv_rbp(%RSP)
	mov		%R12, sv_r12(%RSP)
	mov		%R13, sv_r13(%RSP)
	mov		%R14, sv_r14(%RSP)
	mov		%R15, sv_r15(%RSP)
	stmxcsr	sv_mxcsr(%RSP)
	fnstcw	sv_x87cw(%RSP)

	mov		%RSP, tc_stack_ptr(%RDI)
skip_save:
	mov		tc_stack_ptr(%RSI), %RSP

	mov		sv_rbx(%RSP), %RBX
	mov		sv_rbp(%RSP), %RBP
	mov		sv_r12(%RSP), %R12
	mov		sv_r13(%RSP), %R13
	mov		sv_r14(%RSP), %R14
	mov		sv_r15(%RSP), %R15
	ldmxcsr	sv_mxcsr(%RSP)
	fldcw	sv_x87cw(%RSP)
	add		$sv_size, %RSP

	ret
