#ifndef jpeg_mem_src_h_DEFINED
#define jpeg_mem_src_h_DEFINED

#include <sys/types.h>
#include <stdio.h>

extern "C" {
#include "jpeglib.h"
void jpeg_mem_src (j_decompress_ptr cinfo, void* buffer, long nbytes);
}

#endif /* jpeg_mem_dest_h_DEFINED */
