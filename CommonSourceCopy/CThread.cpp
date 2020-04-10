/*
 * CThread.cpp
 *
 *  Created on: 9 Oct 2019
 *      Author: richard
 */

#include "CThread.h"




CThread::CThread(IThread* ithread) :
	_ithread(ithread),
	_run(false)
{

}

CThread::~CThread()
{//https://www.youtube.com/watch?v=FRu8SRWuUko
}

bool CThread::Start()
{
    if (!_run.load())
    {
        if (_ithread->Initialise())
        {
        	std::future<bool> fok = _ok.get_future();
            _run.store(true);
            _wait = std::async(std::launch::async, [this] () -> bool { return ThreadMainEntry(); } );

            // check that ThreadInitialise() reports success
            // timeout after 5 seconds
            auto res = fok.wait_for(std::chrono::duration<long, std::milli>(5000));
            if (res != std::future_status::ready)
            {
               return false;
            }
            return fok.get();
        }
    }
    return false;
}

bool CThread::Stop()
{
    if (_run.load())
    {
        _run.store(false);
        auto res = _wait.wait_for(std::chrono::duration<long, std::milli>(5000));
        if (res == std::future_status::ready)
        {
        	_ithread->Uninitialise();
        	return true;
        }
    }
    return false;
}

bool CThread::ThreadMainEntry()
{
    if (_ithread->ThreadInitialise())
    {
    	// signal main thread initialised ok
    	_ok.set_value(true);

		while (_run.load())
		{
			if (!_ithread->ThreadMain())
			{
				_run.store(false);
				break;
			}
		}
		_ithread->ThreadUninitialise();
		return true;
    }
    else
    {
    	_ok.set_value(false);
    	return false;
    }
}


