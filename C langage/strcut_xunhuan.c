
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
        if(cur_i!=(n-1))		// ��Ȼ���ǲ���λ��ǰһ����㣬����
		{
			printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
			return -1;	// ����û�� n ��ô��
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
    ListNode* cur = list;	// curָ��ǰ��㣬��ʼ��ָ��ͷ���
	int cur_i=0;			// i��ʾ��ǰ�������,0-ͷ���
    while(cur->next!=list && cur_i<(n-1))
    {
        cur = cur->next;
		cur_i++;
    }

    if(cur->next==list)
    {   
		printf("[%s %d]error din't have No.%d\n", __FUNCTION__,__LINE__, n);
		return -1;			// ����û�� n ��ô��
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
    ListInsert(list, 2, 1);		// ������ʱ����֤����
    ListInsert(list, 3, 2);
    ListInsert(list, 4, 3);
    ListDelete(list,&data,2);
    printf("data is %d in ListDelete\n",data);
    ListShow(list);
    
}
