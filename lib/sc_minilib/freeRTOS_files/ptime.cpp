#include "includes/ptime.h"
#include "FreeRTOS.h"
#include "pico/stdlib.h"
#include "task.h"


#define NS_PER_SEC 1000000000LL
#define TICKS_PER_SEC configTICK_RATE_HZ
#define NS_PER_TICK (NS_PER_SEC / TICKS_PER_SEC)

TickType_t customTimespecToTicks(const tspec *ts) {
    int64_t total_ns = ts->tv_sec * NS_PER_SEC + ts->tv_nsec;
    TickType_t ticks = total_ns / NS_PER_TICK;
    return ticks;
}

TickType_t elapsedTicksBetweenTimespecs(const tspec *start, const tspec *end) {
    TickType_t start_ticks = customTimespecToTicks(start);
    TickType_t end_ticks = customTimespecToTicks(end);

    if (end_ticks >= start_ticks) {
        return end_ticks - start_ticks;
    } else {
        // En cas de débordement (end < start), par exemple si le timer a bouclé
        return (TickType_t) 0; // ou gérer le débordement comme vous le souhaitez
    }
}

void get_custom_tspec(tspec *ts) {
    TickType_t ticks_since_boot = xTaskGetTickCount();
    int64_t us_since_boot = (int64_t)ticks_since_boot * (1000000 / TICKS_PER_SEC);
    ts->tv_sec = us_since_boot / 1000000;
    ts->tv_nsec = (us_since_boot % 1000000) * 1000;
}




struct unit_conv {
    long mul;
    long div;
};

const struct unit_conv conv_table[] = {
    {1L, 1000000000L}, // SEC
    {1000L, 1000000L}, // MILLI
    {1000000L, 1000L}, // MICRO
    {1000000000L, 1L}  // NANO
};



static tspec tspec_t0;

const tspec tspec_zero = {0, 0};

tspec tspec_get_ref() { return tspec_t0; }

void tspec_init() { get_custom_tspec( &tspec_t0); }

/**
   Given a timespec, converts to a long according to unit.
 */
long tspec_to(const tspec *t, int unit) {
    long tu;
    tu = (t->tv_sec) * conv_table[unit].mul;
    tu += (t->tv_nsec) / conv_table[unit].div;

    return tu;
}

/**
   Given a long integer, expressed as unit, converts it into a
   timespec.
 */
tspec tspec_from(long tu, int unit) {
    tspec t;

    long mm = tu % conv_table[unit].mul;

    t.tv_sec = tu / conv_table[unit].mul;
    t.tv_nsec = mm * conv_table[unit].div;

    return t;
}

/**
   Given a timespec, converts to a long according to unit.
 */
long tspec_to_rel(const tspec *t, int unit) {
    long tu;
    tu = (t->tv_sec - tspec_t0.tv_sec) * conv_table[unit].mul;
    tu += (t->tv_nsec - tspec_t0.tv_nsec) / conv_table[unit].div;

    return tu;
}

/**
   Given a long integer, expressed as unit, converts it into a
   timespec.
 */
tspec tspec_from_rel(long tu, int unit) {
    tspec t;

    long mm = tu % conv_table[unit].mul;

    t.tv_sec = tu / conv_table[unit].mul + tspec_t0.tv_sec;
    t.tv_nsec = mm * conv_table[unit].div + tspec_t0.tv_nsec;

    return t;
}

long ptime_gettime(int unit) {
    tspec t;

    get_custom_tspec( &t);

    return tspec_to_rel(&t, unit);
}

tspec tspec_add_delta(const tspec *a, ptime delta, int unit) {
    tspec d = tspec_from(delta, unit);
    tspec s;
    s.tv_nsec = a->tv_nsec + d.tv_nsec;
    s.tv_sec = a->tv_sec + d.tv_sec;
    while (s.tv_nsec >= 1000000000) {
        s.tv_nsec = s.tv_nsec - 1000000000;
        s.tv_sec += 1;
    }
    return s;
}

tspec tspec_add(const tspec *a, const tspec *b) {
    tspec s;
    s.tv_nsec = a->tv_nsec + b->tv_nsec;
    s.tv_sec = a->tv_sec + b->tv_sec;
    while (s.tv_nsec >= 1000000000) {
        s.tv_nsec = s.tv_nsec - 1000000000;
        s.tv_sec += 1;
    }
    return s;
}

/**
   Compares two timespecs
 */
int tspec_cmp(const tspec *a, const tspec *b) {
    if (a->tv_sec > b->tv_sec)
        return 1;
    else if (a->tv_sec < b->tv_sec)
        return -1;
    else if (a->tv_sec == b->tv_sec) {
        if (a->tv_nsec > b->tv_nsec)
            return 1;
        else if (a->tv_nsec == b->tv_nsec)
            return 0;
        else
            return -1;
    }
    return 0;
}

tspec tspec_sub(const tspec *a, const tspec *b) {
    tspec d;

    d.tv_nsec = a->tv_nsec - b->tv_nsec;
    d.tv_sec = a->tv_sec - b->tv_sec;
    if (a->tv_nsec < b->tv_nsec) {
        d.tv_nsec += 1000000000;
        d.tv_sec -= 1;
    }
    return d; // tspec_to(&d, unit);
}

/**
   d = a - b
 */
tspec tspec_sub_delta(const tspec *a, ptime delta, int unit) {
    tspec d;
    tspec b = tspec_from(delta, unit);
    d.tv_nsec = a->tv_nsec - b.tv_nsec;
    d.tv_sec = a->tv_sec - b.tv_sec;
    if (a->tv_nsec < b.tv_nsec) {
        d.tv_nsec += 1000000000;
        d.tv_sec -= 1;
    }
    return d;
}
