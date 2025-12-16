#ifndef __SM_DEFS_H__
#define __SM_DEFS_H__

#include "FreeRTOS.h"
#include "semphr.h"


#define MAX_QUEUED_EVENTS 1000
#define ERR_QUEUE_FULL -1

#ifndef MAX_SC_STACK_SIZE
#define MAX_SC_STACK_SIZE 1024
#endif

typedef int Evt_t;

class EvtQueue {
    Evt_t array[MAX_QUEUED_EVENTS];
    int head;
    int tail;
    int num;
    SemaphoreHandle_t m;
    SemaphoreHandle_t c;
 public:
    EvtQueue();
    int insert(Evt_t evt);
    Evt_t extract();
    bool is_empty() const ;
};
    
class StateMachine {
    EvtQueue queue;
 public:
    StateMachine();
    /* sends an event to the machine */
    void send(Evt_t evt);
    /* expose the queue */
    EvtQueue & get_queue() { return queue; }
    /* handles an event */
    virtual void handler(Evt_t evt) = 0;
    /* waits for one event and then process it */
    void process_one();
    /* If there is an event in the queue, process it and returns true
       else return false */
    bool process_one_nb();
    /* While there are events in the queue, process them.
       This is a blocking function !*/
    void process_all_nb();
};

#endif 
