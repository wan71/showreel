
// 浣跨敤鎸囬拡鍙橀噺杩斿洖鐨勫嚱鏁帮紝鍙?鍋氭寚閽堝嚱鏁?
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
    printf("请输入第一个字母:\r\n");
    scanf("%c",&input);

    printf("%s\n",getWord(input));

    return 0;
}