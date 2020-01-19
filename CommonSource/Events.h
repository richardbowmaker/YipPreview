/*
 * Event.h
 *
 *  Created on: 19 Jan 2020
 *      Author: richard
 */

#ifndef COMMON_EVENTS_H_
#define COMMON_EVENTS_H_

#include <wx/wx.h>


class Events
{
public:

	virtual ~Events() = default;

private:

	Events() = default;
	Events(const Events&) = default;
	Events(const Events&&);



};

#endif /* COMMON_EVENTS_H_ */
