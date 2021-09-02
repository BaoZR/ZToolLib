#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "ztl_mempool.h"
#include "ztl_linklist.h"
#include "ztl_dlist.h"
#include "ztl_threads.h"


typedef struct 
{
    void*                   data;
    ztl_queue_t             link;
}ztl_dlnode_t;

struct ztl_dlist_st
{
    ztl_queue_t             que;
    ztl_mempool_t*          mp;
    uint32_t                size;
    int                    (*cmp)(const void* expect, const void* actual);
    int                    (*vfree)(void* val);

    ztl_thread_mutex_t      lock;
    ztl_thread_rwlock_t     rwlock;
};


static inline int _ztl_dlist_cmp(const void* expect, const void* actual)
{
    if (expect == actual) {
        return 0;
    }
    else if (expect < actual) {
        return -1;
    }
    else {
        return 1;
    }
}

static inline void* _ztl_dlist_remove(ztl_dlist_t* dl, ztl_queue_t* quelink)
{
    void*           data;
    ztl_dlnode_t*   dlnode;

    dlnode = ztl_queue_data(quelink, ztl_dlnode_t, link);
    data = dlnode->data;

    --dl->size;
    ztl_queue_remove(quelink);
    ztl_mp_free(dl->mp, dlnode);
    return data;
}

//////////////////////////////////////////////////////////////////////////
ztl_dlist_t* ztl_dlist_create(int reserve_nodes,
    int(*cmp)(const void* expect, const void* actual),
    int(*vfree)(void* val))
{
    ztl_dlist_t* dl;
    ztl_thread_mutexattr_t mattr;

    dl = (ztl_dlist_t*)malloc(sizeof(ztl_dlist_t));
    memset(dl, 0, sizeof(*dl));

    if (reserve_nodes <= 0) {
        reserve_nodes = ZTL_DLIST_REVERSE_NODES;
    }

    ztl_queue_init(&dl->que);
    dl->mp      = ztl_mp_create(sizeof(ztl_dlnode_t), reserve_nodes, true);
    dl->size    = 0;
    dl->cmp     = cmp ? cmp : _ztl_dlist_cmp;
    dl->vfree   = vfree;

    ztl_thread_mutexattr_init(&mattr);
    ztl_thread_mutexattr_settype(&mattr, ZTL_THREAD_MUTEX_RECURSIVE_NP);
    ztl_thread_mutex_init(&dl->lock, &mattr);
    ztl_thread_mutexattr_destroy(&mattr);
    ztl_thread_rwlock_init(&dl->rwlock);

    return dl;
}

void ztl_dlist_release(ztl_dlist_t* dl)
{
    ztl_dlist_iterator_t* iter;
    iter = ztl_dlist_iter_new(dl, ZTL_DLSTART_HEAD);
    while (true)
    {
        void* actual = ztl_dlist_next(dl, iter);
        if (!actual) {
            break;
        }
        if (dl->vfree) {
            dl->vfree(actual);
        }
    }

    ztl_mp_release(dl->mp);
    free(dl);
}

void* ztl_dlist_head(ztl_dlist_t* dl)
{
    if (dl->size == 0)
        return NULL;

    ztl_dlnode_t* dlnode = ztl_queue_data(ztl_queue_head(&dl->que), ztl_dlnode_t, link);
    return dlnode->data;
}

void* ztl_dlist_tail(ztl_dlist_t* dl)
{
    if (dl->size == 0)
        return NULL;
    ztl_dlnode_t* dlnode = ztl_queue_data(ztl_queue_last(&dl->que), ztl_dlnode_t, link);
    return dlnode->data;
}

int ztl_dlist_size(ztl_dlist_t* dl)
{
    return dl->size;
}

int ztl_dlist_insert_head(ztl_dlist_t* dl, void* data)
{
    ztl_dlnode_t* dlnode;
    dlnode = (ztl_dlnode_t*)ztl_mp_alloc(dl->mp);
    if (!dlnode) {
        return -1;
    }
    dlnode->data = data;

    ztl_queue_insert_head(&dl->que, &dlnode->link);
    ++dl->size;
    return 0;
}

int ztl_dlist_insert_tail(ztl_dlist_t* dl, void* data)
{
    ztl_dlnode_t* dlnode;
    dlnode = (ztl_dlnode_t*)ztl_mp_alloc(dl->mp);
    if (!dlnode) {
        return -1;
    }
    dlnode->data = data;

    ztl_queue_insert_tail(&dl->que, &dlnode->link);
    ++dl->size;
    return 0;
}

void* ztl_dlist_pop(ztl_dlist_t* dl)
{
    if (dl->size == 0) {
        return NULL;
    }

    ztl_queue_t* quelink;
    quelink = ztl_queue_head(&dl->que);
    return _ztl_dlist_remove(dl, quelink);
}

void* ztl_dlist_pop_back(ztl_dlist_t* dl)
{
    if (dl->size == 0) {
        return NULL;
    }

    ztl_queue_t* quelink;
    quelink = ztl_queue_last(&dl->que);
    return _ztl_dlist_remove(dl, quelink);
}

bool ztl_dlist_have(ztl_dlist_t* dl, void* expect)
{
    void* actual;
    actual = ztl_dlist_search(dl, expect);
    return actual ? true : false;
}

void* ztl_dlist_search(ztl_dlist_t* dl, void* expect)
{
    void* actual = NULL;
    ztl_dlist_iterator_t* iter;
    iter = ztl_dlist_iter_new(dl, ZTL_DLSTART_HEAD);
    while (true)
    {
        actual = ztl_dlist_next(dl, iter);
        if (actual == NULL) {
            break;
        }
        else if (dl->cmp(expect, actual) == 0) {
            break;
        }
        actual = NULL;
    }

    ztl_dlist_iter_del(dl, iter);

    return actual;
}

void* ztl_dlist_remove(ztl_dlist_t* dl, void* expect)
{
    void* actual = NULL;
    ztl_dlist_iterator_t* iter;
    iter = ztl_dlist_iter_new(dl, ZTL_DLSTART_HEAD);
    while (true)
    {
        actual = ztl_dlist_next(dl, iter);
        if (actual == NULL) {
            break;
        }
        else if (dl->cmp(expect, actual) == 0) {
            ztl_dlist_erase(dl, iter);
            break;
        }
        actual = NULL;
    }

    ztl_dlist_iter_del(dl, iter);

    return actual;
}

ztl_dlist_iterator_t* ztl_dlist_iter_new(ztl_dlist_t* dl, int direction)
{
    ztl_dlist_iterator_t* iter;
    iter = (ztl_dlist_iterator_t*)malloc(sizeof(ztl_dlist_iterator_t));

    iter->direction = direction;
    if (direction == ZTL_DLSTART_HEAD)
        iter->nodelink = ztl_queue_head(&dl->que);
    else
        iter->nodelink = ztl_queue_last(&dl->que);
    return iter;
}

void ztl_dlist_iter_del(ztl_dlist_t* dl, ztl_dlist_iterator_t* iter)
{
    (void)dl;
    if (iter) {
        free(iter);
    }
}

void* ztl_dlist_next(ztl_dlist_t* dl, ztl_dlist_iterator_t* iter)
{
    void*           data;
    ztl_dlnode_t*   dlnode;
    ztl_queue_t*    current;

    current = (ztl_queue_t*)iter->nodelink;
    if (current != ztl_queue_sentinel(&dl->que))
    {
        // retrieve current data, and move iterator to next
        dlnode = ztl_queue_data(current, ztl_dlnode_t, link);
        data = dlnode->data;

        if (iter->direction == ZTL_DLSTART_HEAD) {
            iter->nodelink = ztl_queue_next(current);
        }
        else {
            iter->nodelink = ztl_queue_prev(current);
        }
    }
    else
    {
        data = NULL;
    }

    return data;
}

#if 0 //deprecated
void* ztl_dlist_iter_data(ztl_dlist_t* dl, ztl_dlist_iterator_t* iter)
{
    ztl_dlnode_t*   dlnode;
    ztl_queue_t*    current;

    // ERROR: we should retrieve its old link's data
    current = (ztl_queue_t*)iter->nodelink;
    if (current == ztl_queue_sentinel(&dl->que)) {
        return NULL;
    }

    dlnode = ztl_queue_data(current, ztl_dlnode_t, link);
    return dlnode->data;
}
#endif

bool ztl_dlist_erase(ztl_dlist_t* dl, ztl_dlist_iterator_t* iter)
{
    ztl_queue_t* current;
    current = (ztl_queue_t*)iter->nodelink;
    ztl_queue_t* oldlink;
    if (iter->direction == ZTL_DLSTART_HEAD)
        oldlink = ztl_queue_prev(current);
    else
        oldlink = ztl_queue_next(current);

    if (oldlink == ztl_queue_sentinel(&dl->que)) {
        return false;
    }

    _ztl_dlist_remove(dl, oldlink);
    return true;
}

void ztl_dlist_lock(ztl_dlist_t* dl)
{
    if (dl == NULL)
        return;

    ztl_thread_mutex_lock(&dl->lock);
}

void ztl_dlist_unlock(ztl_dlist_t* dl)
{
    if (dl == NULL)
        return;
    ztl_thread_mutex_unlock(&dl->lock);
}

void ztl_dlist_rwlock_rdlock(ztl_dlist_t* dl)
{
    if (dl == NULL)
        return;
    ztl_thread_rwlock_rdlock(&dl->rwlock);
}

void ztl_dlist_rwlock_wrlock(ztl_dlist_t* dl)
{
    if (dl == NULL)
        return;
    ztl_thread_rwlock_wrlock(&dl->rwlock);
}

void ztl_dlist_rwlock_unlock(ztl_dlist_t* dl)
{
    if (dl == NULL)
        return;
    ztl_thread_rwlock_unlock(&dl->rwlock);
}
