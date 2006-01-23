.section ".text",
.globl _ppc_darwin_gcc_switch
LOCAL_SPACE = 6*4+19*4+1*4
LOCAL_R13 = 6*4
LOCAL_R3 = 6*4+19*4
_ppc_darwin_gcc_switch:
	mflr r0
	stw r0, 8(r1)
	stwu r1, -LOCAL_SPACE(r1)
	stw  r3,LOCAL_R3(r1)
	stmw r13,LOCAL_R13(r1)
	stw r1,0(r3)
	mr r1,r4
restore:
	lmw r13,LOCAL_R13(r1)
	lwz r3,LOCAL_R3(r1)
	lwz r0,LOCAL_SPACE+8(r1)
	addi r1,r1,LOCAL_SPACE
	mtlr r0
	blr
