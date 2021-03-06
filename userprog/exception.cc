// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

// 处理系统调用：执行
void SysCallExec()
{
    char filename[200];
    // arg1 => 4 arg2 => 5 arg3 => 6 arg4 =>7
    // 读取文件名
    int addr = machine->ReadRegister(4);
    for (int i = 0;; i++)
    {
        machine->ReadMem(addr + i, 1, (int *)(filename + i));
        if (filename[i] == 0)
        {
            break;
        }
    }

    printf("[Exec] 运行文件名为 %s 的用户程序\n", filename);

    // 运行程序
    OpenFile *exec = fileSystem->Open(filename);
    if (exec == NULL)
    {
        printf("执行程序加载失败，文件名为：%s\n", filename);
    }
    // 创建用户空间
    AddrSpace *space = new AddrSpace(exec);
    // 创建相应的线程用于执行该程序
    Thread *thread = new Thread(filename);
    // 关中断，将线程放入就绪队列
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    scheduler->ReadyToRun(currentThread);
    interrupt->SetLevel(oldLevel);
    currentThread = thread;
    thread->space = space;

    // 运行程序
    // 1. 初始化寄存器
    space->InitRegisters();
    // 2. 加载页表
    space->RestoreState();
    // 3. 写入当前正在运行的程序
    machine->WriteRegister(2, space->GetSpaceID());
    // 4. 运行
    machine->Run();
    ASSERT(FALSE);
}

// 处理系统调用：打印
void SysCallPuts()
{
    char str[200];
    int addr = machine->ReadRegister(4);
    for (int i = 0;; i++)
    {
        machine->ReadMem(addr + i, 1, (int *)(str + i));
        if (str[i] == 0)
        {
            break;
        }
    }
    printf("【用户程序】打印：%s\n", str);
}

// 处理系统调用：退出
void SysCallExit()
{
    int addr = machine->ReadRegister(4);
    printf("【用户程序】退出：%d\n", addr);
    currentThread->Finish();
}

// 处理缺页错误
void HandlePageFault()
{
    currentThread->space->ReplacePage(machine->ReadRegister(BadVAddrReg));
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void AdvancePC()
{

    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
}

void ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    switch (which)
    {

    case SyscallException:
        switch (type)
        {
        case SC_Halt:
            DEBUG('s', "执行系统调用：ShutDown.\n");
            interrupt->Halt();
            break;
        case SC_Exec:
            DEBUG('s', "执行系统调用: Exec\n");
            SysCallExec();
            AdvancePC();
            break;
        case SC_Puts:
            DEBUG('s', "执行系统调用: Puts\n");
            SysCallPuts();
            AdvancePC();
            break;
        case SC_Exit:
            DEBUG('s', "执行系统调用: Exit\n");
            SysCallExit();
            AdvancePC();
            break;
        default:
            break;
        }
        break;
    case PageFaultException:
        // 缺页异常，代表该进行页面置换
        HandlePageFault();
        break;
    default:
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
        break;
    }
}
