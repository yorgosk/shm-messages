/*
Author: Georgios Kamaras
Date: 28-11-2016
*/
#include "header.h"

using namespace std;

/* filename can be a global variable */
string filename = "text.txt"; // default value

/* main, Consumer C's, function */
int main(int argc, char const *argv[]) {
  /* checking parameters */
  if (argc != 3 && argc != 4)
  {
    cerr << "You must give 3 or 4 parameters. One integer for the number of P processes, another integer for the number of repetitions and (optional) one filename for line-reading. All in that order." << endl;
    exit(-1);
  }

  /* assign parameters' values */
  int N = atoi(argv[1]), K = atoi(argv[2]);
  if (N < 0 || K < 0)
  {
    cerr << "Both parameters' numbers must be greater or equal than zero." << endl;
    exit(-1);
  }

  /* take requested filename and test it */
  if (argc == 4) filename = argv[3];  // if 4 arguments then we expect to read from the file requested by the user
  ifstream myfile(filename.c_str());  // create file-input stream
  if (myfile.is_open()) myfile.close(); // try to open file
  else
  {
    cerr << "Unable to open file named: " << filename << endl;
    exit(-6);
  }

  /* initialize various variables that we are going to use later */
  int messages = 0, pid_match_sum = 0; // messages from Ps and sum of pid_match
  pid_t Ppid;  // pid of P

  /* creating semaphores */
  int semid = semget(SEMKEY, 4, IPC_CREAT|0660);  // create 4 semaphores
  if (semid < 0)  // test for creation error
  {
    cerr << "Semaphores' creation failed!" << endl;
    exit(-5);
  }

  /* initializing semaphores */
  semun arg;
  arg.val = 0;  // in_ds and out_ds reading semaphores (semaphores 1 and 3) will be down
  semctl(semid, 1, SETVAL, arg);
  semctl(semid, 3, SETVAL, arg);
  arg.val = 1;  // in_ds and out_ds writing semaphores (semaphores 0 and 2) will be up
  semctl(semid, 0, SETVAL, arg);
  semctl(semid, 2, SETVAL, arg);

  /* creating shared memory segments */
  int inShmId = shmget(INSHMKEY, sizeof(in_ds_comm_struct), IPC_CREAT|0666);    // in_ds's shared memory segment creation
  int outShmId = shmget(OUTSHMKEY, sizeof(out_ds_comm_struct), IPC_CREAT|0666); // out_ds's shared memory segment creation
  if (inShmId < 0 || outShmId < 0)  // test for creation error
  {
    cerr << "Shared Memory Segment's creation failed!" << endl;
    exit(-4);
  }

  /* attaching the segments to our data space and declaring our communication structures */
  in_ds_comm_struct* in_ds = (in_ds_comm_struct*)shmat(inShmId, NULL, 0);     // in_ds's shared memory segment attachment
  out_ds_comm_struct* out_ds = (out_ds_comm_struct*)shmat(outShmId, NULL, 0); // out_ds's shared memory segment attachment
  if (in_ds == (in_ds_comm_struct*)-1 || out_ds == (out_ds_comm_struct*)-1) // test for attachment error
  {
    cerr << "Shared Memory Segment's attachment failed!" << endl;
    exit(-4);
  }

  /* initialize out_ds's "steps" member for statistics */
  out_ds->steps = 0;

  /* creating producers P */
  for(int i = 0; i < N; i++)
  {
    Ppid = fork();    // fork child-producer P
    if (Ppid < 0)     // test for forking error
    {
      cerr << "P's forking failed!" << endl;
      exit(-2);
    }
    else if (Ppid == 0) // if we are in a child-producer process P
    {
      int pid_match = 0, r = 0;  // "r" is used to ensure randomality in srand
      bool stop = false;  // has it receive termination message from parent?
      while(!stop)  // while we have not received any termination message
      {
        /* read random line from text file */
        srand(getpid()+r);  // initialize srand
        r++;  // increase "r" to ensure randomality later-on
        string pline;
        randomLineFromFile(pline);
        /* write to in_ds */
        down(semid, 0); // in_ds writing semaphore down
        in_ds->pid = getpid();
        strcpy(in_ds->line, pline.c_str());
        up(semid, 1);   // in_ds reading semaphore up
        /* read from out_ds */
        down(semid, 3); // out_ds reading semaphore down
        int readpid = out_ds->pid;
        char readline[BUFFSIZE];
        strcpy(readline, out_ds->line);
        cout << "P with PID = " << getpid() << " read line: " << readline << " from C with PID = " << getppid() << "," << endl;
        if(readpid == getpid()) // if current child-producer process P is the creator of the original message
        {
          pid_match++;        // we have a pid match
          cout << "message initialized from this P" << endl;
        }
        else if(readpid == getppid()) // if (original) message's creator is parent process C
        {
          stop = true;  // we have received a termination message
        }
        else cout << "message initialized from P with PID = " << readpid << endl; // if nothing from the above, then another child-producer process P is the creator of the original message
        out_ds->steps++;  // another execution step has been concluded
        up(semid, 2);     // out_ds writing semaphore up
      }
      exit(pid_match);    // exit by, also, return pid match count for this child-producer process P
    }
  }

  /* consumer C - parent --- P processes never reach here because of the exit() above */
  int responses = 0;  // how many children acknowledge termination?
  while(messages <= K)  // while we have not received K messages from children processes P
  {
    /* read from in_ds */
    down(semid, 1);     // in_ds reading semaphore down
    int prodpid = in_ds->pid;
    char prodline[BUFFSIZE];
    strcpy(prodline, in_ds->line);
    messages++;   // one more message just received from a P
    up(semid, 0);       // in_ds writing semaphore up
    /* capitalizing message */
    for(int i = 0; i < (int)strlen(prodline); i++)
      prodline[i] = toupper(prodline[i]);
    if(messages <= K){  // if we have not yet received K messages from children processes P
      /* write to out_ds */
      down(semid, 2);   // out_ds writing semaphore down
      out_ds->pid = prodpid;
      strcpy(out_ds->line, prodline);
      up(semid, 3);     // out_ds reading semaphore up
    }
    else
    {
      int rv; // integer received by child-producer's process P exit
      /* send termination message wait for all children to finish */
      while(responses < N)  // while there are children processes P that have not responded
      {
        up(semid, 0);   // in_ds reading semaphore up
        down(semid, 2);   // out_ds writing semaphore down
        /* write termination message to out_ds */
        out_ds->pid = getpid();
        strcpy(out_ds->line, "");
        up(semid, 3);   // out_ds reading semaphore up
        wait(&rv);      // receive P's exit value
        if(WIFEXITED(rv)) pid_match_sum += WEXITSTATUS(rv); // add P's pid match to the sum of pid matches
        responses++;    // a child-producer process P has exited
      }
    }
  }

  /* print statistics */
  cout << "\tExecution ended with statistics:" << endl << "number of P = " << N << ",\ttotal steps = " << out_ds->steps << ",\tsum of pid_match = " << pid_match_sum << endl;

  /* detachments and deletions */
  shmdt(in_ds);   // in_ds's shared memory segment detachment
  shmdt(out_ds);  // out_ds's shared memory segment detachment
  shmctl(inShmId, IPC_RMID, (struct shmid_ds*)0);   // remove in_ds shared memory segment
  shmctl(outShmId, IPC_RMID, (struct shmid_ds*)0);  // remove out_ds shared memory segment
  semctl(semid, 0, IPC_RMID, 0);  // delete semaphores

  return 0;
}
