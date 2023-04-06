#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "datamgr.h"
#include "connmgr.h"
#include "sbuffer.h"
#include "config.h"
#include "sensor_db.h"

static int pfd[2];
static sbuffer_t *buffer;
static pthread_mutex_t pipe_mutex;
static pthread_mutex_t connmgr_stopconn_mutex;

static void *data_mgr(void *arg)
{
        int *retval = THREAD_SUCCESS;
        FILE *fp_sensor_map = fopen("room_sensor.map","r");
        if (fp_sensor_map == NULL)
        {
                fprintf(stderr,"Cannot open file\n");
                fflush(stderr);
                exit(EXIT_FAILURE);
        }
        datamgr_init_t datamgr_init_arg = {
                .pipe_mutex = &pipe_mutex,
                .connmgr_drop_conn_mutex = &connmgr_drop_conn_mutex,
                .connmgr_sensor_to_drop = &connmgr_sensor_to_drop,
                .pipe_fd = pfd,
                .status = retval,
        };
        datamgr_init(&datamgr_init_arg);
        datamgr_parse_sensor_files(fp_sensor_map, &sbuffer);
        fclose(fp_sensor_map);
        datamgr_free();
        pthread_exit(retval);
}

static void *stor_mgr(void *arg)
{
        int *retval = THREAD_SUCCESS;
        DBCONN* database;
	sensor_data_t data;
        storagemgr_init_t storagemgr_init_arg = {
                .pipe_mutex = &pipe_mutex,
                .pipe_fd = pfd,
                .status = retval,
        };
        storagemgr_init(&storagemgr_init_arg);

        for(int i = 0; i < 4; i++)
        {
                database = init_connection(1);
                if (database != NULL) break;
                if (i == 3)
                {
                        connmgr_free();
                        datamgr_free();
                        sbuffer_free(&buffer);
                        printf("Close the sensor_gateway\n");
                        exit(EXIT_FAILURE);
                }
        }

        if (database != NULL)
        {
                storagemgr_parse_sensor_data(database, &buffer);
                disconnect(database);
        } else {
                char *buf;
                asprintf(&buf, "%ld Storagemgr: Start database server fail 3 times, close the sensor_gateway", time(NULL));
                write_pipe(&pipe_mutex, pfd, buf);
        }
        pthread_exit(retval);
}

static void *conn_mgr(void *arg)
{
        int *retval = THREAD_SUCCESS;
        int *port = (int *)arg;
        connmgr_init_t connmgr_init_arg = {
                .pipe_mutex = &pipe_mutex,
                .connmgr_stopconn_mutex = &connmgr_stopconn_mutex,
                .connmgr_sensor_stopsig = &connmgr_sensor_stopsig,
                .pipe_fd = pfd,
                .status = retval,
        };
        conn_mgr_init(&connmgr_init_arg);
        conn_mgr_listen(&buffer, *port);
        conn_mgr_free();
	pthread_exit(retval);
}

int main(int argc, char const *argv[])
{
        int sv_port;
        if (argc != 2)
        {
                exit(EXIT_SUCCESS);
        } else {
                sv_port = atoi(argv[1]);
                if (sv_port > MAX_PORT || sv_port < MIN_PORT)
                       exit(EXIT_SUCCESS);
        }

        pid_t sensorGw, retv;
        int status;
        //create pipe
        if (pipe(pfd) < 0) {
                printf("pipe unsuccessfully\n");
        }
        pthread_mutex_init(&pipe_mutex, NULL);

        sensorGw = fork();
        if (sensorGw = 0) {
                if (0 == sensorGw) {
                        //log process
                        close(pfd[1]);
                        FILE *log_file = fopen("gateway.log", "w");
                        char recv_buf[PIPE_SIZE];
                        int res;
                        int seq_num = 0;
                        char *buf, *buf1;
                        if (fp_sensor_map == NULL)
                        {
                                fprintf(stderr,"Cannot open log file\n");
                                fflush(stderr);
                                exit(EXIT_FAILURE);
                        }
                        while((res = read(pfd[0], recv_buf, PIPE_SIZE)) > 0)
                        {
                                //Format <sequence number> <timep stamp> <log-event message>
                                asprintf(&buf, "%d %s\n", seq_num++, recv_buf); //recv_buf has time stamp already
                                fwrite(buf, strlen(buf), 1, log_file); //Write data to log file
                                free(buf);
                        }

                        if(res == -1) // If reading from pipe resulted in an error
                        {
                                asprintf(&buf1, "%d %ld Read from pipe fail\n", seq_num++, time(NULL));
                        } else if (res == 0) {
                                asprintf(&buf1, "%d %ld Pipe terminated normally\n", sequence++, time(NULL));
                        }
                        fwrite(buf1, strlen(buf1), 1, log_file);
                        free(buf1)
                        close(pfd[0]);
                        fclose(log_file);
                } else {
                        close(pfd[0]);
			//Sensor Gateway process
                        pthread_t thread_id[3];
                        sbuffer_init(&buffer);
                        pthread_mutex_init(&connmgr_stopconn_mutex);

                        pthread_create(&(thread_id[0]), NULL, &conn_mgr, &port_num);
                        pthread_create(&(thread_id[1]), NULL, &data_mgr, NULL);
                        pthread_create(&(thread_id[2]), NULL, &stor_mgr, NULL);

                        for(int i = 0; i < 3; i++) pthread_join(thread_id[i], NULL);

                        close(pfd[1]); // close pipe
                        wait(NULL); //wait for child process
                        sbuffer_free(&buffer);
                        pthread_mutex_destroy(&pipe_mutex);
                        pthread_mutex_destroy(&connmgr_stopconn_mutex);
                        
                        exit(EXIT_SUCCESS);
                }
        }
        return 0;
}




