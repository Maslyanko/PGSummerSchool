all: make_plugin

make_plugin:
	gcc -shared -fPIC secondWorker.c ../../utils/utils/utils.c ../../utils/alloc/alloc.c \
	../../utils/worker/worker.c ../../utils/utils/hashtable.c ../../utils/utils/list.c ../../utils/shmem/shmem.c \
	-o secondWorker.so