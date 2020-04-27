# Order is important?
# first object binary comes to first in kernel.
# kasm has entry point and call kernel start in kernel.c.
# So I need to put kasm first.

AS = nasm
CC = gcc

ASFLAGS = -f elf64
CFLAGS  =  -Wall -g -fleading-underscore -fno-builtin -nostdlib -I core -I dev -I lib -I net -I user

OBJS_KERNEL = core/kasm.o
OBJS_KERNEL += core/kernel.o
OBJS_KERNEL += core/int.o
OBJS_KERNEL += core/dsctbl.o
OBJS_KERNEL += core/timer.o
OBJS_KERNEL += core/memory.o
OBJS_KERNEL += core/task.o
OBJS_KERNEL += core/workqueue.o
OBJS_KERNEL += core/netdev.o
OBJS_KERNEL += core/pci.o
OBJS_KERNEL += core/syscall.o
OBJS_KERNEL += core/asm.o
OBJS_KERNEL += core/intasm.o
OBJS_KERNEL += core/keyboard.o
OBJS_KERNEL += net/ether.o
OBJS_KERNEL += net/netutil.o
OBJS_KERNEL += net/arp.o
OBJS_KERNEL += net/ip.o
OBJS_KERNEL += net/icmp.o
OBJS_KERNEL += net/raw.o
OBJS_KERNEL += net/udp.o
OBJS_KERNEL += net/tcp.o
OBJS_KERNEL += lib/list.o
OBJS_KERNEL += lib/lib.o
OBJS_KERNEL += lib/print.o
OBJS_KERNEL += dev/r8169.o

# Will build separately
OBJS_KERNEL += user/user_syscall.o
OBJS_KERNEL += user/user_syscall_asm.o
OBJS_KERNEL += user/user_app.o
OBJS_KERNEL += user/dns.o

kernel: $(OBJS_KERNEL)
	$(LD) -N -T core/kernel.ld -o $@ $^

clean:
	find . -name "*.o" | xargs rm -f
	find . -name "*.lst" | xargs rm -f
	rm -f kernel

# gcc options I thought related
# (descriptions are picked from gcc man page)
#
# -fleading-underscore
#  This option and its counterpart, -fno-leading-underscore,
#  forcibly change the way C symbols are represented in the object file.
#  One use is to help link with legacy assembly code.
#  (By having this, Assembler can call C lang funcion (e.g. func())  as _func)
#
# -fno-common
#  In C code, controls the placement of uninitialized global variables.
#  The -fno-common option specifies that the compiler should place uninitialized global variables in the data section of the object file,
#  rather than generating them as common blocks.
#  This has the effect that if the same variable is declared (without "extern") in two different compilations, you
#  get a multiple-definition error when you link them.
#
# -ffreestanding
#  Assert that compilation targets a freestanding environment.
#  This implies -fno-builtin.  A freestanding environment is one in which the standard library may not
#  exist, and program startup may not necessarily be at "main".
#  The most obvious example is an OS kernel.  This is equivalent to -fno-hosted.
#
# -fno-builtin
#   GCC normally generates special code to handle certain built-in functions more efficiently; for instance, calls to "alloca" may become single instructions which
#   adjust the stack directly, and calls to "memcpy" may become inline copy loops.  The resulting code is often both smaller and faster, but since the function calls no
#   longer appear as such, you cannot set a breakpoint on those calls, nor can you change the behavior of the functions by linking with a different library.  In
#   addition, when a function is recognized as a built-in function, GCC may use information about that function to warn about problems with calls to that function, or to
#   generate more efficient code, even if the resulting code still contains calls to that function.  For example, warnings are given with -Wformat for bad calls to
#   "printf" when "printf" is built in and "strlen" is known not to modify global memory.
#
# -fomit-frame-pointer
#   Don't keep the frame pointer in a register for functions that don't need one.  This avoids the instructions to save, set up and restore frame pointers; it also makes
#   an extra register available in many functions.  It also makes debugging impossible on some machines.
#
# -nostdlib
#  Do not use the standard system startup files or libraries when linking.  No startup files and only the libraries you specify are passed to the linker, and options
#  specifying linkage of the system libraries, such as -static-libgcc or -shared-libgcc, are ignored.
