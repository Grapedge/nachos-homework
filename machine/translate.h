// translate.h
//	Data structures for managing the translation from
//	virtual page # -> physical page #, used for managing
//	physical memory on behalf of user programs.
//
//	The data structures in this file are "dual-use" - they
//	serve both as a page table entry, and as an entry in
//	a software-managed translation lookaside buffer (TLB).
//	Either way, each entry is of the form:
//	<virtual page #, physical page #>.
//
// DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef TLB_H
#define TLB_H

#include "copyright.h"
#include "utility.h"

// 页表结构
class TranslationEntry
{
public:
  // 逻辑页号
  int virtualPage;
  // 物理块号
  int physicalPage;
  // 该页是否有效
  bool valid;
  // 是否允许用户改写
  bool readOnly;
  // 引用位，当前是否引用
  bool use;
  // 该页是否被修改
  bool dirty;
  // use 和 dirty 用于构建虚拟内存
};

#endif
