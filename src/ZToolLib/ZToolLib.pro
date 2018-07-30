######################################################################
# Automatically generated by qmake (3.0) Fri Apr 22 14:06:12 2016
######################################################################
TARGET = ztoollib

build_pass:CONFIG(debug, debug|release) {
        BuildConf = debug
        TARGET = $$join(TARGET,,,d)
        BuildSuff = d
} else {
        BuildConf = release
        BuildSuff = ""
}


TCONFIG -= qt
QT      -= core gui

CONFIG += debug_and_release static
CONFIG += object_parallel_to_source


TEMPLATE = lib



build_pass:CONFIG(debug, debug|release) {
        DEFINES += DEBUG _DEBUG ZTL_DEBUG
        DESTDIR = ../../build/debug
} else {
        DEFINES += NDEBUG
        DESTDIR = ../../build/release
}

QMAKE_CFLAGS += -E -std=c99


# Input
HEADERS += lockfreequeue.h \
    ztl_aes.h \
    ztl_array.h \
    ztl_atomic.h \
    ztl_base64.h \
    ztl_bitset.h \
    ztl_buffer.h \
    ztl_common.h \
    ztl_config.h \
    ztl_crypt.h \
    ztl_dict.h \
    ztl_dyso.h \
    ztl_event_timer.h \
    ztl_evloop.h \
    ztl_evloop_private.h \
    ztl_fixapi.h \
    ztl_hash.h \
    ztl_linklist.h \
    ztl_locks.h \
    ztl_logger.h \
    ztl_malloc.h \
    ztl_map.h \
    ztl_mempool.h \
    ztl_msg_buffer.h \
    ztl_network.h \
    ztl_palloc.h \
    ztl_producer_consumer.h \
    ztl_protocol.h \
    ztl_rbtree.h \
    ztl_shm.h \
    ztl_simple_event.h \
    ztl_threadpool.h \
    ztl_times.h \
    ztl_tcp_server.h \
    ztl_threads.h \
    ztl_unit_test.h \
    ztl_utils.h 

SOURCES += lockfreequeue.c \
    ztl_aes.c \
    ztl_array.c \
    ztl_base64.c \
    ztl_buffer.c \
    ztl_config.c \
    ztl_crypt.c \
    ztl_dict.c \
    ztl_dyso.c \
    ztl_event_timer.c \
    ztl_evloop.c \
    ztl_evloop_epoll.c \
    ztl_evloop_private.c \
    ztl_fixapi.c \
    ztl_hash.c \
    ztl_locks.c \
    ztl_logger.c \
    ztl_malloc.c \
    ztl_map.c \
    ztl_mempool.c \
    ztl_msg_buffer.c \
    ztl_network.c \
    ztl_palloc.c \
    ztl_producer_consumer.c \
    ztl_rbtree.c \
    ztl_shm.c \
    ztl_simple_event.c \
    siphash.c \
    ztl_threadpool.c \
    ztl_tcp_server.c \
    ztl_threads.c \
    ztl_times.c \
    ztl_unit_test.c \
    ztl_utils.c 
