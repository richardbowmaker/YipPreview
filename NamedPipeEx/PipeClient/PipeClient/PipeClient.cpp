// PipeClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <iostream>
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>


int main()
{
    std::cout << "Pipe client!\n";

	HANDLE hPipe = CreateFile(
		LR"(\\.\pipe\mypipe)",   // pipe name 
															//GENERIC_READ |  // read and write access 
		GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, //,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		int le = GetLastError();
		std::cout << "CreateFile failed " << le << "\n";
		int n;
		std::cin >> n;
		exit(-1);
	}

	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		hPipe,    // pipe handle 
		&dwMode,  // new pipe mode 
		NULL,     // don't set maximum bytes 
		NULL);    // don't set maximum time 

	if (!fSuccess)
	{
		std::cout << "SetNamedPipeHandleState failed. GLE=" << GetLastError() << std::endl;
		int n;
		std::cin >> n;
		exit(-1);
	}


	for (int i = 0; i < 10; ++i)
	{
		Sleep(500);
		char buf[100];
		int len = snprintf(buf, sizeof(buf) / sizeof(char), "message %d\n", i);
		DWORD lenw;
		BOOL b = WriteFile(hPipe, buf, len + 1, &lenw, NULL);

		if (!b) std::cout << "WriteFile failed\n";

		std::cout << buf;
	}

	CloseHandle(hPipe);

	//while (true)
	//{
	//	std::cout << ".";
	//}

	exit(0);
}

