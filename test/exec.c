#include "syscall.h"

int main()
{
  Puts("开始执行：Exec 调用");
  Exec("../test/halt.noff");
  Puts("执行结束！");
}
