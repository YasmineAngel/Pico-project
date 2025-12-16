#include <sm_defs.h>

//static void evtq_init(EvtQueue *q);
//static int evtq_insert(EvtQueue *q, Evt_t evt);
//static Evt_t evtq_extract(EvtQueue *q);

void *sm_thread(void *arg)
{
    StateMachine *sm = (StateMachine *) arg;
    Evt_t evt;
    while(1) {
        evt = sm->get_queue().extract();
        sm->handler(evt);
    }
}

EvtQueue::EvtQueue()
{
    head = 0;
    tail = 0;
    num  = 0;
    pthread_mutex_init(&m, NULL);
    pthread_cond_init(&c, NULL);
}

bool EvtQueue::is_empty() const
{
    return num == 0;  
}

int EvtQueue::insert(Evt_t evt)
{
    int err = 0;
    pthread_mutex_lock(&m);
    
    if (num < MAX_QUEUED_EVENTS) {
        array[head] = evt;
        head = (head + 1) % MAX_QUEUED_EVENTS;
        num ++;
        err = evt;
        pthread_cond_signal(&c);
    }
    else err = ERR_QUEUE_FULL;

    pthread_mutex_unlock(&m);
    return err;
}

int EvtQueue::extract()
{
    Evt_t ret = 0;
    pthread_mutex_lock(&m);
    
    while (num == 0)
        pthread_cond_wait(&c, &m);
        
    ret = array[tail];
    tail = (tail + 1) % MAX_QUEUED_EVENTS;
    num--;
    
    pthread_mutex_unlock(&m);
    return ret;
}


StateMachine::StateMachine() : queue()
{
    pthread_create(&thread_id, NULL, sm_thread, this);
}

void StateMachine::send(Evt_t evt) { queue.insert(evt); }

bool StateMachine::process_one_nb()
{
    if (queue.is_empty()) return false;
    Evt_t evt = queue.extract();
    handler(evt);
    return true;
}

void StateMachine::process_all_nb()
{
    while(process_one_nb());
}

void StateMachine::process_one()
{
    Evt_t evt = queue.extract();
    handler(evt);
}
