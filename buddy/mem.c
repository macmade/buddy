/*******************************************************************************
 * XEOS - X86 Experimental Operating System
 * 
 * Copyright (c) 2010-2012, Jean-David Gadina <macmade@eosgarden.com>
 * All rights reserved.
 * 
 * XEOS Software License - Version 1.0 - December 21, 2012
 * 
 * Permission is hereby granted, free of charge, to any person or organisation
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to deal in the Software, with or without
 * modification, without restriction, including without limitation the rights
 * to use, execute, display, copy, reproduce, transmit, publish, distribute,
 * modify, merge, prepare derivative works of the Software, and to permit
 * third-parties to whom the Software is furnished to do so, all subject to the
 * following conditions:
 * 
 *      1.  Redistributions of source code, in whole or in part, must retain the
 *          above copyright notice and this entire statement, including the
 *          above license grant, this restriction and the following disclaimer.
 * 
 *      2.  Redistributions in binary form must reproduce the above copyright
 *          notice and this entire statement, including the above license grant,
 *          this restriction and the following disclaimer in the documentation
 *          and/or other materials provided with the distribution, unless the
 *          Software is distributed by the copyright owner as a library.
 *          A "library" means a collection of software functions and/or data
 *          prepared so as to be conveniently linked with application programs
 *          (which use some of those functions and data) to form executables.
 * 
 *      3.  The Software, or any substancial portion of the Software shall not
 *          be combined, included, derived, or linked (statically or
 *          dynamically) with software or libraries licensed under the terms
 *          of any GNU software license, including, but not limited to, the GNU
 *          General Public License (GNU/GPL) or the GNU Lesser General Public
 *          License (GNU/LGPL).
 * 
 *      4.  All advertising materials mentioning features or use of this
 *          software must display an acknowledgement stating that the product
 *          includes software developed by the copyright owner.
 * 
 *      5.  Neither the name of the copyright owner nor the names of its
 *          contributors may be used to endorse or promote products derived from
 *          this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT OWNER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, TITLE AND NON-INFRINGEMENT ARE DISCLAIMED.
 * 
 * IN NO EVENT SHALL THE COPYRIGHT OWNER, CONTRIBUTORS OR ANYONE DISTRIBUTING
 * THE SOFTWARE BE LIABLE FOR ANY CLAIM, DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN ACTION OF CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF OR IN CONNECTION WITH
 * THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

/* $Id$ */

#include "mem.h"
#include "types.h"
#include "macros.h"
#include "globals.h"
#include "lib.h"
#include "debug.h"
#include <stdio.h>

uint64_t XEOS_Mem_AllocPage( void )
{
    return XEOS_Mem_AllocPages( 1 );
}

uint64_t XEOS_Mem_AllocPages( unsigned int n )
{
    XEOS_Mem_ZoneRef      zone;
    XEOS_Mem_ZoneBuddyRef buddy;
    uint64_t              i;
    uint64_t              j;
    uint64_t              k;
    uint64_t              nBlocks;
    volatile uint32_t *   blocks;
    int32_t               set;
    uint64_t              address;
    bool                  split;
    unsigned int          zoneIndex;
    unsigned int          order;
    unsigned int          x;
    
    if( n == 0 )
    {
        return 0;
    }
    
    printf( "Allocating pages: %i pages requested", n );
    
    if( n <= ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) )
    {
        /* Round number of requested pages to the next power of 2 */
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
    }
    else
    {
        x  = n / ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
        x += ( ( n % ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) ) > 0 ) ? 1 : 0;
        n  = x * ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
    }
    
    printf( " - %i pages will be allocated\n", n );
    
    order = 0;
    x     = n;
    
    while( x >>= 1 )
    {
        if( order >= __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 )
        {
            order = __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1;
            
            break;
        }
        
        order++;
    }
    
    printf( "Buddy order: %i\n", order );
    
    zoneIndex = 0;
    zone      = XEOS_Mem_GetZoneAtIndex( zoneIndex );
    
    while( zone != NULL )
    {
        printf( "Processing memory zone %u: ", zoneIndex + 1 );
        
        if( zone->type != XEOS_Mem_ZoneTypeUsable )
        {
            printf( "unusable memory zone\n" );
            
            zoneIndex++;
            
            zone = zone->next;
            
            continue;
        }
        
        printf( "usable memory zone\n" );
        
        address = zone->base;
        buddy   = &( zone->buddies[ order ] );
        
        if( buddy->nFreeBlocks > 0 )
        {
            printf( "    *** Buddy %i has free blocks (%llu/%llu)\n", order, buddy->nFreeBlocks, buddy->nBlocks );
            
            blocks = buddy->blocks;
            
            for( i = 0; i < buddy->nBlocks; i++ )
            {
                if( *( blocks ) == 0 )
                {
                    blocks++;
                    
                    printf( "    *** Skipping used blocks #%llu -> #%llu (0x%llX -> 0x%llX)\n", i, i + ( 31 - ( i % 32 ) ), address - ( uint64_t )XEOS_Mem, ( address + 0x1000 * ( 31 - ( i % 32 ) ) ) - ( uint64_t )XEOS_Mem );
                    
                    address += 0x1000 * ( 32 - ( i % 32 ) );
                    i       += 31 - ( i % 32 );
                    
                    continue;
                }
                
                printf( "    *** Processing block #%llu: ", i );
                
                set     = ( 8 * ( 1 + ( i % 32 ) / 8 ) ) - ( 1 + ( i % 32 ) % 8 );
                nBlocks = ( n * 0x1000 ) / buddy->blockSize;
                
                if( System_Atomic_TestAndClear( ( uint32_t )set, blocks ) == true )
                {
                    System_Atomic_Decrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
                    printf( "Free - Allocation successfull at 0x%llX\n", address - ( uint64_t )XEOS_Mem );
                    
                    nBlocks--;
                    
                    while( nBlocks-- > 0 )
                    {}
                    
                    Buddy_Debug_PrintBuddies( zoneIndex );
                    
                    return address;
                }
                
                printf( "Used - Cannot allocate (0x%llX)\n", address - ( uint64_t )XEOS_Mem );
                
                address += 0x1000;
                
                if( i > 0 && ( i % 31 ) == 0 )
                {
                    blocks++;
                }
            }
        }
        
        printf( "    *** No free block in buddy %i - Split required\n", order );
        
        split = false;
        
        for( i = order + 1; i < __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; i++ )
        {
            buddy = &( zone->buddies[ i ] );
            
            if( buddy->nFreeBlocks == 0 )
            {
                printf( "    *** Buddy %llu has no free blocks\n", i );
                
                continue;
            }
            
            printf( "    *** Buddy %llu has free blocks (%llu/%llu)\n", i, buddy->nFreeBlocks, buddy->nBlocks );
            
            blocks = buddy->blocks;
            
            for( j = 0; j < buddy->nBlocks; j++ )
            {
                if( *( blocks ) == 0 )
                {
                    blocks++;
                    
                    printf( "    *** Skipping used blocks #%llu -> #%llu\n", j, j + ( 31 - ( j % 32 ) ) );
                    
                    j += 31 - ( j % 32 );
                    
                    continue;
                }
                
                printf( "    *** Processing block #%llu: ", i );
                
                set = ( 8 * ( 1 + ( j % 32 ) / 8 ) ) - ( 1 + ( j % 32 ) % 8 );
                
                if( System_Atomic_TestAndClear( ( uint32_t )set, blocks ) == true )
                {
                    System_Atomic_Decrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
                    printf( "Free\n" );
                    
                    split = true;
                    
                    break;
                }
                
                printf( "Used - Cannot split\n" );
                
                if( j > 0 && ( j % 31 ) == 0 )
                {
                    blocks++;
                }
            }
            
            if( split == true )
            {
                for( k = 0; k < i - order; k++ )
                {
                    printf( "    *** Splitting block #%llu of buddy %llu\n", j, i - k );
                    
                    j *= 2;
                    
                    buddy   = &( zone->buddies[ i - k - 1 ] );
                    blocks  = buddy->blocks;
                    blocks += j / 32;
                    
                    set = ( 8 * ( 1 + ( ( j % 32 ) + 1 ) / 8 ) ) - ( 1 + ( ( j % 32 ) + 1 ) % 8 );
                    
                    System_Atomic_TestAndSet( ( uint32_t )set, blocks );
                    System_Atomic_Increment64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                }
                
                printf( "    *** Allocating block #%llu of buddy %u\n", j, order );
                printf( "    *** Block #%llu of buddy %u is now free\n", j + 1, order );
                
                Buddy_Debug_PrintBuddies( zoneIndex );
                
                return zone->base + ( j * 0x1000 );
            }
        }
        
        zoneIndex++;
        
        zone = zone->next;
    }
    
    return 0;
}

void XEOS_Mem_FreePage( uint64_t address )
{
    XEOS_Mem_FreePages( address, 1 );
}

void XEOS_Mem_FreePages( uint64_t address, unsigned int n )
{
    XEOS_Mem_ZoneRef      zone;
    unsigned int          zoneIndex;
    unsigned int          order;
    unsigned int          x;
    
    ( void )address;
    
    if( n == 0 )
    {
        return;
    }
    
    printf( "Freeing %i pages: ", n );
    
    if( n <= ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) )
    {
        /* Round number of requested pages to the next power of 2 */
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
    }
    else
    {
        x  = n / ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
        x += ( ( n % ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) ) > 0 ) ? 1 : 0;
        n  = x * ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
    }
    
    printf( "%i pages to free\n", n );
    
    order = 0;
    x     = n;
    
    while( x >>= 1 )
    {
        if( order >= __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 )
        {
            order = __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1;
            
            break;
        }
        
        order++;
    }
    
    printf( "Buddy order: %i\n", order );
    
    zoneIndex = 0;
    zone      = XEOS_Mem_GetZoneAtIndex( zoneIndex );
    
    while( zone != NULL )
    {
        printf( "Processing memory zone %u: ", zoneIndex + 1 );
        printf( "\n" );
        
        zoneIndex++;
        
        zone = zone->next;
    }
}

XEOS_Mem_ZoneRef XEOS_Mem_GetZoneAtIndex( unsigned int index )
{
    switch( index )
    {
        case 0:     return XEOS_Mem_Z1;
        case 1:     return XEOS_Mem_Z2;
        case 2:     return XEOS_Mem_Z3;
        case 3:     return XEOS_Mem_Z4;
        case 4:     return XEOS_Mem_Z5;
        default:    return NULL;
    }
}
