#include <stdio.h>
#include <stdlib.h>

char *someFun1()
{
	static char temp[] = "string";
	return temp;
}

char *someFun2()
{
	char temp[] = {'s'},{'t'},{'r'},{'i'},'\0';
	return temp;
}

main()
{
	puts(someFun1());
	puts(someFun2());
}
