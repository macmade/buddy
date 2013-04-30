/*******************************************************************************
 * Copyright (c) 2009, Jean-David Gadina - www.xs-labs.com
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *  -   Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *  -   Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *  -   Neither the name of 'Jean-David Gadina' nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/* $Id$ */

#ifndef __BUDDY_TYPES_H__
#define __BUDDY_TYPES_H__
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "warnings.h"
#include "macros.h"
#include <stdint.h>

typedef enum
{
    XEOS_Mem_ZoneTypeUnknown            = 0,
    XEOS_Mem_ZoneTypeUsable             = 1,
    XEOS_Mem_ZoneTypeReserved           = 2,
    XEOS_Mem_ZoneTypeACPIReclaimable    = 3,
    XEOS_Mem_ZoneTypeACPINVS            = 4,
    XEOS_Mem_ZoneTypeBad                = 5
}
XEOS_Mem_ZoneType;

struct __XEOS_Mem_ZoneBuddy
{
    uint64_t                      order;
    uint64_t                      blockSize;
    uint64_t                      nBlocks;
    volatile uint64_t             nFreeBlocks;
    volatile uint32_t           * blocks;
    struct __XEOS_Mem_ZoneBuddy * prev;
    struct __XEOS_Mem_ZoneBuddy * next;
};

typedef struct __XEOS_Mem_ZoneBuddy * XEOS_Mem_ZoneBuddyRef;

struct __XEOS_Mem_Zone
{
    XEOS_Mem_ZoneType           type;
    char                        __pad[ 4 ];
    uint64_t                    base;
    uint64_t                    length;
    struct __XEOS_Mem_Zone    * next;
    struct __XEOS_Mem_Zone    * prev;
    uint64_t                    nPages;
    struct __XEOS_Mem_ZoneBuddy buddies[ __XEOS_MEM_ZONE_BUDDY_MAX_ORDER ];
    volatile uint32_t         * buddyBlocks;
};

typedef struct __XEOS_Mem_Zone * XEOS_Mem_ZoneRef;

#ifdef __cplusplus
}
#endif

#endif /* __BUDDY_TYPES_H__ */
