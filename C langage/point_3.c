#include <stdio.h>
#include <stdlib.h>

// ����ԭ�ͣ�����һ��int�͵Ķ���ָ����Ϊ����
void allocate_2D_array(int rows,int **array) {
    // �����ڴ��һά����
    *array = (int *)malloc(rows * sizeof(int));
    for (int i = 0; i < rows; i++) {
        (*array)[i] = i;
}
}

int main() {
    int *myArray; // ����һ������ָ��

    int rows = 3;
    

    // ���ú������������һά����ĵ�ַ�洢��myArray��
    allocate_2D_array(rows, &myArray);

    // ��ӡһά����
    printf("һά���飺\n");
    for (int i = 0; i < rows; i++) {
       
            printf("%d\n", myArray[i]);
    }
    free(myArray);
    return 0;
}
