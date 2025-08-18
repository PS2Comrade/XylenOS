/*
 * xylen/kernel/boot.S
 *
 * Xylen Project. All rights reserved.
 * PS2Comrade and VeryEpicKebap.
 * Distributed under the terms of the MIT License.
 */



#define ASM_FILE	1

#include <xylen/segments.h>
#include <xylen/multiboot1.h>

#define MULTIBOOT_HEADER_FLAGS	MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO

/* this section was forked off Fiwix's boot.S. */

/* flags for CR0 (control register) */
#define CR0_MP	0x00000002	/* bit 01 -> enable monitor coprocessor */
#define CR0_NE	0x00000020	/* bit 05 -> enable native x87 FPU mode */
#define CR0_WP	0x00010000	/* bit 16 -> enable write protect (for CoW) */
#define CR0_AM	0x00040000	/* bit 18 -> enable alignment checking */
#define CR0_PG	0x80000000	/* bit 31 -> enable paging */

#ifdef __TINYC__
.data
#else
.section .setup, "a"		/* "a" attribute means Allocatable section */
#endif

.align 4
tmp_gdtr:
	.word ((3 * 8) - 1)
tmp_gdta:
	.long tmp_gdt

.align 4
tmp_gdt:
	/* NULL DESCRIPTOR */
	.word	0x0000
	.word	0x0000
	.word	0x0000
	.word	0x0000

	/* KERNEL CODE */
	.word	0xFFFF		/* segment limit 15-00 */
	.word	0x0000		/* base address 15-00 */
	.byte	0x00		/* base address 23-16 */
	.byte	0x9A		/* P=1 DPL=00 S=1 TYPE=1010 (exec/read) */
	.byte	0xCF		/* G=1 DB=1 0=0 AVL=0 SEGLIM=1111 */
	.byte	GDT_BASE >> 24	/* base address 31-24 */

	/* KERNEL DATA */
	.word	0xFFFF		/* segment limit 15-00 */
	.word	0x0000		/* base address 15-00 */
	.byte	0x00		/* base address 23-16 */
	.byte	0x92		/* P=1 DPL=00 S=1 TYPE=0010 (read/write) */
	.byte	0xCF		/* G=1 DB=1 0=0 AVL=0 SEGLIM=1111 */
	.byte	GDT_BASE >> 24	/* base address 31-24 */

/* ong */ 

#ifdef __TINYC__
.text
#endif

.align 4
multiboot_header:			
	.long	MULTIBOOT_HEADER_MAGIC	
	.long	MULTIBOOT_HEADER_FLAGS	

	.long	-(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)


	.long	0			
	.long	0			
	.long	0			
	.long	0			
	.long	0			/* mov dih :wilted_rose: */


	.long	0			/* mode_type */
	.long	0			/* width */
	.long	0			/* height */
	.long	0			/* depth */

.align 4
.globl _start; _start:
	cli
#ifdef __TINYC__
	movl	$tmp_gdt, %esi
	subl	$PAGE_OFFSET, %esi
	movl	$tmp_gdta, %edi
	subl	$PAGE_OFFSET, %edi
	movl	%esi, (%edi)
	movl	$tmp_gdtr, %esi
	subl	$PAGE_OFFSET, %esi
	lgdt	(%esi)			/* load GDTR with the temporary GDT */
#else
	lgdt	tmp_gdtr		
#endif
	movw	$KERNEL_DS, %cx
	movw	%cx, %ds
	movw	%cx, %es
	movw	%cx, %fs
	movw	%cx, %gs
	movw	%cx, %ss
	ljmp	$KERNEL_CS, $setup_kernel

.text

.align 4
.globl setup_kernel; setup_kernel:
	movl	$PAGE_OFFSET + 0x10000, %esp	
	pushl	$0			
	popf

	pushl	%ebx			
	pushl	%eax			
	call	setup_tmp_pgdir		
	movl	%eax, %cr3

	movl	%cr0, %eax
	andl	$0x00000011, %eax
	orl	$CR0_PG, %eax		
	orl	$CR0_AM, %eax		
	orl	$CR0_WP, %eax		
	orl	$CR0_NE, %eax		
	orl	$CR0_MP, %eax		
	movl	%eax, %cr0

	call	bss_init		/* initialize BSS segment */
	call	gdt_init		
	call	get_last_boot_addr
	popl	%ecx			
	popl	%ebx			

	pushl	%eax			
	pushl	%ebx			
	pushl	%ecx			
	call	start_kernel

	jmp	cpu_idle

