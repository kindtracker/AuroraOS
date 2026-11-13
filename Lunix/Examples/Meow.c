static long syscall(long nr, long a0, long a1, long a2) {
  register long r0 asm("r0") = a0;
  register long r1 asm("r1") = a1;
  register long r2 asm("r2") = a2;
  register long r7 asm("r7") = nr;

  asm volatile("svc #0"
               : "+r"(r0)
               : "r"(r1), "r"(r2), "r"(r7)
               : "memory", "cc");

  return r0;
}

void _start(void) {
  static const char msg[] = "Meow\n";

  syscall(64, 1, (long)msg, sizeof(msg) - 1);
  syscall(93, 0, 0, 0);

  __builtin_unreachable();
}
