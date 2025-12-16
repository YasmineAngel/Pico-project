#pragma once

#include <string>
#include <vector>
#include <transition.hpp>


class BrokenStructure : public std::exception {
    std::string err_msg;
public:
    BrokenStructure(std::string msg) : err_msg(msg) {}
    const char* what() const throw() override {
        return err_msg.c_str();
    }
};



class CompositeState;

/** This class represents a simple state in a state chart diagram.  It
   is also the root of the composite pattern hierarchy.

   +--------------------+
   |    SimpleState     |
   +--------------------+<----+
   | handler(Evt_t evt) |     |
   +--------------------+     |
             A                |
             |                |
             |                |
   +--------------------+     |
   |   CompositeState   |     |
   +--------------------+-----+
   | handler(Evt_t evt) |
   +--------------------+

   It implements the basic machinery for handling events with
   transitions. Every SimpleState has a set of outgoing
   transitions. When the handler is called, it simply analyses all
   transitions looking for the one that handles the event.
*/        
class SimpleState {
    friend class Transition;

    /** Adds a new transition.
        This function is called from the constructor of class Transition */
    void add_transition(const Transition &t);
    
    // the set of outgoing transitions
    std::vector<Transition> transitions;

    // entry action
    action_t entry_fun = empty_action;
    // exit action
    action_t exit_fun = empty_action;
    // do actions are not supported
    
protected:
    // the name of the state 
    std::string name;
    // the parent state (if any)
    CompositeState *parent = nullptr;
    // how much the state is nested inside other composite states
    int nesting_level = 0;
    
    /** sets the parent state. It must be a composite state */
    void set_parent(CompositeState &s);

    friend class CompositeState;
    
    /** returns the parent state */
    inline CompositeState *get_parent() const { return parent; }
    
    /** returns the nesting level */
    inline int get_nesting_level() const { return nesting_level; }
    
public:
    /** Simplest constructor */
    SimpleState(const std::string&n) : name (n) {}
    
    /** Full constructor
       n : name
       en : entry action
       ex : exit action */
    SimpleState(const std::string& n, action_t en, action_t ex) :
        entry_fun(en), exit_fun(ex), name(n) {}

    /** Returns the name of the state */
    std::string get_name() const { return name; }

    /** Returns a string representing the current configuration. For simple
        states this is the same as the name */
    virtual std::string get_configuration() const { return name; }

    /** The handler of the events for this state */
    virtual SimpleState* handler(Evt_t evt);

    /** Enters the state.
       This function :
       - calls the entry function of the state
       - if necessary, sets the current pointer of the parent state
         to point to this state. 
     */
    virtual void entry();

    /** Enters the state.
       This function is only useful for composite states.
       For simple states, this is the same as the entry() method. 
     */
    virtual void explicit_entry();

    /** Exits from the state.
        This function simply calls the exit function
        but it does not modify the current pointer of the parent state. */
    virtual void exit();

    virtual ~SimpleState() {}
};


/**
   A composite state.
   It derives (so it has the same interface as) from SimpleState.
   It can contain other SimpleStates to form a hierarchy. 
 */
class CompositeState : public SimpleState {
    SimpleState *current = nullptr;
    std::vector<SimpleState *> inner;
    SimpleState *initial = nullptr;
    bool history = false;
public:
    /** Simplest constructor */
    CompositeState(const std::string&n) : SimpleState(n) {}
    /** Full constructor :
        n : name
        en : entry action
        ex : exit action */
    CompositeState(const std::string& n, action_t en, action_t ex) :
        SimpleState(n, en, ex) {}

    /** Returns the current configuration */
    std::string get_configuration() const override
        { return name + "." + current->get_configuration(); }

    SimpleState * handler(Evt_t evt) override;

    /** Adds a contained state */
    void add_child(SimpleState &child);
    /** Sets the initial state */
    void set_initial(SimpleState &ini);
    /** Returns the current state */
    SimpleState * get_current() { return current; }
    /** Changes the current state after a transition happens */ 
    void set_current(SimpleState *p) { current = p; }
    /** bring back to the initial state (if not history) */
    void reset();
    /** set "history" for this state. Only shallow histoiry is supported currently */
    void set_history(bool h) { history = h; }
    /** Enters the state */
    void entry() override;

    /** Enters one of the states internal to this composite state. In
     *  other words, this state is not the direct destination of the
     *  Transition.
     */
    void explicit_entry() override;
    
    /** Exit from the state */
    void exit() override;
};

/** An orthogonal state

    It contains parallel "regions", each region is a composite state.
    This class is not yet completely tested ! */
class OrthogonalState : public SimpleState {
    std::vector<CompositeState *> regions;
public:
    OrthogonalState(const std::string&n) : SimpleState(n) {}
    OrthogonalState(const std::string&n, action_t en, action_t ex) :
        SimpleState(n, en, ex) {}

    std::string get_configuration() const override;
    
    void add_region(CompositeState &r);
    SimpleState *handler(Evt_t evt) override;
    void entry() override;
    void exit() override;
};
