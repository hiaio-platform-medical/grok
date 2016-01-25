/*
*    Copyright (C) 2016 Grok Image Compression Inc.
*
*    This source code is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This source code is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*
*    This source code incorporates work covered by the following copyright and
*    permission notice:
*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2008, Jerome Fimes, Communications & Systemes <jerome.fimes@c-s.fr>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "opj_includes.h"

/** @defgroup MQC MQC - Implementation of an MQ-Coder */
/*@{*/

/** @name Local static functions */
/*@{*/

/**
Output a byte, doing bit-stuffing if necessary.
After a 0xff byte, the next byte must be smaller than 0x90.
@param mqc MQC handle
*/
static void opj_mqc_byteout(opj_mqc_t *mqc);
/**
Renormalize mqc->a and mqc->c while encoding, so that mqc->a stays between 0x8000 and 0x10000
@param mqc MQC handle
*/
static void opj_mqc_renorme(opj_mqc_t *mqc);
/**
Encode the most probable symbol
@param mqc MQC handle
*/
static void opj_mqc_codemps(opj_mqc_t *mqc);
/**
Encode the most least symbol
@param mqc MQC handle
*/
static void opj_mqc_codelps(opj_mqc_t *mqc);
/**
Fill mqc->c with 1's for flushing
@param mqc MQC handle
*/
static void opj_mqc_setbits(opj_mqc_t *mqc);
/**
FIXME DOC
@param mqc MQC handle
@return
*/
static inline int32_t opj_mqc_mpsexchange(opj_mqc_t *const mqc);
/**
FIXME DOC
@param mqc MQC handle
@return
*/
static inline int32_t opj_mqc_lpsexchange(opj_mqc_t *const mqc);
/**
Input a byte
@param mqc MQC handle
*/
static inline void opj_mqc_bytein(opj_mqc_t *const mqc);
/**
Renormalize mqc->a and mqc->c while decoding
@param mqc MQC handle
*/
static inline void opj_mqc_renormd(opj_mqc_t *const mqc);
/*@}*/

/*@}*/

/* <summary> */
/* This array defines all the possible states for a context. */
/* </summary> */
static opj_mqc_state_t mqc_states[47 * 2] = {
    {0x5601, 0, &mqc_states[2], &mqc_states[3]},
    {0x5601, 1, &mqc_states[3], &mqc_states[2]},
    {0x3401, 0, &mqc_states[4], &mqc_states[12]},
    {0x3401, 1, &mqc_states[5], &mqc_states[13]},
    {0x1801, 0, &mqc_states[6], &mqc_states[18]},
    {0x1801, 1, &mqc_states[7], &mqc_states[19]},
    {0x0ac1, 0, &mqc_states[8], &mqc_states[24]},
    {0x0ac1, 1, &mqc_states[9], &mqc_states[25]},
    {0x0521, 0, &mqc_states[10], &mqc_states[58]},
    {0x0521, 1, &mqc_states[11], &mqc_states[59]},
    {0x0221, 0, &mqc_states[76], &mqc_states[66]},
    {0x0221, 1, &mqc_states[77], &mqc_states[67]},
    {0x5601, 0, &mqc_states[14], &mqc_states[13]},
    {0x5601, 1, &mqc_states[15], &mqc_states[12]},
    {0x5401, 0, &mqc_states[16], &mqc_states[28]},
    {0x5401, 1, &mqc_states[17], &mqc_states[29]},
    {0x4801, 0, &mqc_states[18], &mqc_states[28]},
    {0x4801, 1, &mqc_states[19], &mqc_states[29]},
    {0x3801, 0, &mqc_states[20], &mqc_states[28]},
    {0x3801, 1, &mqc_states[21], &mqc_states[29]},
    {0x3001, 0, &mqc_states[22], &mqc_states[34]},
    {0x3001, 1, &mqc_states[23], &mqc_states[35]},
    {0x2401, 0, &mqc_states[24], &mqc_states[36]},
    {0x2401, 1, &mqc_states[25], &mqc_states[37]},
    {0x1c01, 0, &mqc_states[26], &mqc_states[40]},
    {0x1c01, 1, &mqc_states[27], &mqc_states[41]},
    {0x1601, 0, &mqc_states[58], &mqc_states[42]},
    {0x1601, 1, &mqc_states[59], &mqc_states[43]},
    {0x5601, 0, &mqc_states[30], &mqc_states[29]},
    {0x5601, 1, &mqc_states[31], &mqc_states[28]},
    {0x5401, 0, &mqc_states[32], &mqc_states[28]},
    {0x5401, 1, &mqc_states[33], &mqc_states[29]},
    {0x5101, 0, &mqc_states[34], &mqc_states[30]},
    {0x5101, 1, &mqc_states[35], &mqc_states[31]},
    {0x4801, 0, &mqc_states[36], &mqc_states[32]},
    {0x4801, 1, &mqc_states[37], &mqc_states[33]},
    {0x3801, 0, &mqc_states[38], &mqc_states[34]},
    {0x3801, 1, &mqc_states[39], &mqc_states[35]},
    {0x3401, 0, &mqc_states[40], &mqc_states[36]},
    {0x3401, 1, &mqc_states[41], &mqc_states[37]},
    {0x3001, 0, &mqc_states[42], &mqc_states[38]},
    {0x3001, 1, &mqc_states[43], &mqc_states[39]},
    {0x2801, 0, &mqc_states[44], &mqc_states[38]},
    {0x2801, 1, &mqc_states[45], &mqc_states[39]},
    {0x2401, 0, &mqc_states[46], &mqc_states[40]},
    {0x2401, 1, &mqc_states[47], &mqc_states[41]},
    {0x2201, 0, &mqc_states[48], &mqc_states[42]},
    {0x2201, 1, &mqc_states[49], &mqc_states[43]},
    {0x1c01, 0, &mqc_states[50], &mqc_states[44]},
    {0x1c01, 1, &mqc_states[51], &mqc_states[45]},
    {0x1801, 0, &mqc_states[52], &mqc_states[46]},
    {0x1801, 1, &mqc_states[53], &mqc_states[47]},
    {0x1601, 0, &mqc_states[54], &mqc_states[48]},
    {0x1601, 1, &mqc_states[55], &mqc_states[49]},
    {0x1401, 0, &mqc_states[56], &mqc_states[50]},
    {0x1401, 1, &mqc_states[57], &mqc_states[51]},
    {0x1201, 0, &mqc_states[58], &mqc_states[52]},
    {0x1201, 1, &mqc_states[59], &mqc_states[53]},
    {0x1101, 0, &mqc_states[60], &mqc_states[54]},
    {0x1101, 1, &mqc_states[61], &mqc_states[55]},
    {0x0ac1, 0, &mqc_states[62], &mqc_states[56]},
    {0x0ac1, 1, &mqc_states[63], &mqc_states[57]},
    {0x09c1, 0, &mqc_states[64], &mqc_states[58]},
    {0x09c1, 1, &mqc_states[65], &mqc_states[59]},
    {0x08a1, 0, &mqc_states[66], &mqc_states[60]},
    {0x08a1, 1, &mqc_states[67], &mqc_states[61]},
    {0x0521, 0, &mqc_states[68], &mqc_states[62]},
    {0x0521, 1, &mqc_states[69], &mqc_states[63]},
    {0x0441, 0, &mqc_states[70], &mqc_states[64]},
    {0x0441, 1, &mqc_states[71], &mqc_states[65]},
    {0x02a1, 0, &mqc_states[72], &mqc_states[66]},
    {0x02a1, 1, &mqc_states[73], &mqc_states[67]},
    {0x0221, 0, &mqc_states[74], &mqc_states[68]},
    {0x0221, 1, &mqc_states[75], &mqc_states[69]},
    {0x0141, 0, &mqc_states[76], &mqc_states[70]},
    {0x0141, 1, &mqc_states[77], &mqc_states[71]},
    {0x0111, 0, &mqc_states[78], &mqc_states[72]},
    {0x0111, 1, &mqc_states[79], &mqc_states[73]},
    {0x0085, 0, &mqc_states[80], &mqc_states[74]},
    {0x0085, 1, &mqc_states[81], &mqc_states[75]},
    {0x0049, 0, &mqc_states[82], &mqc_states[76]},
    {0x0049, 1, &mqc_states[83], &mqc_states[77]},
    {0x0025, 0, &mqc_states[84], &mqc_states[78]},
    {0x0025, 1, &mqc_states[85], &mqc_states[79]},
    {0x0015, 0, &mqc_states[86], &mqc_states[80]},
    {0x0015, 1, &mqc_states[87], &mqc_states[81]},
    {0x0009, 0, &mqc_states[88], &mqc_states[82]},
    {0x0009, 1, &mqc_states[89], &mqc_states[83]},
    {0x0005, 0, &mqc_states[90], &mqc_states[84]},
    {0x0005, 1, &mqc_states[91], &mqc_states[85]},
    {0x0001, 0, &mqc_states[90], &mqc_states[86]},
    {0x0001, 1, &mqc_states[91], &mqc_states[87]},
    {0x5601, 0, &mqc_states[92], &mqc_states[92]},
    {0x5601, 1, &mqc_states[93], &mqc_states[93]},
};

/*
==========================================================
   local functions
==========================================================
*/

static void opj_mqc_byteout(opj_mqc_t *mqc)
{
    if (mqc->bp == mqc->start - 1) {
        mqc->bp++;
        *mqc->bp = (uint8_t)(mqc->c >> 19);
        mqc->c &= 0x7ffff;
        mqc->ct = 8;
    } else if (*mqc->bp == 0xff) {
        mqc->bp++;
        *mqc->bp = (uint8_t)(mqc->c >> 20);
        mqc->c &= 0xfffff;
        mqc->ct = 7;
    } else {
        if ((mqc->c & 0x8000000) == 0) {
            mqc->bp++;
            *mqc->bp = (uint8_t)(mqc->c >> 19);
            mqc->c &= 0x7ffff;
            mqc->ct = 8;
        } else {
            (*mqc->bp)++;
            if (*mqc->bp == 0xff) {
                mqc->c &= 0x7ffffff;
                mqc->bp++;
                *mqc->bp = (uint8_t)(mqc->c >> 20);
                mqc->c &= 0xfffff;
                mqc->ct = 7;
            } else {
                mqc->bp++;
                *mqc->bp = (uint8_t)(mqc->c >> 19);
                mqc->c &= 0x7ffff;
                mqc->ct = 8;
            }
        }
    }
}

static void opj_mqc_renorme(opj_mqc_t *mqc)
{
    do {
        mqc->a <<= 1;
        mqc->c <<= 1;
        mqc->ct--;
        if (mqc->ct == 0) {
            opj_mqc_byteout(mqc);
        }
    } while ((mqc->a & 0x8000) == 0);
}

static void opj_mqc_codemps(opj_mqc_t *mqc)
{
    mqc->a -= (*mqc->curctx)->qeval;
    if ((mqc->a & 0x8000) == 0) {
        if (mqc->a < (*mqc->curctx)->qeval) {
            mqc->a = (*mqc->curctx)->qeval;
        } else {
            mqc->c += (*mqc->curctx)->qeval;
        }
        *mqc->curctx = (*mqc->curctx)->nmps;
        opj_mqc_renorme(mqc);
    } else {
        mqc->c += (*mqc->curctx)->qeval;
    }
}

static void opj_mqc_codelps(opj_mqc_t *mqc)
{
    mqc->a -= (*mqc->curctx)->qeval;
    if (mqc->a < (*mqc->curctx)->qeval) {
        mqc->c += (*mqc->curctx)->qeval;
    } else {
        mqc->a = (*mqc->curctx)->qeval;
    }
    *mqc->curctx = (*mqc->curctx)->nlps;
    opj_mqc_renorme(mqc);
}

static void opj_mqc_setbits(opj_mqc_t *mqc)
{
    uint32_t tempc = mqc->c + mqc->a;
    mqc->c |= 0xffff;
    if (mqc->c >= tempc) {
        mqc->c -= 0x8000;
    }
}

static inline int32_t opj_mqc_mpsexchange(opj_mqc_t *const mqc)
{
    int32_t d;
    if (mqc->a < (*mqc->curctx)->qeval) {
        d = (int32_t)(1 - (*mqc->curctx)->mps);
        *mqc->curctx = (*mqc->curctx)->nlps;
    } else {
        d = (int32_t)(*mqc->curctx)->mps;
        *mqc->curctx = (*mqc->curctx)->nmps;
    }

    return d;
}

static inline int32_t opj_mqc_lpsexchange(opj_mqc_t *const mqc)
{
    int32_t d;
    if (mqc->a < (*mqc->curctx)->qeval) {
        mqc->a = (*mqc->curctx)->qeval;
        d = (int32_t)(*mqc->curctx)->mps;
        *mqc->curctx = (*mqc->curctx)->nmps;
    } else {
        mqc->a = (*mqc->curctx)->qeval;
        d = (int32_t)(1 - (*mqc->curctx)->mps);
        *mqc->curctx = (*mqc->curctx)->nlps;
    }

    return d;
}

static void opj_mqc_bytein(opj_mqc_t *const mqc)
{
    if (mqc->bp != mqc->end) {
        uint32_t c;
        if (mqc->bp + 1 != mqc->end) {
            c = *(mqc->bp + 1);
        } else {
            c = 0xff;
        }
        if (*mqc->bp == 0xff) {
            if (c > 0x8f) {
                mqc->c += 0xff00;
                mqc->ct = 8;
            } else {
                mqc->bp++;
                mqc->c += c << 9;
                mqc->ct = 7;
            }
        } else {
            mqc->bp++;
            mqc->c += c << 8;
            mqc->ct = 8;
        }
    } else {
        mqc->c += 0xff00;
        mqc->ct = 8;
    }
}


static inline void opj_mqc_renormd(opj_mqc_t *const mqc)
{
    do {
        if (mqc->ct == 0) {
            opj_mqc_bytein(mqc);
        }
        mqc->a <<= 1;
        mqc->c <<= 1;
        mqc->ct--;
    } while (mqc->a < 0x8000);
}

/*
==========================================================
   MQ-Coder interface
==========================================================
*/

opj_mqc_t* opj_mqc_create(void)
{
    opj_mqc_t *mqc = (opj_mqc_t*)opj_malloc(sizeof(opj_mqc_t));
    return mqc;
}

void opj_mqc_destroy(opj_mqc_t *mqc)
{
    if(mqc) {
        opj_free(mqc);
    }
}

uint32_t opj_mqc_numbytes(opj_mqc_t *mqc)
{
    const ptrdiff_t diff = mqc->bp - mqc->start;
    return (uint32_t)diff;
}

void opj_mqc_init_enc(opj_mqc_t *mqc, uint8_t *bp)
{
    /* TODO MSD: need to take a look to the v2 version */
    opj_mqc_setcurctx(mqc, 0);
    mqc->a = 0x8000;
    mqc->c = 0;
    mqc->bp = bp - 1;
    mqc->ct = 12;
    mqc->start = bp;
}

void opj_mqc_encode(opj_mqc_t *mqc, uint32_t d)
{
    if ((*mqc->curctx)->mps == d) {
        opj_mqc_codemps(mqc);
    } else {
        opj_mqc_codelps(mqc);
    }
}

void opj_mqc_flush(opj_mqc_t *mqc)
{
    opj_mqc_setbits(mqc);
    mqc->c <<= mqc->ct;
    opj_mqc_byteout(mqc);
    mqc->c <<= mqc->ct;
    opj_mqc_byteout(mqc);

    if (*mqc->bp != 0xff) {
        mqc->bp++;
    }
}

void opj_mqc_bypass_init_enc(opj_mqc_t *mqc)
{
    mqc->c = 0;
    mqc->ct = 8;
}

void opj_mqc_bypass_enc(opj_mqc_t *mqc, uint32_t d)
{
    mqc->ct--;
    mqc->c = mqc->c + (d << mqc->ct);
    if (mqc->ct == 0) {
        mqc->bp++;
        *mqc->bp = (uint8_t)mqc->c;
        mqc->ct = 8;
        if (*mqc->bp == 0xff) {
            mqc->ct = 7;
        }
        mqc->c = 0;
    }
}

uint32_t opj_mqc_bypass_flush_enc(opj_mqc_t *mqc)
{
    uint8_t bit_padding;

    bit_padding = 0;

    if (mqc->ct != 0) {
        while (mqc->ct > 0) {
            mqc->ct--;
            mqc->c += (uint32_t)(bit_padding << mqc->ct);
            bit_padding = (bit_padding + 1) & 0x01;
        }
        mqc->bp++;
        *mqc->bp = (uint8_t)mqc->c;
        mqc->ct = 8;
        mqc->c = 0;
    }

    return 1;
}

void opj_mqc_reset_enc(opj_mqc_t *mqc)
{
    opj_mqc_resetstates(mqc);
    opj_mqc_setstate(mqc, T1_CTXNO_UNI, 0, 46);
    opj_mqc_setstate(mqc, T1_CTXNO_AGG, 0, 3);
    opj_mqc_setstate(mqc, T1_CTXNO_ZC, 0, 4);
}

uint32_t opj_mqc_restart_enc(opj_mqc_t *mqc)
{
    uint32_t correction = 1;

    /* <flush part> */
    int32_t n = (int32_t)(27 - 15 - mqc->ct);
    mqc->c <<= mqc->ct;
    while (n > 0) {
        opj_mqc_byteout(mqc);
        n -= (int32_t)mqc->ct;
        mqc->c <<= mqc->ct;
    }
    opj_mqc_byteout(mqc);

    return correction;
}

void opj_mqc_restart_init_enc(opj_mqc_t *mqc)
{
    /* <Re-init part> */
    opj_mqc_setcurctx(mqc, 0);
    mqc->a = 0x8000;
    mqc->c = 0;
    mqc->ct = 12;
    mqc->bp--;
    if (*mqc->bp == 0xff) {
        mqc->ct = 13;
    }
}

void opj_mqc_erterm_enc(opj_mqc_t *mqc)
{
    int32_t k = (int32_t)(11 - mqc->ct + 1);

    while (k > 0) {
        mqc->c <<= mqc->ct;
        mqc->ct = 0;
        opj_mqc_byteout(mqc);
        k -= (int32_t)mqc->ct;
    }

    if (*mqc->bp != 0xff) {
        opj_mqc_byteout(mqc);
    }
}

void opj_mqc_segmark_enc(opj_mqc_t *mqc)
{
    uint32_t i;
    opj_mqc_setcurctx(mqc, 18);

    for (i = 1; i < 5; i++) {
        opj_mqc_encode(mqc, i % 2);
    }
}

bool opj_mqc_init_dec(opj_mqc_t *mqc, uint8_t *bp, uint32_t len)
{
    opj_mqc_setcurctx(mqc, 0);
    mqc->start = bp;
    mqc->end = bp + len;
    mqc->bp = bp;
    if (len==0)
        mqc->c = 0xff << 16;
    else
        mqc->c = (uint32_t)(*mqc->bp << 16);
    opj_mqc_bytein(mqc);
    mqc->c <<= 7;
    mqc->ct -= 7;
    mqc->a = 0x8000;
    return true;
}

int32_t opj_mqc_decode(opj_mqc_t *const mqc)
{
    int32_t d;
    mqc->a -= (*mqc->curctx)->qeval;
    if ((mqc->c >> 16) < (*mqc->curctx)->qeval) {
        d = opj_mqc_lpsexchange(mqc);
        opj_mqc_renormd(mqc);
    } else {
        mqc->c -= (*mqc->curctx)->qeval << 16;
        if ((mqc->a & 0x8000) == 0) {
            d = opj_mqc_mpsexchange(mqc);
            opj_mqc_renormd(mqc);
        } else {
            d = (int32_t)(*mqc->curctx)->mps;
        }
    }

    return d;
}

void opj_mqc_resetstates(opj_mqc_t *mqc)
{
    uint32_t i;
    for (i = 0; i < MQC_NUMCTXS; i++) {
        mqc->ctxs[i] = mqc_states;
    }
}

void opj_mqc_setstate(opj_mqc_t *mqc, uint32_t ctxno, uint32_t msb, int32_t prob)
{
    mqc->ctxs[ctxno] = &mqc_states[msb + (uint32_t)(prob << 1)];
}


