//函数指针就是把函数做指针
// 如 int (*fp)(int,int)

#include <string.h>

#include <stdio.h>
int add(int num1, int num2)
{
    return (num1 + num2);
}

int sub(int num1, int num2)
{
    return (num1 - num2);
}

int calc(int (*fp)(int, int), int num1, int num2)
{
return (*fp)(num1, num2);

}

int main()
{
    printf("3+5=%d\n",calc(add,3,5));
    printf("5-2=%d\n",calc(sub,5,2));
    return 0;
}