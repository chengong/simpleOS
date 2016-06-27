#include "gdt.h"
#include "string.h"
#include "idt.h"
#include "console.h"
#include "debug.h"
#include "timer.h"
#include "pmm.h"
#include "vmm.h"

void kern_init();

multiboot_t *glb_mboot_ptr;

char kern_stack[STACK_SIZE];

__attribute__((section(".init.data"))) pgd_t* pgd_tmp  = (pgd_t *)0x1000;
__attribute__((section(".init.data"))) pgd_t* pte_low  = (pgd_t *)0x2000;
__attribute__((section(".init.data"))) pgd_t* pte_high = (pgd_t *)0x3000;

__attribute__((section(".init.text"))) void kern_entry()
{
	pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;
	pgd_tmp[PGD_INDEX(PAGE_OFFSET)] = (uint32_t)pte_high | PAGE_PRESENT | PAGE_WRITE;

	int i;
	for (i = 0; i < 1024; i++) {
		pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}


	for (i = 0; i < 1024; i++) {
		pte_high[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}

	asm volatile ("mov %0, %%cr3" : : "r" (pgd_tmp));

	uint32_t cr0;

	asm volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));

	uint32_t kern_stack_top = ((uint32_t)kern_stack + STACK_SIZE) & 0xFFFFFFF0;
	asm volatile ("mov %0, %%esp\n\t"
			"xor %%ebp, %%ebp" : : "r" (kern_stack_top));

	glb_mboot_ptr = mboot_ptr_tmp + PAGE_OFFSET;

	kern_init();
}

void kern_init()
{
	init_debug();
	init_gdt();
	init_idt();

	console_clear();
	printk_color(rc_black, rc_green, "Hello world!\n");

	init_timer(200);

//	asm volatile ("sti");

	printk("Kernel in memory start: 0x%08X\n", kern_start);
	printk("Kernel in memory end  : 0x%08X\n", kern_end);
	printk("Kernel in memory used : %d KB\n\n", (kern_end -kern_start + 1023) / 1024);

	show_memory_map();

	init_pmm();

	printk_color(rc_black, rc_red, "\n Total physical page is %u\n", phy_page_count);

	uint32_t alloc_addr = NULL;

	printk_color(rc_black, rc_light_brown, "Test physical page alloc : \n");

	alloc_addr = pmm_alloc_page();
	printk_color(rc_black, rc_light_brown, "Alloc physical addr: 0x%08X\n", alloc_addr);
	alloc_addr = pmm_alloc_page();
	printk_color(rc_black, rc_light_brown, "Alloc physical addr: 0x%08X\n", alloc_addr);
	alloc_addr = pmm_alloc_page();
	printk_color(rc_black, rc_light_brown, "Alloc physical addr: 0x%08X\n", alloc_addr);
	alloc_addr = pmm_alloc_page();
	printk_color(rc_black, rc_light_brown, "Alloc physical addr: 0x%08X\n", alloc_addr);

	while (1) {
		asm volatile ("hlt");
	}
}
