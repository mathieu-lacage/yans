.section ".text",
.globl _ppc_darwin_gcc_switch
LOCAL_SPACE = 6*4+19*4+1*4+18*8
LOCAL_F14 = 6*4+19*4+1*4
LOCAL_R13 = 6*4
LOCAL_R3 = 6*4+19*4
_ppc_darwin_gcc_switch:
	mflr r0
	stw r0, 8(r1)
	stwu r1, -LOCAL_SPACE(r1)
	stw  r3,LOCAL_R3(r1)
	stmw r13,LOCAL_R13(r1)
	stfd f14,LOCAL_F14+8*0(r1)
	stfd f15,LOCAL_F14+8*1(r1)
	stfd f16,LOCAL_F14+8*2(r1)
	stfd f17,LOCAL_F14+8*3(r1)
	stfd f18,LOCAL_F14+8*4(r1)
	stfd f19,LOCAL_F14+8*5(r1)
	stfd f20,LOCAL_F14+8*6(r1)
	stfd f21,LOCAL_F14+8*7(r1)
	stfd f22,LOCAL_F14+8*8(r1)
	stfd f23,LOCAL_F14+8*9(r1)
	stfd f24,LOCAL_F14+8*10(r1)
	stfd f25,LOCAL_F14+8*11(r1)
	stfd f26,LOCAL_F14+8*12(r1)
	stfd f27,LOCAL_F14+8*13(r1)
	stfd f28,LOCAL_F14+8*14(r1)
	stfd f29,LOCAL_F14+8*15(r1)
	stfd f30,LOCAL_F14+8*16(r1)
	stfd f31,LOCAL_F14+8*17(r1)
	stw r1,0(r3)
	mr r1,r4
restore:
	lfd f14,LOCAL_F14+8*0(r1)
	lfd f15,LOCAL_F14+8*1(r1)
	lfd f16,LOCAL_F14+8*2(r1)
	lfd f17,LOCAL_F14+8*3(r1)
	lfd f18,LOCAL_F14+8*4(r1)
	lfd f19,LOCAL_F14+8*5(r1)
	lfd f20,LOCAL_F14+8*6(r1)
	lfd f21,LOCAL_F14+8*7(r1)
	lfd f22,LOCAL_F14+8*8(r1)
	lfd f23,LOCAL_F14+8*9(r1)
	lfd f24,LOCAL_F14+8*10(r1)
	lfd f25,LOCAL_F14+8*11(r1)
	lfd f26,LOCAL_F14+8*12(r1)
	lfd f27,LOCAL_F14+8*13(r1)
	lfd f28,LOCAL_F14+8*14(r1)
	lfd f29,LOCAL_F14+8*15(r1)
	lfd f30,LOCAL_F14+8*16(r1)
	lfd f31,LOCAL_F14+8*17(r1)
	lmw r13,LOCAL_R13(r1)
	lwz r3,LOCAL_R3(r1)
	lwz r0,LOCAL_SPACE+8(r1)
	addi r1,r1,LOCAL_SPACE
	mtlr r0
	blr
