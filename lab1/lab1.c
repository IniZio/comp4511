#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Convert an upper case character to lower case (and vice versa)
char reverseUpperLower(char ch) {
	if ( ch >= 'a' && ch <= 'z' ) 
		return ch-'a'+'A';
	else if ( ch >= 'A' && ch <= 'Z' )
		return ch-'A'+'a';
	return ch;		
}

// Reverse a subchar* st
//   from: start index (inclusive)
//   to: end index (inclusive)
//   assumption: Both indices are valid 
void reverseStrFromTo(char* st, int from, int to) {
	
	for (int i=from;i<=to;i++)
		st[i] = reverseUpperLower(st[i]);
	
	int sz = (to-from)/2;
	char tmp ;
	for (int i=0;i<=sz;i++) {
		tmp =  st[from+i] ;		
		st[from+i]=st[to-i];
		st[to-i]=tmp;
	}
}

// read input from a file
char* readInput(char* fileName) {
	char* line;
  size_t len = 0;
	FILE *fin = fopen(fileName, "r");

	getline(&line, &len, fin);

	fclose(fin);

	return line;
}

// write output to a file
void writeOutput(char* fileName, const char* str) {
	FILE *fout= fopen(fileName, "w");
  fprintf(fout, "%s\n", str);
	fclose(fout);
}

// My encryption 
char* myEncrypt(char* lin) {
  char* line;

  strcpy(&line, &lin);

	int prevIndex = 0;
	// reverse tokens in the line
	for (int i=0; i< strlen(line);i++) {
		if ( line[i] == ' ' ) {
			reverseStrFromTo(line, prevIndex, i-1);
			prevIndex = i+1;
		}
	}
	// reverse the last token
	if (prevIndex < strlen(line)-1 ) 
		reverseStrFromTo(line, prevIndex, strlen(line)-1 );
	return line;
}

int main() {
	char* line;
  char* encrypted;

	line = readInput((char*)"input.txt");

	printf("Original : %s\n", line);
	encrypted = myEncrypt(line);
  printf("Encrypted: %s\n", encrypted);
	writeOutput((char*)"output.txt", encrypted);
	return 0;
}
