#include "includes/sm_defs.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

TaskHandle_t  sm_thread_handle = NULL;

void sm_thread_task(void *arg)
{
    StateMachine *sm = (StateMachine *) arg;
    Evt_t evt;
    while(1) {
        sm->process_one();
        //printf("Event processing completed\n");
    }
}

EvtQueue::EvtQueue()
{
    head = 0;
    tail = 0;
    num  = 0;
    m = xSemaphoreCreateMutex();
    c = xSemaphoreCreateCounting( 1000, 0 );
}


bool EvtQueue::is_empty() const { return num == 0; }

int EvtQueue::insert(Evt_t evt)
{
    int err = 0;
    xSemaphoreTake(m, portMAX_DELAY);
    
    if (num < MAX_QUEUED_EVENTS) {
        array[head] = evt;
        head = (head + 1) % MAX_QUEUED_EVENTS;
        num ++;
        err = evt;
        xSemaphoreGive(c);
    }
    else err = ERR_QUEUE_FULL;

    xSemaphoreGive(m);
    return err;
}

int EvtQueue::extract()
{
    Evt_t ret = 0;
    xSemaphoreTake(c, portMAX_DELAY);
    ret = array[tail];
    tail = (tail + 1) % MAX_QUEUED_EVENTS;
    num--;
    return ret;
}


StateMachine::StateMachine() : queue()
{
    xTaskCreate(sm_thread_task, "sm_thread", MAX_SC_STACK_SIZE, this , 1, &sm_thread_handle);
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
    Evt_t evt = queue.extract(); // blocking
    handler(evt);
}
