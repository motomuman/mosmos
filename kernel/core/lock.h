#ifndef _LOCK_H_
#define _LOCK_H_

void mutex_init(uint64_t *mtx);
void mutex_lock(uint64_t *mtx);
void mutex_unlock(uint64_t *mtx);

#endif
