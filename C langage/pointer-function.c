
// 使用指针变量返回的函数，�?做指针函�?
#include <stdio.h>


char *getWord(char c) 
{
    switch (c)
    {
    case 'A': return "Apple";
    case 'B': return "Banana";
    case 'C': return "Cat";
    case 'D': return "Dog";
    default:
       return "None";
    }
}


int main()
{
    char input;
    printf("�������һ����ĸ:\r\n");
    scanf("%c",&input);

    printf("%s\n",getWord(input));

    return 0;
}