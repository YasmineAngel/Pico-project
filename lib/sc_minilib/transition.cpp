#include "transition.hpp"
#include "state_chart.hpp"
#include <iostream>

/* new way of handling a transition

   - if the source and destination states are the same, nothing to do
   - if the source and destination states have the same parent,
     - just exit, action, enter
   - if source and destination have not the same parent :
     - search the common parent
     - from source, exit all states until the common parent
     - from then action
     - then enter all state until destination (of course in reverse order)
   The common parent can be nullptr !

   - to search the common parent :
     - compare the nesting levels
     - if nesting(source) > nesting(dest),
          - move up s until nesting(s) == nesting(d)
          - store the pointers in a deque (from head)
     - if nesting(source) < nesting(dest)
          - move up d until nesting(s) == nesting(d)
          - store the pointers in a deque (from tail)
     - if they are the same, compare their parents, if the same we are over,
       otherwise move up both and store the respective pointers
   - if no common parent is found, return an error

   The result is two deques, containing the list of pointers to the
   states to exit and to enter, respectively
   
*/
void Transition::build_path(SimpleState &source, SimpleState &dest)
{
    if (&source == &dest) return;
    exit_path.push_back(&source);
    //entry_path.push_front(&dest);

    SimpleState *sp = source.get_parent();
    SimpleState *dp = dest.get_parent();
    
    while(sp != dp) {
        //std::cout << "Not the same parent, going up " << std::endl;

        if (source.get_nesting_level() < dest.get_nesting_level()) {
            if (dp == nullptr) throw("Bad destination parent!");
            entry_path.push_front(dp);
            dp = dp->get_parent();
        }
        else {
            if (sp == nullptr) throw("Bad source parent");
            exit_path.push_back(sp);
            sp = sp->get_parent();
        }
    }
}

void Transition::move() const
{
    for (auto p : exit_path) {
        p->exit();
    }
    
    action();

    for (auto p : entry_path)
        p->explicit_entry();
    
    if (source != dest) dest->entry();
}

Transition::Transition(SimpleState &s, SimpleState &d) : source(&s), dest(&d)
{
    build_path(s,d);
    source->add_transition(*this);
}

Transition::Transition(SimpleState &s, SimpleState &d, Evt_t event) :
    source(&s), dest(&d)
{
    build_path(s,d);
    guard_fun = [event](Evt_t evt) { return event == evt; };
    source->add_transition(*this);
}

Transition::Transition(SimpleState &s, SimpleState &d, Evt_t event, action_t a) :
    source(&s), dest(&d), action_fun(a)
{
    build_path(s,d);
    guard_fun = [event](Evt_t evt) { return event == evt; };
    source->add_transition(*this);
}

Transition::Transition(SimpleState &s, SimpleState &d, guard_t g, action_t a) :
    source(&s), dest(&d), guard_fun(g), action_fun(a)
{
    build_path(s,d);
    source->add_transition(*this);
}

bool Transition::guard(Evt_t evt) const
{
    return guard_fun(evt);
}

void Transition::action() const
{
    action_fun();
}


