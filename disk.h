#ifndef DISK_H_
#define DISK_H_

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cstdlib>
#include <map>
using namespace std;

class Disk {
  public:

  int diskSize;
  int blkSize;
  int blkCount;
  char *diskFilename;

    Disk(int sz, int blksz, char *fname);
    ~Disk();
    int initDisk();
    int readDiskBlock(int blknum, char *blkdata);
    int writeDiskBlock(int blknum, char *blkdata);
    int getBlockSize() {return (blkSize);};
    int getBlockCount() {return (blkCount);};
};

#endif
