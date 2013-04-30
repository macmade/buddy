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

#include "debug.h"
#include "types.h"
#include "mem.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void Buddy_Debug_PrintBuddies( unsigned int zoneIndex )
{
    XEOS_Mem_ZoneRef      zone;
    XEOS_Mem_ZoneBuddyRef buddy;
    uint64_t              i;
    uint64_t              j;
    volatile uint32_t   * blocks;
    char                  s[ 512 ];
    
    zone = XEOS_Mem_GetZoneAtIndex( zoneIndex );
    
    if( zone == NULL )
    {
        return;
    }
    
    printf( "--------------------------------------------------------------------------------\n" );
    printf( "DEBUG - Buddies for memory zone %u:\n\n", zoneIndex + 1 );
    
    for( i = 0; i < __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; i++ )
    {
        buddy = &( zone->buddies[ i ] );
        
        printf( "Buddy %llu: ", i );
        
        blocks = buddy->blocks;
        
        memset( s, 0, sizeof( s ) );
        
        j  = 1 << i;
        j -= 1;
        
        while( j > 0 )
        {
            strcat( s, "-" );
            
            j--;
        }
        
        for( j = 0; j < buddy->nBlocks; j++ )
        {
            printf( "%c%s", ( ( *( blocks ) & ( 1 << ( j % 32 ) ) ) == 0 ) ? 'X' : 'O', s );
            
            if( j > 0 && ( j % 31 ) == 0 )
            {
                blocks++;
            }
        }
        
        printf( "\n" );
    }
    
    printf( "--------------------------------------------------------------------------------\n" );
}
