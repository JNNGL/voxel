rwildcard=$(wildcard $(addsuffix $2, $1)) $(foreach d,$(wildcard $(addsuffix *, $1)),$(call rwildcard,$d/,$2))

BUILDDIR=build

RAMDISK_BASE=ramdisk_base

GENERAL_CFLAGS=-c -nostdlib -fno-builtin -ffreestanding
GENERAL_LDFLAGS=-nostdlib

KERNEL_DIR=kernel
KERNEL_SOURCES=$(call rwildcard,$(KERNEL_DIR)/,*.c) $(call rwildcard,$(KERNEL_DIR)/,*.s)
KERNEL_OBJECTS=$(KERNEL_SOURCES:%=$(BUILDDIR)/%.o)

kernel_CFLAGS=$(GENERAL_CFLAGS) -fno-stack-protector -I $(KERNEL_DIR) -O0 -mno-red-zone
KERNEL_LDFLAGS=$(GENERAL_LDFLAGS) -T link.ld

LIBC_DIR=libc
LIBC_CRT_SOURCES=$(call rwildcard,$(LIBC_DIR)/,*.c) $(call rwildcard,$(LIBC_DIR)/,*.s)
LIBC_CRT_OBJECTS=$(LIBC_CRT_SOURCES:%=$(BUILDDIR)/%.o)
CRT_SOURCES=$(LIBC_DIR)/crt/crt0.s $(LIBC_DIR)/crt/crti.s $(LIBC_DIR)/crt/crtn.s
CRT_OBJECTS=$(CRT_SOURCES:%=$(BUILDDIR)/%.o) $(shell $(CC) -print-file-name=crtbegin.o) $(shell $(CC) -print-file-name=crtend.o)
LIBC_SOURCES=$(filter-out $(CRT_SOURCES),$(LIBC_CRT_SOURCES))
LIBC_OBJECTS=$(LIBC_SOURCES:%=$(BUILDDIR)/%.o)

libc_CFLAGS=$(GENERAL_CFLAGS) -fno-stack-protector -I $(LIBC_DIR)/include -fPIC -nostdinc
LIBC_LDFLAGS=$(GENERAL_LDFLAGS) -shared

HOST_CC=gcc
HOST_CFLAGS=-O2

CC=x86_64-elf-gcc
LD=x86_64-elf-ld
AS=x86_64-elf-as
AR=x86_64-elf-ar

USER_CC=x86_64-voxel-gcc

all: kernel link libc userspace iso

kernel: $(KERNEL_OBJECTS)

.PHONY: libc
libc: $(LIBC_CRT_OBJECTS)
	$(LD) $(LIBC_LDFLAGS) -o $(BUILDDIR)/libc.so $(LIBC_OBJECTS)
	$(AR) cr $(BUILDDIR)/libc.a $(LIBC_OBJECTS)

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)

link:
	@mkdir -p $(BUILDDIR)
	$(LD) $(KERNEL_LDFLAGS) -o $(BUILDDIR)/voxel-kernel $(KERNEL_OBJECTS)

iso: ramdisk
	@mkdir -p $(BUILDDIR)/iso/boot/grub
	cp -f grub.cfg $(BUILDDIR)/iso/boot/grub/grub.cfg
	cp -f $(BUILDDIR)/voxel-kernel $(BUILDDIR)/iso/boot/voxel
	cp -f $(BUILDDIR)/voxel-ramdisk $(BUILDDIR)/iso/boot/voxel_ramdisk
	grub-mkrescue -o $(BUILDDIR)/voxel.iso $(BUILDDIR)/iso

bochs: all
	bochs -f .bochsrc

qemu: all
	qemu-system-x86_64 -m 512M -cdrom $(BUILDDIR)/voxel.iso

install_headers:
	rm -rf $(RAMDISK_BASE)/usr/include
	mkdir -p $(RAMDISK_BASE)/usr
	cp -rf libc/include $(RAMDISK_BASE)/usr

install_lib: install_headers libc
	rm -rf $(RAMDISK_BASE)/usr/lib
	mkdir -p $(RAMDISK_BASE)/usr/lib
	cp -f $(BUILDDIR)/libc/crt/crt0.s.o $(RAMDISK_BASE)/usr/lib/crt0.o
	cp -f $(BUILDDIR)/libc/crt/crti.s.o $(RAMDISK_BASE)/usr/lib/crti.o
	cp -f $(BUILDDIR)/libc/crt/crtn.s.o $(RAMDISK_BASE)/usr/lib/crtn.o
	cp -f $(BUILDDIR)/libc.a $(RAMDISK_BASE)/usr/lib/libc.a

.PHONY: ramdiskfs_tool
ramdiskfs_tool:
	$(HOST_CC) $(HOST_CFLAGS) -o $(BUILDDIR)/ramdisk.mkfs ramdiskfs_tool/ramdiskfs.c ramdiskfs_tool/main.c

.PHONY: ramdisk
ramdisk: ramdiskfs_tool install_headers
	mkdir -p $(RAMDISK_BASE)/bin
	cp -f $(BUILDDIR)/userspace/init $(RAMDISK_BASE)/bin
	cp -f $(BUILDDIR)/userspace/sh $(RAMDISK_BASE)/bin
	cp -f $(BUILDDIR)/userspace/hello $(RAMDISK_BASE)/bin
	cp -f $(BUILDDIR)/userspace/ls $(RAMDISK_BASE)/bin
	cp -f $(BUILDDIR)/userspace/uname $(RAMDISK_BASE)/bin
	$(BUILDDIR)/ramdisk.mkfs --label=RAMDISK $(RAMDISK_BASE) $(BUILDDIR)/voxel-ramdisk

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $($(word 2,$(subst /, ,$@))_CFLAGS) $< -o $@

$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $($(word 2,$(subst /, ,$@))_ASFLAGS) $< -o $@

$(BUILDDIR)/userspace/init: install_lib
	@mkdir -p $(@D)
	$(USER_CC) userspace/init.c -o $@

$(BUILDDIR)/userspace/sh: install_lib
	@mkdir -p $(@D)
	$(USER_CC) userspace/sh.c -o $@

$(BUILDDIR)/userspace/hello: install_lib
	@mkdir -p $(@D)
	$(USER_CC) userspace/hello.c -o $@

$(BUILDDIR)/userspace/ls: install_lib
	@mkdir -p $(@D)
	$(USER_CC) userspace/ls.c -o $@

$(BUILDDIR)/userspace/uname: install_lib
	@mkdir -p $(@D)
	$(USER_CC) userspace/uname.c -o $@

.PHONY: userspace
userspace: $(BUILDDIR)/userspace/init $(BUILDDIR)/userspace/sh $(BUILDDIR)/userspace/hello $(BUILDDIR)/userspace/ls $(BUILDDIR)/userspace/uname
