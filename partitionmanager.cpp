#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include <iostream>
using namespace std;

//BitVector name :freeSpace

PartitionManager::PartitionManager(DiskManager *dm, char partitionname, int partitionsize)
{
  myDM = dm;
  myPartitionName = partitionname;
  myPartitionSize = myDM->getPartitionSize(myPartitionName);
  printf("sizes %d %d\n", myPartitionSize, partitionsize);
	freeSpace = new BitVector(partitionsize);
	char buffer[64];
	for(int i = 0; i < 64; i++){
		buffer[i] = 'c';
	}

  /* If needed, initialize bit vector to keep track of free and       allocted blocks in this partition */
  myDM -> readDiskBlock(myPartitionName, 0, buffer);
  if(buffer[0] != 'c')
  {
  	// create bitvector from stored data
  	freeSpace -> setBitVector((unsigned int*) buffer);
  }
  else
  {
  	//BitVector is all 0's except for blocks 0 and 1
  	freeSpace -> setBit(0);
  	freeSpace -> setBit(1);
  	
  	freeSpace -> getBitVector((unsigned int *) buffer);
  	int r;
  	r = myDM -> writeDiskBlock(myPartitionName, 0, buffer);
  	}
}

PartitionManager::~PartitionManager()
{
  delete [] freeSpace;
}

/*
 * return blocknum, -1 otherwise
 */
int PartitionManager::getFreeDiskBlock()
{
  /* write the code for allocating a partition block */
  char buffer[65];
  	for(int i = 0; i < 64; i++){
    	buffer[i] = 'c';
  	}

  	/* write the code for allocating a partition block */
	for(int index = 0; index < myPartitionSize; index++)
	{
		if(freeSpace->testBit(index) == 0)
		{
			// update bitvector
			freeSpace->setBit(index);
			freeSpace->getBitVector((unsigned int *) buffer);
			int r;
			r = myDM->writeDiskBlock(myPartitionName, 0, buffer);
			return index;
		}
	}
	return -1;
}

/*
 * return 0 for sucess, -1 otherwise
 */
int PartitionManager::returnDiskBlock(int blknum)
{
  /* write the code for deallocating a partition block */
  // if blknum is out of bounds, fail with -1
  if(blknum < 0 || blknum > myPartitionSize)
    return -1;
  // else if block is deallocated, do nothing, return 0 for success
  else if(freeSpace->testBit(blknum) == 0)
    return 0;
  // else, free the allocated block
  else{
    // create "blank" buffer filled with 'c'
    char buffer[64];
    for (int i = 0; i < 64; i++){
        buffer[i] = 'c';
    }
    // write blank buffer to disk
    myDM->writeDiskBlock(myPartitionName,blknum, buffer);
    // reset bit at location blknum
    freeSpace->resetBit(blknum);
    // get updated bitvector
    freeSpace->getBitVector((unsigned int*) buffer);
    // write updated bitVector to loc 0 of this partition
    myDM->writeDiskBlock(myPartitionName,0,buffer);

    return 0;
  }
}


int PartitionManager::readDiskBlock(int blknum, char *blkdata)
{
  return myDM->readDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::writeDiskBlock(int blknum, char *blkdata)
{
  return myDM->writeDiskBlock(myPartitionName, blknum, blkdata);
}

int PartitionManager::getBlockSize()
{
  return myDM->getBlockSize();
}
