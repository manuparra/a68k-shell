#include <dos/dos.h>
#include <inline/dos_protos.h>

extern struct DosLibrary *DOSBase;

int main(void)
{
    Execute((STRPTR)"run >CON:0/12/640/180/A68K Shell/CLOSE a68ksh", 0, 0);
    return 0;
}

