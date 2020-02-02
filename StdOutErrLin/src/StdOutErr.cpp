//============================================================================
// Name        : StdOutErr.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <unistd.h>
using namespace std;

int main()
{
	for (int i = 0; i< 5; ++i)
	{
		cout << "stdout: " << i << endl;
		cerr << "stderr: " << i << endl;
		sleep(1);
	}
	return 0;
}
