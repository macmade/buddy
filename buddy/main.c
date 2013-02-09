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
    Buddy_Setup_Memory();
    
    printf( "--------------------------------------------------------------------------------\n" );
    
    Buddy_Setup_Buddies();
    
    Buddy_Debug_PrintBuddies( 3 );
    
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
    
	free( XEOS_Mem );
	free( XEOS_Mem_Z1 );
    
	return 0;
}
