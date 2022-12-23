obj-m += kmod.o

CFLAGS = -pedantic-errors -Wall -Werror -g3 -O0 --std=c99 -fsanitize=address,undefined,leak

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

build_user:
	gcc $(CFLAGS) umod.c -o umod
