#include "disk.h"
#include "diskmanager.h"
#include <iostream>
using namespace std;

DiskManager::DiskManager(Disk *d, int partcount, DiskPartition *dp)
{
  myDisk = d;
  partCount= partcount;
  int r = myDisk->initDisk();
  char buffer[64];

  /* If needed, initialize the disk to keep partition information */
  diskPart = new DiskPartition[partCount];
	if ( r == 1 )
	{
		//set the diskPartition to the partition passed
		diskPart = dp;

		//write superblock(meta data of meta data)
		char superBlockFiller[64];

		for( int i = 0; i < partCount; i++ )
		{
			sprintf( superBlockFiller, "%c", diskPart[i].partitionName );
			strcat( buffer, superBlockFiller );
		}
		myDisk -> writeDiskBlock( 0, buffer );
	}
  /* else  read back the partition information from the DISK1 */
	else
	{
		printf("%s\n", "You are in the read super block section");

		char size[6];
		char partName;
		int index = 0;
		int offset;

		while(1)
		{
			partName = buffer[ index * 5 ];
			if( partName == 0 )
			{
				break;
			}
			// set partition name
			diskPart[index].partitionName = partName;
			offset = 1 + ( 5 * index );
			//copy size
			strncpy( size, buffer + offset, 4 );
			//set partition size
			diskPart[index].partitionSize = atoi( size );
		}
	}

}

/*
 *   returns:
 *   0, if the block is successfully read;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds; (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::readDiskBlock(char partitionname, int blknum, char *blkdata)
{
  /* write the code for reading a disk block from a partition */
  int offset = 1;
  int index = 0;
  while(diskPart[index].partitionName != partitionname)
  {
    offset = offset + diskPart[index].partitionSize;
    index ++;
    if (index > partCount)
    {
      return -3;
    }
  }
  return (myDisk -> readDiskBlock(offset + blknum, blkdata));
}


/*
 *   returns:
 *   0, if the block is successfully written;
 *  -1, if disk can't be opened; (same as disk)
 *  -2, if blknum is out of bounds;  (same as disk)
 *  -3 if partition doesn't exist
 */
int DiskManager::writeDiskBlock(char partitionname, int blknum, char *blkdata)
{
  /* write the code for reading a disk block from a partition */
  int offset = 1;
  int index = 0;
  while(diskPart[index].partitionName != partitionname)
  {
    offset = offset + diskPart[index].partitionSize;
    index ++;
    if (index > partCount)
    {
      return -3;
    }
  }
  return (myDisk -> writeDiskBlock(offset + blknum, blkdata));
}

/*
 * return size of partition
 * -1 if partition doesn't exist.
 */
int DiskManager::getPartitionSize(char partitionname)
{
  /* write the code for returning partition size */
	int index = 0;
	//get the proper index
	while( diskPart[ index ].partitionName != partName )
	{
		// if # of partitions is exceeded exit function
		if ( index > partCount )
		{
			return -1;
		}
                index++;
	}
	return diskPart[ index ].partitionSize;
}
