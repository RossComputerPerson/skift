#ifndef __LIBDESKTOP_PROP_H_
#define __LIBDESKTOP_PROP_H_ 1

#include <libsystem/cstring.h>

#define LIBDESKTOP_PROP_STR 1
#define LIBDESKTOP_PROP_INT 2
#define LIBDESKTOP_PROP_LONG 3

#define LIBDESKTOP_FLAG_PROP_ARRAY (1 << 1)
#define LIBDESKTOP_FLAG_PROP_ALLOC (1 << 2)

typedef struct {
	int type;
	unsigned int flags;
	size_t size;
	size_t nmem;
	void* value;
} libdesktop_prop;

#endif
