#pragma once

#include <sm_defs.h>
#include <functional>
#include <deque>

class SimpleState;

/** An action is just a function from void to void */
typedef std::function<void(void)> action_t;

/** A guard is a just function from Event to bool */
typedef std::function<bool(Evt_t)> guard_t;

/** the empty action is a function that does nothing */
const action_t empty_action = []() {};

/** the empty guard always returns true */
const guard_t empty_guard = [](Evt_t e) { return true; };


/**
   This class holds together the three elements of every
   transitions. It redirects the action() and guard() towards the
   appropriate user-defined functions.
 */
class Transition {
    SimpleState *source, *dest;

    guard_t guard_fun = empty_guard;
    action_t action_fun = empty_action;

    std::deque<SimpleState *> exit_path;
    std::deque<SimpleState *> entry_path;

    void build_path(SimpleState &source, SimpleState &dest);

public:
    /** constructor with no guard and no action */
    Transition(SimpleState &source, SimpleState &dest);
    /** constructor with a simple event as a guard */
    Transition(SimpleState &source, SimpleState &dest, Evt_t event);
    /** constuctor with a simple event as a guard and an action */
    Transition(SimpleState &source, SimpleState &dest, Evt_t event, action_t a);
    /** constuctor with a complex guard and an action */
    Transition(SimpleState &source, SimpleState &dest, guard_t g, action_t a);
    
    /** evaluates the event and the guard */
    bool guard(Evt_t evt) const;
    /** performs the action */
    void action() const;
    /** returns the destination */
    SimpleState *next() const { return dest; }

    /** Changes states */
    void move() const;

};
