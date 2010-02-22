/*
 * Copyright (c) 2009 Lukas Mejdrech
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup net
 *  @{
 */

/** @file
 *  Internet protocol address conversion functions implementation.
 */

#include <errno.h>
#include <mem.h>
#include <stdio.h>
#include <string.h>

#include "include/in.h"
#include "include/in6.h"
#include "include/inet.h"
#include "include/socket_codes.h"

int inet_pton( uint16_t family, const char * address, uint8_t * data ){
	const char *	next;
	char *			last;
	int				index;
	int				count;
	int				base;
	size_t			bytes;
	size_t			shift;
	unsigned long	value;

	if( ! data ) return EINVAL;
	switch( family ){
		case AF_INET:
			count = 4;
			base = 10;
			bytes = 1;
			break;
		case AF_INET6:
			count = 16;
			base = 16;
			bytes = 4;
			break;
		default:
			return ENOTSUP;
	}
	if( ! address ){
		bzero( data, count );
		return ENOENT;
	}
	next = address;
	index = 0;
	do{
		if( next && ( * next )){
			if( index ) ++ next;
			value = strtoul( next, & last, base );
			next = last;
			shift = bytes - 1;
			do{
				// like little endian
				data[ index + shift ] = value;
				value >>= 8;
			}while( shift -- );
			index += bytes;
		}else{
			bzero( data + index, count - index );
			return EOK;
		}
	}while( index < count );
	return EOK;
}

int inet_ntop( uint16_t family, const uint8_t * data, char * address, size_t length ){
	if(( ! data ) || ( ! address ))	return EINVAL;
	switch( family ){
		case AF_INET:	if( length < INET_ADDRSTRLEN ) return ENOMEM;
						snprintf( address, length, "%hhu.%hhu.%hhu.%hhu", data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
						return EOK;
		case AF_INET6:	if( length < INET6_ADDRSTRLEN ) return ENOMEM;
						snprintf( address, length, "%hhx%hhx:%hhx%hhx:%hhx%hhx:%hhx%hhx:%hhx%hhx:%hhx%hhx:%hhx%hhx:%hhx%hhx", data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ], data[ 7 ], data[ 8 ], data[ 9 ], data[ 10 ], data[ 11 ], data[ 12 ], data[ 13 ], data[ 14 ], data[ 15 ] );
						return EOK;
		default:		return ENOTSUP;
	}
}

/** @}
 */