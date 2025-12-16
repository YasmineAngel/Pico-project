#include "state_chart.hpp"
#include <iostream>
#include <algorithm>

using namespace std;

SimpleState *SimpleState::handler(Evt_t evt)
{
    SimpleState *next = nullptr;
    for (auto const& t : transitions) {
        if (t.guard(evt)) {
            t.move();
            // if (this != t.next()) this->exit();
            // t.action();
            // next = t.next();
            // if (this != next) next->entry();
            return t.next();
        }
    }
    return nullptr;
}

void SimpleState::add_transition(const Transition &t)
{
    transitions.push_back(t);
}

void SimpleState::set_parent(CompositeState &s)
{
     parent = &s;
     nesting_level = s.nesting_level + 1;
}


void SimpleState::entry()
{
    if (parent) {
      parent->set_current(this);
    }
    entry_fun();
}

void SimpleState::explicit_entry()
{
    SimpleState::entry();
}

void SimpleState::exit()
{
    exit_fun();
}

void CompositeState::set_initial(SimpleState &ini)
{
    if (std::find(begin(inner), end(inner), &ini) == end(inner))
        throw BrokenStructure("Cannot find initial state in composite state " + get_name());
    initial = &ini;
    current = &ini;
}

void CompositeState::add_child(SimpleState &child)
{
    inner.push_back(&child);
    child.set_parent(*this);    
}

void CompositeState::reset()
{
    if (not history) {
        current = initial;
    }
}

void CompositeState::entry()
{
    reset();
    SimpleState::entry();
    if (current != nullptr)
        current->entry();
}

void CompositeState::explicit_entry()
{
    SimpleState::entry();
}

void CompositeState::exit()
{
    if (current != nullptr) current->exit();
    SimpleState::exit();
}

SimpleState * CompositeState::handler(Evt_t evt)
{
    SimpleState *next = nullptr;
    if (current == nullptr) return nullptr;
    
    next = current->handler(evt);
    if (next != nullptr) {
        //current = next;
        return nullptr;
    }
    else return SimpleState::handler(evt);
}

SimpleState* OrthogonalState::handler(Evt_t evt)
{
    for (auto *c : regions) 
        c->handler(evt);
    return SimpleState::handler(evt);
}

void OrthogonalState::add_region(CompositeState &r)
{
    regions.push_back(&r);
}

void OrthogonalState::entry()
{
    SimpleState::entry();
    for(auto *s : regions) s->entry();
}

void OrthogonalState::exit()
{
    for(auto *s: regions) s->exit();
    SimpleState::exit();
}

std::string OrthogonalState::get_configuration() const
{
    std::string n = name;
    n += ".(";
    int i = 0;
    for (auto *r : regions) {
        i++;
        n += r->get_name();
        if (i < regions.size()) n += "||";
    }
    n += ")";
    return n;
}
