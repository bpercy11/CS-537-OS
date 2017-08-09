#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"


struct pstat pstat;

int
sys_getpinfo(void)
{

   struct pstat* ps;

   if(argptr(0, (void*) &ps, sizeof(ps)) < 0)
     return -1;
   getpinfo(ps);

   /*
   int i;
   for(i = 0; i < NPROC; i++){
    


     ps->inuse[i] = pstat.inuse[i];
     ps->priority[i] = pstat.priority[i];
     ps->pid[i] = pstat.pid[i];
     ps->ticks[i][0] = pstat.ticks[i][0];
     ps->ticks[i][1] = pstat.ticks[i][1];
     ps->ticks[i][2] = pstat.ticks[i][2];
     ps->ticks[i][3] = pstat.ticks[i][3];
     
     //     ps->pid[i] = getpid();
   }
   */
  return 0;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}