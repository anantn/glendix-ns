unsigned long RFNAMEG = 1;

#define NR_SYSCALL_RFORK 19
#define PLAN9_SYSTEM_CALL 0x40 
long res;
/*
#define __syscall_return_actual(type, res) \
	do { \
		if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) { \
			errno = -(res); \
			res = -1; \
		} \
		return (type) (res); \
	} while (0)


#define __syscall_return(type, res) \
	do { \
		if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) { \
			res = -1; \
		} \
		return (type) (res); \
	} while (0)


#define _syscall1( rettype, func, type1, arg1 ) \
	rettype func (type1 arg1) \
{ \
	long __res; \
	__asm__ volatile ("int $0x40" \
			      : "=a" (__res) \
			      : "0" (NR_SYSCALL_RFORK), "b" ((long)(& arg1))); \
	__syscall_return(rettype, __res); \
}

_syscall1(long, rfork, struct pt_regs, regs)
*/

long rfork(long flags) 
{
	__asm__ volatile ("int $0x40" 
			      : "=a" (res)
			      : "0" (NR_SYSCALL_RFORK), "b" ((unsigned long)(flags))); 
		do {
                if ((unsigned long)(res) >= (unsigned long)(-(128 + 1))) {
                        res = -1;
                }
                return res;
        } while (0);

}


int main()
{
	long flags = RFNAMEG;
	rfork(flags);
	return 0;
}
