/*
 * ARSim
 * Copyright (c) 2000-2010 Tommaso Cucinotta
 *
 * This file is part of ARSim.
 *
 * ARSim is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * ARSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with ARSim. If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __ARSIM_EVENTS_HPP__
#  define __ARSIM_EVENTS_HPP__

/** The main representation of time quantities within the simulator */
typedef double Time;
#define TIME_FMT "%g"

#include <list>

class Event;

using namespace std;

/** Base event class **/
class Event {
public:
  Time delta_time;
  void *p_data;
  Event(Time delta_time, void *p_data)
    : delta_time(delta_time), p_data(p_data) {  }
  virtual void dispatch() = 0;
  virtual ~Event() { }
};

/** Template event subclass **/
template <class T>
class EventT : public Event {
public:
  typedef void (T::*EventTMethodPtr)(const Event & ev);

  EventT(Time delta_time, T *p_handler, EventTMethodPtr p_method, void *p_data = 0)
  : Event(delta_time, p_data), p_handler(p_handler), p_method(p_method) {  }
  virtual void dispatch() {
    (p_handler->*p_method)(*this);
  }
private:
  T *p_handler;
  EventTMethodPtr p_method;
};

/** Template event builder allowing specification of any method of any class
 ** to be activated when the event is dispatched.
 **/
template <class T>
Event *makeEvent(Time delta_time,
		 T *p_handler,
		 void (T::*p_method)(const Event & ev),
		 void *p_data = 0)
{
  return new EventT<T>(delta_time, p_handler, p_method, p_data);
}

/** The next events to be managed       **/
class EventList {
private:
  list<Event*> v_events;
  Time curr_time;        /**< Current time (ms)          */
  Event *getNextEvent() {
    if (v_events.empty())
      return 0;
    return *(v_events.begin());
  }

public:

  EventList();

  /** Insert a new event into the event list            **/
  void insert(Event *p_ev);
  /** Returns true if element was found	(and removed)   **/
  bool remove(Event *p_ev);
  /** Performs the dispatch of the next event           **/
  void dispatch() {
    Event *p_ev = *(v_events.begin());
    v_events.pop_front();
    p_ev->dispatch();
    delete p_ev;
  }
  // Singleton pattern and enforcement of correct static construction order.
  static EventList & events();
  static Time getTime() { return events().curr_time; }

  /** Perform an event-based simulation step
   **
   ** This method advances time to the nearest event, and performs
   ** the dispatch of all the events occurring at the same new time
   **/
  void step();
};

#endif
