all: make_utils

make_utils:
	gcc main.c utils/utils/utils.c utils/alloc/alloc.c utils/logger/logger.c utils/svc/svc.c utils/workso/workso.c \
	utils/config/config.c utils/cache/cache.c utils/utils/list.c utils/utils/hashtable.c