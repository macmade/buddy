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
    
    /* Checks if the number of pages may be allocated from a low order */
    if( n <= ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) )
    {
        /* Yes - Rounds the number of requested pages to the next power of 2 */
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
        /*
         * No, rounds the number of pages so it fits the block size of the highest order
         */
        x  = n / ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
        x += ( ( n % ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) ) ) > 0 ) ? 1 : 0;
        n  = x * ( 1 << ( __XEOS_MEM_ZONE_BUDDY_MAX_ORDER - 1 ) );
    }
    
    printf( " - %i pages will be allocated\n", n );
    
    order = 0;
    x     = n;
    
    /* Finds the starting order in which to allocate the pages */
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
    
    /* Process each memory zone to find a suitable one for the allocation */
    while( zone != NULL )
    {
        printf( "Processing memory zone %u: ", zoneIndex + 1 );
        
        /* Only process usable memory zones */
        if( zone->type != XEOS_Mem_ZoneTypeUsable )
        {
            printf( "unusable memory zone\n" );
            
            zoneIndex++;
            
            zone = zone->next;
            
            continue;
        }
        
        printf( "usable memory zone\n" );
        
        /* Gets the zone base address */
        address = zone->base;
        
        /* Gets the buddy for the requested order */
        buddy   = &( zone->buddies[ order ] );
        
        /* Checks if the order has free blocks */
        if( buddy->nFreeBlocks > 0 )
        {
            printf( "    *** Buddy %i has free blocks (%llu/%llu)\n", order, buddy->nFreeBlocks, buddy->nBlocks );
            
            /* Pointer to the blocks data */
            blocks = buddy->blocks;
            
            /* Process each block of the order */
            for( i = 0; i < buddy->nBlocks; i++ )
            {
                /*
                 * Checks if all blocks in the current group are used
                 * (block infos are stored as 32 bits integers)
                 */
                if( *( blocks ) == 0 )
                {
                    printf( "    *** Skipping used blocks #%llu -> #%llu (0x%llX -> 0x%llX)\n", i, i + ( 31 - ( i % 32 ) ), address - ( uint64_t )XEOS_Mem, ( address + 0x1000 * ( 31 - ( i % 32 ) ) ) - ( uint64_t )XEOS_Mem );
                    
                    /* Yes - We can process the next block group */
                    blocks++;
                    
                    /* Adjust current address, so used blocks are skipped */
                    address += 0x1000 * ( 32 - ( i % 32 ) );
                    
                    /* Adjusts the loop counter, so used blocks are skipped */
                    i += 31 - ( i % 32 );
                    
                    continue;
                }
                
                printf( "    *** Processing block #%llu: ", i );
                
                /* Bit position for the atomic operation */
                set = ( int32_t )( ( i / 8 ) * 8 ) + ( int32_t )( 7 - ( i % 8 ) ); /* ( 8 * ( 1 + ( i % 32 ) / 8 ) ) - ( 1 + ( i % 32 ) % 8 ) */
                
                /* Number of blocks to allocate in the order */
                nBlocks = ( n * 0x1000 ) / buddy->blockSize;
                
                /* Tries to mark the first block as used */
                if( System_Atomic_TestAndClear( ( uint32_t )set, blocks ) == true )
                {
                    /* Success, decrements the number of free blocks */
                    System_Atomic_Decrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
                    printf( "Free - Allocation successfull at 0x%llX\n", address - ( uint64_t )XEOS_Mem );
                    
                    /* One block has successfully been allocated */
                    nBlocks--;
                    
                    /* Continues trying to allocated blocks till we allocated enough */
                    while( nBlocks-- > 0 )
                    {
                        #pragma mark -
                        #pragma mark !!! To-Do: try to allocate remaining blocks
                        #pragma mark -
                    }
                    
                    Buddy_Debug_PrintBuddies( zoneIndex );
                    
                    /* Return the allocated memory address */
                    return address;
                }
                
                printf( "Used - Cannot allocate (0x%llX)\n", address - ( uint64_t )XEOS_Mem );
                
                /* Adjusts the address to the next block */
                address += buddy->blockSize;
                
                /*
                 * Block infos are stored as 32 bits integers - Process
                 * next block group when applicable
                 */
                if( i > 0 && ( i % 31 ) == 0 )
                {
                    blocks++;
                }
            }
        }
        
        printf( "    *** No free block in buddy %i - Split required\n", order );
        
        /*
         * Since we're here, it means we failed to allocate memory from
         * the requested order. So we'll try to split a block from a bigger
         * order to fulfill the allocation.
         */
        
        split = false;
        
        /* Process each bigger orders */
        for( i = order + 1; i < __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; i++ )
        {
            /* Gets the buddy for the requested order */
            buddy = &( zone->buddies[ i ] );
            
            /* Checks if the order has free blocks */
            if( buddy->nFreeBlocks == 0 )
            {
                printf( "    *** Buddy %llu has no free blocks\n", i );
                
                continue;
            }
            
            printf( "    *** Buddy %llu has free blocks (%llu/%llu)\n", i, buddy->nFreeBlocks, buddy->nBlocks );
            
            /* Pointer to the blocks data */
            blocks = buddy->blocks;
            
            /* Process each block of the current order */
            for( j = 0; j < buddy->nBlocks; j++ )
            {
                /*
                 * Checks if all blocks in the current group are used
                 * (block infos are stored as 32 bits integers)
                 */
                if( *( blocks ) == 0 )
                {
                    printf( "    *** Skipping used blocks #%llu -> #%llu\n", j, j + ( 31 - ( j % 32 ) ) );
                    
                    /* Yes - We can process the next block group */
                    blocks++;
                    
                    /* Adjusts the loop counter, so used blocks are skipped */
                    j += 31 - ( j % 32 );
                    
                    continue;
                }
                
                printf( "    *** Processing block #%llu: ", i );
                
                /* Bit position for the atomic operation */
                set = ( int32_t )( ( j / 8 ) * 8 ) + ( int32_t )( 7 - ( j % 8 ) ); /* ( 8 * ( 1 + ( j % 32 ) / 8 ) ) - ( 1 + ( j % 32 ) % 8 ) */
                
                /* Tries to mark the block as used */
                if( System_Atomic_TestAndClear( ( uint32_t )set, blocks ) == true )
                {
                    /* Success, decrements the number of free blocks */
                    System_Atomic_Decrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
                    printf( "Free\n" );
                    
                    /*
                     * We can now split the block we just marked as used so blocks
                     * from lower orders can be marked as free
                     */
                    split = true;
                    
                    break;
                }
                
                printf( "Used - Cannot split\n" );
                
                /*
                 * Block infos are stored as 32 bits integers - Process
                 * next block group when applicable
                 */
                if( j > 0 && ( j % 31 ) == 0 )
                {
                    blocks++;
                }
            }
            
            /* Checks if we've successfully marked a block as used */
            if( split == true )
            {
                /* Process each lower orders till the one requested originally */
                for( k = 0; k < i - order; k++ )
                {
                    printf( "    *** Splitting block #%llu of buddy %llu\n", j, i - k );
                    
                    /* Block position in the lower order */
                    j *= 2;
                    
                    /* Gets the buddy for the lower order */
                    buddy   = &( zone->buddies[ i - k - 1 ] );
                    
                    /* Pointer to the blocks data */
                    blocks  = buddy->blocks;
                    
                    /*
                     * Adjusts the data pointer so it points to the block
                     * to mark as free
                     */
                    blocks += j / 32;
                    
                    /* Bit position for the atomic operation */
                    set = ( int32_t )( ( j / 8 ) * 8 ) + ( int32_t )( 7 - ( j % 8 ) ); /* ( 8 * ( 1 + ( j % 32 ) / 8 ) ) - ( 1 + ( j % 32 ) % 8 ) */
                    
                    /* Sets the block as free */
                    System_Atomic_TestAndSet( ( uint32_t )set, blocks );
                    System_Atomic_Increment64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                }
                
                printf( "    *** Allocating block #%llu of buddy %u\n", j, order );
                printf( "    *** Block #%llu of buddy %u is now free\n", j + 1, order );
                
                Buddy_Debug_PrintBuddies( zoneIndex );
                
                return zone->base + ( j * 0x1000 );
            }
        }
        
        /* Process the next zone */
        zone = zone->next;
        
        zoneIndex++;
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
