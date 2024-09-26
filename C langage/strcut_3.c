#include <stdio.h>

struct Book
{
    char title[128];
    char author[40];
    struct  Book *next;
};

void getInput(struct Book *book)
{
    printf("������������");
    scanf("%s", book->title);
    printf("���������ߣ�:");
    scanf("%s", book->author);

}

void addBook(struct Book **library)
{
    struct Book *book, *temp;
    book=(struct Book *)malloc(sizeof(struct Book));

    if(book == NULL)
    {
        printf("�ڴ����ʧ����!\n");
        exit(1);
    }
    else{

        getInput(book);

        if(*library != NULL)
        {
            temp=*library;
            *library=book;
            book->next=temp;
        }
        else{
            *library=book;
            book->next=NULL;
        }
    }

}

void printLibrary(struct Book *library)
{
    struct Book *book;
    int count=1;
    book=library;

    while(book!=NULL){
        printf("Book %d\n",count);
        printf("������%s\n",book->title);
        printf("���ߣ�%s\n",book->author);
        book=book->next;
        count++;
    }


}

void releaseLibrary(struct Book *library)
{
    while(library!=NULL)
    {
        library=library->next;
        free(library);
    }

}

int main(void)
{
    struct Book *library = NULL;
    int ch;

    while(1)
    {
        printf("�����Ƿ�Ҫ¼���鼮��Ϣ(Y/N):");
        do
        {
            ch=getchar();/* code */
        } while ( ch!='Y' && ch!= 'N' );

        if(ch=='Y')
        {
            addBook(&library);
        }
        else{
            break;
        }

    }
    
        printf("�����Ƿ���Ҫ��ӡͼ��ݵ���Ϣ(Y/N):");
        do
        {
            ch=getchar();/* code */
        } while ( ch!='Y' && ch!= 'N' );
        if(ch=='Y')
        {
            printLibrary(library);
        }
        else{
           releaseLibrary(library);
        }
        
        return 0;


}