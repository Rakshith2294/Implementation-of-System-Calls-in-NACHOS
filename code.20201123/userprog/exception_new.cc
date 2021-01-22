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
      case SC_Exec:
		 DEBUG(dbgSys, "Exec Initiated" << " \n");
	{	
		 char buf[40],tch;
		 int temp;
		for (int i=0; (i<40) && (tch=='\0') ;i++)
			{
			 kernel->machine->ReadMem(kernel->machine->ReadRegister(4),1,&temp);
			 tch = (char) temp;
	 	 	 buf[i]= tch;
			}
		buf[i] = '\0';
		 //create a buffer and retrieve the filename (string)
		 //delete the current thread pointed to address space 
                delete kernel->currentthread->space;
		//call load function to allocate new space and load new binary 
		kernel->currentthread->space = new AddrSpace ;
		kernel->currentthread->space->Load(buf);
	{
         /* set previous programm counter (debugging only)*/
         kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
         /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
         kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
         /* set next programm counter for brach execution */
         kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
        }
        }
         return;
         ASSERTNOTREACHED();
         break;

      case SC_Exit:
	 DEBUG(dbgSys, "Exit Initiated" << " \n");
        {
	 kernel->stats->Print();
	 kernel->currentThread->Finish();
 	{
	 /* set previous programm counter (debugging only)*/
	 kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
	 /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	 kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	 /* set next programm counter for brach execution */
	 kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg) + 4);
 	}
        }
	 return;
	 ASSERTNOTREACHED();
	 break;


	case SC_Write:
	 DEBUG(dbgSys, "Write: Hello: " << kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5)  << ", "  << kernel->machine->ReadRegister(6) << " \n");
	 {
	 int t2=0;
         char tch2;
	 addr_write = (int) kernel->machine->ReadRegister(4); //addr of string to write
	 len_write = (int) kernel->machine->ReadRegister(5); //no of charaters to write
	 write_id = (int) kernel->machine->ReadRegister(6); //file descriptor
	 DEBUG(dbgSys, "Entering Write Systemcall" << " \n");
         for(t2=0; t2<len_write; t2++ ) {
           tch2=kernel->machine->mainMemory[addr_write+t2]; // perhaps will change when tlb is implemented, and use of readmem and writemem
           kernel->synchConsoleOut->PutChar(tch2);
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
        }
	 return;
	 ASSERTNOTREACHED();
	 break;

      case SC_Read:
	 DEBUG(dbgSys, "Read: Hello2: " << kernel->machine->ReadRegister(4) << ", " << kernel->machine->ReadRegister(5)  << ", "  << kernel->machine->ReadRegister(6) << " \n");
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
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
