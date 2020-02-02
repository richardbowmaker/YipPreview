// PipeServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h> 
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>


int main()
{
    std::cout << "Pipe Server\n";

	OVERLAPPED overlap = { 0 };
	overlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

	if (overlap.hEvent == NULL)
	{
		std::cout << "CreateEvent failed\n";
		exit(-1);
	}

	HANDLE hPipe = CreateNamedPipe(
		LR"(\\.\pipe\mypipe)",            // pipe name 
		PIPE_ACCESS_DUPLEX |     // read/write access 
		FILE_FLAG_OVERLAPPED,    // overlapped mode 
		PIPE_TYPE_MESSAGE |      // message-type pipe 
		PIPE_READMODE_MESSAGE |  // message-read mode 
		PIPE_WAIT,               // blocking mode 
		5,						 // number of instances 
		10000,					 // output buffer size 
		10000,				     // input buffer size 
		5000,					 // client time-out 
		NULL);                   // default security attributes 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "CreateNamedPipe failed\n";
		exit(-1);
	}

	HANDLE hWrite = CreateFile(
		LR"(\\.\pipe\mypipe)",
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING, // very important flag!
		FILE_ATTRIBUTE_NORMAL,
		0 // no template file for OPEN_EXISTING
	);

	if (hWrite == INVALID_HANDLE_VALUE)
	{
		std::cout << "Create file failed\n";
		DisconnectNamedPipe(hPipe);
		exit(-1);
	}

	//HANDLE hStdOutWrite;
	//BOOL b = DuplicateHandle(
	//	GetCurrentProcess(),
	//	hWrite,
	//	GetCurrentProcess(),
	//	&hStdOutWrite,
	//	0,
	//	TRUE,
	//	DUPLICATE_SAME_ACCESS);

	//if (!b)
	//{
	//	std::cout << "DuplicateHandle failed\n";
	//	DisconnectNamedPipe(hPipe);
	//	exit(-1);
	//}

	//CloseHandle(hWrite);

	// pipe connection
	while (true)
	{
		BOOL fConnected = ConnectNamedPipe(hPipe, &overlap);

		if (fConnected)
		{
			std::cout << "ConnectNamedPipe returned true\n";
			DisconnectNamedPipe(hPipe);
			exit(-1);
		}

		int le = GetLastError();

		if (le == ERROR_PIPE_CONNECTED)
		{
			std::cout << "good connection";
			break;
		}

		if (le != ERROR_IO_PENDING)
		{
			std::cout << "ConnectNamedPipe failed\n";
			DisconnectNamedPipe(hPipe);
			exit(-1);
		}

		Sleep(100);
	}

	TCHAR szCmdline[] = LR"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)";
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = NULL;
	siStartInfo.hStdOutput = hWrite;
	siStartInfo.hStdInput = NULL; 
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 
	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	char buff[1000];
	DWORD bytesRead;

	// If an error occurs, exit the application. 
	if (!bSuccess)
	{
		std::cout << "Error creating process\n";
		DisconnectNamedPipe(hPipe);
		exit(-1);
	}

	while (true)
	{
		// overlapped read
		char buf[10000];
		DWORD lenr;
		BOOL b = ReadFile(hPipe, buf, sizeof(buf), &lenr, &overlap);

		if (b)
		{
			std::string s(buf, lenr);
			std::cout << "rx1: " << s << std::endl;
		}
		else
		{
			int le = GetLastError();
			if (le == ERROR_IO_PENDING)
			{
				while (true)
				{
					//std::cout << "IO pending " << lenr << std::endl;

					DWORD wait = WaitForSingleObject(overlap.hEvent, 100);

					if (wait == WAIT_OBJECT_0)
					{
						DWORD lenr;
						BOOL b = GetOverlappedResult(hPipe, &overlap, &lenr, FALSE);
						std::string s(buf, lenr);
						std::cout << "rx2: " << s;
						break;
					}
					else if (wait == WAIT_TIMEOUT)
					{
						std::cout << ".";
					}
				}
			}
			else if (le == ERROR_BROKEN_PIPE)
			{
				std::cout << "client closed pipe " << std::endl;
//				CloseHandle(hRead);
				DisconnectNamedPipe(hPipe);
				exit(-1);
			}
			else
			{
				std::cout << "pipe error " << le << std::endl;
				DisconnectNamedPipe(hPipe);
				exit(-1);
			}
		}
	}
}

// version 1 read pipe overlapped

//int main()
//{
//	std::cout << "Pipe Server\n";
//
//	OVERLAPPED overlap = { 0 };
//	overlap.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
//
//	if (overlap.hEvent == NULL)
//	{
//		std::cout << "CreateEvent failed\n";
//		exit(-1);
//	}
//
//	HANDLE hPipe = CreateNamedPipe(
//		LR"(\\.\pipe\mypipe)",            // pipe name 
//		PIPE_ACCESS_DUPLEX |     // read/write access 
//		FILE_FLAG_OVERLAPPED,    // overlapped mode 
//		PIPE_TYPE_MESSAGE |      // message-type pipe 
//		PIPE_READMODE_MESSAGE |  // message-read mode 
//		PIPE_WAIT,               // blocking mode 
//		5,						 // number of instances 
//		10000,					 // output buffer size 
//		10000,				     // input buffer size 
//		5000,					 // client time-out 
//		NULL);                   // default security attributes 
//
//	if (hPipe == INVALID_HANDLE_VALUE)
//	{
//		std::cout << "CreateNamedPipe failed\n";
//		exit(-1);
//	}
//
//	// pipe connection
//	while (true)
//	{
//		BOOL fConnected = ConnectNamedPipe(hPipe, &overlap);
//
//		if (fConnected)
//		{
//			std::cout << "ConnectNamedPipe returned true\n";
//			DisconnectNamedPipe(hPipe);
//			exit(-1);
//		}
//
//		int le = GetLastError();
//
//		if (le == ERROR_PIPE_CONNECTED)
//		{
//			std::cout << "good connection";
//			break;
//		}
//
//		if (le != ERROR_IO_PENDING)
//		{
//			std::cout << "ConnectNamedPipe failed\n";
//			DisconnectNamedPipe(hPipe);
//			exit(-1);
//		}
//
//		Sleep(100);
//	}
//
//	std::cout << "Starting read\n";
//
//	while (true)
//	{
//		// overlapped read
//		char buf[10000];
//		DWORD lenr;
//		BOOL b = ReadFile(hPipe, buf, sizeof(buf), &lenr, &overlap);
//
//		if (b)
//		{
//			std::string s(buf, lenr);
//			std::cout << "rx1: " << s << std::endl;
//		}
//		else
//		{
//			int le = GetLastError();
//			if (le == ERROR_IO_PENDING)
//			{
//				while (true)
//				{
//					//std::cout << "IO pending " << lenr << std::endl;
//
//					DWORD wait = WaitForSingleObject(overlap.hEvent, 100);
//
//					if (wait == WAIT_OBJECT_0)
//					{
//						DWORD lenr;
//						BOOL b = GetOverlappedResult(hPipe, &overlap, &lenr, FALSE);
//						std::string s(buf, lenr);
//						std::cout << "rx2: " << s;
//						break;
//					}
//					else if (wait == WAIT_TIMEOUT)
//					{
//						std::cout << ".";
//					}
//				}
//			}
//			else if (le == ERROR_BROKEN_PIPE)
//			{
//				std::cout << "client closed pipe " << std::endl;
//				DisconnectNamedPipe(hPipe);
//				exit(-1);
//			}
//			else
//			{
//				std::cout << "pipe error " << le << std::endl;
//				DisconnectNamedPipe(hPipe);
//				exit(-1);
//			}
//		}
//	}
//}
