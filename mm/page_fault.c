#include "vmm.h"
#include "debug.h"

void page_fault(pt_regs *regs)
{
    uint32_t cr2;
    asm volatile ("mov %%cr2, %0" : "=r" (cr2));

    printk("Page fault at 0x%x, virtual faulting addr 0x%x \n", regs->eip, cr2);
    printk("Error code is %x\n", regs->err_code);
    while(1);
    
}
