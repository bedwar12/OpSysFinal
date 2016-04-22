#include "disk.h"
#include "diskmanager.h"
#include "partitionmanager.h"
#include "filesystem.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
using namespace std;


//Mike: Unlock and Lock
//Luke: ReadFile and WriteFile
//Kyle: Seek and Create


FileSystem::FileSystem(DiskManager *dm, char fileSystemName)
{

}
int FileSystem::createFile(char *filename, int fnameLen)
{

}
int FileSystem::createDirectory(char *dirname, int dnameLen)
{

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

}
int FileSystem::deleteDirectory(char *dirname, int dnameLen)
{

}
int FileSystem::openFile(char *filename, int fnameLen, char mode, int lockId)
{

}
int FileSystem::closeFile(int fileDesc)
{

}
int FileSystem::readFile(int fileDesc, char *data, int len)
{
  return 0;
}
int FileSystem::writeFile(int fileDesc, char *data, int len)
{

  return 0;
}
int FileSystem::appendFile(int fileDesc, char *data, int len)
{

}
int FileSystem::seekFile(int fileDesc, int offset, int flag)
{

}
int FileSystem::renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2)
{

}
int FileSystem::getAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{

}
int setAttribute(char *filename, int fnameLen /* ... and other parameters as needed */)
{

}
