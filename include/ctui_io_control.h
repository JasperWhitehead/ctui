#ifndef CTUI_IO_CONTROL_H
#define CTUI_IO_CONTROL_H

#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#include <dynamic_array.h>
#include <ctui_time_check.h>
struct ctui_write_buffer
{
  double time_since_last_write;
  char *sds_data;
  int frame_rate;
};
int ctui_write_buffer_create(struct ctui_write_buffer *wb);
static inline void ctui_write_buffer_free(struct ctui_write_buffer *wb)
{
  sds_da_free(wb->sds_data);
}
int ctui_write_buffer_write(struct ctui_write_buffer *wb);
int ctui_write_buffer_ready_to_write(struct ctui_write_buffer *wb, double delta_time);
struct ctui_read_thread_shared_data
{
  pthread_mutex_t read_mutex;
  pthread_mutex_t signal_mutex;
  char *sds_read_buffer;
  int end;
  int swap_buffers;
};
void *ctui_read_thread(void *sd);
int ctui_read_thread_init(struct ctui_read_thread_shared_data *sd, pthread_t *thread);
int ctui_read_thread_exit(struct ctui_read_thread_shared_data *sd, pthread_t *thread);
int ctui_read_thread_free(struct ctui_read_thread_shared_data *sd);
int ctui_read_thread_update(struct ctui_read_thread_shared_data *sd, char **buffer, int *updated);
#endif