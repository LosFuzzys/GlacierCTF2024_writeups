#include <asm/prctl.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/prctl.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

#include <seccomp.h>

void setup_seccomp(char** argv, char** env) {
  scmp_filter_ctx filt = seccomp_init(SCMP_ACT_KILL_PROCESS);
  if(!filt)
    goto fail;

  // We can't compare string argv elements to check the path
  // Not good, but better than nothing.
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(execve), 3,
    SCMP_A0(SCMP_CMP_EQ, (scmp_datum_t)argv[0]),
    SCMP_A1(SCMP_CMP_EQ, (scmp_datum_t)argv),
    SCMP_A2(SCMP_CMP_EQ, (scmp_datum_t)env)
  );

  // Needed for the LD
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(access), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
  // Only allow reading opens
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(openat), 1,
    SCMP_A2(SCMP_CMP_MASKED_EQ, O_WRONLY | O_RDWR | O_CREAT | O_APPEND | O_TRUNC, 0)
  );
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fstat64), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fstat), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 1,
    SCMP_A1(SCMP_CMP_EQ, F_GETFL)
  );
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fcntl), 1,
    SCMP_A1(SCMP_CMP_EQ, F_GETFD)
  );
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(pread64), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);

  // Needed for glibc
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(arch_prctl), 1,
    SCMP_A0(SCMP_CMP_EQ, ARCH_SET_FS)
  );
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(set_tid_address), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(rseq), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(mprotect), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(prlimit64), 0); // TODO: Maybe limit to RLIMIT_STACK
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(getrandom), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(futex), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(statfs), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(getdents64), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(sched_getaffinity), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(getcwd), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(getpid), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(gettid), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(readlink), 0);

  // Needed for ImageMagick
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(times), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(sysinfo), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigaction), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(clone), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(clone3), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(madvise), 0);

  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(writev), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 0);
  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);

  seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(mremap), 0);

  // ImageMagick needs these additional syscalls if FS caching is enabled:
  //seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0);
  //seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fchmod), 0);
  //seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(pwrite64), 0);
  //seccomp_rule_add(filt, SCMP_ACT_ALLOW, SCMP_SYS(fallocate), 0);

  prctl(PR_SET_NO_NEW_PRIVS, 1);

  if (seccomp_load(filt))
    goto fail;
  seccomp_release(filt);
  return;

fail:
  printf("ERROR: Failed to set up seccomp filter!\n");
  exit(1);
}

char ** read_env(const char* env_file) {
  char buf[1024];
  size_t allocd = 1;

  FILE* file = fopen(env_file, "r");
  if (!file)
    return NULL;

  char ** env = malloc(sizeof(*env));
  if (!env)
    goto out_close_file;
  env[0] = NULL;

  size_t i = 0;
  while (fgets(buf, sizeof(buf), file) != NULL) {
    // Check if the line is a comment
    if (buf[0] == '#')
      continue;

    size_t len = strlen(buf);

    // Check if the line is a valid assignment
    size_t env_name_len = strspn(buf, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
    // env_name_len either indexes the first REJECT (should be '=') or NULL if no REJECT.
    // It is safe to check here.
    if (env_name_len == 0 || buf[env_name_len] != '=') {
      printf("WARN: Invalid env assignment: >%s< - ignoring...\n", buf);
      continue;
    }

    // We are good to go - copy the line, remove trailing newline
    char* copy = malloc(len + 1);
    memcpy(copy, buf, len);
    if (copy[len-1] == '\n')
      copy[len-1] = '\0';
    else
      copy[len] = '\0';

    // Insert into env list
    // But at first, resize the list if necessary
    while (i >= (allocd-1)) {
      size_t new_allocd = 2*allocd;
      char ** new_env = realloc(env, new_allocd*sizeof(*env));
      if (!new_env) {
        free(env);
        goto out_close_file;
      }
      env = new_env;

      for (size_t j = allocd; j < new_allocd; j++)
        env[j] = NULL;

      allocd = new_allocd;
    }
    env[i] = copy;

    i++;
  }

out_close_file:
  fclose(file);
  return env;
}

int main(int argc, char** argv) {
  char buf[64];
  char snow_globe_path[PATH_MAX];

  if (argc != 4) {
    printf("ERROR: Invalid amount of arguments: %d\n", argc);
    printf("USAGE: %s <IMAGE_RGB> <IMAGE_IPTC> <OUT_IMG>\n", argv[0]);
  }

  int img_fd = open(argv[3], O_WRONLY | O_TRUNC | O_CREAT, 0666);
  if (img_fd < 0) {
    printf("ERROR: Could not open output image file!\n");
    exit(1);
  }
  snprintf(buf, sizeof(buf), "fd:%d", img_fd);

  // Build together snow_globe path
  if (realpath(argv[0], snow_globe_path) == NULL)
    abort();
  char* pos_last_slash = strrchr(snow_globe_path, '/');
  *(pos_last_slash+1) = '\0';
  strncpy(pos_last_slash+1, "snow_globe", sizeof(snow_globe_path) - (pos_last_slash+1 - snow_globe_path));

  char* new_argv[] = {
    snow_globe_path,
    argv[1],
    buf,
    NULL,
  };

  printf("Reading environment...\n");
  char ** new_env = read_env(argv[2]);
  if (!new_env) {
    printf("ERROR: Could not read env file!\n");
    exit(1);
  }


  setresuid(1338, 1338, 1338);
  setresgid(1337, 1337, 1337);

  printf("Setting up seccomp...\n");
  setup_seccomp(new_argv, new_env);

  printf("Running %s...\n", new_argv[0]);
  execve(new_argv[0], new_argv, new_env);
  // Should not happen
  return 127;
}
