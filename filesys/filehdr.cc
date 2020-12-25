// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool FileHeader::Allocate(BitMap *freeMap, int fileSize)
{
    numBytes = fileSize;
    numSectors = divRoundUp(fileSize, SectorSize);
    if (freeMap->NumClear() < numSectors)
        return FALSE;

    if (numSectors < NumDirect)
    {
        for (int i = 0; i < numSectors; i++)
            dataSectors[i] = freeMap->Find();
        dataSectors[NumDirect - 1] = -1;
    }
    else
    {
        for (int i = 0; i < NumDirect; i++)
            dataSectors[i] = freeMap->Find(); //最后一项是二级块的地址
        int dataSectors2[NumIndirect];
        for (int i = 0; i < numSectors - NumDirect + 1; i++)
            dataSectors2[i] = freeMap->Find();
        //将二级索引保存
        synchDisk->WriteSector(dataSectors[NumDirect - 1], (char *)dataSectors2);
    }
    return TRUE;
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void FileHeader::Deallocate(BitMap *freeMap)
{
    if (numSectors < NumDirect)
        for (int i = 0; i < numSectors; i++)
        {
            ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
            freeMap->Clear((int)dataSectors[i]);
        }
    else
    {
        int dataSectors2[NumIndirect];
        synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)dataSectors2);

        for (int i = 0; i < NumDirect; i++)
        {
            ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
            freeMap->Clear((int)dataSectors[i]);
        }
        for (int i = 0; i < numSectors - NumDirect + 1; i++)
        {
            ASSERT(freeMap->Test((int)dataSectors2[i])); // ought to be marked!
            freeMap->Clear((int)dataSectors2[i]);
        }
    }
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void FileHeader::FetchFrom(int sector)
{
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int FileHeader::ByteToSector(int offset)
{
    int sec = offset / SectorSize;
    if (sec < NumDirect - 1)
        return (dataSectors[sec]);
    else
    {
        int dataSectors2[NumIndirect];
        synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)dataSectors2);
        return (dataSectors2[sec - NumDirect + 1]);
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void FileHeader::Print()
{
    int i, j, k;
    char *data = new char[SectorSize];

    if (numSectors < NumDirect)
    {

        printf("文件大小: %d.  直接索引:\n", numBytes);
        for (i = 0; i < numSectors; i++)
            printf("%d ", dataSectors[i]);
    }
    else
    {
        printf("二级索引：");
        int dataSectors2[NumIndirect];
        synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)dataSectors2);
        printf("文件大小: %d.  文件直接索引:\n", numBytes);
        for (i = 0; i < NumDirect - 1; i++)
            printf("%d ", dataSectors[i]);
        printf("\n文件二级索引扇区编号：%d ", dataSectors[NumDirect - 1]);
        for (i = 0; i < numSectors - NumDirect + 1; i++)
            printf("%d ", dataSectors2[i]);
    }
    delete[] data;
}

bool FileHeader::SetLength(BitMap *freeMap, int size)
{
    if (size > numSectors * SectorSize)
    {
        int oldNum = numSectors;
        numSectors = divRoundUp(len, SectorSize);
        if (freeMap->NumClear() < numSectors - oldNum)
        {
            numSectors = oldNum;
            return false;
        }
        for (int i = oldNum; i < NumDirect - 1 && i < numSectors; i++)
        {
            dataSectors[i] = freeMap->Find();
        }
        if (numSectors >= NumDirect)
        {
            dataSectors[NumDirect - 1] = freeMap->Find();
            int dataSectors2[NumIndirect];
            printf("Start %d\n", numSectors - NumDirect);
            for (int i = 0; i < numSectors - NumDirect + 1; i++)
                dataSectors2[i] = freeMap->Find();
            //将二级索引保存
            synchDisk->WriteSector(dataSectors[NumDirect - 1], (char *)dataSectors2);
            puts("二级索引扩展成功");
        }
    }
    numBytes = len;
    return true;
}