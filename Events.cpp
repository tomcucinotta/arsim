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

#include "Events.hpp"

#include "util.hpp"

EventList::EventList() : v_events(), curr_time(0.0) {  }

/// Singleton pattern and enforcement of correct static initialization order.
EventList & EventList::events() {
  static EventList event_list;
  return event_list;
}

void EventList::insert(Event *p_ev) {
  list<Event*>::iterator it = v_events.begin();
  while ((it != v_events.end()) && (p_ev->delta_time >= (*it)->delta_time)) {
    p_ev->delta_time -= (*it)->delta_time;
    ++it;
  }
  if (it != v_events.end())
    (*it)->delta_time -= p_ev->delta_time;
  v_events.insert(it, p_ev);
}

bool EventList::remove(Event *p_ev) {
  list<Event*>::iterator it = v_events.begin();
  while ((it != v_events.end()) && ((*it) != p_ev))
    ++it;
  if (it == v_events.end())
    return false;

  list<Event*>::iterator it_next = it;
  ++it_next;
  if (it_next != v_events.end())
    (*it_next)->delta_time += p_ev->delta_time;
  v_events.erase(it);
  return true;
}

void EventList::step() {
  Event *p_ev = events().getNextEvent();
  /* The event list should never be empty       */
  CHECK(p_ev != 0, "Empty event list. Add a scheduler using the -s option");
  /* Update current time                        */
  curr_time += p_ev->delta_time;
  /* Dispatch all simultaneous events, if any   */
  do {
    Logger::debugLog("# T=%g: extracted event with dt=%g\n", curr_time, p_ev->delta_time);
    events().dispatch();
    p_ev = events().getNextEvent();
  } while ((p_ev != 0) && (p_ev->delta_time == 0));
}
