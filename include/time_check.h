#ifndef CTUI_TIME_CHECK_H
#define CTUI_TIME_CHECK_H
#include <pthread.h>
#include <time.h>

struct ctui_time_data
{
  struct timespec time;
  double delta_time;
};
int ctui_time_data_create(struct ctui_time_data *td);
int ctui_time_data_update(struct ctui_time_data *td);
struct ctui_time_data_with_lock
{
  struct ctui_time_data td;
  pthread_rwlock_t time_rwlock;
};
int ctui_time_data_with_lock_create(struct ctui_time_data_with_lock *td_l);
int ctui_time_data_with_lock_update(struct ctui_time_data_with_lock *td_l);
int ctui_time_data_with_lock_free(struct ctui_time_data_with_lock *td_l);
#endif