#include <stdio.h>
#include <string.h>
struct num{
    int id;
    int age;
    char *name;
};

int main()
{
   struct num *ed;
   //printf("%d\n", ed);
   ed->id = 0;
   //ed->age =18;
  //strcpy(ed->name, "wl");
   //ed->name = "wl";
   //printf("ed->id is %d, ed->age is %d,ed->name is %s\n", ed->id, ed->age, ed->name);
   printf("%p\n", (void *)ed);
   

    return 0;
}