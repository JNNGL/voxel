#include "tty.h"

#include <sys/process.h>
#include <sys/errno.h>
#include <lib/stdlib.h>
#include <lib/alloc.h>
#include <lib/string.h>

static struct {
    intptr_t index;
    pty_t* pty;
}* pty_map;
static size_t pty_map_entries;
static size_t pty_map_capacity;

static fs_node_t* pty_dir;
static fs_node_t* dev_tty;

void pty_add(pty_t* pty) {
    if (pty_map_entries >= pty_map_capacity) {
        pty_map_capacity += 32;
        pty_map = realloc(pty_map, sizeof(*pty_map) * pty_map_capacity);
    }

    pty_map[pty_map_entries].index = pty->name;
    pty_map[pty_map_entries].pty = pty;
    ++pty_map_entries;
}

void pty_remove(intptr_t index) {
    size_t i;
    for (i = 0; i <= pty_map_entries; i++) {
        if (i == pty_map_entries) {
            return;
        }

        if (pty_map[i].index == index) {
            break;
        }
    }

    --pty_map_entries;
    size_t entries_after = pty_map_entries - i;
    if (entries_after) {
        memmove(&pty_map[i], &pty_map[i + 1], sizeof(*pty_map) * entries_after);
    }
}

pty_t* pty_get(intptr_t index) {
    for (size_t i = 0; i < pty_map_entries; i++) {
        if (pty_map[i].index == index) {
            return pty_map[i].pty;
        }
    }

    return 0;
}

static void pty_write_in(pty_t* pty, uint8_t c) {
    ringbuffer_write(pty->in, 1, &c);
}

static void pty_write_out(pty_t* pty, uint8_t c) {
    ringbuffer_write(pty->out, 1, &c);
}

void tty_output_process_slave(pty_t* pty, uint8_t c) {
    if (!(pty->ios.oflag & OPOST)) {
        pty->write_out(pty, c);
        return;
    }

    if (c == '\n' && (pty->ios.oflag & ONLCR)) {
        pty->write_out(pty, '\n');
        pty->write_out(pty, '\r');
        return;
    }

    if (c == '\r' && (pty->ios.oflag & ONLRET)) {
        return;
    }

    if ((pty->ios.oflag & OLCUC) && c >= 'a' && c <= 'z') {
        c += 'A';
        c -= 'a';
    }

    pty->write_out(pty, c);
}

void tty_output_process(pty_t* pty, uint8_t c) {
    tty_output_process_slave(pty, c);
}

void tty_input_process(pty_t* pty, uint8_t c) {
    if (pty->ios.iflag & ISTRIP) {
        c &= 0x7F;
    }

    if ((pty->ios.iflag * IGNCR) && c == '\r') {
        return;
    }

    if ((pty->ios.iflag & INLCR) && c == '\n') {
        c = '\r';
    } else if ((pty->ios.iflag & ICRNL) && c == '\r') {
        c = '\n';
    }

    if ((pty->ios.iflag & IUCLC) && c >= 'A' && c <= 'Z') {
        c -= 'A';
        c += 'a';
    }

    if (pty->ios.lflag & ECHO) {
        tty_output_process(pty, c);
    }

    pty->write_in(pty, c);
}

static void tty_fill_name(pty_t* pty, char* out) {
    strcpy(out, "/dev/pts/");
    itoa(pty->name, out + 9, 10);
}

int ioctl_pty(fs_node_t* pty, size_t request, void* arg) {
    switch (request) {
        case IOCTL_TYPE:
            return IOCTL_TYPE_TTY;
        default:
            return -EINVAL;
    }
}

size_t read_pty_master(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    pty_t* pty = (pty_t*) node->device;
    return ringbuffer_read(pty->out, size, buffer);
}

size_t write_pty_master(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    pty_t* pty = (pty_t*) node->device;

    size_t processed = 0;
    for (uint8_t* ptr = buffer; processed < size; ++ptr, ++processed) {
        tty_input_process(pty, *ptr);
    }

    return processed;
}

void open_pty_master(fs_node_t* node, uint32_t flags) {

}

void close_pty_master(fs_node_t* node) {

}

fs_node_t* pty_master_create(pty_t* pty) {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    strcpy(fnode->name, "pty master");
    fnode->uid = current_process->uid;
    fnode->gid = current_process->gid;
    fnode->mask = 0666;
    fnode->device = pty;
    fnode->flags = FS_PIPE;
    fnode->read = read_pty_master;
    fnode->write = write_pty_master;
    fnode->open = open_pty_master;
    fnode->readdir = 0;
    fnode->finddir = 0;
    fnode->ioctl = ioctl_pty;
    return fnode;
}

size_t read_pty_slave(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    pty_t* pty = (pty_t*) node->device;

    if (pty->ios.cflag & ICANON) {
        return ringbuffer_read(pty->in, size, buffer);
    } else {
        if (pty->ios.cc[VMIN] == 0) {
            size_t readable = ringbuffer_readable(pty->in);
            return ringbuffer_read(pty->in, size < readable ? size : readable, buffer);
        } else {
            return ringbuffer_read(pty->in, pty->ios.cc[VMIN] < size ? pty->ios.cc[VMIN] : size, buffer);
        }
    }
}

size_t write_pty_slave(fs_node_t* node, size_t offset, size_t size, uint8_t* buffer) {
    pty_t* pty = (pty_t*) node->device;

    size_t processed = 0;
    for (uint8_t* ptr = buffer; processed < size; ++ptr, processed++) {
        tty_output_process_slave(pty, *ptr);
    }

    return processed;
}

void open_pty_slave(fs_node_t* node, uint32_t flags) {

}

void close_pty_slave(fs_node_t* node) {
    pty_t* pty = (pty_t*) node->device;
    if (pty->name) {
        pty_remove(pty->name);
    }
}

fs_node_t* pty_slave_create(pty_t* pty) {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    strcpy(fnode->name, "pty slave");
    fnode->uid = current_process->uid;
    fnode->gid = current_process->gid;
    fnode->mask = 0620;
    fnode->device = pty;
    fnode->flags = FS_CHARDEVICE;
    fnode->read = read_pty_slave;
    fnode->write = write_pty_slave;
    fnode->open = open_pty_slave;
    fnode->close = close_pty_slave;
    fnode->readdir = 0;
    fnode->finddir = 0;
    fnode->ioctl = ioctl_pty;
    return fnode;
}

static size_t readlink_dev_tty(fs_node_t* node, char* buf, size_t size) {
    pty_t* pty = 0;
    for (uint32_t i = 0; i < (current_process->fd_length < 3 ? current_process->fd_length : 3); i++) {
        fs_node_t* node = current_process->fds[i];
        if (!node) {
            continue;
        }

        if (ioctl_fs(node, IOCTL_TYPE, 0) == IOCTL_TYPE_TTY) {
            pty = (pty_t*) node->device;
            break;
        }
    }

    char name[256];
    if (!pty) {
        strcpy(name, "/dev/null");
    } else {
        pty->fill_name(pty, name);
    }

    size_t len = strlen(name) + 1;
    if (size < len) {
        memcpy(buf, name, size);
        buf[size - 1] = 0;
        return size - 1;
    } else {
        memcpy(buf, name, len);
        return len - 1;
    }
}

static fs_node_t* create_dev_tty() {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, "tty");
    fnode->mask = 0777;
    fnode->uid = 0;
    fnode->gid = 0;
    fnode->flags = FS_FILE | FS_SYMLINK;
    fnode->readlink = readlink_dev_tty;
    fnode->length = 1;
    fnode->nlink = 1;
    return fnode;
}

static struct dirent* readdir_pty(fs_node_t* node, size_t index) {
    if (index == 0) {
        struct dirent* dirent = malloc(sizeof(struct dirent));
        memset(dirent, 0, sizeof(struct dirent));
        dirent->ino = 0;
        strcpy(dirent->name, ".");
        return dirent;
    }

    if (index == 1) {
        struct dirent* dirent = malloc(sizeof(struct dirent));
        memset(dirent, 0, sizeof(struct dirent));
        dirent->ino = 0;
        strcpy(dirent->name, "..");
        return dirent;
    }

    index -= 2;

    if (index >= pty_map_entries) {
        return 0;
    }

    pty_t* pty = pty_map[index].pty;
    if (!pty) {
        return 0;
    }

    struct dirent* dirent = malloc(sizeof(struct dirent));
    memset(dirent, 0, sizeof(struct dirent));
    dirent->ino = pty->name;
    itoa(pty->name, dirent->name, 10);
    return dirent;
}

static fs_node_t* finddir_pty(fs_node_t* node, char* name) {
    if (!name) {
        return 0;
    }

    if (strlen(name) < 1) {
        return 0;
    }

    intptr_t n = atoi(name, 10);
    pty_t* pty = pty_get(n);
    if (!pty) {
        return 0;
    }

    fs_node_t* copy = malloc(sizeof(fs_node_t)); // TODO: Don't create copies
    memcpy(copy, pty->slave, sizeof(fs_node_t));
    return copy;
}

static fs_node_t* create_pty_dir() {
    fs_node_t* fnode = malloc(sizeof(fs_node_t));
    memset(fnode, 0, sizeof(fs_node_t));
    fnode->inode = 0;
    strcpy(fnode->name, "pty");
    fnode->mask = 0555;
    fnode->uid = 0;
    fnode->gid = 0;
    fnode->flags = FS_DIRECTORY;
    fnode->read = 0;
    fnode->write = 0;
    fnode->open = 0;
    fnode->close = 0;
    fnode->readdir = readdir_pty;
    fnode->finddir = finddir_pty;
    fnode->nlink = 1;
    return fnode;
}

void tty_output_process_slave(pty_t* pty, uint8_t c);
void tty_output_process(pty_t* pty, uint8_t c);
void tty_input_process(pty_t*, uint8_t c);

pty_t* pty_new(struct winsize* size, int index) {
    pty_t* pty = malloc(sizeof(pty_t));
    pty->in = ringbuffer_alloc(4096);
    pty->out = ringbuffer_alloc(4096);
    pty->master = pty_master_create(pty);
    pty->slave = pty_slave_create(pty);
    pty->name = index;
    pty->fill_name = tty_fill_name;
    pty->write_in = pty_write_in;
    pty->write_out = pty_write_out;
    if (size) {
        memcpy(&pty->size, size, sizeof(struct winsize));
    } else {
        pty->size.row = 25;
        pty->size.col = 80;
    }
    pty->control_pid = 0;
    pty->foreground_pid = 0;
    pty->ios.iflag = ICRNL | BRKINT;
    pty->ios.oflag = ONLCR | OPOST;
    pty->ios.lflag = ECHO | ECHOE | ECHOK | ICANON | ISIG | IEXTEN;
    pty->ios.cflag = CREAD | CS8 | B38400;
    pty->ios.cc[VEOF] = 4;
    pty->ios.cc[VEOL] = 0;
    pty->ios.cc[VERASE] = 0x7F;
    pty->ios.cc[VINTR] = 3;
    pty->ios.cc[VKILL] = 21;
    pty->ios.cc[VMIN] = 1;
    pty->ios.cc[VQUIT] = 28;
    pty->ios.cc[VSTART] = 17;
    pty->ios.cc[VSTOP] = 19;
    pty->ios.cc[VSUSP] = 26;
    pty->ios.cc[VTIME] = 0;
    pty->ios.cc[VTIME] = 22;
    pty->ios.cc[VWERASE] = 23;
    pty->canon_buf = malloc(4096);
    pty->canon_size = 4094;
    pty->canon_len = 0;
    if (index) {
        pty_add(pty);
    }
    return pty;
}

void pty_init() {
    pty_map_capacity = 32;
    pty_map = malloc(sizeof(*pty_map) * pty_map_capacity);

    vfs_mount("/dev/pts", pty_dir = create_pty_dir());
    vfs_mount("/dev/tty", dev_tty = create_dev_tty());
}