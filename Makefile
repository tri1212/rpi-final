SHELL = cmd

CFILES = $(wildcard ./kernel/*.c)
OFILES = $(CFILES:./kernel/%.c=./build/%.o)

GCCFLAGS = -Wall -ffreestanding -nostdinc -nostdlib -O2
LDFLAGS = -nostdlib 

all: clean kernel8.img run

./build/boot.o: ./kernel/boot.S
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

./build/%.o: ./kernel/%.c
	aarch64-none-elf-gcc $(GCCFLAGS) -c $< -o $@

kernel8.img: ./build/boot.o $(OFILES)
	aarch64-none-elf-ld $(LDFLAGS) ./build/boot.o $(OFILES) -T ./kernel/link.ld -o ./build/kernel8.elf
	aarch64-none-elf-objcopy -O binary ./build/kernel8.elf $@

clean:
	del *.img .\build\*.elf .\build\*.o

run:
	qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
