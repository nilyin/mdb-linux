%module mdv

%{
#include <mdv_alloc.h>
%}

%rename(alloc) mdv_alloc;
%rename(free)  mdv_free;

void *mdv_alloc(size_t size);
void mdv_free(void *ptr);