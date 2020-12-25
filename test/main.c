#include "stdio.h"
#include "syscall.h"

int main()
{
  Print("开始执行停机指令！");
  Exec("../test/halt.noff");
  // 不会触达
  Print("停机指令执行结束");
}