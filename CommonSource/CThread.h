/*
 * CThread.h
 *
 *  Created on: 9 Oct 2019
 *      Author: richard
 */

#ifndef CTHREAD_H_
#define CTHREAD_H_

#include <future>
#include <atomic>
#include <string>

class IThread
{
public:

	IThread() {};
	virtual ~IThread() {};

    // optional overrides
    virtual bool Initialise() = 0;          // client thread initialise
    virtual void Uninitialise() = 0;        // client thread uninitialise
    virtual bool ThreadInitialise() = 0;    // in thread initialise
    virtual void ThreadUninitialise() = 0;  // in thread uninitialise
    virtual bool ThreadMain() = 0;          // thread worker, called repeatedly                                        	
											// return false to stop thread
};

class CThread
{
public:

	CThread(IThread* ithread);
	virtual ~CThread();

    bool Start();
    bool Stop();

private:

	CThread(const CThread&);
    bool ThreadMainEntry();

	IThread* _ithread;
    std::atomic <bool> _run;
    std::future<bool> _wait;
    std::promise<bool> _ok;
};


#endif /* CTHREAD_H_ */
