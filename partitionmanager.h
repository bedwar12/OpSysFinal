#ifndef PARTITIONMANAGER_H_
#define PARTITIONMANAGER_H_

#include "diskmanager.h"
#include "bitvector.h"
using namespace std;

class PartitionManager {
  DiskManager *myDM;
  BitVector *freeSpace;


  public:
    char myPartitionName;
    int myPartitionSize;
    PartitionManager(DiskManager *dm, char partitionname, int partitionsize);
    ~PartitionManager();
    int readDiskBlock(int blknum, char *blkdata);
    int writeDiskBlock(int blknum, char *blkdata);
    int getBlockSize();
    int getFreeDiskBlock();
    int returnDiskBlock(int blknum);
};

#endif
