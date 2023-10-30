rwildcard=$(wildcard $(addsuffix $2, $1)) $(foreach d,$(wildcard $(addsuffix *, $1)),$(call rwildcard,$d/,$2))

BUILDDIR=build

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
CRT_SOURCES=$(LIBC_DIR)/crt/crt0.s $(LIBC_DIR)/crt/crti.s $(LIBC_DIR)/crt/crtn.s $(LIBC_DIR)/crt/_main.c
CRT_OBJECTS=$(CRT_SOURCES:%=$(BUILDDIR)/%.o) $(shell $(CC) -print-file-name=crtbegin.o) $(shell $(CC) -print-file-name=crtend.o)
LIBC_SOURCES=$(filter-out $(CRT_SOURCES),$(LIBC_CRT_SOURCES))
LIBC_OBJECTS=$(LIBC_SOURCES:%=$(BUILDDIR)/%.o)

libc_CFLAGS=$(GENERAL_CFLAGS) -fno-stack-protector -I $(LIBC_DIR)
LIBC_LDFLAGS=$(GENERAL_LDFLAGS) -shared

HOST_CC=gcc
HOST_CFLAGS=-O2

CC=x86_64-elf-gcc
LD=x86_64-elf-ld
AS=x86_64-elf-as
AR=x86_64-elf-ar

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

.PHONY: ramdiskfs_tool
ramdiskfs_tool:
	$(HOST_CC) $(HOST_CFLAGS) -o $(BUILDDIR)/ramdisk.mkfs ramdiskfs_tool/ramdiskfs.c ramdiskfs_tool/main.c

.PHONY: ramdisk
ramdisk: ramdiskfs_tool
	mkdir -p ramdisk_base/bin
	cp -f $(BUILDDIR)/userspace/init ramdisk_base/bin
	$(BUILDDIR)/ramdisk.mkfs --label=RAMDISK ramdisk_base $(BUILDDIR)/voxel-ramdisk

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $($(word 2,$(subst /, ,$@))_CFLAGS) $< -o $@

$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $($(word 2,$(subst /, ,$@))_ASFLAGS) $< -o $@

$(BUILDDIR)/userspace/init:
	@mkdir -p $(@D)
	$(CC) $(CRT_OBJECTS) userspace/init.c -o $@ -nostdlib -ffreestanding

.PHONY: userspace
userspace: $(BUILDDIR)/userspace/init
