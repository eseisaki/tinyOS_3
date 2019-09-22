
#include "tinyos.h"
#include "kernel_sched.h"
#include "kernel_proc.h"


void start_thread()
{
  rlnode* sel=&(CURPROC->ptcb_node);
  int exitval;

  Task call =  sel->ptcb->thread_task;
  int argl = sel->ptcb->argl;
  void* args = sel->ptcb->args;

  exitval = call(argl,args);
   
    ThreadExit(exitval);
}


/** 
  @brief Create a new thread in the current process.
  */
Tid_t CreateThread(Task task, int argl, void* args)
{
  TCB* tcb=CURTHREAD;
  PTCB* ptcb=create_ptcb(task, argl, args);
  
 
  tcb->owner_pcb->ptcb_node=ptcb->ptcb_node;
  rlist_push_front(& tcb->owner_pcb->ptcb_list, & tcb->owner_pcb->ptcb_node);
  
  ptcb->thread_pointer= spawn_thread(tcb->owner_pcb, start_thread);
  
  tcb->owner_ptcb=ptcb;
  wakeup(ptcb->thread_pointer);
  
  
	return (Tid_t) ptcb->thread_pointer;
}


/**
  @brief Return the Tid of the current thread.
 */
Tid_t ThreadSelf()
{
	return (Tid_t) CURTHREAD;
}

/**
  @brief Join the given thread.
  */
int ThreadJoin(Tid_t tid, int* exitval)
{
 int exit=-1;
 TCB* tcb=(TCB*) tid;
 

 
 TCB* main_t= CURTHREAD; 
 if(tcb!=main_t && tcb!=NULL && main_t->owner_pcb==tcb->owner_pcb && ThreadDetach(tid)!=0)
{
    exit=0;
   
  if( tcb->state!=EXITED){ 
     
     Mutex_Lock(& main_t->owner_pcb->pcb_spinlock);
     Cond_Wait(& main_t->owner_pcb->pcb_spinlock, &main_t->owner_pcb->ptcb_cv);
     Mutex_Unlock(& main_t->owner_pcb->pcb_spinlock);
     
   }
   
   *exitval=tcb->owner_pcb->exitval;

}
 else{
   exit=-1;
}

	return exit;
}

/**
  @brief Detach the given thread.
  */
int ThreadDetach(Tid_t tid)
{
int exit;
TCB* tcb= (TCB*) tid;

if (tid!=0 || tcb->state==EXITED)
{
exit=-1;
}
else
{
exit=0;
}
	return exit;
}

/**
  @brief Terminate the current thread.
  */
void ThreadExit(int exitval)
{
  PCB* pcb=CURPROC;
  TCB* tcb=CURTHREAD;
  PTCB* ptcb=tcb->owner_ptcb;
   Mutex_Lock(& pcb->pcb_spinlock);
   Cond_Broadcast(&pcb->ptcb_cv);
   Mutex_Unlock(& pcb->pcb_spinlock);
   
   sleep_releasing(tcb->state, & pcb->pcb_spinlock);

   if(ptcb->args) {
    free(ptcb->args);
    ptcb->args = NULL;
    ptcb->exitval=exitval;
  }
   

}


/**
  @brief Awaken the thread, if it is sleeping.

  This call will set the interrupt flag of the
  thread.

  */
int ThreadInterrupt(Tid_t tid)
{
	return -1;
}


/**
  @brief Return the interrupt flag of the 
  current thread.
  */
int ThreadIsInterrupted()
{
	return 0;
}

/**
  @brief Clear the interrupt flag of the
  current thread.
  */
void ThreadClearInterrupt()
{

}

