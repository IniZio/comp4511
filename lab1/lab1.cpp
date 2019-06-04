#include <fstream>
#include <iostream>
#include <string>
using namespace std;

// Convert an upper case character to lower case (and vice versa)
char reverseUpperLower(char ch) {
	if ( ch >= 'a' && ch <= 'z' ) 
		return ch-'a'+'A';
	else if ( ch >= 'A' && ch <= 'Z' )
		return ch-'A'+'a';
	return ch;		
}

// Reverse a substring st
//   from: start index (inclusive)
//   to: end index (inclusive)
//   assumption: Both indices are valid 
void reverseStrFromTo(string& st, int from, int to) {
	
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
string readInput(char* fileName) {
	string line;
	ifstream fin(fileName);
	getline(fin, line);
	fin.close();
	return line;
}

// write output to a file
void writeOutput(char* fileName, const string& str) {
	ofstream fout(fileName);
	fout << str << endl;
	fout.close();
}

// My encryption 
string myEncrypt(string line) {
	int prevIndex = 0;
	// reverse tokens in the line
	for (int i=0; i<line.size();i++) {
		if ( line[i] == ' ' ) {
			reverseStrFromTo(line, prevIndex, i-1);
			prevIndex = i+1;
		}
	}
	// reverse the last token
	if (prevIndex < line.size()-1 ) 
		reverseStrFromTo(line, prevIndex, line.size()-1 );
	return line;
}

int main() {
	string line, encrypted;
	line = readInput((char*)"input.txt");
	encrypted = myEncrypt(line);
	cout << "Original : " << line << endl;
	cout << "Encrypted: " << encrypted << endl;
	writeOutput((char*)"output.txt", encrypted);
	return 0;
}
