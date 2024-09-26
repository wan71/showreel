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

struct Book getInput(struct Book book)
{
    printf("����������: \n");
    scanf("%s", book.title);
    printf("����������:\n");
    scanf("%s", book.author);
    printf("�������������:\n");
    scanf("%d-%d-%d",&book.date.year,&book.date.month,&book.date.day);
    return book;
}

void printBook(struct Book book)
{
    printf("������%s\n",book.title);
    printf("����: %s\n",book.author);
    printf("��������: %d-%d-%d\n",book.date.year,book.date.month,book.date.day);

}



int main(void)
{
    struct Book b1, b2;

    printf("��¼���һ����\n");
    b1=getInput(b1);
    putchar('\n');
    printBook(b1);

}
