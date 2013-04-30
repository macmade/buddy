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

#include "types.h"
#include "macros.h"
#include "globals.h"
#include "mem.h"
#include "debug.h"
#include "setup.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main( void )
{
    /*
     * Initial setup of a dummy physical memory layout:
     * 
     * Zone 0:  0x0000000000000000 -> 0x000000000009FBFF: Reserved - 654336 B
     * Zone 1:  0x000000000009FC00 -> 0x000000000009FFFF: Reserved - 1024 B
     * Zone 2:  0x00000000000F0000 -> 0x00000000000FFFFF: Reserved - 65536 B
     * Zone 3:  0x0000000000100000 -> 0x00000000001A0FFF: Usable   - 659456 B
     * Zone 4:  0x000000003FFF0000 -> 0x000000003FFFFFFF: ACPI     - 65536 B
     */
    Buddy_Setup_Memory();
    Buddy_Setup_Buddies();
    Buddy_Debug_PrintBuddies( 3 );
    
    /*
    {
        uint64_t p1;
        uint64_t p2;
        uint64_t p3;
        uint64_t p4;
        uint64_t p5;
        uint64_t p6;
        uint64_t p7;
        uint64_t p8;
        uint64_t p9;
        uint64_t p10;
        uint64_t p11;
        uint64_t p12;
        uint64_t p13;
        uint64_t p14;
        uint64_t p15;
        
        p1 = XEOS_Mem_AllocPage();
        p2 = XEOS_Mem_AllocPage();
        
        XEOS_Mem_FreePage( p1 );
        
        p3  = XEOS_Mem_AllocPage();
        p4  = XEOS_Mem_AllocPage();
        p5  = XEOS_Mem_AllocPage();
        p6  = XEOS_Mem_AllocPage();
        p7  = XEOS_Mem_AllocPage();
        p8  = XEOS_Mem_AllocPage();
        p9  = XEOS_Mem_AllocPage();
        p10 = XEOS_Mem_AllocPage();
        
        p11 = XEOS_Mem_AllocPages( 2 );
        p12 = XEOS_Mem_AllocPages( 2 );
        p13 = XEOS_Mem_AllocPages( 12 );
        p14 = XEOS_Mem_AllocPages( 12 );
        p15 = XEOS_Mem_AllocPages( 12 );
        
        printf( "P1:  0x%llX\n", ( p1  == 0  ) ? 0 : p1  - ( uint64_t )XEOS_Mem );
        printf( "P2:  0x%llX\n", ( p2  == 0  ) ? 0 : p2  - ( uint64_t )XEOS_Mem );
        printf( "P3:  0x%llX\n", ( p3  == 0  ) ? 0 : p3  - ( uint64_t )XEOS_Mem );
        printf( "P4:  0x%llX\n", ( p4  == 0  ) ? 0 : p4  - ( uint64_t )XEOS_Mem );
        printf( "P5:  0x%llX\n", ( p5  == 0  ) ? 0 : p5  - ( uint64_t )XEOS_Mem );
        printf( "P6:  0x%llX\n", ( p6  == 0  ) ? 0 : p6  - ( uint64_t )XEOS_Mem );
        printf( "P7:  0x%llX\n", ( p7  == 0  ) ? 0 : p7  - ( uint64_t )XEOS_Mem );
        printf( "P8:  0x%llX\n", ( p8  == 0  ) ? 0 : p8  - ( uint64_t )XEOS_Mem );
        printf( "P9:  0x%llX\n", ( p9  == 0  ) ? 0 : p9  - ( uint64_t )XEOS_Mem );
        printf( "P10: 0x%llX\n", ( p10 == 0  ) ? 0 : p10 - ( uint64_t )XEOS_Mem );
        
        printf( "P11: 0x%llX\n", ( p11  == 0 ) ? 0 : p11 - ( uint64_t )XEOS_Mem );
        printf( "P12: 0x%llX\n", ( p12  == 0 ) ? 0 : p12 - ( uint64_t )XEOS_Mem );
        printf( "P13: 0x%llX\n", ( p13  == 0 ) ? 0 : p13 - ( uint64_t )XEOS_Mem );
        printf( "P14: 0x%llX\n", ( p14  == 0 ) ? 0 : p14 - ( uint64_t )XEOS_Mem );
        printf( "P15: 0x%llX\n", ( p15  == 0 ) ? 0 : p15 - ( uint64_t )XEOS_Mem );
        
        XEOS_Mem_FreePage( p2 );
        XEOS_Mem_FreePage( p3 );
        XEOS_Mem_FreePage( p4 );
        XEOS_Mem_FreePage( p5 );
        XEOS_Mem_FreePage( p6 );
        XEOS_Mem_FreePage( p7 );
        XEOS_Mem_FreePage( p8 );
        XEOS_Mem_FreePage( p9 );
        XEOS_Mem_FreePage( p10 );
        
        XEOS_Mem_FreePages( p11, 2 );
        XEOS_Mem_FreePages( p12, 2 );
        XEOS_Mem_FreePages( p13, 12 );
        XEOS_Mem_FreePages( p14, 12 );
        XEOS_Mem_FreePages( p15, 12 );
        
        printf( "--------------------------------------------------------------------------------\n" );
    }
    */
    
    XEOS_Mem_AllocPages( 32 );
    XEOS_Mem_AllocPages( 32 );
    
    return 0;
}
