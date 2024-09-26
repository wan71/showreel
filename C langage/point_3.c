#include <stdio.h>
#include <stdlib.h>

// 函数原型，接受一个int型的二重指针作为参数
void allocate_2D_array(int rows,int **array) {
    // 分配内存给一维数组
    *array = (int *)malloc(rows * sizeof(int));
    for (int i = 0; i < rows; i++) {
        (*array)[i] = i;
}
}

int main() {
    int *myArray; // 声明一个二重指针

    int rows = 3;
    

    // 调用函数，将分配的一维数组的地址存储在myArray中
    allocate_2D_array(rows, &myArray);

    // 打印一维数组
    printf("一维数组：\n");
    for (int i = 0; i < rows; i++) {
       
            printf("%d\n", myArray[i]);
    }
    free(myArray);
    return 0;
}
