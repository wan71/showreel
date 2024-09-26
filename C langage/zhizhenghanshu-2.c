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

int (*select(char op))(int num1, int num2)
{
    switch (op)
    {
    case '+': return add;
    case '-': return sub;
    default:
        break;
    }


}

int main()
{
   int num1, num2;
   char op;
   int (*fp)(int, int);

   printf("è¯·å¡«å…¥ä¸€ä¸?¼å­?(å¦?1+3)");
   scanf("%d%c%d", &num1,&op,&num2);

   fp=select(op);
   printf("%d %c %d =%d", num1,op, num2,calc(fp,num1,num2));

    return 0;
}