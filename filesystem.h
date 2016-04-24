#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include "partitionmanager.h"


class finode{
public:
	char name[1];
	char size[5];
	char type[1];
	// 2 added attributes
	char time[13];
	//char isInRoot[1];
	char direct[3][5];
	char indirect[5];

	finode()
	{
		name[0] = 'c';
		type[0] = 'c';
		memset(size, 'c', 4);
		memset(time, 'c', 12);
		for(int i = 0; i < 4; i++)
		{
			memset(direct[i], 'c', 4);
			//printf("%s\n", point[i]);
		}
		memset(indirect, 'c', 4);
		//printf("%c\n", indirect[4]);
	}


	~finode()
	{
		free(name);
		free(size);
		free(time);
		for(int i = 0; i< 4; i++)
		{
			free(direct[i]);
		}
		free(indirect);
		free(type);
	}

	int writeFBlock(int blcknum, PartitionManager * pm){
		char dummy[65];
		char myc;
		int offset = 6;
		for(int k = 0; k < 64; k++)
		{
			dummy[k] = 'c';
		}

		dummy[0] = name[0];
		for (int i = 0; i  < 4; i++)
		{
			dummy[1+i] = size[i];
		}
		dummy[5] = type[0];
		for (int j = 0; j < 12; j++)
		{
			dummy[6+j] = time[j];
		}

		for(int k = 0; k < 3; k++)
		{
			for(int l = 0; l < 4; l++)
			{
				dummy[18+l*k+l] = direct[k][l];
			}
		}

		for(int m = 0; m < 4; m++)
		{
			dummy[29+m] = indirect[m];
			//printf("%c\n", indirect[3]);
		}

		//printf("%s\n", dummy);
		//printf("%c\n", point[0][0]);
		return pm->writeDiskBlock(blcknum, dummy);
	}

	finode* createfinode(char *buf)
	{
		finode *rtn = new finode();
		rtn->name[0] = buf[0];

		for (int i = 0; i  < 4; i++)
		{
			size[i] = buf[1+i];
		}
		type[0] = buf[5];
		for (int j = 0; j < 12; j++)
		{
			time[j] = buf[6+j];
		}

		for(int k = 0; k < 3; k++)
		{
			for(int l = 0; l < 4; l++)
			{
				direct[k][l] = buf[18+l*k+l];
			}
		}

		for(int m = 0; m < 4; m++)
		{
			indirect[m] = buf[29+m];
			//printf("%c\n", indirect[3]);
		}
		return rtn;
	}
};

class dinode{
public:
	char name[10];
	char point[10][5];
	char type[10];
	char next[5];

	dinode()
	{

		memset(name, 'c', 10);
		memset(type, 'c', 10);
		for(int i = 0; i < 10; i++)
		{
			memset(point[i], 'c', 4);
			//printf("%s\n", point[i]);
		}
		memset(next, 'c', 4);
	}


	~dinode()
	{
		free(name);
		for(int i = 0; i< 10; i++)
		{
			free(point[i]);
		}
		free(type);
		free(next);
	}

	int writeBlock(int blcknum, PartitionManager * pm){
		char dummy[65];
		char myc;
		int offset = 6;
		for(int k = 0; k < 64; k++)
		{
			dummy[k] = 'c';
		}

		for(int i = 0; i < 10; i++)
		{

			myc = point[i+1][0];
			//printf("%d\n", i);
			dummy[i*offset] = name[i];
			//printf("%c\n", name[i]);
			for(int k = 1; k < 5; k++)
			{
				dummy[i*offset+k] = point[i][k-1];

			}
			//printf("%s\n", point[i]);
			dummy[i*offset+5] = type[i];
			//printf("%c\n", point[i][0]);
		}
		for(int j = 0; j < 4; j++)
		{
			dummy[60+j] = next[j];
		}
		//printf("%s\n", dummy);
		//printf("%c\n", point[0][0]);
		return pm->writeDiskBlock(blcknum, dummy);
	}

	dinode* createdinode(char *buf)
	{
		dinode *rtn = new dinode();
		int bytes = 6;
		int offset = 0;
		for(int i = 0; i < 10; i++)
		{
			offset = i*bytes;
			rtn->name[i] = buf[offset];
			offset++;
			strncpy(rtn->point[i], buf+offset, 4);
			offset += 4;
			rtn->type[i] = buf[offset];
		}
		strncpy(rtn->next, buf+60, 4);
		return rtn;
	}
};

class iinode{
public:
	char addr[16][5];

	iinode()
	{
		for(int k = 0; k < 16; k++)
		{
			memset(addr[k], 'c', 4);
		}
	}

	~iinode()
	{
		free(addr);
	}

	int writeIBlock(int blknum, PartitionManager *pm)
	{
		char buffer[65];
		for(int j = 0; j < 16; j++)
		{
			for(int i = 0; i < 4; i++)
			{
				buffer[j*4+i] = addr[j][i];
			}
		}
		return pm->writeDiskBlock(blknum, buffer);
	}

	iinode *createiinode(char* buffer)
	{
		iinode * myII = new iinode();
		for(int j = 0; j < 16; j++)
		{
			for(int i = 0; i < 4; i++)
			{
				myII->addr[j][i] = buffer[j*4+i];
			}
		}
		return myII;
	}
};


class FileSystem {
  DiskManager *myDM;
  map<char *, int> myLockTable;
  char myfileSystemName;
  int myfileSystemSize;
  PartitionManager *myPM;
  dinode *myDI;
  char openNames[100][64];
  char myMode[100];
  int fdesc[100];
  int rwptr[100];

  /* declare other private members here */
	void deleteIInode(int blknum);

  public:
    FileSystem(DiskManager *dm, char fileSystemName);
	~FileSystem();
	int isLocked(char* filename);
	int validName(char* filename, int length);
	int doesExist(char* path);
	int findBlockNum(char* path, int &curDir, int &newDir);
	int isOpen(int fileDesc);
    int createFile(char *filename, int fnameLen);
    int createDirectory(char *dirname, int dnameLen);
    int lockFile(char *filename, int fnameLen);
    int unlockFile(char *filename, int fnameLen, int lockId);
    int deleteFile(char *filename, int fnameLen);
    int deleteDirectory(char *dirname, int dnameLen);
    int openFile(char *filename, int fnameLen, char mode, int lockId);
    int closeFile(int fileDesc);
    int readFile(int fileDesc, char *data, int len);
    int writeFile(int fileDesc, char *data, int len);
    int appendFile(int fileDesc, char *data, int len);
    int seekFile(int fileDesc, int offset, int flag);
    int renameFile(char *filename1, int fnameLen1, char *filename2, int fnameLen2);
    int getAttribute(char *filename, int fnameLen /* ... and other parameters as needed */);
    int setAttribute(char *filename, int fnameLen /* ... and other parameters as needed */);

    /* declare other public members here */


};

#endif
