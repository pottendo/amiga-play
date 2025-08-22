#ifndef __PARPORT_H__
#define __PARPORT_H__

#include <devices/parallel.h>

IOExtPar *open_parport(void);
int close_parport(IOExtPar *ParallelIO);
int write_parport(IOExtPar *ParallelIO, const char buf[], size_t len);
int read_parport(IOExtPar *ParallelIO, char buf[], size_t len);
void status_parport(IOExtPar *pario, const char *where = "unknown");


#endif /* __PARPORT_H__ */