#include <ctui_io_control.h>

#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include <dynamic_array.h>

int ctui_write_buffer_create(struct ctui_write_buffer *wb)
{
  if (sds_da_create(&(wb->data), sizeof(char), 0))
  {
    return 1;
  }
  wb->time_since_last_write = 0;
  wb->frame_rate = 30;
  return 0;
}
int ctui_write_buffer_write(struct ctui_write_buffer *wb)
{
  size_t total_write = 0;
  while (total_write < sds_da_get_size(wb->data))
  {
    size_t writen_amount = write(STDOUT_FILENO, wb->data + total_write, sds_da_get_size(wb->data) - total_write);
    if (writen_amount == -1)
    {
      return 1;
    }
    total_write += writen_amount;
  }
  sds_da_clear(wb->data);
  wb->time_since_last_write = 0;
  return 0;
}
int ctui_write_buffer_ready_to_write(struct ctui_write_buffer *wb, double delta_time)
{
  wb->time_since_last_write += delta_time;
  if (wb->time_since_last_write > 1 / (wb->frame_rate))
  {
    return 1;
  }
  return 0;
}
/*
// write thread
void *ctui_write_thread(void *shared_data)
{
  struct ctui_write_thread_shared_data *casted_data = (struct ctui_write_thread_shared_data *)shared_data;
  while (1)
  {
    pthread_mutex_lock(&(casted_data->write_thread_signal_mutex));
    if (casted_data->write_thread_signal == -1)
    {
      return 0;
    }
    casted_data->write_thread_signal = 1;
    pthread_mutex_unlock(&(casted_data->write_thread_signal_mutex));

    while (!(casted_data->write_buffer_is_ready))
    {
      pthread_cond_wait(&(casted_data->write_buffer_cond), &(casted_data->write_buffer_cond_mutex));
    }
    casted_data->write_buffer_is_ready = 0;
    
    ctui_time_data_update(&(casted_data->time));
    if (casted_data->time.delta_time < (1/60))
    {
      usleep((useconds_t)((1/60) - casted_data->time.delta_time));
    }
    
    pthread_mutex_lock(&(casted_data->write_buffer_mutex));
    size_t total_write = 0;
    while (total_write < sds_da_get_size(casted_data->write_buffer))
    {
      size_t writen_amount = write(STDOUT_FILENO, casted_data->write_buffer + total_write, sds_da_get_size(casted_data->write_buffer) - total_write);
      if (writen_amount > 0)
      {
        total_write += writen_amount;
      }
    }
    sds_da_clear(casted_data->write_buffer);
    pthread_mutex_unlock(&(casted_data->write_buffer_mutex));
  }
}
int ctui_write_thread_init(struct ctui_write_thread_shared_data *shared_data, pthread_t *thread)
{
  if (ctui_time_data_create(&(shared_data->time)))
  {
    return 1;
  }
  if (pthread_mutex_init(&(shared_data->write_thread_signal_mutex), NULL))
  {
    return 1;
  }
  if (pthread_mutex_init(&(shared_data->write_buffer_mutex), NULL))
  {
    return 1;
  }
  if (pthread_mutex_init(&(shared_data->write_buffer_cond_mutex), NULL))
  {
    return 1;
  }
  if (pthread_cond_init(&(shared_data->write_buffer_cond), NULL))
  {
    return 1;
  }
  if (sds_da_create(&(shared_data->write_buffer), sizeof(char), 0))
  {
    return 1;
  }
  shared_data->write_buffer_is_ready = 0;
  shared_data->write_thread_signal = 0;
  if (pthread_create(thread, NULL, ctui_write_thread, shared_data))
  {
    return 1;
  }
  return 0;
}
int ctui_write_thread_exit(struct ctui_write_thread_shared_data *shared_data, pthread_t *thread)
{
  pthread_mutex_destroy(&(shared_data->write_thread_signal_mutex));
  pthread_mutex_destroy(&(shared_data->write_buffer_mutex));
  pthread_mutex_destroy(&(shared_data->write_buffer_cond_mutex));
  pthread_cond_destroy(&(shared_data->write_buffer_cond));
  sds_da_free(shared_data->write_buffer);
  return 0;
}
int ctui_write_thread_write_buffer_ready_signal(struct ctui_write_thread_shared_data *shared_data)
{
  if (pthread_mutex_lock(&(shared_data->write_buffer_cond_mutex)))
  {
    return 1;
  }
  shared_data->write_buffer_is_ready = 1;
  if (pthread_cond_signal(&(shared_data->write_buffer_cond)))
  {
    return 1;
  }
  if (pthread_mutex_unlock(&(shared_data->write_buffer_cond_mutex)))
  {
    return 1;
  }
  return 0;
}
// read thread

int ctui_io_threads_init(struct ctui_io_threads_shared_data *shared_data)
{
}
void *ctui_read_thread(void *shared_data)
{
  char *local_read_buffer;
  sds_da_create(&local_read_buffer, sizeof(char), 1);
  sds_da_add(&local_read_buffer, 1, sizeof(char));
  size_t bytes_read = 0;
  size_t size = 0;
  while (1)
  {
    
    if ((bytes_read = read(STDIN_FILENO, local_read_buffer + size, 1)) == -1)
    {
      return 1;
    }
    size += bytes_read;
    sds_da_add(&local_read_buffer, bytes_read, sizeof(char));
    
    pthread_mutex_lock(&((struct ctui_io_threads_shared_data *)shared_data)->read_thread_signal_mutex);
    switch (((struct ctui_io_threads_shared_data *)shared_data)->read_thread_signal)
    {
      case 1:
      {
        ((struct ctui_io_threads_shared_data *)shared_data)->read_thread_signal = 0;
        char *temp = local_read_buffer;
        pthread_mutex_lock(&((struct ctui_io_threads_shared_data *)shared_data)->read_buffer_mutex);
        local_read_buffer = ((struct ctui_io_threads_shared_data *)shared_data)->read_buffer;
        ((struct ctui_io_threads_shared_data *)shared_data)->read_buffer = temp;
        pthread_mutex_unlock(&((struct ctui_io_threads_shared_data *)shared_data)->read_buffer_mutex);
        sds_da_clear(local_read_buffer);
        size = 0;
        break;
      }
      case -1:
      {
        return 0;
      }
    }
    pthread_mutex_unlock(&((struct ctui_io_threads_shared_data *)shared_data)->read_thread_signal_mutex);
  }
}

int ctui_io_threads_check_ready_to_write(struct ctui_io_threads_shared_data *shared_data)
{
}
int ctui_io_threads_signal_write_buffer_finish(struct ctui_io_threads_shared_data *shared_data)
{
}
int ctui_io_threads_signal_read_buffer_finish(struct ctui_io_threads_shared_data *shared_data)
{
}
void ctui_io_threads_write_buffer_append(struct ctui_io_threads_shared_data *shared_data, char *data, size_t size)
{
}
*/