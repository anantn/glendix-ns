#define NR_SYSCALL_MOUNT 46
#define PLAN9_SYSTEM_CALL 0x40

long res;

long mount()
{
	__asm__ volatile ("int $0x40"
				: "=a" (res)
				: "0" (NR_SYSCALL_MOUNT));
		 do {
                if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) {
                        res = -1;
                }
                return res;
        } while (0);
}

int main()
{
	mount();
	return 0;

}
