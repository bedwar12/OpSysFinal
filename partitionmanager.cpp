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

  /* If needed, initialize bit vector to keep track of free and allocted
     blocks in this partition */
}

PartitionManager::~PartitionManager()
{
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
		if(myBV->testBit(index) == 0)
		{
			// update bitvector
			myBV->setBit(index);
			myBV->getBitVector((unsigned int *) buffer);
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
