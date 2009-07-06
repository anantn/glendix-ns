#include <sched.h>
#include <syscall.h>
#include <unistd.h>
#include <stdio.h>

//int main(int argc, char *argv[]) {
int main(){
  return syscall(SYS_unshare, CLONE_NEWNS);
	
  return execv("/bin/sh", NULL);
}
