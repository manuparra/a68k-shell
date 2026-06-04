#include <dos/dos.h>
#include <inline/dos_protos.h>

extern struct DosLibrary *DOSBase;

int main(void)
{
    BPTR console;

    console = Open((STRPTR)"CON:0/12/640/180/A68K Shell/WAIT/CLOSE", MODE_NEWFILE);
    if (console == 0) {
        return 20;
    }

    Execute((STRPTR)"a68ksh", console, console);
    Close(console);

    return 0;
}
