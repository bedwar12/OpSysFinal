#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
using namespace std;


//Mike: getAttribute and setAttribute
//Luke: ReadFile and appendFile and some changes to writeFile
//Kyle: CloseFile and RenameFile


FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{
	char *str;
	myDM = dm;
	myfileSystemName = fileSystemName;
	myfileSystemSize = myDM->getPartitionSize(myfileSystemName);
	char buffer[65] = "";
	myPM = new PartitionManager(myDM, myfileSystemName, myfileSystemSize);

	// Create the root directory
	myDI = new dinode();
	myDI->name[0] = fileSystemName;
	myDI->point[0][0] = '-';
	myDI->type[0] = 'd';
	myDI->writeBlock(1, myPM);

	for (int i = 0; i < 100; i++)
	{
		memset(openNames[i], 0, 64);
	}
	memset(myMode, 0, 100);
	memset(fdesc, -1, 100);
	memset(rwptr, -1, 100);
}

FileSystem::~FileSystem()
{
	free(myMode);
	for(int i = 0; i < 100; i++)
	{
		free(openNames[i]);
	}
	free(fdesc);
	free(rwptr);
}
int FileSystem::createFile(char *filename, int fnameLen)
{
  // check if name is valid
	if(!validName(filename, fnameLen))
	{
		return -3;
	}

	int result, par, child;
	result = doesExist(filename);
	printf("%d\n", result);
	if(result == (-1))
	{
		return (-1);
	}

	finode *curF;
	int fdb;
	bool success = false;
	// get free disk block
  	if((fdb = myPM->getFreeDiskBlock()) == (-1))
    {
      return (-2);
    }
	char prefix[256];
	char buffer[65];
	dinode * curDI;
	strncpy(prefix, filename, fnameLen-2);
	result = findBlockNum(prefix, par, child);
	// Look in block #child to find empty entry for new file
	curDI = myDI;
	myPM->readDiskBlock(child, buffer);
	if(child == 1)
	{
		curDI = myDI;
	}
	else
	{
		curDI = curDI->createdinode(buffer);
	}
	while(true)
	{
		// find an empty spot to put file entry
		for(int i = 0; i < 10; i++)
		{
			// free if name is 'c'
	  		if (curDI->name[i] == 'c' && curDI->type[i] == 'c')
	    	{
				time_t createTime;
		  		curDI->name[i] = filename[fnameLen-1];
				char myfdb[5];
		  		sprintf(myfdb, "%04d", fdb);
				for(int l = 0; l < 5; l++)
				{
					curDI->point[i][l] = myfdb[l];
				}
		  		curDI->type[i] = 'f';
				curDI->writeBlock(child, myPM);
		  		success = true;

				curF = new finode();
				curF->name[0] = filename[fnameLen-1];
				curF->type[0] = 'f';
				createTime = time(NULL);
				sprintf(curF->time, "%012u\n", createTime);

		  		break;
	    	}
		}

		if(success == true)
		{
			curF->writeFBlock(fdb, myPM);
	  		break;
		}

		// if a spot wasn't located, check if next is set, if not then create it
      	if(curDI->next[0] == 'c')
		{
			// use the free disk block to allocate ptr for next dinode
	  		sprintf(curDI->next, "%04d", fdb);
			result = curDI->writeBlock(child, myPM);
			// dinode created to extend next pointer
	  		dinode *newDI = new dinode();
	  		newDI->name[0] = filename[fnameLen -1];
	  		newDI->type[0] = 'f';

			// get another free disk block for the finode we create
    		int fdb2;
      		if((fdb2 = myPM->getFreeDiskBlock()) == (-1))
			{
	  			return (-2);
			}

      		sprintf(newDI->point[0], "%04d", fdb2);
      		newDI->writeBlock(fdb, myPM);
			curF->writeFBlock(fdb2, myPM);
			return 0;
		}
		else
		{
			// next has a value, get it and go to that directory
			child = atoi(curDI->next);
			char nextDI[65];
			myPM->readDiskBlock(child, nextDI);
			curDI = curDI->createdinode(nextDI);
		}
	}
	return 0;
}
int FileSystem::createDirectory(char *dirname, int dnameLen)
{
	// check if name is valid

	if(!validName(dirname, dnameLen))
	{
		return -3;
	}

	dinode *curDI;
	// get free disk block
	char prefix[256];
	char buffer[65];
	bool success = false;
		int fdb;
	int result, par, child;

	result = doesExist(dirname);
	if(result == (-1))
	{
		return -1;
	}

		if((fdb = myPM->getFreeDiskBlock()) == (-1))
		{
			return (-2);
		}
	strncpy(prefix, dirname, (dnameLen-2));
	result = findBlockNum(prefix, par, child);
	if(result == (-1))
	{
		return(-3);
	}

	// Look in block #child to find empty entry for new directory
	curDI = myDI;
	myPM->readDiskBlock(child, buffer);
	if(child == 1)
	{
		curDI = myDI;
	}
	else
	{
		curDI = curDI->createdinode(buffer);
	}
	while(true)
	{
		// find an empty spot to put directory entry
		for(int i = 0; i < 10; i++)
		{
			// free if name is 'c'
				if (curDI->name[i] == 'c')
				{
					curDI->name[i] = dirname[dnameLen-1];
				char myfdb[5];
					sprintf(myfdb, "%04d", fdb);
				for(int l = 0; l < 5; l++)
				{
					curDI->point[i][l] = myfdb[l];
				}
					curDI->type[i] = 'd';
					success = true;
					break;
				}
		}

		if(success == true)
		{
			curDI->writeBlock(child, myPM);
				break;
		}

		// if a spot wasn't located, check if next is set, if not then create it
				if(curDI->next[0] == 'c')
		{
			// use the free disk block to allocate ptr for next dinode
				sprintf(curDI->next, "%04d", fdb);
			result = curDI->writeBlock(child, myPM);
			// dinode created to extend next pointer
				dinode *newDI = new dinode();
				newDI->name[0] = dirname[dnameLen -1];
				newDI->type[0] = 'd';

			// get another free disk block for the dinode we create
				int fdb2;
					if((fdb2 = myPM->getFreeDiskBlock()) == (-1))
			{
					return (-2);
			}

					sprintf(newDI->point[0], "%04d", fdb2);
					newDI->writeBlock(fdb, myPM);
			return 0;
		}
		else
		{
			// next has a value, get it and go to that directory
			child = atoi(curDI->next);
			char nextDI[65];
			myPM->readDiskBlock(child, nextDI);
			curDI = curDI->createdinode(nextDI);
		}
	}
}
int FileSystem::lockFile(char *filename, int fnameLen)
{
  int location, par, child;

	if(!validName(filename, fnameLen))
	{
		return -4;
	}

	// check if file exists
	location = findBlockNum( filename, par, child );
	if( location == (-1) )
	{
		// file does not exist
		return (-2);
	}

	int lock;
	if( isLocked( filename ) )
	{
		return( -1 ); // already locked
	}

	//
	char buffer[65];

	// write new lock value to disk, and return it
	lock = rand();
	myLockTable[filename] = lock;
	return lock;
}

int FileSystem::unlockFile(char *filename, int fnameLen, int lockId)
{
  int lock;

	if(!validName(filename, fnameLen))
	{
		return -4;
	}
	if(isLocked(filename))
	{
		if(myLockTable[filename] == lockId)
		{
			myLockTable.erase(filename);
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else
	{
		return -2;
	}

}
int FileSystem::deleteFile(char *filename, int fnameLen)
{
	// check name validation
	if(!validName(filename, fnameLen))
	{
		return -1;
	}

	// check if locked
	if(isLocked(filename))
	{
		return -2;
	}

	// check if open
	for(int l = 0; l < 100; l++)
	{
		if(strcmp(openNames[l], filename) == 0)
		{
			return -2;
		}
	}

	int r, parAddr, addr;
	r = findBlockNum(filename, parAddr, addr);
	if(r == -1)
	{
		return -1;
	}

	char buffer[65];
	int next;
	finode *myFI = new finode();
	dinode *curDI = myDI;
	myPM->readDiskBlock(addr, buffer);
	myFI->createfinode(buffer);
	// if indirect
	if(myFI->indirect[0] != 'c')
	{
		next = atoi(myFI->indirect);
		deleteIInode(next);
		myPM->returnDiskBlock(next);
	}
	for(int i = 0; i < 3; i++)
	{
		if(myFI->direct[i][0] != 'c')
		{
			next = atoi(myFI->direct[i]);
			myPM->returnDiskBlock(next);
		}
	}
	myPM->returnDiskBlock(addr);
	if(parAddr == 1)
	{
		curDI = myDI;
	}
	else
	{
		myPM->readDiskBlock(parAddr, buffer);
		curDI = curDI->createdinode(buffer);
	}
	for(int j = 0; j < 10; j++)
	{
		if(curDI->name[j] == filename[fnameLen-1] && curDI->type[j] == 'f')
		{
			curDI->name[j] = 'c';
			curDI->type[j] = 'c';
			for(int v = 0; v < 4; v++)
			{
				curDI->point[j][v] = 'c';
			}
		}
	}
	curDI->writeBlock(parAddr, myPM);
}
int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{
	// check if name is valid

	if(!validName(dirname, dnameLen))
	{
		return -3;
	}

	int r, addr, parAddr;
	char pathname[256];
	r = findBlockNum(dirname, parAddr, addr);

	if (r == -1)
	{
		return -1;
	}
	char buf[65];
	myPM->readDiskBlock(addr, buf);
	dinode* CurDI = myDI;
	CurDI = CurDI->createdinode(buf);
	if(CurDI->next[0] != 'c')
	{
		return -2;
	}
	for (int i = 0; i < 10; i++)
	{
		if(CurDI->name[i] != 'c' || CurDI->type[i] != 'c')
		{
			return -2;
		}
	}

	myPM->returnDiskBlock(addr);
	if(parAddr == 1)
	{
		printf("%s\n", "Looking at myDI");
		CurDI = myDI;
	}
	else
	{
		myPM->readDiskBlock(parAddr, buf);
		CurDI = CurDI->createdinode(buf);
	}
	for(int j = 0; j < 10; j++)
	{
		if(CurDI->name[j] == dirname[dnameLen-1])
		{
			CurDI->name[j] = 'c';
			CurDI->type[j] = 'c';
			for(int v = 0; v < 4; v++)
			{
				CurDI->point[j][v] = 'c';
			}
		}
	}
	CurDI->writeBlock(parAddr, myPM);
}
int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{
  int r, parAddr, myAddr, lock;
	char buffer[64];
	bool read = false;
	bool write = false;
	bool rw = false;

	if(!validName(filename, fnameLen))
	{
		return -1;
	}

	// check if file exists
	r = doesExist(filename);
	if (r == (0))
	{
		return -1;
	}

	// check if lockId is correct
	if (isLocked(filename))
	{
		lock = myLockTable[filename];
	}
	else
	{
		lock = -1;
	}

	if (lockId != lock)
	{
		return -3;
	}

	// validate mode
	switch(mode){
		case 'r':
			break;
		case 'w':
			break;
		case 'm':
			break;
		default:
			return -2;
	}

	// add entry to open block table
	int i = 0;
	while(true)
	{
		// if we find an open spot
		if(openNames[i][0] == 0)
		{
			strcpy(openNames[i], filename);
			fdesc[i] = rand();
			rwptr[i] = 0;
			myMode[i] = mode;
			break;
		}
		if(i == 100)
		{
			return -4;
		}
		i++;
	}

	return fdesc[i];
}
int FileSystem::closeFile(int fileDesc)
{
	return 0;
}
int FileSystem::readFile(int fileDesc, char *data, int len)
{
  return 0;
}
int FileSystem::writeFile(int fileDesc, char *data, int len)
{

	// error checks
int index = isOpen( fileDesc );
if( index == -1 )
{
	return -1; // file isn't open
}
if( len < 0 )
{
	return -2; //negative length
	}

// get file size and check for overflow before setting
int rw = rwptr[index];
bool inFinode = true;
int par, child, finder = rw, currentBlock, index;
char buffer[64];
int result = findBlockNum( openNames[index], par, child );
myPM->readDiskBlock( child, buffer );
finode* myFile = new finode();
myFile = myFile->createfinode( buffer );
int size = atoi( myFile->size );

// find the current block
// check if rw is in a direct block
if(finder < 3*64)
	{
			currentBlock = rw / 64;

			// read currentBlock into buffer
			myPm->readBlock(myFile->direct[currentBlock],buffer);
	}
	else // we have to read in an inode and rw is somewhere in it
	{
			// new we are in the iinode
			// set finder
			finder -= (3*64);

			// create inode
			myPM -> readDiskBlock(myFile->indirect);
			iinode* myIinode = new iinode();
			myIinode0 = myIinode0->createiinode(buffer);

			//set currentBlock
			currentBlock = (finder / 64) + 3;

			// read currentBlock into buffer
			myPm->readBlock(atoi(myIinode0->addr[currentBlock]),buffer);
	}

	// start writing

	index = rw % 64;
	int dataIndex = 0;

	// write the rest of currentBlock
	while(index < 64 && rw < len){
			buffer[index] = data[dataIndex];
			myPM->writeBlock(atoi(myIinode->addr[currentBlock]),buffer);
			index++;
			dataIndex++;
			rw++;
			currentBlock++;
	}

	if(rw < 3*64) // still in the finode direct blocks
	{
			while(rw < 3*64)
			{
					strncpy(buffer, data+dataIndex, 64);
					myPm->writeBlock(atoi(myIinode->addr[currentBlock]),buffer);
					dataIndex+=64;
					rw+=64;
					currentBlock++;
			}
	}
	else// rw > 3*64, we are in the inode
	{
			if(rw >= (3*64) + (16*64))
			{
					// file is too long
			}

			// if bytes left to write is > 64, we can write block by block
			while(len - rw > 64)
			{
					strncpy(buffer, data+dataIndex, 64);
					myPm->writeBlock(atoi(myIinode->addr[currentBlock]),buffer);
					dataIndex+=64;
					rw+=64;
					currentBlock++;
			}

			// reset index for new currentBlock
			index = rw % 64;

			// write the rest of currentBlock
			while(index < 64 && rw < len)
			{
					buffer[index] = data[dataIndex];
					myPM->writeBlock(atoi(myIinode->addr[currentBlock]),buffer);
					index++;
					dataIndex++;
					rw++;
					currentBlock++;
			}
	}
return 0;
}
int FileSystem::appendFile(int fileDesc, char *data, int len)
{
	return 0;
}
int FileSystem::seekFile(int fileDesc, int offset, int flag)
{
	int index = isOpen( fileDesc );
	if( index == -1 )
		return -1; // file isn't open

	// find desired rw
	int rw;
	if( flag = 0 )
	{
		rw = rwptr[index] + offset;
	}
	else
	{
		rw = offset;
	}

	// get file size and check for overflow before setting
	int par, child;
	char buffer[65];
	int result = findBlockNum( openNames[index], par, child );
	myPM->readDiskBlock( child, buffer );
	finode* myFile = new finode();
	myFile = myFile->createfinode( buffer );
	int size = atoi( myFile->size );
	if( rw > 0 && rw + offset < size )
	{
		rwptr[index] = rw;
		return 0; // success
	}
	else
		return -2; // overflow attempt
}
int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{
	return 0;
}
int FileSystem::getAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{
	int inode = getFileInode(filename,fnameLen);
	if(inode == -1) return inode;//file not found
	if(inode < 0) return -3;//other error
	char inodeBuff[myDM->getBlockSize()];
	myPM->readDiskBlock(inode,inodeBuff);
	readCount = readIntFromBuffer(inodeBuff+22);
	writeCount = readIntFromBuffer(inodeBuff+26);
	return 0;
}
int setAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{
	int inode = getFileInode(filename,fnameLen);
	if(inode == -1) return inode;//file not found
	if(inode < 0) return -3;//other error
	for(int i = 0; i < (int)fileDescriptors.size(); ++i)
	{
	 if(fileDescriptors[i].file_inode_block == inode)return -2;
	}
	for(int i = 0; i < (int)fileLocks.size(); ++i)
	{
	 if(fileLocks[i].file_inode_block == inode)return -2;
	}
	char inodeBuff[myDM->getBlockSize()];
	myPM->readDiskBlock(inode,inodeBuff);
	writeIntToBuffer(inodeBuff+22,readCount);
	writeIntToBuffer(inodeBuff+26,writeCount);
	myPM->writeDiskBlock(inode,inodeBuff);
	return 0;
}

int File:System::isOpen ( int fileDesc )
{
	for ( int i = 0; i < 100; i++ )
	{
		if ( fdesc[i] == fileDesc )
		{
			return i;
		}
	}
	return -1;
}

int FileSystem::findBlockNum(char* path, int &curAddr, int &newAddr)
{
	char curPath;
	curAddr = 1;
	newAddr = 1;
	int i = 1;
	dinode *curDI = myDI;
	char nextDI[65];

	while(true)
	{
		if(isalpha(path[i]))
		{
			curPath = path[i];
		}
		else
		{
			return 0;
		}
		for(int b = 0; b < 10; b++)
		{
			//if exists
	      	if (curDI->name[b] == curPath && curDI->type[b] != 'c')
			{
				curAddr = newAddr;
				newAddr = atoi(curDI->point[b]);
				break;
			}
	    }

		if(curAddr != newAddr)
		{
			myPM->readDiskBlock(newAddr, nextDI);
			curDI = curDI->createdinode(nextDI);
		}
		else if(curDI->next[0] == 'c')
		{
			return -1;
		}
		else
		{
			newAddr = atoi(curDI->next);
			curAddr = newAddr;
			myPM->readDiskBlock(curAddr, nextDI);
			curDI = curDI->createdinode(nextDI);
		}
		i+=2;
	}
}
