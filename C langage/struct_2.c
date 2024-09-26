#include <stdio.h>

struct Date
{
    int year;
    int month;
    int day;
};

struct Book
{
    char title[100];
    char author[100];
    struct Date date;
    /* data */
};

void getInput(struct Book *book)
{
    printf("请输入书名: \n");
    scanf("%s", book->title);
    printf("请输入作者:\n");
    scanf("%s", book->author);
    printf("请输入出版日期:\n");
    scanf("%d-%d-%d",&book->date.year,&book->date.month,&book->date.day);
   
}

void printBook(struct Book book)
{
    printf("书名：%s\n",book.title);
    printf("作者: %s\n",book.author);
    printf("出版日期: %d-%d-%d\n",book.date.year,book.date.month,book.date.day);

}



int main(void)
{
    struct Book b1, b2;

    printf("请录入第一本书\n");
    getInput(&b1);
    putchar('\n');
    printBook(b1);

}
