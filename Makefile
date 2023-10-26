rwildcard=$(wildcard $(addsuffix $2, $1))$(foreach d,$(wildcard $(addsuffix *, $1)),$(call rwildcard,$d/,$2))

BUILDDIR=build

SOURCES=$(call rwildcard,kernel/,*.c) $(call rwildcard,kernel/,*.s)
OBJECTS=$(SOURCES:%=$(BUILDDIR)/%.o)

CFLAGS=-c -nostdlib -fno-builtin -fno-stack-protector -ffreestanding -Ikernel
LDFLAGS=-T link.ld -nostdlib
ASFLAGS=

CC=x86_64-elf-gcc
LD=x86_64-elf-ld
AS=x86_64-elf-as

all: $(OBJECTS) link iso

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)

link:
	@mkdir -p $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/voxel-kernel $(OBJECTS)

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
	gcc -o $(BUILDDIR)/ramdisk.mkfs ramdiskfs_tool/ramdiskfs.c ramdiskfs_tool/main.c -O2

.PHONY: ramdisk
ramdisk: ramdiskfs_tool
	$(BUILDDIR)/ramdisk.mkfs --label=RAMDISK ramdisk_base $(BUILDDIR)/voxel-ramdisk

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@
