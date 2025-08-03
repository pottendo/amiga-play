#include <stdio.h>
/*
 * Parallel.c
 *
 * Parallel device example
 *
 * Compile with SAS C 5.10: LC -b1 -cfistq -v -y -L
 *
 * Run from CLI only
 */

#include <exec/types.h>
#include <exec/exec.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <devices/parallel.h>
#include <hardware/cia.h>
#include <resources/cia.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>

#include <stdio.h>
#include <errno.h>


static struct MsgPort *ParallelMP = nullptr;  /* Define storage for one pointer */
static struct IOExtPar *ParallelIO = nullptr; /* Define storage for one pointer */
extern struct CIA ciab;

#define BUFSIZE 8000
static unsigned char str[BUFSIZE];

inline void set_select(bool sel)
{
    ciab.ciaddra |= CIAF_PRTRSEL; // this tell, select is an output
    if (sel)
        ciab.ciapra |= CIAF_PRTRSEL; // set it HIGH
    else
        ciab.ciapra &= ~CIAF_PRTRSEL; // set it LOW
}

void status_parport(IOExtPar *pario, const char *where = "unknown")
{
    int err = pario->IOPar.io_Error;
    pario->IOPar.io_Command = PDCMD_QUERY; /* indicate query */
    DoIO((struct IORequest *)pario);
    printf("%s(%s), err=%d: 0x%02x\n", __FUNCTION__, where, err, pario->io_Status);
}

IOExtPar *open_parport(void)
{
    if ((ParallelMP = CreatePort(0, 0)) != nullptr)
    {
        if ((ParallelIO = (struct IOExtPar *)
            CreateExtIO(ParallelMP, sizeof(struct IOExtPar))) != nullptr)
        {
            int err;
            ParallelIO->io_ParFlags = PARF_ACKMODE;
            if ((err = OpenDevice(PARALLELNAME, 0L, (struct IORequest *)ParallelIO, 0)) == 0)
            {
                ParallelIO->IOPar.io_Command = CMD_START;
                DoIO((struct IORequest *)ParallelIO);
                ParallelIO->IOPar.io_Command = CMD_RESET;
                DoIO((struct IORequest *)ParallelIO);
                ParallelIO->IOPar.io_Command = PDCMD_SETPARAMS;
                ParallelIO->IOPar.io_Flags = PARF_ACKMODE;
                DoIO((struct IORequest *)ParallelIO);
                //set_select(true);
                ciab.ciaddra |= CIAF_PRTRPOUT; // this tell, select is an output
                ciab.ciapra &= ~CIAF_PRTRPOUT; // set POUT low
                ciab.ciaddra &= ~CIAF_PRTRPOUT; // this tell, POUT is an input
                status_parport(ParallelIO, __FUNCTION__);
            }
            else
            {
                printf("%s did not open: %d\n", PARALLELNAME, err);
                DeleteExtIO((struct IORequest *)ParallelIO);
                ParallelIO = nullptr;
            }
        }
    }
    return ParallelIO;
}

void close_parport(IOExtPar *ParallelIO)
{
    AbortIO((struct IORequest *)ParallelIO); /* Ask device to abort request, if pending */
    WaitIO((struct IORequest *)ParallelIO);  /* Wait for abort, then clean up */
    CloseDevice((struct IORequest *)ParallelIO);
    DeletePort(ParallelMP);
}

int write_parport(IOExtPar *ParallelIO, const unsigned char buf[], size_t len)
{
    static ULONG WaitMask = SIGBREAKF_CTRL_C | (1L << ParallelMP->mp_SigBit);
    ULONG wm;
    if (ciab.ciapra & CIAF_PRTRPOUT)
        return 0;
    set_select(true);   // maybe redundant, but tell we're goint to write by raising SELECT
    ParallelIO->IOPar.io_Command = CMD_WRITE;
    // ParallelIO->IOPar.io_Flags = PARF_ACKMODE;
    ParallelIO->IOPar.io_Length = len;
    ParallelIO->IOPar.io_Data = (APTR)buf;
    SendIO((struct IORequest *)ParallelIO); /* execute write */
    if (ParallelIO->IOPar.io_Error)
        goto werr;
    wm = Wait(WaitMask);
    if (SIGBREAKF_CTRL_C & wm)
        return -EINTR;
    if (CheckIO((struct IORequest *)ParallelIO)) /* If request is complete... */
    {
        WaitIO((struct IORequest *)ParallelIO); /* clean up and remove reply */
        if (ParallelIO->IOPar.io_Error)
            goto werr;
    }
    else
        goto werr;
    set_select(false);   // lower SELECT to indicate we're done writing
    return ParallelIO->IOPar.io_Actual;
werr:
    //printf("error - %d\n", ParallelIO->IOPar.io_Error);
    errno = ParallelIO->IOPar.io_Error;
    set_select(false);   // lower SELECT to indicate we're done writing
    return -EIO;
}

int read_parport(IOExtPar *ParallelIO, const unsigned char buf[], size_t len)
{
    static ULONG WaitMask = SIGBREAKF_CTRL_C | (1L << ParallelMP->mp_SigBit);
    ULONG wm;
    set_select(false); 
    ParallelIO->IOPar.io_Length   = len;
    ParallelIO->IOPar.io_Data     = (APTR)buf;
    ParallelIO->IOPar.io_Command  = CMD_READ;
    SendIO((struct IORequest *)ParallelIO);
    if (ParallelIO->IOPar.io_Error)
        goto rerr;
    printf("read returned\n");
    wm = Wait(WaitMask);
    if (SIGBREAKF_CTRL_C & wm)
        return -EINTR;
    if (CheckIO((struct IORequest *)ParallelIO)) /* If request is complete... */
    {
        WaitIO((struct IORequest *)ParallelIO); /* clean up and remove reply */
        if (ParallelIO->IOPar.io_Error)
            goto rerr;
    }
    else
        goto rerr;
    return ParallelIO->IOPar.io_Actual;
rerr:
    //printf("error - %d\n", ParallelIO->IOPar.io_Error);
    errno = ParallelIO->IOPar.io_Error;
    return -EIO; 
}

void busywrite(IOExtPar *ParallelIO, int del)
{
    for (int i = 0; i < BUFSIZE; i++)
        str[i] = i;
    status_parport(ParallelIO, __FUNCTION__);
    while (1)
    {
        if (write_parport(ParallelIO, str, BUFSIZE) == -EINTR)
            break;
        Delay(del);
    }
}

static void busyread(IOExtPar *ParallelIO)
{
    int res;
    char t[8];
    char outstr[BUFSIZE * 4];
    outstr[0] = '\0';
    printf("%s: reading parport...\n", __FUNCTION__);
    while (1)
    {
        status_parport(ParallelIO, __FUNCTION__);
        if ((res = read_parport(ParallelIO, str, 1)) < 0)
        {
            if (res == -EINTR)
                printf("%s: EINTR received\n", __FUNCTION__);
            else
                printf("%s: read failed received: %d\n", __FUNCTION__, errno);
            break;
        }
        printf("%s: read %d bytes: ", __FUNCTION__, res);
        for (int i = 0; i < res; i++)
        {
            snprintf(t, 8, "%02x ", str[i]);
            strcat(outstr, t);
        }
        printf("%s\n", outstr);
    }
}

int main(int argc, char *argv[])
{
    printf("playground for parallel port - chunk = %d, EINTR = %d\n", BUFSIZE, EINTR);
    if ((ParallelIO = open_parport()) != nullptr)
    {
        //busywrite(ParallelIO, 25); // 25 ticks delay
        busyread(ParallelIO);
        close_parport(ParallelIO);
    }
}