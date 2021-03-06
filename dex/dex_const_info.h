/*@
XOC Release License

Copyright (c) 2013-2014, Alibaba Group, All rights reserved.

    compiler@aliexpress.com

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Su Zhenyu nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

author: Su Zhenyu
@*/
#ifndef __DEX_CONST_INFO_H__
#define __DEX_CONST_INFO_H__

//Host integer and float point type.
//e.g: Build XOC on x8664, HOST_INT should be 64bit.
//Or build XOC on ARM, HOST_INT should be 32bit,
//of course 64bit is ok if you want.
#define HOST_INT LONGLONG
#define HOST_UINT LONGLONG
#define HOST_FP double

//If the number of OR of one BB is larger than following value,
//all local optimizations are disabled.
#define MAX_OR_BB_OPT_BB_LEN    1000

//Define machine word/half-word/byte/bit size
#define BIT_PER_BYTE            8
#define BYTE_PER_CHAR           1
#define BYTE_PER_SHORT          2
#define BYTE_PER_INT            4
#define BYTE_PER_LONG           4
#define BYTE_PER_LONGLONG       8
#define BYTE_PER_FLOAT          4
#define BYTE_PER_DOUBLE         8
#define BYTE_PER_ENUM           4
#define BYTE_PER_POINTER        4
#define GENERAL_REGISTER_SIZE   (BYTE_PER_POINTER)

//Bit size of word length of host machine.
#define WORD_LENGTH_OF_HOST_MACHINE    (sizeof(HOST_UINT) * HOST_BIT_PER_BYTE)

//Bit size of word length of target machine.
#define WORD_LENGTH_OF_TARGET_MACHINE  (GENERAL_REGISTER_SIZE * BIT_PER_BYTE)

//Represent target machine word with host type.
#define TMWORD                           UINT

//Target machine relatived heap memory allocate function name
#define MALLOC_NAME                       "malloc"

//Setting for compiler build-environment. Byte length.
#define HOST_BIT_PER_BYTE                8

//Length of string buffer of OR and SR's name.
#define MAX_OR_SR_NAME_BUF_LEN           1024

//Length of string buffer of func-level/bb-level internal VAR name.
#define MAX_VAR_NAME_BUF_LEN             64

//Maximum memory space of stack variables.
#define MAX_STACK_VAR_SPACE              16*1024*1024

//The order of push parameter when function was call.
//true: from right to left
//false: from left to right
#define PUSH_PARAM_FROM_RIGHT_TO_LEFT    true

//The number of registers which be used to store return value.
#define NUM_OF_RETURN_VAL_REGISTERS      2

//Define whether target machine support predicate register.
//Note the first opnd must be predicate register if target support.
#define HAS_PREDICATE_REGISTER           false

//Define the max/min integer value range of target machine.
#define MAX_INT_VALUE                    0x7fffFFFF
#define MIN_INT_VALUE                    0x80000000
#define MAX_UINT_VALUE                   0xffffFFFF
#define EPSILON                          0.000001

#define REG_UNDEF ((USHORT)-1) //Reserved undefined physical register id
#endif
