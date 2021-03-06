/* ghighres.c
 *
 * Copyright (c) 2011-2016, Lawrence Livermore National Security, LLC.
 * LLNL-CODE-645430
 *
 * Produced at Lawrence Livermore National Laboratory
 * Written by  Barry Rountree,   rountree@llnl.gov
 *             Daniel Ellsworth, ellsworth8@llnl.gov
 *             Scott Walker,     walker91@llnl.gov
 *             Kathleen Shoga,   shoga1@llnl.gov
 *
 * All rights reserved.
 *
 * This file is part of libmsr.
 *
 * libmsr is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * libmsr is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libmsr. If not, see <http://www.gnu.org/licenses/>.
 *
 * This material is based upon work supported by the U.S. Department of
 * Energy's Lawrence Livermore National Laboratory. Office of Science, under
 * Award number DE-AC52-07NA27344.
 *
 */
/*! \file ghighres.c
 * \brief high resolution sleep function
 */

#include <sys/select.h>
#include <time.h>

struct mstimer
{
    /// @brief When we started tracking the timer.
    unsigned long startms;
    /// @brief Which time is the next interval.
    uint step;
    /// @brief How many ms between firings.
    uint interval;
    /// @brief When does the timer expire next.
    unsigned long nextms;
};

#if 0 /* Deprecated. */
/* Get a number of millis from realtime clock. */
unsigned long now_rt_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    unsigned long sec = t.tv_sec * 1000;
    unsigned long msec = (t.tv_nsec + 500000) / 1000000;
    return sec + msec;
}
#endif

/// @brief Get a number of millis from a monotonic clock.
unsigned long now_ms(void)
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    unsigned long sec = t.tv_sec * 1000;
    unsigned long msec = (t.tv_nsec + 500000) / 1000000;
    return sec + msec;
}

/// @brief Use a select to sleep a given number of millis.
void sleep_ms(long ms)
{
    struct timeval i;
    i.tv_sec = ms / 1000;
    i.tv_usec = (ms%1000) * 1000;
    select(0, NULL, NULL, NULL, &i);
}

/// @brief Initialize a msTimer.
void init_msTimer(struct mstimer *t, int ms_interval)
{
    t->step = 1;
    t->interval = ms_interval;
    t->startms = now_ms();
    t->nextms = t->startms + t->step * t->interval;
}

/// @brief Initialize a msTimer.
void init_sync_msTimer(struct mstimer *t, int ms_interval, unsigned long start)
{
    t->step = 1;
    t->interval = ms_interval;
    t->startms = start;
    t->nextms = t->startms + t->step * t->interval;
}

/// @brief Sleep until timer time has elapsed.
int timer_sleep(struct mstimer *t)
{
    unsigned long now = now_ms();
    if (now >= t->nextms)
    {
        int cadd = 0;
        while (t->nextms <= now)
        {
            cadd++;
            t->step++;
            t->nextms = t->startms + t->step * t->interval;
        }
        /* We slept this many intervals. */
        return cadd;
    }
    sleep_ms(t->nextms - now);
    t->step++;
    t->nextms = t->startms + t->step * t->interval;
    return 0;
}
