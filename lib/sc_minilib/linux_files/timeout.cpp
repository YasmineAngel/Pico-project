#include <timeout.h>
#include <sm_defs.h>
#include <stdio.h>


struct TimerElement {
    int id;
    struct timespec wakeup;
    int evt_id;
    StateMachine *sm;
    int next;
};

struct TimerList {
    TimerElement array[MAX_TIMEOUTS];
    int free;
    int head;
    int num;
    pthread_mutex_t m;
    pthread_cond_t c;
    int next_id;
};

TimerList tlist;

int tlist_getfree()
{
    int pos = tlist.free;
    tlist.free = tlist.array[pos].next;
    return pos;
}

void tlist_insert(int pos)
{
    int p = -1;
    int q = tlist.head;

    while (q != -1 ) {
        if (tspec_cmp(&tlist.array[q].wakeup, &tlist.array[pos].wakeup) > 0 )
            break;
        else {
            p = q;
            q = tlist.array[q].next;
        }
    }
    if (p == -1) tlist.head = pos;
    else tlist.array[p].next = pos;
    tlist.array[pos].next = q;
    tlist.num++;     
}

void tlist_free(int pos)
{
    tlist.head = tlist.array[pos].next;
    tlist.array[pos].id = -1;
    tlist.array[pos].next = tlist.free;
    tlist.free = pos;
    tlist.num--;
}

void *timeout_thread(void *arg);
pthread_t timeout_thread_id;

void timer_list_init()
{
    tlist.head = -1;
    tlist.free = 0;
    tlist.num = 0;
    tlist.next_id = 0;
    // chained list
    // starting at free = 0, next = i+1
    for (int i=0; i<MAX_TIMEOUTS-1; i++) {
        tlist.array[i].id = -1;
        tlist.array[i].next = (i+1);
    }
    tlist.array[MAX_TIMEOUTS-1].id = -1;
    tlist.array[MAX_TIMEOUTS-1].next = -1;
    
    pthread_mutex_init(&tlist.m, NULL);
    pthread_cond_init(&tlist.c, NULL);

    pthread_create(&timeout_thread_id, NULL, timeout_thread, NULL);
}


void *timeout_thread(void *arg)
{
    struct timespec now, wakeup;
    int flag;
    TimerElement elem;
    int err;
    int pos = -1;
    
    while(1) {
        pthread_mutex_lock(&tlist.m);
        clock_gettime(CLOCK_REALTIME, &now);
        if (tlist.head < 0) {
            wakeup = tspec_add_delta(&now, 100, SEC);
            pos = -1;
        }
        else {
            pos = tlist.head;
            wakeup = tlist.array[pos].wakeup;
        }
        err = pthread_cond_timedwait(&tlist.c, &tlist.m, &wakeup);
        
        if ((err != 0) && (pos >=0)) {
            if (tlist.array[pos].id >= 0) {
                tlist.array[pos].sm->send(tlist.array[pos].evt_id);
            }
            tlist_free(pos);
        }
        else pos = -1;

        pthread_mutex_unlock(&tlist.m);
    }
}


int set_timeout(int howlong, int unit, int evt_id, StateMachine *sm)
{
    struct timespec now;
    
    pthread_mutex_lock(&tlist.m);
    clock_gettime(CLOCK_REALTIME, &now);
    
    if (tlist.num >= MAX_TIMEOUTS) {
        pthread_mutex_unlock(&tlist.m);
        return ERR_MAX_TIMEOUTS;
    }

    TimerElement elem;
    elem.id = tlist.next_id++;
    if (tlist.next_id < 0) tlist.next_id = 0;
    
    elem.wakeup = tspec_add_delta(&now, howlong, unit); 
    elem.evt_id = evt_id;
    elem.sm = sm;
    elem.next = -1;
    
    int pos = tlist_getfree();
    tlist.array[pos] = elem;
    tlist_insert(pos);
    
    pthread_cond_signal(&tlist.c);
    
    pthread_mutex_unlock(&tlist.m);
    return elem.id;
}

void remove_timeout(int id)
{
    pthread_mutex_lock(&tlist.m);
    int p = tlist.head;
    while (p != -1) {
        if (tlist.array[p].id == id) {
            tlist.array[p].id = -1;
            break;
        }
        else p = tlist.array[p].next;
    }
    pthread_mutex_unlock(&tlist.m);
}
