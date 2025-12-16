#ifndef __TIMEOUT_H__
#define __TIMEOUT_H__

#include <ptime.h>
#include <pthread.h>
#include <sm_defs.h>

#define MAX_TIMEOUTS 10
#define ERR_MAX_TIMEOUTS -2

void timer_list_init();
int set_timeout(int howlong, int unit, int evt_id, StateMachine *sm);
void remove_timeout(int id);

#endif 
