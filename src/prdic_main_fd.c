/*
 * Copyright (c) 2019 Sippy Software, Inc., http://www.sippysoft.com
 * Copyright (c) 2016-2018, Maksym Sobolyev <sobomax@sippysoft.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/time.h>
#include <assert.h>
#include <math.h>
//#define PRD_DEBUG 1
#if defined(PRD_DEBUG)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "elperiodic.h"
#include "prdic_math.h"
#include "prdic_timespecops.h"
#include "prdic_fd.h"
#include "prdic_pfd.h"
#include "prdic_main_fd.h"
#include "prdic_band.h"
#include "prdic_time.h"

int
_prdic_procrastinate_FD(struct prdic_band *pip_ab)
{
    struct timespec tsleep, tremain;
    int rval;
    double eval, teval;
    struct timespec eptime;
#if defined(PRD_DEBUG)
    static long long nrun = -1;

    nrun += 1;
#endif

    if (pip_ab->add_delay_fltrd.lastval <= 0) {
         goto skipdelay;
    }
    dtime2timespec(pip_ab->add_delay_fltrd.lastval * pip_ab->period, &tremain);

#if defined(PRD_DEBUG)
    fprintf(stderr, "nrun=%lld add_delay=%f add_delay_fltrd=%f lastval=%f\n",
      nrun, pip_ab->add_delay, pip_ab->add_delay_fltrd.lastval,
      pip_ab->loop_error.lastval);
    fflush(stderr);
#endif
    do {
        tsleep = tremain;
        memset(&tremain, '\0', sizeof(tremain));
        rval = nanosleep(&tsleep, &tremain);
    } while (rval < 0 && !timespeciszero(&tremain));

skipdelay:
    getttime(&eptime, 1);

    timespecsub(&eptime, &pip_ab->epoch);
    timespecmul(&pip_ab->last_tclk, &eptime, &pip_ab->tfreq_hz);

    eval = _prdic_FD_get_error(&pip_ab->detector.freq, &pip_ab->last_tclk);
    eval = pip_ab->loop_error.lastval + erf(eval - pip_ab->loop_error.lastval);
    _prdic_recfilter_apply(&pip_ab->loop_error, eval);
    pip_ab->add_delay = pip_ab->add_delay_fltrd.lastval + eval;
    _prdic_recfilter_apply(&pip_ab->add_delay_fltrd, pip_ab->add_delay);
    if (pip_ab->add_delay_fltrd.lastval < 0.0) {
        pip_ab->add_delay_fltrd.lastval = 0;
    } else if (pip_ab->add_delay_fltrd.lastval > 1.0) {
        pip_ab->add_delay_fltrd.lastval = 1.0;
    }
    if (pip_ab->add_delay_fltrd.lastval > 0) {
        teval = 1.0 - pip_ab->add_delay_fltrd.lastval;
    } else {
        teval = 1.0 - pip_ab->loop_error.lastval;
    }
    _prdic_recfilter_apply(&pip_ab->sysload_fltrd, teval);

#if defined(PRD_DEBUG)
    fprintf(stderr, "run=%lld raw_error=%f filtered_error=%f teval=%f filtered_teval=%f\n", nrun,
      eval, pip_ab->loop_error.lastval, teval, pip_ab->sysload_fltrd.lastval);
    fflush(stderr);
#endif

#if defined(PRD_DEBUG)
    fprintf(stderr, "error=%f\n", eval);
    if (eval == 0.0 || 1) {
        fprintf(stderr, "last=%lld target=%lld\n", SEC(&pip_ab->last_tclk),
          SEC(&pip_ab->detector.freq.last_tclk));
    }
    fflush(stderr);
#endif

    return (0);
}
