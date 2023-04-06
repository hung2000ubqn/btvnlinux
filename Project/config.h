
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

#define THREAD_SUCCESS 0
#define CONNMGR_OPEN_SOCKET_ERROR 1
#define CONNMGR_POLL_ERROR 2
#define CONNMGR_CLOSE_SOCKET_ERROR 3
#define PIPE_SIZE 100


typedef uint16_t sensor_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

/**
 * structure to hold sensor data
 */
typedef struct {
    sensor_id_t id;         /** < sensor id */
    sensor_value_t value;   /** < sensor value */
    sensor_ts_t ts;         /** < sensor timestamp */
} sensor_data_t;

typedef struct {
	pthread_mutex_t *pipe_mutex;
	pthread_mutex_t *connmgr_stopconn_mutex;
	int *pipe_fd;
	int *status;
	sensor_id_t *connmgr_sensor_stopsig;
} connmgr_init_t;
	
typedef struct {
	pthread_mutex_t *pipe_mutex;
	pthread_mutex_t *connmgr_stopconn_mutex;
	int *pipe_fd;
	int *status;
	sensor_id_t *connmgr_sensor_stopsig;
} datamgr_init_t;

typedef struct {
	pthread_mutex_t *pipe_mutex;
	int *pipe_fd;
	int *status;
} storagemgr_init_t;

void write_pipe(pthread_mutex_t *pipe_mutex, int *pfds, char* buff)
{
	pthread_mutex_lock(pipe_mutex);
	write(*(pfds+1), buff, strlen(buff)+1);
	pthread_mutex_unlock(pipe_mutex);
	free(buff);
}

#endif /* _CONFIG_H_ */

