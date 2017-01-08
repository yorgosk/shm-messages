/*
Author: Georgios Kamaras
Date: 28-11-2016
*/
#ifndef __HEADER__
#define __HEADER__

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cctype>       // for toupper
#include <ctime>        // for time(NULL) in srand
#include <cstring>
#include <unistd.h>     // for getpid
#include <sys/wait.h>   // for wait
#include <sys/types.h>	// for shared memory
#include <sys/ipc.h>		// for shared memory
#include <sys/shm.h>		// for shared memory
#include <sys/sem.h>    // for semaphores
#include <vector>       // for random line-reading algorithm
#include <iomanip>      // for random line-reading algorithm

#define BUFFSIZE 255    // buffer's size for message's reading and storing
#define SEMKEY 1234     // semaphores' key
#define INSHMKEY 5678   // in_ds shared memory segment's key
#define OUTSHMKEY 8765  // out_ds shared memory segment's key

/* filename can be a global variable */
extern std::string filename;

typedef struct in_ds_communication_structure{
  int pid;              // creator's PID
  char line[BUFFSIZE];  // message's line
} in_ds_comm_struct;

typedef struct out_ds_communication_structure{
  int pid;              // initial message's creator PID
  char line[BUFFSIZE];  // message's line (post-processing / post-capitalizing)
  int steps;            // program's execution steps (for requested statistics)
} out_ds_comm_struct;

typedef union semum{
  int val;
  struct semid_ds* bf;
  unsigned short* array;
} semun;

void randomLineFromFile(std::string&);  // line-reading function
void down(int, int);  // semaphore-down function
void up(int, int);    // semaphore-up function

#endif
