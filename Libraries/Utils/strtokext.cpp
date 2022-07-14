#include "../Libraries/Utils/strtokext.h"
#include <string.h>


int splittedCount(char str[], const char* delims) {
	strtok(str,delims);
	int cnt=1;
	while (strtok(NULL,delims))
		cnt++;
	return cnt;
}

void catchSplits(char *items[], char str[],int cnt, bool ltrim) {
	char *p = items[0] = str;
	int index=1;
	while (index < cnt) {
		while (*p++);
		if (ltrim) 
			while (*p == ' ')
				p++;
		items[index++]=p;
	}
}
