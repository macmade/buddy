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

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <libkern/OSAtomic.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/

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

/*******************************************************************************
 * Types
 ******************************************************************************/

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
    uint64_t                    base;
    uint64_t                    length;
    struct __XEOS_Mem_Zone    * next;
    struct __XEOS_Mem_Zone    * prev;
    uint64_t                    nPages;
    struct __XEOS_Mem_ZoneBuddy buddies[ __XEOS_MEM_ZONE_BUDDY_MAX_ORDER ];
    volatile uint32_t         * buddyBlocks;
};

typedef struct __XEOS_Mem_Zone * XEOS_Mem_ZoneRef;

/*******************************************************************************
 * Globals
 ******************************************************************************/

static void * __mem;

static XEOS_Mem_ZoneRef __z1;
static XEOS_Mem_ZoneRef __z2;
static XEOS_Mem_ZoneRef __z3;
static XEOS_Mem_ZoneRef __z4;
static XEOS_Mem_ZoneRef __z5;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

uint64_t            XEOS_Mem_AllocPage( void );
uint64_t            XEOS_Mem_AllocPages( unsigned int n );
void                XEOS_Mem_FreePage( uint64_t address );
void                XEOS_Mem_FreePages( uint64_t address, unsigned int n );
XEOS_Mem_ZoneRef    XEOS_Mem_GetZoneAtIndex( unsigned int index );

static void __Debug_PrintBuddies( unsigned int zoneIndex );

int main( void )
{
    uint64_t          i;
    uint64_t          j;
    uint64_t          k;
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
    
	__mem = malloc( MEM );
    
    printf( "--------------------------------------------------------------------------------\n" );
    printf( "XEOS:         Buddy physical memory allocator\n" );
	printf( "Total memory: %llu MB (%llu B) at %p\n", ( unsigned long long )( ( MEM / 1024 ) / 1024 ), ( unsigned long long )MEM, __mem );
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
    
    __z1 = malloc( zonesMem );
    memset( __z1, 0, zonesMem );
    
    __z2 = ( void * )( ( uint64_t )__z1 + z1Length );
    __z3 = ( void * )( ( uint64_t )__z2 + z2Length );
    __z4 = ( void * )( ( uint64_t )__z3 + z3Length );
    __z5 = ( void * )( ( uint64_t )__z4 + z4Length );
    
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
        zone->base   = base + ( uint64_t )__mem;
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
        
        printf( "0x%016llX -> 0x%016llX: %s - %lli B\n", ( uint64_t )( zone->base - ( uint64_t )__mem ), ( uint64_t )( zone->base - ( uint64_t )__mem ) + zone->length - 1, typeString, zone->length );
    }
    
    printf( "--------------------------------------------------------------------------------\n" );
    
    /***************************************************************************
     * BUDDIES SETUP
     **************************************************************************/
    
    for( i = 0; i < 5; i++ )
    {
        {
            XEOS_Mem_ZoneBuddyRef buddy;
            XEOS_Mem_ZoneBuddyRef previousBuddy;
            uint64_t              blocks;
            uint64_t              buddyBlocks;
            volatile uint32_t   * buddyData;
            
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
    
    __Debug_PrintBuddies( 3 );
    
    XEOS_Mem_AllocPages( 65 );
    
    printf( "Single page allocation:\n\n" );
    
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
        
        printf( "P1:  0x%llX\n", ( p1  == 0  ) ? 0 : p1  - ( uint64_t )__mem );
        printf( "P2:  0x%llX\n", ( p2  == 0  ) ? 0 : p2  - ( uint64_t )__mem );
        printf( "P3:  0x%llX\n", ( p3  == 0  ) ? 0 : p3  - ( uint64_t )__mem );
        printf( "P4:  0x%llX\n", ( p4  == 0  ) ? 0 : p4  - ( uint64_t )__mem );
        printf( "P5:  0x%llX\n", ( p5  == 0  ) ? 0 : p5  - ( uint64_t )__mem );
        printf( "P6:  0x%llX\n", ( p6  == 0  ) ? 0 : p6  - ( uint64_t )__mem );
        printf( "P7:  0x%llX\n", ( p7  == 0  ) ? 0 : p7  - ( uint64_t )__mem );
        printf( "P8:  0x%llX\n", ( p8  == 0  ) ? 0 : p8  - ( uint64_t )__mem );
        printf( "P9:  0x%llX\n", ( p9  == 0  ) ? 0 : p9  - ( uint64_t )__mem );
        printf( "P10: 0x%llX\n", ( p10 == 0  ) ? 0 : p10 - ( uint64_t )__mem );
        
        printf( "P11: 0x%llX\n", ( p11  == 0 ) ? 0 : p11 - ( uint64_t )__mem );
        printf( "P12: 0x%llX\n", ( p12  == 0 ) ? 0 : p12 - ( uint64_t )__mem );
        printf( "P13: 0x%llX\n", ( p13  == 0 ) ? 0 : p13 - ( uint64_t )__mem );
        printf( "P14: 0x%llX\n", ( p14  == 0 ) ? 0 : p14 - ( uint64_t )__mem );
        printf( "P15: 0x%llX\n", ( p15  == 0 ) ? 0 : p15 - ( uint64_t )__mem );
        
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
    
	free( __mem );
	free( __z1 );
    
	return 0;
}

static void __Debug_PrintBuddies( unsigned int zoneIndex )
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
                    
                    printf( "    *** Skipping used blocks #%llu -> #%llu (0x%llX -> 0x%llX)\n", i, i + ( 31 - ( i % 32 ) ), address - ( uint64_t )__mem, ( address + 0x1000 * ( 31 - ( i % 32 ) ) ) - ( uint64_t )__mem );
                    
                    address += 0x1000 * ( 32 - ( i % 32 ) );
                    i       += 31 - ( i % 32 );
                    
                    continue;
                }
                
                printf( "    *** Processing block #%llu: ", i );
                
                set     = ( 8 * ( 1 + ( i % 32 ) / 8 ) ) - ( 1 + ( i % 32 ) % 8 );
                nBlocks = ( n * 0x1000 ) / buddy->blockSize;
                
                if( OSAtomicTestAndClear( set, blocks ) == true )
                {
                    OSAtomicDecrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
                    printf( "Free - Allocation successfull at 0x%llX\n", address - ( uint64_t )__mem );
                    
                    nBlocks--;
                    
                    while( nBlocks-- > 0 )
                    {}
                    
                    __Debug_PrintBuddies( zoneIndex );
                    
                    return address;
                }
                
                printf( "Used - Cannot allocate (0x%llX)\n", address - ( uint64_t )__mem );
                
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
                
                if( OSAtomicTestAndClear( set, blocks ) == true )
                {
                    OSAtomicDecrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                    
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
                    
                    OSAtomicTestAndSet( set, blocks );
                    OSAtomicIncrement64( ( volatile int64_t * )&( buddy->nFreeBlocks ) );
                }
                
                printf( "    *** Allocating block #%llu of buddy %u\n", j, order );
                printf( "    *** Block #%llu of buddy %u is now free\n", j + 1, order );
                
                __Debug_PrintBuddies( zoneIndex );
                
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
        case 0:     return __z1;
        case 1:     return __z2;
        case 2:     return __z3;
        case 3:     return __z4;
        case 4:     return __z5;
        default:    return NULL;
    }
}
