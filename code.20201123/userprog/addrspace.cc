// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you are using the "stub" file system, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#include "noff.h"

int pagetracker[128];

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
#ifdef RDATA
    noffH->readonlyData.size = WordToHost(noffH->readonlyData.size);
    noffH->readonlyData.virtualAddr = 
           WordToHost(noffH->readonlyData.virtualAddr);
    noffH->readonlyData.inFileAddr = 
           WordToHost(noffH->readonlyData.inFileAddr);
#endif 
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);

#ifdef RDATA
    DEBUG(dbgAddr, "code = " << noffH->code.size <<  
                   " readonly = " << noffH->readonlyData.size <<
                   " init = " << noffH->initData.size <<
                   " uninit = " << noffH->uninitData.size << "\n");
#endif
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
   static int initonce=0;
   DEBUG(dbgAddr,"AddrSpace: constructor");
   if (initonce==0){
        for (int i = 0; i < 128; i++) {
                pagetracker[i] = 0;
        }    // zero out the entire address space
     initonce=1;
  }
  numPages=0;
  pageTable=NULL;
    //bzero(kernel->machine->mainMemory, MemorySize);
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   DEBUG(dbgAddr,"AddrSpace: destructor");
   for (int i =0; i < this->getNumPage(); i++)
	pagetracker[pageTable[i].physicalPage] = 0 ;
   delete pageTable;
}



//***********your code goes here*****************//
//you need to add something or change something in this function//
//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

int 
AddrSpace::allocatefreepage(int start){
for (unsigned int i = start ; i < NumPhysPages ; i++){
DEBUG(dbgAddr, "allocatefreepage: pagetracker[" << i << "] " << pagetracker[i] << "\n" );
if (pagetracker[i] == 0)
{
DEBUG(dbgAddr, "allocatefreepage: Free page found:" << i << "\n" );
return i;
}
}
//panic 
DEBUG(dbgAddr, "allocatefreepage: Error found: PANIC: Free page nor found:" << "\n" );
printf("allocaefreepage: Error found : Free page not found\n");
return -1;
}

bool 
AddrSpace::Load(char *fileName) 
{
    OpenFile *executable = kernel->fileSystem->Open(fileName);
    NoffHeader noffH;
    unsigned int size;

    if (executable == NULL) {
	cerr << "Unable to open file " << fileName << "\n";
	return FALSE;
    }

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

#ifdef RDATA
// how big is address space?
    size = noffH.code.size + noffH.readonlyData.size + noffH.initData.size +
           noffH.uninitData.size + UserStackSize;	
                                                // we need to increase the size
						// to leave room for the stack
#else
// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
#endif
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG(dbgAddr, "Initializing address space: " << numPages << ", " << size);

	//setup a page table of size numpages 
	//ensure not to re use pages used by other pre existing threads 
	int f_page=0; 
	pageTable = new TranslationEntry[numPages];
	for (int i = 0; i < numPages; i++) {
		pageTable[i].virtualPage = i;	// for now, virt page # = phys page #
		f_page = allocatefreepage(f_page);
		if (f_page==-1)
		  DEBUG(dbgAddr, "Error: PANIC Free page not found \n");
		pageTable[i].physicalPage = f_page; //now holds a free page
                pagetracker[f_page]=1;
                // DEBUG(dbgAddr, "pagetracker[" << f_page << "]=" << pagetracker[f_page] << "\n");
                f_page++;
		pageTable[i].valid = TRUE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;
	}
	

// then, copy in the code and data segments into memory
// Note: this code assumes that virtual address = physical address
    if (noffH.code.size > 0) {
        DEBUG(dbgAddr, "Initializing code segment.");
	DEBUG(dbgAddr, noffH.code.virtualAddr << ", " << noffH.code.size);
        executable->ReadAt(
		&(kernel->machine->mainMemory[noffH.code.virtualAddr]), 
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG(dbgAddr, "Initializing data segment.");
	DEBUG(dbgAddr, noffH.initData.virtualAddr << ", " << noffH.initData.size);
        executable->ReadAt(
		&(kernel->machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

#ifdef RDATA
    if (noffH.readonlyData.size > 0) {
        DEBUG(dbgAddr, "Initializing read only data segment.");
	DEBUG(dbgAddr, noffH.readonlyData.virtualAddr << ", " << noffH.readonlyData.size);
        executable->ReadAt(
		&(kernel->machine->mainMemory[noffH.readonlyData.virtualAddr]),
			noffH.readonlyData.size, noffH.readonlyData.inFileAddr);
    }
#endif

    delete executable;			// close file
    return TRUE;			// success
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program using the current thread
//
//      The program is assumed to have already been loaded into
//      the address space
//
//----------------------------------------------------------------------

void 
AddrSpace::Execute() 
{

    kernel->currentThread->space = this;

    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start", which
    //  is assumed to be virtual address zero
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    // Since instructions occupy four bytes each, the next instruction
    // after start will be at virtual address four.
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}


//----------------------------------------------------------------------
// AddrSpace::Translate
//  Translate the virtual address in _vaddr_ to a physical address
//  and store the physical address in _paddr_.
//  The flag _isReadWrite_ is false (0) for read-only access; true (1)
//  for read-write access.
//  Return any exceptions caused by the address translation.
//----------------------------------------------------------------------
ExceptionType
AddrSpace::Translate(unsigned int vaddr, unsigned int* paddr, int isReadWrite) //
{
	TranslationEntry* pte;
	int pfn;
	unsigned int      vpn = vaddr / PageSize;
	unsigned int      offset = vaddr % PageSize;

	//if(vpn >= numPages) {
	//    return AddressErrorException;
	//}

	// check TLB first

	// if not in TLB, check for all pte
	bool isInMem = FALSE;

	for (int i = 0; i < numPages; i++) {
		if (pageTable[i].virtualPage == vpn && pageTable[i].valid == TRUE) {
			isInMem = TRUE;
		}
	}

	if (isInMem == TRUE) {
		pte = &pageTable[vpn];
		if (isReadWrite && pte->readOnly) {
			return ReadOnlyException;
		}

		pfn = pte->physicalPage;
		pte->use = TRUE;          // set the use, dirty bits

		if (isReadWrite)
			pte->dirty = TRUE;

		*paddr = pte->physicalPage + offset;
		cout << "PHYMEM:" << *paddr << "\n";
		ASSERT((*paddr >= 0) && ((*paddr) <= MemorySize));
		DEBUG(dbgAddr, "phys addr = " << *paddr);
		ASSERT((*paddr < MemorySize));

		//cerr << " -- AddrSpace::Translate(): vaddr: " << vaddr <<
		//  ", paddr: " << *paddr << "\n";

		return NoException;
	}
	else {
		return PageFaultException;
	}

	// if the pageFrame is too big, there is something really wrong!
	// An invalid translation was loaded into the page table or TLB.
	if (pfn >= NumPhysPages) {
		DEBUG(dbgAddr, "Illegal physical page " << pfn);
		return BusErrorException;
	}
}

TranslationEntry*
AddrSpace::getPageEntry(int pageFaultPhysicalNum) {
	return &pageTable[pageFaultPhysicalNum]; // return pointer of the required entry.
}

//your code goes here to add something into the copy constructor//
AddrSpace::AddrSpace(const AddrSpace& copiedItem) { // copy constructor
	int f_page=0;
	const TranslationEntry * oldpageTable = copiedItem.getPageTable();
	int phys_page_new_addr, phys_page_old_addr;
   DEBUG(dbgAddr,"AddrSpace: constructor (Copy Constructor)\n");
         numPages=copiedItem.getNumPage();
         pageTable = new TranslationEntry[numPages];
         for (int i = 0; i <numPages; i++) {
                DEBUG(dbgAddr, "AddrSpace: allocating page << " << i << "\n");
                pageTable[i].virtualPage = i;   
                f_page = allocatefreepage(f_page);
                if(f_page==-1)
                DEBUG(dbgAddr,"AddrSpace: Error: Page not found \n");
                pageTable[i].physicalPage = f_page; //now holds a free page
                pagetracker[f_page]=1; // note that page is no longer available in pagetracker
                DEBUG(dbgAddr, "AddrSpace: pagetracker[" << f_page << "]=" << pagetracker[f_page] << "\n");
                f_page++;
                pageTable[i].valid = TRUE;
                pageTable[i].use = FALSE;
                pageTable[i].dirty = FALSE;
                pageTable[i].readOnly = FALSE;

                // copy the contents of the page
		phys_page_new_addr = pageTable[i].physicalPage;
		phys_page_old_addr = oldpageTable[i].physicalPage;
                DEBUG(dbgAddr, "copy constructor: page from " << phys_page_old_addr << " to " << phys_page_new_addr << "\n"); 
		for (int j = 0 ; j < 128 ; j++)
		{
			// kernel->machine->mainMemory[phys_page_new_addr+j] = kernel->machine->mainMemory[phys_page_old_addr+j]; // << BUG!! WRONG
			kernel->machine->mainMemory[phys_page_new_addr*PageSize+j] = kernel->machine->mainMemory[phys_page_old_addr*PageSize+j];
		}
        }
  DEBUG(dbgAddr, "AddrSpace: copy construtor DONE\n");
	
//allocate a second page table which is of the same size as copied item 
//ensure not to reuse any pages used by other pre existing threads 
//copy the page table tranlation entry properties of every page 
//in a loop copy the content of every page from the copied item to new space 
	
}

int
AddrSpace::getNumPage() {
	return numPages;
}


TranslationEntry*
AddrSpace::getPageTable() {
	return pageTable;
}

const int
AddrSpace::getNumPage() const{
        return numPages;
}


const TranslationEntry*
AddrSpace::getPageTable() const{
        return pageTable;
}


