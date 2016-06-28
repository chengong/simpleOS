#ifndef INCLUDE_TASK_H_
#define INCLUDE_TASK_H_

#include "types.h"
#include "vmm.h"
#include "pmm.h"

typedef
enum task_state {
    TASK_UNINIT =  0,
    TASK_SLEEPING = 1,
    TASK_RUNNABLE = 2,
    TASK_ZOMBIE = 3,
} task_state;

struct context {
    uint32_t esp;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
};

struct mm_struct {
    pgd_t *pgd_dir;
};

struct task_struct {
    volatile task_state state;
    pid_t pid;
    void *stack;
    struct mm_struct *mm;
    struct context context;
    struct task_struct *next;
};

extern pid_t now_pid;

int32_t kernel_thread(int (*fn)(void *), void *arg);

void kthread_exit();

#endif
