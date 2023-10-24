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

clean:
	-rm -rf $(BUILDDIR)

link:
	@mkdir -p $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $(BUILDDIR)/voxel-kernel $(OBJECTS)

iso:
	@mkdir -p $(BUILDDIR)/iso/boot/grub
	cp -f grub.cfg $(BUILDDIR)/iso/boot/grub/grub.cfg
	cp -f $(BUILDDIR)/voxel-kernel $(BUILDDIR)/iso/boot/voxel
	grub-mkrescue -o $(BUILDDIR)/voxel.iso $(BUILDDIR)/iso

bochs: all
	bochs -f .bochsrc

$(BUILDDIR)/%.c.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -o $@

$(BUILDDIR)/%.s.o: %.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) $< -o $@