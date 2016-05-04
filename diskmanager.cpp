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

    // initialize buffer to 'c'
     myDisk-> readDiskBlock(0,buffer);
    
    /* If needed, initialize the disk to keep partition information */
    diskPart = new DiskPartition[partCount];
	if ( r == 1 )
	{
		//set the diskPartition to the partition passed
		diskPart = dp;

		//write superblock(meta data of meta data)
		//char superBlockFiller[64];

		for( int i = 0; i < partCount; i++ )
		{
			sprintf( buffer + i*5, "%c", diskPart[i].partitionName );
			sprintf( buffer + (i*5) + 1, "%c", diskPart[i].partitionSize );
		}
		myDisk -> writeDiskBlock( 0, buffer );
	}
    /* else  read back the partition information from the DISK1 */
	else
	{
		printf("%s\n", "You are in the read super block section");

		char size[5];
		char partName;
		int index = 0;
		int offset;
        myDisk -> readDiskBlock( 0, buffer );
		while(1)
		{
			partName = buffer[ index * 5 ];
			if( partName == 'c' )
			{
				break;
			}
			// set partition name
			diskPart[index].partitionName = partName;
			offset = 1 + ( 5 * index );
			//copy size
			strncpy( size, buffer + offset, 4 );
			//printf("size is %d\n", size);
			//set partition size
			diskPart[index].partitionSize = atoi( size );
			index++;
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
	while( diskPart[ index ].partitionName != partitionname )
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
