#include <sched.h>
#include <sys/mount.h>
#include <syscall.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
	int ret;

	syscall(SYS_unshare, CLONE_NEWNS);
	ret = mount ("/tmp/tt","/tmp","none",MS_BIND,NULL);
	if(ret != 0)
		printf("problem in mount and return_value of mount is %d\n", ret);
	
	//return 0;
 	return execve("/bin/bash", NULL, NULL);
}

