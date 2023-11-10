#pragma once

#include <sys/fs/vfs.h>
#include <lib/tree.h>
#include <cpu/mmu.h>
#include <cpu/idt.h>
#include <stdint.h>
#include <stddef.h>

#define PROC_FLAG_IS_TASKLET  0x01
#define PROC_FLAG_FINISHED    0x02
#define PROC_FLAG_STARTED     0x04
#define PROC_FLAG_RUNNING     0x08
#define PROC_FLAG_SLEEP_INT   0x10
#define PROC_FLAG_SUSPENDED   0x20
#define PROC_FLAG_HAS_WAITERS 0x40

struct process;

struct process_queue {
    struct process* process;
    struct process_queue* next;
};

typedef struct {
    uintptr_t sp;
    uintptr_t bp;
    uintptr_t ip;
    uintptr_t tls_base;
    uintptr_t saved[5];
} thread_context_t;

typedef struct {
    thread_context_t context;
    pagemap_entry_t* page_directory;
} thread_t;

typedef struct {
    uintptr_t entry;
    uintptr_t heap;
    uintptr_t stack;
    uintptr_t userstack;
} image_t;

typedef struct process {
    int pid;
    uint32_t flags;
    uint32_t uid;
    uint32_t gid;
    uint32_t mask;
    uint32_t status;
    char* name;
    char* pwd;
    fs_node_t* pwd_node;
    tree_t* tree_entry;
    struct regs* syscall_regs;
    uint64_t interrupted_syscall;
    thread_t thread;
    image_t image;
    fs_node_t** fds;
    uint32_t fd_capacity;
    uint32_t fd_length;
    struct process_queue queue_node;
} process_t;

extern volatile process_t* current_process;

process_t* get_process(int pid);
int append_process_fd(fs_node_t* node);
fs_node_t* get_process_fd(int fd);
long process_dup2(int from, int to);
void process_tree_init();
void multitasking_init();
void switch_next();
void switch_task(uint8_t reschedule);
void make_process_ready(volatile process_t* process);
volatile process_t* next_ready_process();
void process_delete(process_t* proc);
void process_exit(int rval);
int fork();