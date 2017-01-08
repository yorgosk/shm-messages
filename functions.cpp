/*
Author: Georgios Kamaras
Date: 28-11-2016
*/
#include "header.h"

using namespace std;

/* line-reading function */
void randomLineFromFile(string& line){
  // Produce a random line continuously
  ifstream myfile(filename.c_str());  // open file for input
  vector<string> positions;
  vector<string>::size_type linenum;
  string buffer;
  while(getline(myfile, buffer, '\n'))  // read each line and place it on a vector
    positions.push_back(buffer);
  linenum = static_cast<vector<string >::size_type>((double(rand())/RAND_MAX)*positions.size());  // get a random line from the text file
  line = positions[linenum].c_str();  // randomly picked line
  myfile.close(); // close file
}

/* semaphore-down function */
void down(int semid, int semnum){
  struct sembuf semopr;
  semopr.sem_num = semnum;  // which semaphore we are placing "down"
  semopr.sem_op = -1;
  semopr.sem_flg = 0;
  if (semop(semid, &semopr, 1) == -1) // test for "down"'s failure
  {
    cerr << "Semaphore down operation failed!" << endl;
    exit(-7);
  }
}

/* semaphore-up function */
void up(int semid, int semnum){
  struct sembuf semopr;
  semopr.sem_num = semnum;  // which semaphore we are placing "up"
  semopr.sem_op = 1;
  semopr.sem_flg = 0;
  if (semop(semid, &semopr, 1) == -1) // test for "up"'s failure
  {
    cerr << "Semaphore down operation failed!" << endl;
    exit(-7);
  }
}
