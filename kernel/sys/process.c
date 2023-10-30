#include "process.h"

#include <lib/string.h>
#include <lib/alloc.h>
#include <lib/tree.h>

static volatile struct process_queue* process_queue = 0;
static volatile struct process_queue* process_queue_end = 0;

tree_t* process_tree;
volatile process_t* current_process;
volatile process_t* kernel_idle_task;

int save_context(volatile thread_t* thread);
void restore_context(volatile thread_t* thread);

void process_tree_init() {
    process_tree = tree_create();
}

int get_next_pid() {
    static int _current_pid = 1;
    return ++_current_pid;
}

process_t* spawn_init() {
    process_t* init = malloc(sizeof(process_t));
    memset(init, 0, sizeof(process_t));
    process_tree->value = init;
    init->tree_entry = process_tree;
    init->pid = 1;
    init->name = strdup("init");
    init->uid = 0;
    init->gid = 0;
    init->flags = PROC_FLAG_STARTED | PROC_FLAG_RUNNING;
    init->pwd = strdup("/");
    if (fs_root) {
        init->pwd_node = malloc(sizeof(fs_node_t));
        memcpy(init->pwd_node, fs_root, sizeof(fs_node_t));
    }
    init->image.entry = 0;
    init->image.heap = 0;
    init->image.stack = (mmu_request_frames(9) << 12) + 0x9000;
    for (int i = 1; i <= 9; ++i) {
        mmu_lock_frame(init->image.stack - 0x1000 * i);
    }
    init->image.stack = (uintptr_t) mmu_from_physical(init->image.stack);
    init->thread.page_directory = (pagemap_entry_t*) current_pml;
    init->queue_node.process = init;
    return init;
}

static void _idle() {
    while (1) {
        asm volatile("sti\n"
                     "hlt\n"
                     "cli\n");
    }
}

static void _burn() {
    while (1) {
        asm volatile("sti\n"
                     "hlt\n"
                     "cli\n");
        switch_next();
    }
}

process_t* spawn_idle(int bsp) {
    process_t* idle = malloc(sizeof(process_t));
    memset(idle, 0, sizeof(process_t));
    idle->pid = -1;
    idle->name = strdup("idle");
    idle->flags = PROC_FLAG_IS_TASKLET | PROC_FLAG_STARTED | PROC_FLAG_RUNNING;
    idle->image.stack = (mmu_request_frames(9) << 12) + 0x9000;
    for (int i = 1; i <= 9; ++i) {
        mmu_lock_frame(idle->image.stack - 0x1000 * i);
    }
    idle->image.stack = (uintptr_t) mmu_from_physical(idle->image.stack);
    idle->thread.context.ip = bsp ? (uintptr_t) &_idle : (uintptr_t) &_burn;
    idle->thread.context.sp = idle->image.stack;
    idle->thread.context.bp = idle->image.stack;
    idle->thread.page_directory = mmu_clone(0);
    return idle;
}

process_t* spawn_process(volatile process_t* parent, int flags) {
    process_t* proc = malloc(sizeof(process_t));
    memset(proc, 0, sizeof(process_t));
    proc->pid = get_next_pid();
    proc->name = strdup(parent->name);
    proc->uid = parent->uid;
    proc->gid = parent->gid;
    proc->thread.context.sp = 0;
    proc->thread.context.bp = 0;
    proc->thread.context.ip = 0;
    proc->image.entry = parent->image.entry;
    proc->image.heap = parent->image.heap;
    proc->image.stack = (uintptr_t) (mmu_request_frames(9) << 12) + 0x9000;
    for (int i = 1; i <= 9; ++i) {
        mmu_lock_frame(proc->image.stack - 0x1000 * i);
    }
    proc->image.stack = (uintptr_t) mmu_from_physical(proc->image.stack);
    proc->pwd = strdup(parent->pwd);
    if (parent->pwd_node) {
        proc->pwd_node = malloc(sizeof(fs_node_t));
        memcpy(proc->pwd_node, parent->pwd_node, sizeof(fs_node_t));
    }
    proc->tree_entry = tree_insert_value(parent->tree_entry, proc);
    proc->queue_node.process = proc;
    return proc;
}

void multitasking_init() {
    current_process = spawn_init();
    kernel_idle_task = spawn_idle(1);
}

void switch_next() {
    do {
        current_process = next_ready_process();
    } while (current_process->flags & PROC_FLAG_FINISHED);

    mmu_set_directory(current_process->thread.page_directory);
    __sync_or_and_fetch(&current_process->flags, PROC_FLAG_STARTED);
    asm volatile("" ::: "memory");

    restore_context(&current_process->thread);
    __builtin_unreachable();
}

void switch_task(uint8_t reschedule) {
    if (!current_process) {
        return;
    }

    if (!(current_process->flags & PROC_FLAG_RUNNING) || (current_process == kernel_idle_task)) {
        switch_next();
        return;
    }

    if (save_context(&current_process->thread) == 1) {
        return;
    }

    if (reschedule) {
        make_process_ready((process_t*) current_process);
    }

    switch_next();
}

void make_process_ready(volatile process_t* process) {
    if (process_queue_end) {
        process_queue_end->next = (struct process_queue*) &process->queue_node;
        process_queue_end = &process->queue_node;
    } else {
        process_queue = &process->queue_node;
        process_queue_end = process_queue;
    }
}

volatile process_t* next_ready_process() {
    if (!process_queue) {
        return kernel_idle_task;
    }

    volatile process_t* next = process_queue->process;
    process_queue = process_queue->next;
    if (!process_queue) {
        process_queue_end = 0;
    }

    if (!(next->flags & PROC_FLAG_FINISHED)) {
        __sync_or_and_fetch(&next->flags, PROC_FLAG_RUNNING);
    }

    return next;
}

__attribute__((naked))
void resume_user() {
    asm volatile("pop %r15\n"
                 "pop %r14\n"
                 "pop %r13\n"
                 "pop %r12\n"
                 "pop %r11\n"
                 "pop %r10\n"
                 "pop %r9\n"
                 "pop %r8\n"
                 "pop %rbp\n"
                 "pop %rdi\n"
                 "pop %rsi\n"
                 "pop %rdx\n"
                 "pop %rcx\n"
                 "pop %rbx\n"
                 "pop %rax\n"
                 "add $16, %rsp\n"
                 "swapgs\n"
                 "iretq\n");
}

int fork() {
    uintptr_t sp;
    uintptr_t bp;
    process_t* parent = (process_t*) current_process;
    pagemap_entry_t* directory = mmu_clone(parent->thread.page_directory);
    process_t* new_proc = spawn_process(parent, 0);
    new_proc->thread.page_directory = directory;

    struct regs r;
    memcpy(&r, parent->syscall_regs, sizeof(struct regs));
    sp = new_proc->image.stack;
    bp = sp;

    r.rax = 0;

    sp -= sizeof(struct regs);
    *((volatile struct regs*) sp) = r;

    new_proc->syscall_regs = (void*) sp;
    new_proc->thread.context.sp = sp;
    new_proc->thread.context.bp = bp;
    new_proc->thread.context.tls_base = parent->thread.context.tls_base;
    new_proc->thread.context.ip = (uintptr_t) &resume_user;
    save_context(&parent->thread);
    memcpy(new_proc->thread.context.saved, parent->thread.context.saved, sizeof(parent->thread.context.saved));

    if (parent->flags & PROC_FLAG_IS_TASKLET) {
        new_proc->flags |= PROC_FLAG_IS_TASKLET;
    }

    make_process_ready(new_proc);
    return new_proc->pid;
}