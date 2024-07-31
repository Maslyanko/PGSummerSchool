all: make_utils

make_utils:
	gcc main.c utils/workso/workso.c utils/utils/list.c utils/utils/hashtable.c utils/alloc/alloc.c utils/utils/utils.c