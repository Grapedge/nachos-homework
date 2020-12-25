#include "stdio.h"
#include "syscall.h"

int main()
{
  puts("用户程序启动");
  int a, b;
  scanf("%d%d", &a, &b);
  printf("%d + %d = %d\n", a, b, a + b);
  Exec("../test/halt.noff");
  puts("执行结束");
}