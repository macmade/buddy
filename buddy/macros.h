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

#ifndef __BUDDY_MACROS_H__
#define __BUDDY_MACROS_H__
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "warnings.h"

#define __XEOS_MEM_ZONE_BUDDY_MAX_ORDER     6

#define MEM         ( 1024 * 1024 * 1024 )

#define Z1_START    ( 0x00000000 )
#define Z2_START    ( 0x0009FC00 )
#define Z3_START    ( 0x000F0000 )
#define Z4_START    ( 0x00100000 )
#define Z5_START    ( 0x3FFF0000 )

#define Z1_BYTES    ( 654336 )
#define Z2_BYTES    ( 1024 )
#define Z3_BYTES    ( 65536 )
#define Z4_BYTES    ( ( 65536 * 10 ) + 0x1000 ) /* ( 1072627712 ) */
#define Z5_BYTES    ( 65536 )

#ifdef __cplusplus
}
#endif

#endif /* __BUDDY_MACROS_H__ */
