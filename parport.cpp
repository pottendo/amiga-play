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
#include "parport.h"

static struct MsgPort *ParallelMP = nullptr;  /* Define storage for one pointer */
static struct IOExtPar *ParallelIO = nullptr; /* Define storage for one pointer */
extern struct CIA ciab;

#define SELECT CIAF_PRTRPOUT    // POUT indicates parallel port write mode

inline void set_select(UBYTE pin, bool sel)
{
    ciab.ciaddra |= pin; // this tells, pin is an output
    if (sel)
        ciab.ciapra |= pin; // set it HIGH
    else
        ciab.ciapra &= ~pin; // set it LOW
}

void status_parport(IOExtPar *pario, const char *where)
{
    int err = pario->IOPar.io_Error;
    pario->IOPar.io_Command = PDCMD_QUERY; /* indicate query */
    DoIO((struct IORequest *)pario);
    printf("%s(%s), err=%d, status %s/%s/%s\n", __FUNCTION__, where, err, 
        pario->io_Status & 1 ? "busy" : "idle",
        pario->io_Status & 2 ? "parport busy" : "parport free",
        pario->io_Status & 4 ? "select" : "not selected"
    );
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
                ciab.ciaddra &= ~CIAF_PRTRBUSY; // this tell, BUSY is an input
                status_parport(ParallelIO, __FUNCTION__);
                set_select(SELECT, false); // receive/read mode: SELECT high
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

int close_parport(IOExtPar *ParallelIO)
{
    AbortIO((struct IORequest *)ParallelIO); /* Ask device to abort request, if pending */
    WaitIO((struct IORequest *)ParallelIO);  /* Wait for abort, then clean up */
    CloseDevice((struct IORequest *)ParallelIO);
    DeletePort(ParallelMP);
    return 0;
}

int write_parport(IOExtPar *ParallelIO, const char buf[], size_t len)
{
    static ULONG WaitMask = SIGBREAKF_CTRL_C | (1L << ParallelMP->mp_SigBit);
    ULONG wm;
    if (ciab.ciapra & CIAF_PRTRBUSY)
        return -EBUSY;
    set_select(SELECT, true);   // maybe redundant, but tell we're goint to write by lowering SELECT
    //status_parport(ParallelIO, __FUNCTION__);
    ParallelIO->IOPar.io_Command = CMD_WRITE;
    //ParallelIO->IOPar.io_Flags = PARF_ACKMODE;
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
    set_select(SELECT, false);   // raise SELECT to indicate we're done writing
    return ParallelIO->IOPar.io_Actual;
werr:
    //printf("error - %d\n", ParallelIO->IOPar.io_Error);
    errno = ParallelIO->IOPar.io_Error;
    set_select(SELECT, false);   // raise SELECT to indicate we're done writing
    return -EIO;
}

int read_parport(IOExtPar *ParallelIO, char buf[], size_t len)
{
    static ULONG WaitMask = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | (1L << ParallelMP->mp_SigBit);
    ULONG wm;
    set_select(SELECT, false); // tell we're going to read by raising SELECT
    while (!(ciab.ciapra & CIAF_PRTRBUSY)) // wait until BUSY is low
        Delay(1);
    ParallelIO->IOPar.io_Length   = len;
    ParallelIO->IOPar.io_Data     = (APTR)buf;
    ParallelIO->IOPar.io_Command  = CMD_READ;
    SendIO((struct IORequest *)ParallelIO);
    if (ParallelIO->IOPar.io_Error)
        goto rerr;
    //status_parport(ParallelIO, __FUNCTION__);
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
