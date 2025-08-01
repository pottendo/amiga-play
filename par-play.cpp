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


static struct MsgPort *ParallelMP = nullptr;  /* Define storage for one pointer */
static struct IOExtPar *ParallelIO = nullptr; /* Define storage for one pointer */
extern struct CIA ciab;

#define CHUNK 8000
#define SELECT_BIT ((1L << 2)) // | (1L << 1))
static char str[CHUNK];

inline void set_select(bool sel)
{
    ciab.ciaddra |= SELECT_BIT; // this tell, select is an output
    if (sel)
    {
        ciab.ciapra |= SELECT_BIT; // set it HIGH
    }
    else
    {
        ciab.ciapra &= ~SELECT_BIT; // set it LOW
    }
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

static void busywrite(IOExtPar *ParallelIO)
{
    ULONG WaitMask; /* Collect all signals here       */
    ULONG Temp;     /* Hey, we all need pockets :-)   */
    uint8_t i = 1;

    for (int i = 0; i < CHUNK; i++)
        str[i] = i;
    /* Precalculate a wait mask for the CTRL-C, CTRL-F and message port
     * signals.  When one or more signals are received, Wait() will
     * return.  Press CTRL-C to exit the example.  Press CTRL-F to
     * wake up the example without doing anything. NOTE: A signal
     * may show up without an associated message!
     */
    WaitMask = SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F |
               1L << ParallelMP->mp_SigBit;
    printf("Sleeping until CTRL-C, CTRL-F, or write finish\n");
    set_select(true);
    status_parport(ParallelIO, __FUNCTION__);
    while (1)
    {
        ParallelIO->IOPar.io_Command = CMD_WRITE;
        ParallelIO->IOPar.io_Flags = PARF_ACKMODE;
        ParallelIO->IOPar.io_Length = CHUNK;
        // str[0] = i;
        // i++;

        ParallelIO->IOPar.io_Data = (APTR) & (str[0]);
        SendIO((struct IORequest *)ParallelIO); /* execute write */
        if (ParallelIO->IOPar.io_Error)
            printf("error - %d\n", ParallelIO->IOPar.io_Error);

        Temp = Wait(WaitMask);
        //printf("Just woke up (YAWN!)\n");
        if (SIGBREAKF_CTRL_C & Temp)
            break;

        if (CheckIO((struct IORequest *)ParallelIO)) /* If request is complete... */
        {
            WaitIO((struct IORequest *)ParallelIO); /* clean up and remove reply */
            if (ParallelIO->IOPar.io_Error)
                printf("error - %d\n", ParallelIO->IOPar.io_Error);
        }
        Delay(25);
        if ((i++ % 5) == 0)
        {
            printf("lowering SELECT\n");
            set_select(false);
            Delay(25);
            //set_select(true);
        }
    }
}

int main(int argc, char *argv[])
{
    printf("playground for parallel port - chunk = %d\n", CHUNK);
    if ((ParallelIO = open_parport()) != nullptr)
    {
        busywrite(ParallelIO);
        close_parport(ParallelIO);
    }
}