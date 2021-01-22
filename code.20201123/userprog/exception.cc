// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "synchconsole.h"


void stacktopfunc(void *arg) {
    DEBUG(dbgSys,"stacktopfunc: Entered void stacktopfunc(void *) \n");
    kernel->currentThread->RestoreUserState(); // restore machine registers
    kernel->currentThread->space->RestoreState(); // restore kernel->pageTable
    DEBUG(dbgSys,"stacktopfunc: arg=" << (int)arg << "\n");
    DEBUG(dbgSys,"stacktopfunc: pid=" << kernel->currentThread->pid << "\n");
    DEBUG(dbgSys,"stacktopfunc: space=" << kernel->currentThread->space << "\n");
    DEBUG(dbgSys,"stacktopfunc: PageTable[0].physicalPage=" << kernel->currentThread->space->getPageTable()[0].physicalPage << "\n");
    DEBUG(dbgSys,"stacktopfunc: " << "REGISTER: PC=" << kernel->machine->ReadRegister(PCReg) << "\n");
    DEBUG(dbgSys,"stacktopfunc: " << "REGISTER: Stack=" <<  kernel->machine->ReadRegister(StackReg) << "\n");
    DEBUG(dbgSys,"stacktopfunc: " << "REGISTER: RetAddr=" <<  kernel->machine->ReadRegister(RetAddrReg) << "\n");
    DEBUG(dbgSys,"stacktopfunc: " << "REGISTER: [2]=" <<  kernel->machine->ReadRegister(2) << "\n");
    DEBUG(dbgSys,"stacktopfunc: " << "Entering machine->Run()" << "\n");
    kernel->machine->Run();
    DEBUG(dbgSys,"stacktopfunc: " << "Returned from machine->Run()" << "\n");
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);
    int addr_read,len_write,write_id,addr_write,len_read,read_id;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
	SysHalt();
	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
	
	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);
	
	/* Modify return point */
	{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}
	return;
	ASSERTNOTREACHED();
	break;
      case SC_SysFork:
		DEBUG(dbgSys, "SysFork Initiated" << " \n");
	{
          IntStatus oldLevel;
          int newthpid=1;
        //kernel->currentThread->SaveUserState();

    DEBUG(dbgSys,"syscall:fork: A: parent pid=" << (int)kernel->currentThread->pid  << "\n");
    DEBUG(dbgSys,"syscall:fork: A: parent space=" << (int)kernel->currentThread->space  << "\n");
    DEBUG(dbgSys,"syscall:fork: A: " << " REGISTER: PC=" << kernel->machine->ReadRegister(PCReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: A: " << " REGISTER: Stack=" <<  kernel->machine->ReadRegister(StackReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: A: " << " REGISTER: RetAddr=" <<  kernel->machine->ReadRegister(RetAddrReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: A: " << " REGISTER: [2]=" <<  kernel->machine->ReadRegister(2) << "\n");


        //kernel->currentThread->RestoreUserState(); // ZZZ maybe not necessary
	//increment PC for the parent 
        /* set previous programm counter (debugging only)*/
        kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
        /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
        kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
        /* set next programm counter for brach execution */
        kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        // return the pid of the child thread to the parent process 
        //kernel->currentThread->SaveUserState();

        //if (0) {
	//Create a new child thread 
	Thread* newthread = new Thread("Child Thread");
	//Allocate an address space for the child thread 
        newthpid=newthread->pid;
	newthread->space = new AddrSpace(*kernel->currentThread->space);
        DEBUG(dbgSys,"syscall:fork: child pid=" << newthread->pid << "\n"); 
        DEBUG(dbgSys,"syscall:fork: child space=" << newthread->space << "\n" );

        // fork returns 0 to the child thread 
        kernel->machine->WriteRegister(2, 0);
	//save register states of the child process so that it can restore it later 
        newthread->SaveUserState(); // save the registers as part of the thread for later loading

	//set the child thread to be in ready status 
        //newthread->setStatus(READY); // ZZZ Thread::Fork() does this
    // disable interrupts
    //oldLevel = kernel->interrupt->SetLevel(IntOff);	
    //    kernel->scheduler->ReadyToRun(newthread); // sets the thread status to ready, and appends to scheduler readylist queue
    // re-enable interrupts
    //(void) kernel->interrupt->SetLevel(oldLevel);	
        //} // if (0) 

        //kernel->currentThread->space->RestoreState(); // ZZZ maybe not necessary
        // return the pid of the child thread to the parent process 
        kernel->machine->WriteRegister(2, newthpid);
        //kernel->currentThread->SaveUserState();


    DEBUG(dbgSys,"syscall:fork: B: parent pid=" << (int)kernel->currentThread->pid  << "\n");
    DEBUG(dbgSys,"syscall:fork: B: parent space=" << kernel->currentThread->space  << "\n");
    DEBUG(dbgSys,"syscall:fork: B: " << "REGISTER: PC=" << kernel->machine->ReadRegister(PCReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: B: " << "REGISTER: Stack=" <<  kernel->machine->ReadRegister(StackReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: B: " << "REGISTER: RetAddr=" <<  kernel->machine->ReadRegister(RetAddrReg) << "\n");
    DEBUG(dbgSys,"syscall:fork: B: " << "REGISTER: [2]=" <<  kernel->machine->ReadRegister(2) << "\n");

	//make the simulator create a thread 
	//t->fork , (threadfork) pass the function pointer that sets up the child thread stack 
        newthread->Fork( (VoidFunctionPtr)stacktopfunc, (void *)newthpid);
        
        //Wrong
        //newthread->StackAllocate(&stacktopfunc, (void *)newthread->pid) ; 
        //IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);	// disable interrupts
        //kernel->scheduler->ReadyToRun(newthread);
        //(void) kernel->interrupt->SetLevel(oldLevel);	// re-enable interrupts

	//make the parent yield 
        //kernel->currentThread->Yield();  // optionally let the child process hae a head start
    // // disable interrupts
    // DEBUG(dbgSys,"syscall:fork: parent thread about to sleep" << kernel->currentThread->pid << "\n");
    // oldLevel = kernel->interrupt->SetLevel(IntOff);	
    //     kernel->currentThread->Sleep(FALSE); // make scheduler to RUN another thread
    // // re-enable interrupts
    // (void) kernel->interrupt->SetLevel(oldLevel);	
     DEBUG(dbgSys,"syscall:fork: Leaving Fork SystemCall pid=" << kernel->currentThread->pid << "\n");
     //printf("syscall:fork: Hello 3  %d\n", newthread->pid );
	}
         return;
         ASSERTNOTREACHED();
         break;

      case SC_Exec:
		 DEBUG(dbgSys, "syscall:exec Entering Exec SystemCall" << " \n");
	{	
		char buf[40], tch=1;
		int i;
		for (i=0; (i<40) && (tch!='\0') ;i++)
			{
			 //DEBUG(dbgSys, "Hello 1 " << kernel->machine->ReadRegister(4) << " \n");	
			 tch = kernel->machine->mainMemory[kernel->machine->ReadRegister(4)+i];
	 	 	 buf[i]= tch;
			}
		buf[i] = '\0';
		//printf("%s \n",buf);
		 //create a buffer and retrieve the filename (string)
		 //delete the current thread pointed to address space 
                delete kernel->currentThread->space;
		//call load function to allocate new space and load new binary 
		kernel->currentThread->space = new AddrSpace ;
		kernel->currentThread->space->Load(buf);
		kernel->currentThread->space->RestoreState(); // set the kernel page table
		DEBUG(dbgSys, "Hello 2 " << kernel->machine->ReadRegister(4) << " \n");	
    //for (i = 0; i < NumTotalRegs; i++)
	//kernel->machine->WriteRegister(i, 0);
                //kernel->currentThread->space->InitRegisters();		// set the initial register values
               //kernel->currentThread->SaveUserState(); 
                
	{
         /* set previous programm counter (debugging only)*/
         kernel->machine->WriteRegister(PrevPCReg, 0);
         /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
         kernel->machine->WriteRegister(PCReg, 4);
         /* set next programm counter for brach execution */
         kernel->machine->WriteRegister(NextPCReg, 4);
        }
	 DEBUG(dbgSys, "syscall:exec: Calling addrspace->execute " << " \n");
         kernel->currentThread->space->Execute();
	 DEBUG(dbgSys, "syscall:exec: Leaving Exec Systemcall" << " \n");
        }
         return;
         ASSERTNOTREACHED();
         break;
      
      case SC_Exit:
	 DEBUG(dbgSys, "syscall:exit: Entering Exit Systemcall" << " \n");
        {
	 int exitretval = (int) kernel->machine->ReadRegister(4); // exit status 
         if ( kernel->currentThread->pid == 0 ) {
              // usually 2 threads are left :main-thread and postal-worker-thread  
             while(kernel->ProcessTable->NumInList()>2) {
                DEBUG(dbgSys,"syscall:exit: main process waiting on other threads #threads=" << kernel->ProcessTable->NumInList() << "\n");
                //kernel->currentThread->setStatus(BLOCKED);
	        kernel->currentThread->Yield(); // calls Sleep(true) 
             }
        //   if ( kernel->ProcessTable->NumInList()==1 ) {
        //     DEBUG(dbgSys,"syscall:exit: main process exitting, only proc\n");
	//     kernel->currentThread->Finish(); // calls Sleep(true) which goes to BLOCKED
        //   } else  { // ( kernel->ProcessTable->NumInList()>1 ) {
        //     DEBUG(dbgSys,"syscall:exit: main process exitting, more proc \n");
        //     //kernel->currentThread->setStatus(BLOCKED);
        //     while(kernel->ProcessTable->NumInList()>1) 
	//        kernel->currentThread->Yield(); // calls Sleep(true) 
        //   }
        // }else {
        //   DEBUG(dbgSys,"syscall:exit: process pid!=0 exitting\n");
        // }
	     kernel->stats->Print();
         }
	 DEBUG(dbgSys, "syscall:exit: Calling Thread->Finish() on pid=" << kernel->currentThread->pid << " \n");
	 kernel->currentThread->Finish(); // calls Sleep(true) 
         // The thread destructor does not want you to delete the current thread
         //  delete kernel->currentThread; // will remove from processtable and delete the addrspace
         //  kernel->currentThread=NULL;
         // need to tell the thread 
         //if ( (kernel->currentThread->pid == 0) && (kernel->ProcessTable->NumInList()==1) )
	 //   SysHalt();
         

   //   does not make sense to increase program counter if the process is calling exit i.e. ending
   //   {
   //	  /* set previous programm counter (debugging only)*/
   //	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
   //     /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
   //	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
   //	  /* set next programm counter for brach execution */
   //	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
   //   }
        kernel->machine->WriteRegister(2, exitretval);
	 DEBUG(dbgSys, "syscall:exit: Leaving Exit Systemcall" << " \n");
        }
	 return;
	 ASSERTNOTREACHED();
	 break;


	case SC_Write:
	 DEBUG(dbgSys, "syscall:write: Hello: pid=" << kernel->currentThread->pid << ", args={" << kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5)  << ", "  << kernel->machine->ReadRegister(6) << " }\n");
	 {
	 int t2=0, tchi;
         char tch2;
	 addr_write = (int) kernel->machine->ReadRegister(4); //addr of string to write
	 len_write = (int) kernel->machine->ReadRegister(5); //no of charaters to write
	 write_id = (int) kernel->machine->ReadRegister(6); //file descriptor
	 DEBUG(dbgSys, "syscall:write: Entering Write Systemcall" << " \n");
         for(t2=0; t2<len_write; t2++ ) {
           //tch2=kernel->machine->mainMemory[addr_write+t2]; // perhaps will change when pagetables is implemented, and use of readmem and writemem
           kernel->machine->ReadMem(addr_write+t2,1,&tchi); 
           tch2=(char)tchi;
           //IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
           kernel->synchConsoleOut->PutChar(tch2);
           //kernel->interrupt->SetLevel(oldLevel);
         }	
	 kernel->machine->WriteRegister(2,len_write);
	 /* Modify return point */
 	{
	 /* set previous programm counter (debugging only)*/
	 kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	 /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	 kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	 /* set next programm counter for brach execution */
	 kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
 	}
	 DEBUG(dbgSys, "syscall:write: Leaving Write Systemcall" << " \n");
        }
	 return;
	 ASSERTNOTREACHED();
	 break;

      case SC_Read:
	 DEBUG(dbgSys, "syscall:read: Hello2: args={" << kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5)  << ", "  << kernel->machine->ReadRegister(6) << "} \n");
         {
	 int t1 ; //, sizeread=0 ; //read(arg3,buf,arg2);
         char tch1;
	 addr_read = (int) kernel->machine->ReadRegister(4); //addr of string to store
	 len_read = (int) kernel->machine->ReadRegister(5); //no of charaters to read
	 read_id = (int) kernel->machine->ReadRegister(6); //file descriptor
	 DEBUG(dbgSys, "Entering Read Systemcall" << " \n");
//	 DEBUG(dbgSys, "Read: Hello2: " << arg1 << ", " << arg2  << ", "  << arg3 << " \n");
         t1=0; 
         while(t1<len_read && tch1 != '\n' ) {
           tch1=kernel->synchConsoleIn->GetChar();
           kernel->machine->mainMemory[addr_read+t1] = tch1; // when TLB functionaility is done, readmeme, writemem can be used
           //buf[sizeread++]=tch;
           t1++;
         }
         kernel->machine->mainMemory[t1]='\0';
	 if(t1<0) DEBUG(dbgSys,"Error reading from file\n");
	 DEBUG(dbgSys, "Read: Hello: " << t1 << " \n");
	 kernel->machine->WriteRegister(2,t1);
 	 {
 	 /* set previous programm counter (debugging only)*/
	 kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	 /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	 kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
 	 /* set next programm counter for brach execution */
 	 kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
         }
	 DEBUG(dbgSys, "syscall:read: Leaving Read Systemcall" << " \n");
         }
 	 return;
         ASSERTNOTREACHED();
         break;
      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      }
      break;
    default:
      cerr << "Unexpected user mode exception " << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
