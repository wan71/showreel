
#include <stdio.h>
#include <stdlib.h>

typedef struct _ListNode
{
    int data;
    struct _ListNode *next;
} ListNode;
typedef ListNode *CyclicList;

CyclicList ListInit()
{
    CyclicList list = (CyclicList)malloc(sizeof(ListNode));
    list->data = -1;
    list->next = list;
    return list;
}

int ListInsert(CyclicList list, int data, int n)
{
    if (list == NULL || n < 1)
    {
        return -1;
    }
    ListNode *cur=list;
    int cur_i=0;
    while(cur->next!=list&&cur_i < (n-1)) 
    {
        cur = cur->next;
        cur_i++;
    }
    if(cur->next == list)
    {
        if(cur_i!=(n-1))		// 仍然不是插入位置前一个结点，出错
		{
			printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
			return -1;	// 链表没有 n 那么长
		}
    }
    ListNode* new =(ListNode *)malloc(sizeof(ListNode));
    new->data=data;
    new->next=cur->next;
    cur->next=new;
    return 0;

}
void ListShow(CyclicList list)
{
    ListNode* temp;
    temp=list->next;
    if(list==temp) 
    {
        printf("List is empty\n");
        return;	//
    }
     printf("List as follow\n");
    do
    {
        printf("%5d\r\n",temp->data);
		temp=temp->next;	

        /* code */
    } while (temp!=list);
    printf("\n");

}

void ListDestroy(CyclicList list)
{
    CyclicList cur=list->next;
    CyclicList next=NULL;
    while(cur!=list)
    {
        next=cur->next;
        free(cur);
        cur=next;
    }
    list->next=list;

}
int ListDelete(CyclicList list, int *data, int n)
{
    if(list==NULL || data==NULL || n<1)
		return -1;
    ListNode* cur = list;	// cur指向当前结点，初始化指向头结点
	int cur_i=0;			// i表示当前结点的序号,0-头结点
    while(cur->next!=list && cur_i<(n-1))
    {
        cur = cur->next;
		cur_i++;
    }

    if(cur->next==list)
    {   
		printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
		return -1;			// 链表没有 n 那么长
	}
    ListNode *detect=cur->next;
    *data=detect->data;
    cur->next=detect->next;
    free(detect);
    return 0;


}

int main()
{
    CyclicList list = ListInit();
    int data = 0;

    printf("Cycliclist is empty !!! \n");
    ListInsert(list, 2, 1);		// 空链表时，验证插入
    ListInsert(list, 3, 2);
    ListInsert(list, 4, 3);
    ListDelete(list,&data,2);
    printf("data is %d in ListDelete\n",data);
    ListShow(list);
    
}
