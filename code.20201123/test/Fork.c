#include "syscall.h"


int
main()
{
  int i;
  int pid;
  OpenFileId output = CONSOLEOUTPUT;
  char prompt1[14] = "child process";
  char prompt2[15] = "parent process";
  char prompt3[6] = "FR01F";
  Write(prompt3, 5, output);
  pid = Fork();
  if (pid == 0) 
  {
     Write(prompt1, 13, output);
  }
  else
  {
	Write(prompt2, 14, output);
  }
}
