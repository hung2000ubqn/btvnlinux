#include "connmgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <inttypes.h>
#include <pthread.h>
#include "lib/tcpsock.h"
#include "lib/dplist.h"

typedef struct {
	tcpsock_t *sock_pt;
	int sd;
	time_t last_active;
	uint16_t sensor;
} tcpsock_dpl;

static dplist_t *sock_list;
static tcpsock_t *sv;
static struct pollfd *poll_fd;
static pthread_mutex_t *pipe_mutex;
static pthread_mutex_t *connmgr_stopconn_mutex;
static sensor_id_t *connmgr_sensor_stopsig;
static pollfd *poll_fd;
static int *retval;
static int *pfd;

void connmgr_init(connmgr_init_arg_t *arg)
{
	retval = arg->retval;
	pfd = arg->pipe_fd;
	pipe_mutex = arg->pipe_mutex;
	connmgr_stopconn_mutex = arg->connmgr_stopconn_mutex;
	connmgr_sensor_stopsig = arg->connmgr_sensor_stopsig;
}

void connmgr_listen(sbuffer_t **buffer, int port)
{
	// Check port
	if ((port > MAX_PORT) || (port < MIN_PORT))
	{
		asprintf(&buf, "%ld Connmgr: Invalid port ", time(NULL));
		write_pipe(pipe_mutex, pfd, buf);
		return;
	}
	
	char *buf;
	sock_list = dpl_create(&element_copy, &element_free, &element_compare);
	poll_fd = (struct pollfd *) malloc(sizeof(struct pollfd *));
	
	// Create socket and open in listen mode
	if (tcp_passive_open(&(sv), port) != TCP_NO_ERROR)
	{
		asprintf(&buf, "%ld Connmgr: open tcp socket failed ", time(NULL));
		write_pipe(pipe_mutex, pfd, buf);
		*retval = CONNMGR_OPEN_SOCKET_ERROR;
		return;
	}

	asprintf(&buf, "%ld Connmgr: Start successfully ", time(NULL));
	write_pipe(pipe_mutex, pfd, buf);
	
	// Set sv file descriptor
	tcp_get_sd(sv, &(poll_fd[0].fd));
	poll_fd[0].events = POLLIN; //data to read

	struct tcpsock_dpl *client;
	struct tcpsock_dpl sock_dum;
	dplist_node_t *node;
	sensor_data_t data;
	int counter = 0;
	int poll_ret, tcp_conn_ret, tcp_ret, size_ret, sbuffer_ret;
	
	// Loop until no connection
	while((poll_ret = poll(poll_fd, (counter+1), TIMEOUT*1000)) || counter)
	{
		if (poll_ret == -1) break;
		// when return events received
		if ((poll_fd[0].revents & POLLIN) && (counter < MAX_CONN))
		{
			client = (struct tcpsock_dpl *) malloc(sizeof(struct tcpsock_dpl));
			if ((tcp_conn_ret == tcp_wait_for_connection(sv, &(client->sock_pt))) != TCP_NO_ERROR)
			{
				asprintf(&buf, "%ld Connmgr: can not accept new connection (%d)", time(NULL), tcp_conn_ret);
				write_pipe(pipe_mutex, pfd, buf);
				*retval = CONNMGR_ACCEPT_CONNECTION_ERROR;
			} else {
				counter++;
				//Increase size of poll_fd
				poll_fd = (struct pollfd *) realloc(poll_fd, sizeof(struct pollfd *(counter+1)));
				//Set client file descriptor
				tcp_get_sd(client->sock_pt, &(poll_fd[counter].fd));
				client->sd = poll_fd[counter].fd;
				client->last_alive = (time_t) time(NULL);
				client->sensor = 0;
				poll_fd[counter].events = POLLIN | POLLHUP;  //POLLIN - have data, POLLHUP - hang up
				dpl_insert_sorted(sock_list, client, false); //Insert client into dplist
				asprintf(&buf, "%ld Connmgr: received new connection", time(NULL));
				write_pipe(pipe_mutex, pfd, buf);
			}
			poll_ret--;
		}
		
		for(int i = 1; i < (counter+1) && poll_ret > 0; i++)
		{
			sockdum.sd = poll_fd[i].fd; //Set sd equal to fd to find corresponding client
			node = dpl_get_reference_of_element(sock_list, &sockdum); //get node has element from dplist
			if (node != NULL)
				client = (struct tcpsock_dpl *) dpl_get_element_of_reference(node);
			else 
				client = NULL;
			
			if (client != NULL && ((client->last_active + (time_t) TIMEOUT) > (time_t) time(NULL)) && (poll_fd[i].revents & POLLIN))
			{
				size_ret = sizeof(data.id);
				tcp_ret = tcp_receive(client->sock_pt, (void *) &data.id, &size_ret); //read sensor ID
				size_ret = sizeof(data.value);
				tcp_ret = tcp_receive(client->sock_pt, (void *) &data.value, &size_ret); //read temperature
				size_ret = sizeof(data.ts);
				tcp_ret = tcp_receive(client->sock_pt, (void *) &data.ts, &size_ret); //read timestamp
				
				if ((tcp_ret == TCP_NO_ERROR) && size_ret) 
				{
					client->last_active = (sensor_ts_t) time(NULL); //Update time active
					if (client->sensor == 0) 
						client->sensor = data.id;
					sbuffer_ret = sbuffer_insert(*buffer, &data); //insert data in shared buffer
				} else if (tcp_ret == TCP_CONNECTION_CLOSED) {
					poll_fd[i].events = -1;
					asprintf(&buf, "%ld Connmgr: Lost connection", time(NULL));
					write_pipe(pipe_mutex, pfd, buf);
				}
			}

			pthread_mutex_lock(connmgr_stopconn_mutex);
			if((client != NULL && ((client->sensor == *connmgr_sensor_stopsig) 
					|| (client->last_active + (sensor_ts_t) TIMEOUT < (sensor_ts_t) time(NULL)))) 
					|| (poll_fd[i].revents & POLLHUP) 
					|| (poll_fd[i].events == -1))
			{
				if(client != NULL && client->sensor == *connmgr_sensor_stopsig)
				{
					*connmgr_sensor_stopsig = 0;
					pthread_mutex_unlock(connmgr_stopconn_mutex);
					asprintf(&buf, "%ld Connmgr: Stop connection to %"PRIu16" by signal", time(NULL), client->sensor);
					write_pipe(pipe_mutex, pfd, buf);
				} else if (client != NULL) {
					asprintf(&buf, "%ld Connmgr: Close connection to %"PRIu16"", time(NULL), client->sensor);
			       		write_pipe(pipe_mutex, pfd, buf);
					dpl_remove_node(sock_list, node, true);
				} else
					pthread_mutex_unlock(connmgr_stopconn_mutex);
				for (int id1 = 0, id2 = 0; id1 < conn_counter; id1++, id2++)
				{
					id2 += (id2 == i) ? 1 : 0;
					poll_fd[id1] = poll_fd[id2];
				}
				poll_fd = realloc(poll_fd, sizeof (struct pollfd) *conn_counter);
				conn_counter--;
				i--;
			} else 
				pthread_mutex_unlock(connmgr_stopconn_mutex);
		}
		if (poll_ret == -1)
		{
			*retval = CONNMGR_POLL_ERROR;
			asprintf(&buf, "%ld Connmgr: Poll sockets error", time(NULL));
			write_pipe(pipe_mutex, pfd, buf);
		}
	}
}

void connmgr_free()
{
	char *buf;
	if(sv != NULL && tcp_close(&sv) != TCP_NO_ERROR)
	{
		*retval = CONNMGR_CLOSE_SOCKET_ERROR;
		asprintf(&buf, "%ld Connmgr: Failed to stop", time(NULL));
		write_pipe(pipe_mutex, pfd, buf);
	} else {
		asprintf(&buf, "%ld Connmgr: Stopped successfully", time(NULL));
		write_pipe(pipe_mutex, pfd, buf);
	}
	if (poll_fd != NULL) free(poll_fd);
	if (sock_list != NULL) dpl_free(&sock_list, true);
}

static void *element_copy(void *element)
{
	struct tcpsock_dpl *temp = (struct tcpsock_dpl *) malloc(sizeof(struct(tcpsock_dpl)));
	temp->sock_pt = ((struct tcpsock_dpl *) element)->sock_pt;
	temp->sd = ((struct tcpsock_dpl *) element)->sd;
	temp->last_active = ((struct tcpsock_dpl *) element)->last_active;
	temp->sensor = ((struct tcpsock_dpl *) element)->sensor;
	return temp;
}

static void element_free(void **element)
{
	tcp_close(&(((struct tcpsock_dpl *) *element)->sock_pt));
	free((struct tcpsock_dpl *) *element);
}

static int element_compare(void *x, void *y)
{
	int a,b;
	a = ((struct tcpsock_dpl *)x)->sd;
	b = ((struct tcpsock_dpl *)y)->sd;
	return (a==b) ? 0 : -1;
}
			


