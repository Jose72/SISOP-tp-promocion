/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>

#include <kern/picirq.h>


###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */

TRAPHANDLER_NOEC(divzero, 0)
TRAPHANDLER_NOEC(debug, 1)
TRAPHANDLER_NOEC(nmi, 2)
TRAPHANDLER_NOEC(brkpt, 3)
TRAPHANDLER_NOEC(oflow, 4)
TRAPHANDLER_NOEC(bound, 5)
TRAPHANDLER_NOEC(illop, 6)
TRAPHANDLER_NOEC(device, 7)
TRAPHANDLER(dblflt, 8)
TRAPHANDLER_NOEC(tss, 10)
TRAPHANDLER(segnp, 11)
TRAPHANDLER(stack, 12)
TRAPHANDLER(gpflt, 13)
TRAPHANDLER(pgflt, 14)
TRAPHANDLER_NOEC(fperr, 16)
TRAPHANDLER(align, 17)
TRAPHANDLER_NOEC(mchk, 18)
TRAPHANDLER_NOEC(simderr, 19)

TRAPHANDLER_NOEC(irq0, 32)
TRAPHANDLER_NOEC(irq1, 33)
TRAPHANDLER_NOEC(irq2, 34)
TRAPHANDLER_NOEC(irq3, 35)
TRAPHANDLER_NOEC(irq4, 36)
TRAPHANDLER_NOEC(irq5, 37)
TRAPHANDLER_NOEC(irq6, 38)
TRAPHANDLER_NOEC(irq7, 39)
TRAPHANDLER_NOEC(irq8, 40)
TRAPHANDLER_NOEC(irq9, 41)
TRAPHANDLER_NOEC(irq10, 42)
TRAPHANDLER_NOEC(irq11, 43)
TRAPHANDLER_NOEC(irq12, 44)
TRAPHANDLER_NOEC(irq13, 45)
TRAPHANDLER_NOEC(irq14, 46)
TRAPHANDLER_NOEC(irq15, 47)

handl48: TRAPHANDLER_NOEC(syscll, 48)

/*
 * Lab 3: Your code here for _alltraps
 */

.globl _alltraps
_alltraps:
        pushl %ds
        pushl %es
	pushal

	movl $GD_KD, %eax
	movw %ax, %ds
	movw %ax, %es 

        pushl %esp  //trap frame
        call trap

