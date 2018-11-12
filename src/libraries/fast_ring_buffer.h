/*  MIT License

    Copyright (c) 2018 DaelonSuzuka

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef _FAST_RING_BUFFER_H_
#define _FAST_RING_BUFFER_H_

#include <stdint.h>

/* ************************************************************************** */

// Circular buffer tools
/*  If RING_BUFFER_SIZE is 256, then the head and tail indexes become
    significantly easier to manage. Bounds checking and modulus operations can
    be replaced a single postincrement(x++) instruction, because a uint8_t
    will wraparound from 0xff to 0x00 on its own.

    Yes, this wastes RAM. No, it doesn't matter, because this project has a
    minimum of 4k of RAM and nothing else has a significant RAM footprint. If
    memory becomes an issue, the size of these buffers can be reduced. This
    would require rewriting the helper macros to add the previously mentioned
    bounds checking.
*/
#define FAST_RING_BUFFER_SIZE 256

typedef struct {
    char contents[FAST_RING_BUFFER_SIZE];
    uint8_t head;
    uint8_t tail;
} fast_ring_buffer_t;

#if FAST_RING_BUFFER_SIZE == 256
#define buffer_is_empty(buffer) (buffer.head == buffer.tail)
#define buffer_is_full(buffer) ((buffer.head + 1) == buffer.tail)
#define buffer_write(buffer, data) buffer.contents[buffer.head++] = data
#define buffer_read(buffer) buffer.contents[buffer.tail++]
#define buffer_peek_last(buffer) buffer.contents[buffer.head - 1]
#else
#error FAST_RING_BUFFER_SIZE other than 256 is not currently supported because the buffer depends on uint8_t overflow behavior
#endif

#endif /* _FAST_RING_BUFFER_H_ */