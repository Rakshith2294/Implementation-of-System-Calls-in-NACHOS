#include "syscall.h"

int
main()
{
	SpaceId newProc;
	OpenFileId output = CONSOLEOUTPUT;
	char feedback[17] = "1234567890123456";
	Write(feedback, 17, output);
}
