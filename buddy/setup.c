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

#include "setup.h"
#include "types.h"
#include "globals.h"
#include "macros.h"
#include "mem.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void Buddy_Setup_Memory( void )
{
    uint64_t          i;
    uint64_t          j;
    uint64_t          zonesMem;
    XEOS_Mem_ZoneRef  zone;
    XEOS_Mem_ZoneRef  previousZone;
    uint64_t          base;
    uint64_t          length;
    XEOS_Mem_ZoneType type;
    uint64_t          zoneMem;
    uint64_t          nPages;
    uint64_t          blockSize;
    char            * typeString;
    
    uint64_t z1Length;
    uint64_t z2Length;
    uint64_t z3Length;
    uint64_t z4Length;
    uint64_t z5Length;
    
    type     = XEOS_Mem_ZoneTypeUnknown;
    length   = 0;
    base     = 0;
    z1Length = 0;
    z2Length = 0;
    z3Length = 0;
    z4Length = 0;
    z5Length = 0;
    
	XEOS_Mem = malloc( MEM );
    
    printf( "--------------------------------------------------------------------------------\n" );
    printf( "XEOS:         Buddy physical memory allocator\n" );
	printf( "Total memory: %llu MB (%llu B) at %p\n", ( unsigned long long )( ( MEM / 1024 ) / 1024 ), ( unsigned long long )MEM, XEOS_Mem );
    printf( "--------------------------------------------------------------------------------\n" );
	
    zonesMem = 0;
    
    /***************************************************************************
     * ZONES MEMORY USAGE
     **************************************************************************/
    
    for( i = 0; i < 5; i++ )
    {
        if( i == 0 )
        {
            length  = Z1_BYTES;
        }
        else if( i == 1 )
        {
            length  = Z2_BYTES;
        }
        else if( i == 2 )
        {
            length  = Z3_BYTES;
        }
        else if( i == 3 )
        {
            length  = Z4_BYTES;
        }
        else if( i == 4 )
        {
            length  = Z5_BYTES;
        }
        
        nPages      = length / 0x1000;
        zoneMem     = 0;
        blockSize   = 0x1000;
        
        printf( "Memory zone %llu - ", i + 1 );
        printf( "Length:      %llu KB\n", length / 1024 );
        printf( "              - Pages:       %llu\n", nPages );
        
        for( j = 0; j < __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; j++ )
        {
            {
                uint64_t nBlocks;
                uint64_t buddyMem;
                
                /* Number of blocks for the current buddy */
                nBlocks   = length / blockSize;
                
                /* A byte can store infos for 8 blocks */
                buddyMem  = nBlocks / 8;
                buddyMem += ( ( nBlocks % 8 ) > 0 ) ? 1 : 0;
                
                /* Block infos are stored as 32 bits integers */
                buddyMem += ( ( buddyMem % 4 ) == 0 ) ? 0 : 4 - ( buddyMem % 4 );
                
                printf( "              - Buddy %llu:     %llu blocks of %llu KB\n", j, nBlocks, blockSize / 1024 );
                
                blockSize *= 2;
                zoneMem   += buddyMem;
            }
        }
        
        zoneMem  += sizeof( struct __XEOS_Mem_Zone );
        zonesMem += zoneMem;
        
        if( i == 0 )
        {
            z1Length = zoneMem;
        }
        else if( i == 1 )
        {
            z2Length = zoneMem;
        }
        else if( i == 2 )
        {
            z3Length = zoneMem;
        }
        else if( i == 3 )
        {
            z4Length = zoneMem;
        }
        else if( i == 4 )
        {
            z5Length = zoneMem;
        }
        
        printf( "              - Zone memory: %llu KB (%llu B)\n", zoneMem / 1024, zoneMem );
        printf( "--------------------------------------------------------------------------------\n" );
    }
    
    printf( "Zones memory: %llu KB (%llu B)\n", zonesMem / 1024, zonesMem );
    printf( "--------------------------------------------------------------------------------\n" );
    
    XEOS_Mem_Z1 = malloc( zonesMem );
    memset( XEOS_Mem_Z1, 0, zonesMem );
    
    XEOS_Mem_Z2 = ( void * )( ( uint64_t )XEOS_Mem_Z1 + z1Length );
    XEOS_Mem_Z3 = ( void * )( ( uint64_t )XEOS_Mem_Z2 + z2Length );
    XEOS_Mem_Z4 = ( void * )( ( uint64_t )XEOS_Mem_Z3 + z3Length );
    XEOS_Mem_Z5 = ( void * )( ( uint64_t )XEOS_Mem_Z4 + z4Length );
    
    previousZone = NULL;
    
    /***************************************************************************
     * ZONES SETUP
     **************************************************************************/
    
    printf( "Memory zones:\n\n" );
    
    for( i = 0; i < 5; i++ )
    {
        if( i == 0 )
        {
            base    = Z1_START;
            length  = Z1_BYTES;
            type    = XEOS_Mem_ZoneTypeReserved;
        }
        else if( i == 1 )
        {
            base    = Z2_START;
            length  = Z2_BYTES;
            type    = XEOS_Mem_ZoneTypeReserved;
        }
        else if( i == 2 )
        {
            base    = Z3_START;
            length  = Z3_BYTES;
            type    = XEOS_Mem_ZoneTypeReserved;
        }
        else if( i == 3 )
        {
            base    = Z4_START;
            length  = Z4_BYTES;
            type    = XEOS_Mem_ZoneTypeUsable;
        }
        else if( i == 4 )
        {
            base    = Z5_START;
            length  = Z5_BYTES;
            type    = XEOS_Mem_ZoneTypeACPIReclaimable;
        }
        
        zone = XEOS_Mem_GetZoneAtIndex( ( unsigned int )i );
        
        if( previousZone != NULL )
        {
            previousZone->next = zone;
            zone->prev         = previousZone;
        }
        
        zone->type   = type;
        zone->base   = base + ( uint64_t )XEOS_Mem;
        zone->length = length;
        zone->nPages = length / 0x1000;
        
        switch( zone->type )
        {
            case XEOS_Mem_ZoneTypeUnknown:          typeString = "Unknown "; break;
            case XEOS_Mem_ZoneTypeUsable:           typeString = "Usable  "; break;
            case XEOS_Mem_ZoneTypeReserved:         typeString = "Reserved"; break;
            case XEOS_Mem_ZoneTypeACPIReclaimable:  typeString = "ACPI    "; break;
            case XEOS_Mem_ZoneTypeACPINVS:          typeString = "ACPI NVS"; break;
            case XEOS_Mem_ZoneTypeBad:              typeString = "Bad     "; break;
        }
        
        previousZone = zone;
        
        printf( "0x%016llX -> 0x%016llX: %s - %lli B\n", ( uint64_t )( zone->base - ( uint64_t )XEOS_Mem ), ( uint64_t )( zone->base - ( uint64_t )XEOS_Mem ) + zone->length - 1, typeString, zone->length );
    }
}

void Buddy_Setup_Buddies( void )
{
    uint64_t              i;
    uint64_t              j;
    uint64_t              k;
    XEOS_Mem_ZoneRef      zone;
    uint64_t              blockSize;
    XEOS_Mem_ZoneBuddyRef buddy;
    XEOS_Mem_ZoneBuddyRef previousBuddy;
    uint64_t              blocks;
    uint64_t              buddyBlocks;
    volatile uint32_t   * buddyData;
    
    for( i = 0; i < 5; i++ )
    {
        zone          = XEOS_Mem_GetZoneAtIndex( ( unsigned int )i );
        previousBuddy = NULL;

        blockSize = 0x1000;
        blocks    = ( uint64_t )zone;
        blocks   += sizeof( struct __XEOS_Mem_Zone );
        
        printf( "Memory zone %llu - Address:      0x%llX\n", i + 1, ( long long )zone );
        printf( "              - Buddy blocks: 0x%llX\n", blocks );
        
        for( j = 0; j < __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; j++ )
        {
            buddy = &( zone->buddies[ j ] );
            
            if( previousBuddy != NULL )
            {
                previousBuddy->next = buddy;
                buddy->prev         = previousBuddy;
            }
            
            buddy->order     = j;
            buddy->blockSize = blockSize;
            buddy->nBlocks   = zone->length / blockSize;
            buddy->blocks    = ( buddy->nBlocks == 0 ) ? NULL : ( volatile uint32_t * )blocks;
            
            buddyBlocks  = ( buddy->nBlocks / 8 ) + ( ( ( buddy->nBlocks % 8 ) > 0 ) ? 1 : 0 );
            buddyBlocks += ( ( buddyBlocks % 4 ) == 0 ) ? 0 : 4 - ( buddyBlocks % 4 );
            
            printf( "              - Buddy %llu:      %llu blocks using %llu bytes describing %llu bytes\n", j, buddy->nBlocks, buddyBlocks, buddy->nBlocks * buddy->blockSize );
            
            blocks += buddyBlocks;
            
            blockSize *= 2;
        }
        
        buddyBlocks = 0;
        
        for( j = __XEOS_MEM_ZONE_BUDDY_MAX_ORDER; j > 0; j-- )
        {
            buddy = &( zone->buddies[ j - 1 ] );
            
            if( buddy->nBlocks == 0 )
            {
                continue;
            }
            
            buddyData = buddy->blocks;
            
            if( buddyBlocks == 0 )
            {
                for( k = 0; k < buddy->nBlocks; k++ )
                {
                    *( buddyData ) |= 1 << ( k % 32 );
                    
                    buddy->nFreeBlocks += 1;
                    
                    if( k > 0 && ( k % 31 ) == 0 )
                    {
                        buddyData++;
                    }
                }
            }
            else
            {
                for( k = 0; k < buddyBlocks * 2; k++ )
                {
                    if( k > 0 && ( k % 31 ) == 0 )
                    {
                        buddyData++;
                    }
                }
                
                for( k = buddyBlocks * 2; k < buddy->nBlocks; k++ )
                {
                    *( buddyData ) |= 1 << ( k % 32 );
                    
                    buddy->nFreeBlocks += 1;
                    
                    if( k > 0 && ( k % 31 ) == 0 )
                    {
                        buddyData++;
                    }
                }
            }
            
            buddyBlocks = buddy->nBlocks;
        }
        
        printf( "              - Buddy blocks: 0x%llX\n", blocks );
        
        if( i < 4 )
        {
            printf( "--------------------------------------------------------------------------------\n" );
        }
    }
}
