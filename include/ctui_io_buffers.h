#ifndef CTUI_IO_BUFFERS_H
#define CTUI_IO_BUFFERS_H

#include <semaphore.h>
#include <pthread.h>
#include <time.h>

#include <ctui_time_check.h>

struct ctui_write_thread_shared_data
{
  struct ctui_time_data time;
  pthread_mutex_t write_thread_signal_mutex;
  pthread_mutex_t write_buffer_mutex;
  pthread_mutex_t write_buffer_cond_mutex;
  pthread_cond_t write_buffer_cond;
  char *write_buffer;
  int write_buffer_is_ready;
  int write_thread_signal;
};
void *ctui_write_thread(void *shared_data);
int ctui_write_thread_init(struct ctui_write_thread_shared_data *shared_data, pthread_t *thread);
int ctui_write_thread_exit(struct ctui_write_thread_shared_data *shared_data, pthread_t *thread);
int ctui_write_thread_write_buffer_ready_signal(struct ctui_write_thread_shared_data *shared_data);
/*
struct ctui_read_thread_shared_data
{
  pthread_mutex_t read_buffer_mutex;
  pthread_mutex_t read_thread_signal_mutex;
  char *read_buffer;
  int read_thread_signal;
};
void *ctui_read_thread(void *shared_data);





int ctui_io_threads_init(struct ctui_io_threads_shared_data *shared_data);
int ctui_io_threads_check_ready_to_write(struct ctui_io_threads_shared_data *shared_data);
int ctui_io_threads_signal_write_buffer_finish(struct ctui_io_threads_shared_data *shared_data);
int ctui_io_threads_signal_read_buffer_finish(struct ctui_io_threads_shared_data *shared_data);

void ctui_io_threads_write_buffer_append(struct ctui_io_threads_shared_data *shared_data, char *data, size_t size);
*/
#endif