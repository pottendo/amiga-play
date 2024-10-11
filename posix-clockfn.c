/* Simple_Timer.c
 *
 * A simple example of using the timer device.
 *
 * Compile with SAS C 5.10: LC -b1 -cfistq -v -y -L
 *
 * Run from CLI only
 */

#include <exec/types.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

#include <stdio.h>
#include <time.h>
#include <errno.h>

static struct timerequest *create_timer(ULONG);
static LONG get_sys_time(struct timeval *);
static void delete_timer(struct timerequest *);

int clock_gettime(clockid_t id, struct timespec *ts)
{
    struct timeval tv;

    if (get_sys_time(&tv) != 0)
    {
        errno = ENOTSUP;
        return -1;
    }
    ts->tv_nsec = tv.tv_micro * 1000;
    ts->tv_sec = tv.tv_secs;
    return 0;
}

struct timerequest *create_timer(ULONG unit)
{
    /* return a pointer to a timer request.  If any problem, return NULL */
    LONG error;
    struct MsgPort *timerport;
    struct timerequest *TimerIO;

    timerport = CreatePort(0, 0);
    if (timerport == NULL)
        return (NULL);

    TimerIO = (struct timerequest *)
        CreateExtIO(timerport, sizeof(struct timerequest));
    if (TimerIO == NULL)
    {
        DeletePort(timerport); /* Delete message port */
        return (NULL);
    }

    error = OpenDevice((CONST_STRPTR)TIMERNAME, unit, (struct IORequest *)TimerIO, 0L);
    if (error != 0)
    {
        delete_timer(TimerIO);
        return (NULL);
    }
    return (TimerIO);
}

LONG get_sys_time(struct timeval *tv)
{
    struct timerequest *tr;
    tr = create_timer(UNIT_MICROHZ);

    /* non zero return says error */
    if (tr == 0)
        return (-1);

    tr->tr_node.io_Command = TR_GETSYSTIME;
    DoIO((struct IORequest *)tr);

    /* structure assignment */
    *tv = tr->tr_time;

    delete_timer(tr);
    return (0);
}

void delete_timer(struct timerequest *tr)
{
    struct MsgPort *tp;

    if (tr != 0)
    {
        tp = tr->tr_node.io_Message.mn_ReplyPort;

        if (tp != 0)
            DeletePort(tp);

        CloseDevice((struct IORequest *)tr);
        DeleteExtIO((struct IORequest *)tr);
    }
}

