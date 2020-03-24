OBJS_KERNEL = kernel.o kasm.o

ASM = nasm
CC	= gcc

CFLAGS  =  -Wall -m32 -fleading-underscore
K_OPTION = -Wall -nostdlib #-fno-common  -fomit-frame-pointer -fno-builtin -ffreestanding

kasm.o: kasm.s
	$(ASM) kasm.s -o kasm.o -l kasm.lst -f elf

kernel: $(OBJS_KERNEL)
	gcc $^ -o $@ -Wall $(K_OPTION) -O2 -Wl,-T kernel.ld

clean:
	rm -rf *.img *.lst *.o

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
