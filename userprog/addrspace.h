// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"

#define UserStackSize 1024 // increase this as necessary!
#define MaxNumPhysPages 5

class AddrSpace
{
public:
  AddrSpace(OpenFile *executable);
  ~AddrSpace();
  // 初始化寄存器
  void InitRegisters();
  // 保存用户空间状态
  void SaveState();
  // 恢复用户空间状态
  void RestoreState();
  // 获取用户空间 ID
  unsigned int GetSpaceID();
  // 打印调试用户空间基本信息
  void Print();

  // 查找可置换的页面
  unsigned int FindPageToReplace();
  // 替换页面
  void ReplacePage(int badVAddr);
  
  void WriteBack(int page);
private:
  // 页表数组地址
  TranslationEntry *pageTable;
  // 页表数量
  unsigned int numPages;
  // 用户空间 ID
  unsigned int spaceID;
  // 初始化的帧大小
  unsigned int frames;
  // 可执行程序
  OpenFile *executable;
};

#endif // ADDRSPACE_H
