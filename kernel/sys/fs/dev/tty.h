#pragma once

#include <sys/fs/vfs.h>
#include <lib/ringbuffer.h>
#include <stdint.h>
#include <stddef.h>

struct winsize {
    uint16_t row;
    uint16_t col;
    uint16_t xpixel;
    uint16_t ypixel;
};

struct termios {
    uint32_t iflag;
    uint32_t oflag;
    uint32_t cflag;
    uint32_t lflag;
    uint8_t cc[32];
};

typedef struct pty {
    intptr_t name;
    fs_node_t* master;
    fs_node_t* slave;
    struct winsize size;
    struct termios ios;
    ringbuffer_t* in;
    ringbuffer_t* out;
    char* canon_buf;
    size_t canon_size;
    size_t canon_len;
    int control_pid;
    int foreground_pid;
    void(*write_in)(struct pty*, uint8_t);
    void(*write_out)(struct pty*, uint8_t);
    void(*fill_name)(struct pty*, char*);
    void* context;
} pty_t;

#define VEOF    1
#define VEOL    2
#define VERASE  3
#define VINTR   4
#define VKILL   5
#define VMIN    6
#define VQUIT   7
#define VSTART  8
#define VSTOP   9
#define VSUSP   10
#define VTIME   11
#define VLNEXT  12
#define VWERASE 13

#define BRKINT  0000001
#define ICRNL   0000002
#define IGNBRK  0000004
#define IGNCR   0000010
#define IGNPAR  0000020
#define INLCR   0000040
#define INPCK   0000100
#define ISTRIP  0000200
#define IUCLC   0000400
#define IXANY   0001000
#define IXOFF   0002000
#define IXON    0004000
#define PARMRK  0010000

#define OPOST   0000001
#define OLCUC   0000002
#define ONLCR   0000004
#define OCRNL   0000010
#define ONOCR   0000020
#define ONLRET  0000040
#define OFILL   0000100
#define OFDEL   0000200
#define NLDLY   0000400
#define NL0     0000000
#define NL1     0000400
#define CRDLY   0003000
#define CR0     0000000
#define CR1     0001000
#define CR2     0002000
#define CR3     0003000
#define TABDLY  0014000
#define TAB0    0000000
#define TAB1    0004000
#define TAB2    0010000
#define TAB3    0014000
#define BSDLY   0020000
#define BS0     0000000
#define BS1     0020000
#define FFDLY   0100000
#define FF0     0000000
#define FF1     0100000
#define VTDLY   0040000
#define VT0     0000000
#define VT1     0040000

#define CBAUD   0100017
#define B0      0000000
#define B50     0000001
#define B75     0000002
#define B110    0000003
#define B134    0000004
#define B150    0000005
#define B200    0000006
#define B300    0000007
#define B600    0000010
#define B1200   0000011
#define B1800   0000012
#define B2400   0000013
#define B4800   0000014
#define B9600   0000015
#define B19200  0000016
#define B38400  0000017
#define B57600  0100000
#define B115200 0100001
#define B230400 0100002
#define B460800 0100003
#define B921600 0100004

#define CSIZE   0000060
#define CS5     0000000
#define CS6     0000020
#define CS7     0000040
#define CS8     0000060
#define CSTOPB  0000100
#define CREAD   0000200
#define PARENB  0000400
#define PARODD  0001000
#define HUPCL   0002000
#define CLOCAL  0004000

#define ISIG    0000001
#define ICANON  0000002
#define XCASE   0000004
#define ECHO    0000010
#define ECHOE   0000020
#define ECHOK   0000040
#define ECHONL  0000100
#define NOFLSH  0000200
#define TOSTOP  0000400
#define IEXTEN  0001000

#define TCSANOW   0x0001
#define TCSADRAIN 0x0002
#define TCSAFLUSH 0x0004
#define TCIFLUSH  0x0001
#define TCIOFLUSH 0x0003
#define TCOFLUSH  0x0002
#define TCIOFF    0x0001
#define TCION     0x0002
#define TCOOFF    0x0004
#define TCOON     0x0008

#define NCCS 32

void tty_output_process_slave(pty_t* pty, uint8_t c);
void tty_output_process(pty_t* pty, uint8_t c);
void tty_input_process(pty_t*, uint8_t c);
pty_t* pty_new(struct winsize* size, int index);
void pty_init();