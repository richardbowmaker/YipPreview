

#ifdef WINDOWS_BUILD
	#include <cstdio>
	#include <iostream>
	#include <memory>
	#include <stdexcept>
	#include <string>
	#include <array>
	#include <utility>
	#include <string.h>
	#include <errno.h>
	#include <sstream>
	#include <fcntl.h>
	#include <time.h>
	#include <future>
	#include <atomic>
	#include <string>
	#include <thread>
	#include <wx/wx.h>
	#include <wx/thread.h>
	#include <windows.h> 
	#include <tchar.h>
	#include <stdio.h> 
	#include <strsafe.h>
	#include <tchar.h>
#elif LINUX_BUILD
	#include <sys/wait.h>
	#include <sys/types.h>
	#include <pthread.h>
	#include <cstdio>
	#include <iostream>
	#include <memory>
	#include <stdexcept>
	#include <string>
	#include <array>
	#include <utility>
	#include <string.h>
	#include <errno.h>
	#include <sstream>
	#include <fcntl.h>
	#include <time.h>
	#include <future>
	#include <atomic>
	#include <string>
	#include <thread>
	#include <unistd.h>
	#include <wx/wx.h>
	#include <wx/thread.h>
#endif


#include "Logger.h"
#include "Tryout.h"
#include "Utilities.h"
#include "ShellExecute.h"
#include "Volume.h"

wxDEFINE_EVENT(wxEVT_MY_CUSTOM_COMMAND, wxCommandEvent);

void TryOut::tryout()
{
//	Volume vol(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc)");
//	vol.mount();
//	vol.unmount();

//	bool b = FU::fileExists(L"/media/volume01");
//	b = FU::fileExists(L"/media/volume02");
//	b = FU::fileExists(L"/media/volume03");
//
//	int n = 0;

	bool b = FU::mkDir(L"/media/volume04");

	{
		SudoMode sm;

		{
			SudoMode sm;
			b = FU::mkDir(L"/media/volume04");
			sm.release();
			sm.release();
			sm.release();
			sm.release();
		}
	}
}

// a thread class that will periodically send events to the GUI thread
class MyThread : public wxThread
{
protected:
	wxEvtHandler* m_parent;

public:
	explicit MyThread(wxEvtHandler* parent) : wxThread(), m_parent(parent) {}

	ExitCode Entry() override
	{
		for (unsigned n = 0; n < 10; n++)
		{
//			// notify the main thread
////			wxCommandEvent evt(wxEVT_MY_CUSTOM_COMMAND, wxID_ANY);
//			wxLoggerEvent evt;
//			evt.SetInt(n);
//			evt.SetString(wxString(L"From thread"));
//			evt.setLevel(22);
//			m_parent->AddPendingEvent(evt);

			Logger::info(L"from worker thread");

			this->Sleep(100);
		}

		return ExitCode(nullptr);
	}

};

MyThread* thread;

void TryOut::ThreadEvents(wxEvtHandler* parent)
{
	thread = new MyThread(parent);

	wxThreadError err = thread->Create();

	if (err != wxTHREAD_NO_ERROR)
	{
		Logger::error(L"Couldn't create thread!");
		return;
	}

	err = thread->Run();

	if (err != wxTHREAD_NO_ERROR)
	{
		Logger::error(L"Couldn't run thread!");
		return;
	}

}

#ifdef WINDOWS_BUILD

// https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o?redirectedfrom=MSDN
// https://stackoverflow.com/questions/30914346/read-lines-from-file-async-using-winapi-readfile
// https://stackoverflow.com/questions/51127722/reading-pipe-asynchronously-using-readfile

// https://www.daniweb.com/programming/software-development/threads/295780/using-named-pipes-with-asynchronous-i-o-redirection-to-winapi


void TryOut::ReadAsync()
{
	HANDLE hFile = CreateFile(
		LR"(D:\Projects\WxWidgets\YipPreview\Tryout\test.bat)", 
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		Logger::systemError(GetLastError(), L"INVALID_HANDLE_VALUE");
		return;
	}

	BOOL bResult;
	BYTE bReadBuf[2048];

	OVERLAPPED oRead = { 0 };
	oRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	bResult = ReadFile(hFile, bReadBuf, _countof(bReadBuf), NULL, &oRead);

	if (!bResult && GetLastError() != ERROR_IO_PENDING)
	{
		Logger::systemError(GetLastError(), L"ERROR io pending");
		CloseHandle(hFile);
		return;
	}


	DWORD dwWaitRes = WaitForSingleObject(oRead.hEvent, INFINITE);
	switch (WAIT_OBJECT_0)
	{
	case WAIT_OBJECT_0: // reading finished
	{
		Logger::info(L"String that was read from file: ");
		std::string s((char*)bReadBuf, oRead.InternalHigh);
		Logger::info(L"String that was read from file: %s", SU::strToWStr(s).c_str());
	}
		break;

	default:
		Logger::error(L"Nooo");
	}

	CloseHandle(hFile);

}

void TryOut::ShellAsync()
{
	SECURITY_ATTRIBUTES saAttr;

	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	HANDLE hPipe = CreateNamedPipe(
		LR"(\\.\pipe\yipreviewshellexec)",
		PIPE_ACCESS_INBOUND, // | FILE_FLAG_OVERLAPPED,				// open mode
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, //PIPE_TYPE_BYTE  | PIPE_READMODE_BYTE | PIPE_WAIT,		// pipe mode
		PIPE_UNLIMITED_INSTANCES,
		1000,
		1000,
		5000,
		&saAttr);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		Logger::systemError(GetLastError(), L"Error opening pipe");
		return;
	}

	HANDLE hOutputWrite = CreateFile(
		LR"(\\.\pipe\yipreviewshellexec)",
		FILE_WRITE_DATA | SYNCHRONIZE,
		0,
		&saAttr,
		OPEN_EXISTING, // very important flag!
		FILE_ATTRIBUTE_NORMAL,
		0 // no template file for OPEN_EXISTING
	);

	if (hOutputWrite == INVALID_HANDLE_VALUE)
	{
		Logger::systemError(GetLastError(), L"Error creating stdout write file");
		return;
	}

	SetHandleInformation(hOutputWrite, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

	OVERLAPPED oOverlap;
	ZeroMemory(&oOverlap, sizeof(oOverlap));
	HANDLE hEvent = NULL;

	hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	
	if (hEvent == NULL)
	{
		Logger::systemError(GetLastError(), L"Error creating event");
		return;
	}

	oOverlap.hEvent = hEvent;

	TCHAR szCmdline[] = TEXT(R"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)");
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
	siStartInfo.hStdOutput = hOutputWrite;
	siStartInfo.hStdInput = NULL; // g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(
		NULL,
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
		Logger::systemError(GetLastError(), L"Error CreateProcess");
		return;
	}

	while (true)
	{
		// overlapped read
		char buf[10000];
		DWORD lenr;
//		BOOL b = ReadFile(hPipe, buf, sizeof(buf), &lenr, &oOverlap);
		BOOL b = ReadFile(hPipe, buf, sizeof(buf), &lenr, NULL);

		if (b)
		{
			std::string s(buf, lenr);
			Logger::info(L"rx1: %ls", SU::strToWStr(s).c_str());
		}
		else
		{
			int le = GetLastError();
			if (le == ERROR_IO_PENDING)
			{
				while (true)
				{
					//std::cout << "IO pending " << lenr << std::endl;

					DWORD wait = WaitForSingleObject(oOverlap.hEvent, 100);

					if (wait == WAIT_OBJECT_0)
					{
						DWORD lenr;
						BOOL b = GetOverlappedResult(hPipe, &oOverlap, &lenr, FALSE);
						Logger::info(L"rx2: %ls", SU::strToWStr(buf).c_str());
						break;
					}
					else if (wait == WAIT_TIMEOUT)
					{
						//std::cout << ".";
					}
				}
			}
			else if (le == ERROR_BROKEN_PIPE)
			{
				Logger::info(L"client closed pipe");
				CloseHandle(piProcInfo.hProcess);
				CloseHandle(piProcInfo.hThread);
				DisconnectNamedPipe(hPipe);
				return;
			}
			else
			{
				Logger::systemError(le, L"pipe read error");
				CloseHandle(piProcInfo.hProcess);
				CloseHandle(piProcInfo.hThread);
				DisconnectNamedPipe(hPipe);
				return;
			}
		}
	}


	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	DisconnectNamedPipe(hPipe);
	return;
}

void TryOut::ExecIt()
{
	SECURITY_ATTRIBUTES saAttr;
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;

	HANDLE g_hInputFile = NULL;

	constexpr int BUFSIZE = 4096;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;


	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
	{
		Logger::systemError(GetLastError(), L"Error CreatePipe");
		return;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
	{
		Logger::systemError(GetLastError(), L"Error SetHandleInformation");
		return;
	}

	TCHAR szCmdline[] = TEXT(R"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)");
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	//siStartInfo.hStdInput = g_hChildStd_IN_Rd;
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

	 // If an error occurs, exit the application. 
	if (!bSuccess)
	{
		Logger::systemError(GetLastError(), L"Error CreateProcess");
		return;
	}
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	CloseHandle(g_hChildStd_OUT_Wr);



	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];

	//	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0) break;

		std::string s(chBuf, dwRead);
		Logger::info(L"read: %ls", SU::strToWStr(s).c_str());
	}
}

DWORD WINAPI ThreadMainEntry(void*)
{
	for (int i = 0; i < 5; ++i)
	{
		Logger::info(L"In thread %d", i);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
	return 0;
}

void TryOut::WorkerThread()
{
//	auto w = std::async(std::launch::async, [](){ ThreadMainEntry(); });
	DWORD tid;
	HANDLE hThread = CreateThread(NULL, 0, &ThreadMainEntry, NULL, 0, &tid);
	Logger::info(L"thread started");
}

#elif LINUX_BUILD

FILE * popen2(std::string command, std::string type, int & pid, int &rfd);
int pclose2(FILE * fp, pid_t pid);

void TryOut::ExecIt()
{
//	int n = execl("/bin/ls", "ls", "-al", 0);
//
//	int m = 10;



//	std::array<char, 128> buffer;
//	std::string result;
//	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("/bin/ls", "r"), pclose);
//	if (!pipe)
//	{
//		throw std::runtime_error("popen() failed!");
//	}
//	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
//	{
//		result += buffer.data();
//	}

//--------------------------------------------

//	std::array<char, 128> buffer;
//	std::string result;
//	auto pipe = popen("/bin/ls", "r"); // get rid of shared_ptr
//
//	if (!pipe) throw std::runtime_error("popen() failed!");
//
//	while (!feof(pipe))
//	{
//		if (fgets(buffer.data(), 128, pipe) != nullptr)
//			result += buffer.data();
//	}
//
//	auto rc = pclose(pipe);
//
//	Logger::systemError(rc, L"");

//-------------------------------------------------

//	//char* cmd = "/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al";
////	char* cmd = "/bin/notepadqq";
//	char* cmd = "non-existent program";
//	std::array<char, 128> buffer;
//	std::string result;
//    int return_code = -1;
//    auto pclose_wrapper = [&return_code](FILE* f){ return_code = pclose(f); };
//    { // scope is important, have to make sure the ptr goes out of scope first
//    const std::unique_ptr<FILE, decltype(pclose_wrapper)> pipe(popen(cmd, "r"), pclose_wrapper);
//    if (pipe)
//    {
//        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
//        {
//            result += buffer.data();
//        }
//    }
//    }
//    auto xx = std::make_pair(result, return_code);
//
//    Logger::systemError(return_code >> 8, L"");
//
//
//	int z = 0;

	// get child pid from parent, ps -o pid --ppid 7867
	// pstree shows the process tree

//-------------------------------------------------

    int pid;
//    int timeout = 5;
//    std::string command = "/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al";
    std::string command = "nonexistentprogram";
//    std::string command = "/bin/notepadqq";
    int rfd;
    FILE * fp = popen2(command, "r", pid, rfd);
    char command_out[100] = {0};
    std::stringstream output;

 //   int flags = fcntl(rfd, F_GETFL);
  //  int fderr = fcntl(rfd, F_SETFL, flags | O_NONBLOCK);

    errno = 0;
    int rerr;

 //   timespec ts;
 //   long startt = static_cast<long>(time(nullptr));

    while ((rerr = read(fileno(fp), command_out, sizeof(command_out)-1)) != 0)
    {
    	if (rerr == EAGAIN)
    	{
    		//int nn = 0;
    	}
    	else if (rerr <= 0)
    	{
    		if (errno != EWOULDBLOCK)
    		{
    			break;
    		}
    		else
    		{
//        		// timeout
//    		    long timer = static_cast<long>(time(nullptr));
//    		    if (timer - startt > timeout)
//    		    {
//    		    	int yy = 0;
//    		    	break;
//    		    }

    		}
    	}
    	else if (rerr > 0)
    	{
			output << std::string(command_out);
			//kill(-pid, 9);
			memset(&command_out, 0, sizeof(command_out));
			rerr = read(fileno(fp), command_out, sizeof(command_out)-1);
    	}
    }

    std::string token;
    while (getline(output, token, '\n'))
        printf("OUT: %s\n", token.c_str());

 //   int nn = pclose2(fp, pid);

 //   int zzz =  0;



}


#define READ   0
#define WRITE  1
FILE * popen2(std::string command, std::string type, int & pid, int &rfd)
{
    pid_t child_pid;
    int fd[2];
    pipe(fd);

    if((child_pid = fork()) == -1)
    {
        perror("fork");
        exit(1);
    }

    /* child process */
    if (child_pid == 0)
    {
        if (type == "r")
        {
            close(fd[READ]);    //Close the READ end of the pipe since the child's fd is write-only
            dup2(fd[WRITE], 1); //Redirect stdout to pipe
        }
        else
        {
            close(fd[WRITE]);    //Close the WRITE end of the pipe since the child's fd is read-only
            dup2(fd[READ], 0);   //Redirect stdin to pipe
        }

        setpgid(child_pid, child_pid); //Needed so negative PIDs can kill children of /bin/sh
        execl("/bin/sh", "/bin/sh", "-c", command.c_str(), NULL);
        _exit(0);
    }
    else
    {
        if (type == "r")
        {
            close(fd[WRITE]); //Close the WRITE end of the pipe since parent's fd is read-only
        }
        else
        {
            close(fd[READ]); //Close the READ end of the pipe since parent's fd is write-only
        }
    }

    pid = child_pid;

    if (type == "r")
    {
        rfd = fd[READ];
        return fdopen(fd[READ], "r");
    }

    return fdopen(fd[WRITE], "w");
}

int pclose2(FILE * fp, pid_t pid)
{
    int stat;

 //   int err = fclose(fp);
    while (waitpid(pid, &stat, 0) == -1)
    {
        if (errno != EINTR)
        {
            stat = -1;
            break;
        }
    }
    return stat;
}


void* TheWorkerThread(void*)
{
	Logger::info(L"worker thread id: %ld", Utilities::getThreadId());
	for (int i = 0; i < 5; ++i)
	{
		Logger::info(L"worker thread %d", i);
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		//sleep(1);
	}
	pthread_exit(0);
}

std::future<bool> wait1;

void TryOut::WorkerThread()
{
//	Logger::clear();
//	wait1 = std::async(std::launch::async, [] () -> bool { return TheWorkerThread(); } );
//    int n = 0;

	pthread_t thread_;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&thread_, &attr, TheWorkerThread, 0);

//	int s, i;
//	s = pthread_attr_getdetachstate(&attr, &i);



}

#endif


void ShelExecuteHandler(ShellExecuteResult& result)
{
	Logger::info(L"ShelExecuteHandler\n %ls", result.toString().c_str());
}


void TryOut::AsyncShell(wxEvtHandler *handler)
{

	ShellExecuteResult result;


	//	ShellExecute::shellAsync(LR"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)", ShelExecuteHandler, 1);
	//	ShellExecute::shellAsync(LR"(D:\_Ricks\c#\ZiPreview\Executable\ffmpeg.exe)", ShelExecuteHandler, 2);

//	ShellExecute::shellSync(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", result);
//	Logger::info(L"Shell result %ls", result.toString().c_str());
//
//	ShellExecute::shellAsync(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", ShelExecuteHandler, 3);
//	ShellExecute::shellAsync(LR"(D:\Projects\WxWidgets\YipPreview\StdErrOutWin\Debug\StdErrOutWin.exe)", ShelExecuteHandler, 4);
//	ShellExecute::shellAsyncGui(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)", handler);
//
//	return;

	/////////////////////////

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell launched: firefox");
//	ShellExecute::shell(L"/usr/bin/firefox");
//	ShellExecute::shell(L"/bin/notepadqq");

	Logger::info(L"----------------------------------");
	Logger::info(L"Shell launched: StdOutErr");
	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/StdOutErr/Debug/StdOutErr", result);
	Logger::info(result.toString().c_str());


// /bin/ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af "volumedetect" -vn -sn -dn -f null NUL &2>1
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af \"volumedetect\" -vn -sn -dn -f null NUL");
//	ShellExecute::shellSync(L"/bin/ffmpeg -i /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/f3.mp4 -af \"volumedetect\" -vn -sn -dn -f null NUL &2>1", result, 10000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: ffmpegbat");
//	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/ffmpegbat &2>1", result, 2000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: test.bat");
//	ShellExecute::shellSync(L"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/test.bat", result);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellSync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", result, 1000);
//	Logger::info(result.toString().c_str());
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/nonexistentprogram");
//	ShellExecute::shellSync(L"/bin/nonexistentprogram", result, 1000);
//	Logger::info(result.toString().c_str());

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell sync launched: /bin/notepadqq");
//	ShellExecute::shellSync(L"/bin/notepadqq", result, 2000);
//	Logger::info(result.toString().c_str());
//	result.killChildProcess();

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al, no event handler");
//	ShellExecute::shellAsync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");

	Logger::info(L"----------------------------------");
	Logger::info(L"Shell async launched: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
	ShellExecute::shellAsync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", &ShelExecuteHandler, 200, nullptr, 10000);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/nonexistentprogram");
//    ShellExecute::shellAsync(L"/bin/nonexistentprogram", &ShelExecuteHandler, 201, nullptr, 10000);
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async launched: /bin/notepadqq");
//    ShellExecute::shellAsync(L"/bin/notepadqq", &ShelExecuteHandler, 202, nullptr, 5000);


// GUI callbacks

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched any: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al, no event handler");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched any: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, wxID_ANY, 3001);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched 1: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, 1, 3002);

//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched 2: /bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al");
//	ShellExecute::shellAsyncGui(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", handler, 2, 3003);

	//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched: /bin/nonexistentprogram");
//	ShellExecute::shellAsyncGui(L"/bin/nonexistentprogram", 5000, 301, nullptr, handler);
//
//	Logger::info(L"----------------------------------");
//	Logger::info(L"Shell async GUI launched: /bin/notepadqq");
//	ShellExecute::shellAsyncGui(L"/bin/notepadqq", 5000, 302, nullptr, handler);
}


//--------------------------------------------------------------------------
// trying out area 
//--------------------------------------------------------------------------

//DWORD MyThread(void*)
//{
//	for (int i = 0; i < 10; ++i)
//	{
//		Logger::info(L"From thread %d", i);
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}
//	return 0;
//}

void tryout(wxCommandEvent& event, const int row)
{
	//	std::wstring s(L"12:34:56");
	//
	//    const std::regex rex(R"(((\d{1,2}):)?(\d{1,2}):(\d{1,2}))");
	//
	//    std::smatch m;
	//    std::string s1 = SU::wStrToStr(s);
	//    int hh = 0, mm = 0, ss = 0;
	//
	//    if (std::regex_search(s1, m, rex))
	//    {
	//    	hh = atoi(m[1].str().c_str());
	//    	mm = atoi(m[3].str().c_str());
	//        ss = atoi(m[4].str().c_str());
	//    }
	//
	//
	//    s1 = "4:56";
	//    if (std::regex_search(s1, m, rex))
	//    {
	//    	hh = atoi(m[1].str().c_str());
	//    	mm = atoi(m[3].str().c_str());
	//        ss = atoi(m[4].str().c_str());
	//    }
	//



	return;




	//	bool b = FileProperties::test();
	//
	//	if (!b) Logger::error(L"property test failed");
	//
	//	return;
	//
	////	FileProperties fp;
	//	std::wstring s;
	//	fp.setString(L"p1", L"v1");
	//	fp.setInt(L"p2", 2);
	//	fp.incCount(L"p3");
	//	fp.incCount(L"p3");
	//	fp.incCount(L"p3");
	//	fp.setDateTimeNow(L"p4");
	//	s = fp.toString();
	//
	//	fp.clear();
	//	s = fp.toString();
	//	fp.fromString(L"p1;v1;p2;2");
	//	s = fp.toString();
	//	int c = fp.getSize();
	//	s = fp.getString(L"p1");
	//	c = fp.getInt(L"p2");
	//	s = fp.getCount(L"p2");
	//	fp.remove(L"p1");
	//	s = fp.toString();
	//	fp.setString(L"p2", L"");
	//	s = fp.toString();
	//	fp.fromString(L"");
	//	s = fp.toString();
	//	fp.fromString(L"p1;v1;p2");
	//	s = fp.toString();
	//	fp.fromString(L"p1;v1;p2;2;");
	//	s = fp.toString();
	//	fp.fromString(L"p1;v1;p2;;");
	//	s = fp.toString();
	//	fp.fromString(L"p1;v1;p2;");
	//	s = fp.toString();




	int n = 0;

	//	bool b;

	//
	//	std::wstring afn;
	//	int n;
	//
	//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a0123456789.jpg)", 30);
	//	n = afn.size();
	//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Proects/WxWidgets/YipPreview/Tryout1/a0123456789.jpg)", 30);
	//	n = afn.size();
	//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a123456789.jpg)", 30);
	//	n = afn.size();
	//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projcts/WxWidgets/YipPreview/Tryout1/a012456789.jpg)", 29);
	//	n = afn.size();
	//	afn = FU::abbreviateFilename(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a0123456789.jpg)", 5);
	//	afn = FU::abbreviateFilename(LR"(/media/012345678901234567890123456789.jpg)", 30);
	//	afn = FU::abbreviateFilename(LR"(a0123456789.jpg)", 30);
	//
	//
	//
	//	b = FileSetManager::addFiles(FU::pathToLocal(LR"(\YipPreview\Tryout)").c_str());
	//	Logger::info(L"Files %ls", FileSetManager::toString().c_str());
	//	return;
	//
	//
	//
	//	b = SU::startsWith(L"abcde", L"abc");
	//	b = SU::startsWith(L"ab", L"abc");
	//	b = SU::startsWith(L"ab", L"");
	//	b = SU::startsWith(L"", L"abc");
	//	b = SU::startsWith(L"", L"");
	//
	//
	//
	//#ifdef WINDOWS_BUILD
	//	std::wstring s;
	//	s = FU::getFileStem(LR"(abc)");
	//	s = FU::getFileStem(LR"(abc.txt)");
	//	s = FU::getFileStem(LR"(\home\me\abc)");
	//	s = FU::getFileStem(LR"(\home\me\abc.txt)");
	//	s = FU::getFileStem(LR"(\home\me.you\abc)");
	//	s = FU::getFileStem(LR"(\home\me.you\abc.txt)");
	//	s = FU::getFileStem(LR"(\home\me\abc.txt.exe)");
	//
	//	s = FU::getPathNoExt(LR"(abc.txt)");
	//	s = FU::getPathNoExt(LR"(abc)");
	//	s = FU::getPathNoExt(LR"(\home\me\abc)");
	//	s = FU::getPathNoExt(LR"(\home\me\abc.txt)");
	//	s = FU::getPathNoExt(LR"(\home\me.you\abc.txt)");
	//	s = FU::getPathNoExt(LR"(\home\me.you\abc)");
	//
	//	s = FU::getExt(LR"(abc.txt)");
	//	s = FU::getExt(LR"(abc)");
	//	s = FU::getExt(LR"(\home\me\abc)");
	//	s = FU::getExt(LR"(\home\me\abc.txt)");
	//	s = FU::getExt(LR"(\home\me.you\abc)");
	//	s = FU::getExt(LR"(\home\me.you\abc.txt)");
	//	return;
	//#elif LINUX_BUILD
	//	std::wstring s;
	//	s = FU::getFileStem(LR"(abc)");
	//	s = FU::getFileStem(LR"(abc.txt)");
	//	s = FU::getFileStem(LR"(/home/me/abc)");
	//	s = FU::getFileStem(LR"(/home/me/abc.txt)");
	//	s = FU::getFileStem(LR"(/home/me.you/abc)");
	//	s = FU::getFileStem(LR"(/home/me.you/abc.txt)");
	//	s = FU::getFileStem(LR"(/home/me/abc.txt.exe)");
	//
	//	s = FU::getPathNoExt(LR"(abc.txt)");
	//	s = FU::getPathNoExt(LR"(abc)");
	//	s = FU::getPathNoExt(LR"(/home/me/abc)");
	//	s = FU::getPathNoExt(LR"(/home/me/abc.txt)");
	//	s = FU::getPathNoExt(LR"(/home/me.you/abc.txt)");
	//	s = FU::getPathNoExt(LR"(/home/me.you/abc)");
	//
	//	s = FU::getExt(LR"(abc.txt)");
	//	s = FU::getExt(LR"(abc)");
	//	s = FU::getExt(LR"(/home/me/abc)");
	//	s = FU::getExt(LR"(/home/me/abc.txt)");
	//	s = FU::getExt(LR"(/home/me.you/abc)");
	//	s = FU::getExt(LR"(/home/me.you/abc.txt)");
	//	return;
	//#endif
	//
	//
	//	ShellExecute::shellAsyncGui(LR"(cmd /c dir D:\_Ricks\c#\ZiPreview\Executable)",
	//		MyFrame::getMainFrame().GetEventHandler());
	//	return;
	//
	//	//CreateThread(NULL, 0, &MyThread, NULL, 0, NULL);
	//	//Logger::info(L"thread started");
	//	//return;
	//
	//	player_->setFile(FU::pathToLocal(LR"(\YipPreview\Tryout\f3.mp4)"));
	//	player_->startPreview();
	//	//	player_->Load(LR"(D:\Projects\WxWidgets\YipPreview\Tryout\f3.mp4)");
	//	return;
	//
	//
	//	// linux copy file
	//	b = FU::copyFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::copyFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
	//	b = FU::copyFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a12345.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");
	//
	//	// linux file exists
	//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1)");
	//	b = FU::fileExists(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a12345.jpg)");
	//
	//	// linux delete file
	//	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::deleteFile(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//
	//	// linux move file
	//	b = FU::moveFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::moveFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)", false);
	//	b = FU::moveFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a01.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::moveFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout1/a01.jpg)");
	//	b = FU::moveFile(
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout/a03.jpg)",
	//		LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout2/a03.jpg)");
	//
	//	//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
		//b = FU::deleteFile(LR"(D:\IMAG0036_Copy.jpg)");
	//	bool b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)");
	//	b = FU::copyFile(LR"(D:\IMAG0036.jpg)", LR"(D:\IMAG0036_Copy.jpg)", false);
		//b = FU::fileExists(LR"(D:\emails)");
		//b = FU::fileExists(LR"(D:\myimage.jpg)");
		//b = FU::fileExists(LR"(D:\xyz)");
		//b = false;

		// linux

	//	StringsT files;
	//	FU::findFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files);
	//	Logger::info(files, L"All files");
	//
	//	files.clear();
	//	FU::findMatchingFiles(LR"(/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Tryout)", files, L"a1*.jpg");
	//	Logger::info(files, L"a1*.jpg files");
	//
	//
	//	return;


		// windows
			//StringsT files;
			//StringsT dirs;
			//FU::findFilesDirs(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, dirs);
			//Logger::info(files, L"All files");
			//Logger::info(dirs, L"All directories");

			//files.clear();
			//FU::findMatchingFiles(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, L"*.mp4");
			//Logger::info(files, L"mp4 files");

			//files.clear();
			//FU::findMatchingFilesRex(LR"(D:\Projects\WxWidgets\YipPreview\Tryout)", files, L"a0[1-6].jpg");
			//Logger::info(files, L"regex a01.jpg to a06.jpg files");




		//	TryOut::AsyncShell(GetEventHandler());
		//	TryOut::WorkerThread();
		//	TryOut::ExecIt();

		//	ShellExecuteResult result;
		//    ShellExecute::shellSync(L"/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al", result);
		//    //ShellExecute::shellSync(L"/bin/nonexistentprogram", result);
		//    //ShellExecute::shellSync(L"/bin/notepadqq", result, 5000);
		//    std::wstring ws = result.toString();
		//    Logger::info(ws.c_str());
}







