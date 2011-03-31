	/* max. size needed to protect unallocated stack space */
	.set STACK_SKIP, 125

	.section .interrupt, "ax", @progbits
	.align 3

	.global spu_flih

spu_flih:
	/* save back-chain pointer */
	stqd	$SP, -(STACK_SKIP+82)*16($SP)

	/* save volatile registers 0, 2-79 */
	stqd	$0,  -(STACK_SKIP+80)*16($SP)
	stqd	$2,  -(STACK_SKIP+ 2)*16($SP)
	stqd	$3,  -(STACK_SKIP+ 3)*16($SP)
	stqd	$4,  -(STACK_SKIP+ 4)*16($SP)
	stqd	$5,  -(STACK_SKIP+ 5)*16($SP)
	stqd	$6,  -(STACK_SKIP+ 6)*16($SP)
	stqd	$7,  -(STACK_SKIP+ 7)*16($SP)
	stqd	$8,  -(STACK_SKIP+ 8)*16($SP)
	stqd	$9,  -(STACK_SKIP+ 9)*16($SP)
	stqd	$10, -(STACK_SKIP+10)*16($SP)
	stqd	$11, -(STACK_SKIP+11)*16($SP)
	stqd	$12, -(STACK_SKIP+12)*16($SP)
	stqd	$13, -(STACK_SKIP+13)*16($SP)
	stqd	$14, -(STACK_SKIP+14)*16($SP)
	stqd	$15, -(STACK_SKIP+15)*16($SP)
	stqd	$16, -(STACK_SKIP+16)*16($SP)
	stqd	$17, -(STACK_SKIP+17)*16($SP)
	stqd	$18, -(STACK_SKIP+18)*16($SP)
	stqd	$19, -(STACK_SKIP+19)*16($SP)
	stqd	$20, -(STACK_SKIP+20)*16($SP)
	stqd	$21, -(STACK_SKIP+21)*16($SP)
	stqd	$22, -(STACK_SKIP+22)*16($SP)
	stqd	$23, -(STACK_SKIP+23)*16($SP)
	stqd	$24, -(STACK_SKIP+24)*16($SP)
	stqd	$25, -(STACK_SKIP+25)*16($SP)
	stqd	$26, -(STACK_SKIP+26)*16($SP)
	stqd	$27, -(STACK_SKIP+27)*16($SP)
	stqd	$28, -(STACK_SKIP+28)*16($SP)
	stqd	$29, -(STACK_SKIP+29)*16($SP)
	stqd	$30, -(STACK_SKIP+30)*16($SP)
	stqd	$31, -(STACK_SKIP+31)*16($SP)
	stqd	$32, -(STACK_SKIP+32)*16($SP)
	stqd	$33, -(STACK_SKIP+33)*16($SP)
	stqd	$34, -(STACK_SKIP+34)*16($SP)
	stqd	$35, -(STACK_SKIP+35)*16($SP)
	stqd	$36, -(STACK_SKIP+36)*16($SP)
	stqd	$37, -(STACK_SKIP+37)*16($SP)
	stqd	$38, -(STACK_SKIP+38)*16($SP)
	stqd	$39, -(STACK_SKIP+39)*16($SP)
	stqd	$40, -(STACK_SKIP+40)*16($SP)
	stqd	$41, -(STACK_SKIP+41)*16($SP)
	stqd	$42, -(STACK_SKIP+42)*16($SP)
	stqd	$43, -(STACK_SKIP+43)*16($SP)
	stqd	$44, -(STACK_SKIP+44)*16($SP)
	stqd	$45, -(STACK_SKIP+45)*16($SP)
	stqd	$46, -(STACK_SKIP+46)*16($SP)
	stqd	$47, -(STACK_SKIP+47)*16($SP)
	stqd	$48, -(STACK_SKIP+48)*16($SP)
	stqd	$49, -(STACK_SKIP+49)*16($SP)
	stqd	$50, -(STACK_SKIP+50)*16($SP)
	stqd	$51, -(STACK_SKIP+51)*16($SP)
	stqd	$52, -(STACK_SKIP+52)*16($SP)
	stqd	$53, -(STACK_SKIP+53)*16($SP)
	stqd	$54, -(STACK_SKIP+54)*16($SP)
	stqd	$55, -(STACK_SKIP+55)*16($SP)
	stqd	$56, -(STACK_SKIP+56)*16($SP)
	stqd	$57, -(STACK_SKIP+57)*16($SP)
	stqd	$58, -(STACK_SKIP+58)*16($SP)
	stqd	$59, -(STACK_SKIP+59)*16($SP)
	stqd	$60, -(STACK_SKIP+60)*16($SP)
	stqd	$61, -(STACK_SKIP+61)*16($SP)
	stqd	$62, -(STACK_SKIP+62)*16($SP)
	stqd	$63, -(STACK_SKIP+63)*16($SP)
	stqd	$64, -(STACK_SKIP+64)*16($SP)
	stqd	$65, -(STACK_SKIP+65)*16($SP)
	stqd	$66, -(STACK_SKIP+66)*16($SP)
	stqd	$67, -(STACK_SKIP+67)*16($SP)
	stqd	$68, -(STACK_SKIP+68)*16($SP)
	stqd	$69, -(STACK_SKIP+69)*16($SP)
	stqd	$70, -(STACK_SKIP+70)*16($SP)
	stqd	$71, -(STACK_SKIP+71)*16($SP)
	stqd	$72, -(STACK_SKIP+72)*16($SP)
	stqd	$73, -(STACK_SKIP+73)*16($SP)
	stqd	$74, -(STACK_SKIP+74)*16($SP)
	stqd	$75, -(STACK_SKIP+75)*16($SP)
	stqd	$76, -(STACK_SKIP+76)*16($SP)
	stqd	$77, -(STACK_SKIP+77)*16($SP)
	stqd	$78, -(STACK_SKIP+78)*16($SP)
	stqd	$79, -(STACK_SKIP+79)*16($SP)

	/* save event mask on the stack */
	rdch	$6, $SPU_RdEventMask
	stqd	$6,  -(STACK_SKIP+ 1)*16($SP)

	/* disable and acknowledge current pending events */
	rdch	$3, $SPU_RdEventStat
	andc	$7, $6, $3
	wrch	$SPU_WrEventMask, $7
	wrch	$SPU_WrEventAck, $3

	/* instantiate flih stack frame */
	il	$2, -(STACK_SKIP+82)*16
	a	$SP, $SP, $2

next_event:
	clz	$4, $3			# determine next (left-most) event
	ila	$5, spu_slih_handlers	# address of slih function table
	shli	$4, $4, 2		# make event number a word offset
	lqx	$5, $4, $5		# load four slih function pointers
	rotqby	$5, $5, $4		# rotate slih pointer to preferred slot
	bisl	$0, $5			# branch-and-link to slih function
	brnz	$3, next_event		# more events? ($3 is slih return value)

	/* re-establish previous event mask */
	lqd	$6,  81*16($SP)
	wrch	$SPU_WrEventMask, $6

	/* restore volatile registers 79-2, 0 */
	lqd	$79,  3*16($SP)
	lqd	$78,  4*16($SP)
	lqd	$77,  5*16($SP)
	lqd	$76,  6*16($SP)
	lqd	$75,  7*16($SP)
	lqd	$74,  8*16($SP)
	lqd	$73,  9*16($SP)
	lqd	$72, 10*16($SP)
	lqd	$71, 11*16($SP)
	lqd	$70, 12*16($SP)
	lqd	$69, 13*16($SP)
	lqd	$68, 14*16($SP)
	lqd	$67, 15*16($SP)
	lqd	$66, 16*16($SP)
	lqd	$65, 17*16($SP)
	lqd	$64, 18*16($SP)
	lqd	$63, 19*16($SP)
	lqd	$62, 20*16($SP)
	lqd	$61, 21*16($SP)
	lqd	$60, 22*16($SP)
	lqd	$59, 23*16($SP)
	lqd	$58, 24*16($SP)
	lqd	$57, 25*16($SP)
	lqd	$56, 26*16($SP)
	lqd	$55, 27*16($SP)
	lqd	$54, 28*16($SP)
	lqd	$53, 29*16($SP)
	lqd	$52, 30*16($SP)
	lqd	$51, 31*16($SP)
	lqd	$50, 32*16($SP)
	lqd	$49, 33*16($SP)
	lqd	$48, 34*16($SP)
	lqd	$47, 35*16($SP)
	lqd	$46, 36*16($SP)
	lqd	$45, 37*16($SP)
	lqd	$44, 38*16($SP)
	lqd	$43, 39*16($SP)
	lqd	$42, 40*16($SP)
	lqd	$41, 41*16($SP)
	lqd	$40, 42*16($SP)
	lqd	$39, 43*16($SP)
	lqd	$38, 44*16($SP)
	lqd	$37, 45*16($SP)
	lqd	$36, 46*16($SP)
	lqd	$35, 47*16($SP)
	lqd	$34, 48*16($SP)
	lqd	$33, 49*16($SP)
	lqd	$32, 50*16($SP)
	lqd	$31, 51*16($SP)
	lqd	$30, 52*16($SP)
	lqd	$29, 53*16($SP)
	lqd	$28, 54*16($SP)
	lqd	$27, 55*16($SP)
	lqd	$26, 56*16($SP)
	lqd	$25, 57*16($SP)
	lqd	$24, 58*16($SP)
	lqd	$23, 59*16($SP)
	lqd	$22, 60*16($SP)
	lqd	$21, 61*16($SP)
	lqd	$20, 62*16($SP)
	lqd	$19, 63*16($SP)
	lqd	$18, 64*16($SP)
	lqd	$17, 65*16($SP)
	lqd	$16, 66*16($SP)
	lqd	$15, 67*16($SP)
	lqd	$14, 68*16($SP)
	lqd	$13, 69*16($SP)
	lqd	$12, 70*16($SP)
	lqd	$11, 71*16($SP)
	lqd	$10, 72*16($SP)
	lqd	$9,  73*16($SP)
	lqd	$8,  74*16($SP)
	lqd	$7,  75*16($SP)
	lqd	$6,  76*16($SP)
	lqd	$5,  77*16($SP)
	lqd	$4,  78*16($SP)
	lqd	$3,  79*16($SP)
	lqd	$2,  80*16($SP)
	lqd	$0,   2*16($SP)

	/* restore stack pointer from back-chain pointer */
	lqd	$SP,  0*16($SP)

	/* return through SRR0, re-enable interrupts */
	irete
