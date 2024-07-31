all: make_utils

make_utils:
	gcc -export-dynamic main.c utils/utils/utils.c utils/alloc/alloc.c utils/logger/logger.c utils/svc/svc.c utils/workso/workso.c \
	utils/config/config.c utils/cache/cache.c utils/worker/worker.c utils/shmem/shmem.c utils/utils/hashtable.c utils/utils/list.c \
	utils/proxy/proxy.c