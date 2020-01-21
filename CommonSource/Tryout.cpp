


#include <wx/wx.h>
#include <wx/thread.h>

#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	#include <unistd.h>
#endif

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <utility>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <sstream>
#include <fcntl.h>
#include <time.h>

#include "Logger.h"
#include "Tryout.h"
#include "Events.h"



wxDEFINE_EVENT(wxEVT_MY_CUSTOM_COMMAND, wxCommandEvent);

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
    int timeout = 5;
//    std::string command = "/bin/ls /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview -al";
    std::string command = "nonexistentprogram";
//    std::string command = "/bin/notepadqq";
    int rfd;
    FILE * fp = popen2(command, "r", pid, rfd);
    char command_out[100] = {0};
    std::stringstream output;

    int flags = fcntl(rfd, F_GETFL);
    int fderr = fcntl(rfd, F_SETFL, flags | O_NONBLOCK);

    errno = 0;
    int rerr;

    timespec ts;
    long startt = static_cast<long>(time(nullptr));

    while (rerr = read(fileno(fp), command_out, sizeof(command_out)-1))
    {
    	if (rerr == EAGAIN)
    	{
    		int nn = 0;
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

    int nn = pclose2(fp, pid);

    int zzz =  0;



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

    int err = fclose(fp);
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

//int main()
//{
//    int pid;
//    std::string command = "ping 8.8.8.8";
//    FILE * fp = popen2(command, "r", pid);
//    char command_out[100] = {0};
//    std::stringstream output;
//
//    //Using read() so that I have the option of using select() if I want non-blocking flow
//    while (read(fileno(fp), command_out, sizeof(command_out)-1) != 0)
//    {
//        output << std::string(command_out);
//        kill(-pid, 9);
//        memset(&command_out, 0, sizeof(command_out));
//    }
//
//    std::string token;
//    while (getline(output, token, '\n'))
//        printf("OUT: %s\n", token.c_str());
//
//    pclose2(fp, pid);
//
//    return 0;
//}


