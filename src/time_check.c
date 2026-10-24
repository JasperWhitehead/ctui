#include <ctui_time_check.h>

#include <pthread.h>
#include <time.h>
// time data
int ctui_time_data_create(struct ctui_time_data *td)
{
  td->time = 0;
  td->delta_time = 0;
  if (!timespec_get(&(td->time), TIME_UTC))
  {
    return 1;
  }
  return 0;
}
int ctui_time_data_update(struct ctui_time_data *td)
{
  struct timespec current_time;
  if (!timespec_get(&(current_time), TIME_UTC))
  {
    return 1;
  }
  td->delta_time = (double)(current_time.tv_sec - td->time.tv_sec) + (double)(current_time.tv_nsec - td->time.tv_nsec) / 1000000000.0
  td->time = current_time;
  return 0;
}
// time data with lock
int ctui_time_data_with_lock_create(struct ctui_time_data_with_lock *td_l)
{
  if (ctui_time_data_create(&(td_l->td)))
  {
    return 1;
  }
  if (pthread_rwlock_init(&(td_l->time_rwlock), NULL))
  {
    return 1;
  }
  return 0;
}
int ctui_time_data_with_lock_update(struct ctui_time_data_with_lock *td_l)
{
  if (pthread_rwlock_wrlock(&(td_l->time_rwlock)))
  {
    return 1;
  }
  if (ctui_time_data_update(&(td_l->td)))
  {
    return 1;
  }
  if (pthread_rwlock_unlock(&(td_l->time_rwlock)))
  {
    return 1;
  }
  return 0;
}
int ctui_time_data_with_lock_free(struct ctui_time_data_with_lock *td_l)
{
  if (pthread_rwlock_destroy(&(td_l->time_rwlock)))
  {
    return 1;
  }
  return 0;
}