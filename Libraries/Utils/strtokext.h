#ifndef strtokext_h
#define strtokext_h


/**
 * example
 *
 * char ar[] = "et fire fem";
 * int size;
 * char *par[size = splittedCount(ar," ")];
 * catchSplits(par, ar, size, false);

 * After call to splittedCount the string has become null delimiteds string
 * @param str to be splitted
 * @param delims contains the character which delimits
 * @return number of substrings
 **/	
int splittedCount(char str[], const char* delims);

/**
 * @param items is array of pointer that would index the item (like *argv[] til c programs)
 * @param str array that is nul byte splitted 
 * @param cnt is number of items which pointer to is assigned *items 
 * @param ltrim, when true advances to first non space char each string  
 **/
void catchSplits(char *items[], char str[],int cnt, bool ltrim);

#endif