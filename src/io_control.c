#include <ctui_io_control.h>

#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#include <dynamic_array.h>

int ctui_write_buffer_create(struct ctui_write_buffer *wb)
{
  if (sds_da_create(&(wb->sds_data), sizeof(char), 0))
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
  while (total_write < sds_da_get_size(wb->sds_data))
  {
    size_t writen_amount = write(STDOUT_FILENO, wb->sds_data + total_write, sds_da_get_size(wb->sds_data) - total_write);
    if (writen_amount == -1)
    {
      return 1;
    }
    total_write += writen_amount;
  }
  sds_da_clear(wb->sds_data);
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
// read thread
void *ctui_read_thread(void *sd)
{
  struct ctui_read_thread_shared_data *casted_data = (struct ctui_read_thread_shared_data *)sd;

  char *sds_read_into_buffer;
  sds_da_create(&sds_read_into_buffer, sizeof(char), 0);

  struct pollfd fds;
  memset(&fds, 0, sizeof(fds));
  fds.fd = STDIN_FILENO;
  fds.events = POLLIN;

  size_t used_size = 0;
  size_t read_return = 0;
  int thread_end = 0;
  int swap = 0;
  while (!thread_end)
  {
    if (poll(&fds, 1, 100))
    {
      if (sds_da_get_size(sds_read_into_buffer) - used_size < 1)
      {
        sds_da_add(&sds_read_into_buffer, sizeof(char), 1);
      }
      read_return = read(STDIN_FILENO, sds_read_into_buffer + used_size, sds_da_get_size(sds_read_into_buffer) - used_size);
      if (read_return == -1){}
      else
      {
        used_size += read_return;
      }
      
    }
    else
    {
      pthread_mutex_lock(&(casted_data->signal_mutex));
      thread_end = casted_data->end;
      swap = casted_data->swap_buffers;
      pthread_mutex_unlock(&(casted_data->signal_mutex));

      if (casted_data->swap_buffers)
      {
        casted_data->swap_buffers = 0;
        used_size = 0;

        char *temp = sds_read_into_buffer;
        pthread_mutex_lock(&(casted_data->read_mutex));
        sds_read_into_buffer = casted_data->sds_read_buffer;
        casted_data->sds_read_buffer = temp;
        pthread_mutex_unlock(&(casted_data->read_mutex));
      }
      
    }
  }
  sds_da_free(sds_read_into_buffer);
  pthread_exit(0);
}
int ctui_read_thread_init(struct ctui_read_thread_shared_data *sd, pthread_t *thread)
{
  sd->end = 0;
  sd->swap_buffers = 0;
  if (sds_da_create(&(sd->sds_read_buffer), sizeof(char), 0))
  {
    return 1;
  }
  if (pthread_mutex_init(&(sd->read_mutex), NULL))
  {
    return 1;
  }
  if (pthread_mutex_init(&(sd->signal_mutex), NULL))
  {
    return 1;
  }
  if (pthread_create(thread, NULL, ctui_read_thread, sd))
  {
    return 1;
  }
  return 0;
}
int ctui_read_thread_exit(struct ctui_read_thread_shared_data *sd, pthread_t *thread)
{
  pthread_mutex_lock(&(sd->signal_mutex));
  sd->end = 1;
  sd->swap_buffers = 1;
  pthread_mutex_unlock(&(sd->signal_mutex));
  pthread_join(*thread, NULL);
  return 0;
}
int ctui_read_thread_free(struct ctui_read_thread_shared_data *sd)
{
  pthread_mutex_destroy(&(sd->read_mutex));
  pthread_mutex_destroy(&(sd->signal_mutex));
  sds_da_free(sd->sds_read_buffer);
  return 0;
}
int ctui_read_thread_update(struct ctui_read_thread_shared_data *sd, char **sds_buffer, int *updated)
{
  pthread_mutex_lock(&(sd->signal_mutex));
  *updated = !(sd->swap_buffers);
  pthread_mutex_unlock(&(sd->signal_mutex));

  if (updated)
  {
    char *temp = *sds_buffer;
    pthread_mutex_lock(&(sd->read_mutex));
    *sds_buffer = sd->sds_read_buffer;
    sd->sds_read_buffer = temp;
    pthread_mutex_unlock(&(sd->read_mutex));

    pthread_mutex_lock(&(sd->signal_mutex));
    sd->swap_buffers = 1;
    pthread_mutex_unlock(&(sd->signal_mutex));
  }
  return 0;
}