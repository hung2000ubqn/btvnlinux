#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "pthread.h"
#include "datamgr.h"

#define N 20

static dplist_t *dplist;
static pthread_mutex_t *connmgr_stopconn_mutex;
static pthread_mutex_t *pipe_mutex;
static sensor_id_t *connmgr_sensor_stopsig;
static int *retval;
static int *pfd;

typedef struct {
    uint16_t room_id;
    sensor_data_t sensor;
    sensor_value_t data_list[RUN_AVG_LENGTH];
} sensor_node_t;

void datamgr_init(datamgr_init_t *arg)
{
    retval = arg->status;
    pfd = arg->pipe_fd;
	pipe_mutex = arg->pipe_mutex;
    connmgr_stopconn_mutex = arg->connmgr_stopconn_mutex;
    connmgr_sensor_stopsig = arg->connmgr_sensor_stopsig;
}

void datamgr_parse_sensor_files(FILE * fp_sensor_map, sbuffer_t *sbuffer)
{
    char *buf;
    sensor_node_t node;
    char line[N];
    dplist = dpl_create(&coppy_sensor, &free_sensor, &compare_sensor);

    ERROR_HANDLER(fp_sensor_map == NULL, "File sensor map error\n");
    for(int i = 0; i < RUN_AVG_LENGTH; i++) node.data_list[i] = 0;
    node.sensor.ts = (sensor_ts_t) 0;
    node.sensor.value = (sensor_value_t) 0;
    node.data_list = (unsigned char) 0;

    while(fgets(line, N, fp_sensor_map) != NULL)
    {
        sscanf(line, "%hu %hu", &(node.room_id), &(node.sensor.id)); //get room id, sensor id
        dpl_insert_sorted(dplist, &node, true); //insert node to dplist
    }

    sensor_node_t tempnode;
    int i;

    while(sbuffer_remove(sbuffer, &(tempnode.sensor)) != SBUFFER_FAILURE)
    {
        i = dpl_get_index_of_element(dplist, (void *)(tempnode.sensor.id));
        if (i == -1)
        {
            fprintf(stderr, "Invalid sensor ID" "%" PRIu16 "\n", tempnode.sensor.id);
            fflush(stderr);
            asprintf(&buf,"%ld Datamgr: Sensor with ID %" PRIu16 "doesn't exist", time(NULL), tempnode.sensor.id);
            write_pipe(pipe_mutex, pfd, buf);

            pthread_mutex_lock(connmgr_stopconn_mutex);
            *connmgr_sensor_stopsig = tempnode.sensor.id;
            pthread_mutex_unlock(connmgr_stopconn_mutex);
            continue;
        }
        sensor_node_t *temp = dpl_get_element_of_index(dplist, i);
        temp->last_modified = tempnode.sensor.ts;
        sensor_value_t sum = 0;
        int size = RUN_AVG_LENGTH;

        for (int i = 0; i < RUN_AVG_LENGTH; i++)
        {
            if (temp->data_list[i] == 0)
            {
                size = i;
                break;
            }
        }

        if (size < RUN_AVG_LENGTH) 
        {
            temp->data_list[size] = tempnode.sensor.value;
        } else {
            for (int i = 0; i < (RUN_AVG_LENGTH-1); i++)
            {
                temp->data_list[i] = temp->data_list[i+1]
            }

            for (int i = 0; i < RUN_AVG_LENGTH; i++)
            {
                sum += temp->data_list[i];
            }
            temp->sensor.value = sum / RUN_AVG_LENGTH;
            if (temp->sensor.value > MAX_TEMP)
            {
                fprintf(stderr,"Room %" PRIu16 ": The sensor node with %" PRIu16 "report. Temperature is %lf. Too hot!\n",temp->room_id,temp->sensor.id,temp->sensor.value);
                fflush(stderr);
                asprintf(&buf, "%ld Datamgr: The sensor node with %" PRIu16 " report it's TOO HOT\n", time(NULL), temp->sensor.id);
                write_pipe(pipe_mutex, pfd, buf);
            } else if (temp->sensor.value < MIN_TEMP) {
                fprintf(stderr,"Room %" PRIu16 ": The sensor node with %" PRIu16 "report. Temperature is %lf. Too cold!\n",temp->room_id,temp->sensor.id,temp->sensor.value);
                fflush(stderr);
                asprintf(&buf, "%ld Datamgr: The sensor node with %" PRIu16 " report it's TOO COLD\n", time(NULL), temp->sensor.id);
                write_pipe(pipe_mutex, pfd, buf);
            }
        }
    }
}

void datamgr_free()
{
    assert(dplist != NULL);
    dpl_free(&dplist, true);
    char *buf;
    asprintf(&buf, "%ld Datamgr: Clean up and free all used memory\n", time(NULL));
    write_pipe(pipe_mutex, pfd, buf);
}

uint16_t datamgr_get_room_id(sensor_id_t sensor_id)
{
    assert(dplist != NULL);
    sensor_node_t temp;
    temp.room_id = 0;
    temp.sensor.id = sensor_id;
    sensor_node_t *node = dpl_get_element_of_index(dplist, dpl_get_index_of_element(dplist, (void *)(temp.sensor.id)));
    if (temp == NULL)
    {
        fprintf(stderr, "Cannot get room id of the sensor node with %" PRIu16 "\n", sensor_id);
        fflush(stderr);
        return -1;
    }
    return node->room_id;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id)
{
    assert(dplist != NULL);
    sensor_node_t temp;
    temp.room_id = 0;
    temp.sensor.id = sensor_id;
    sensor_node_t *node = dpl_get_element_of_index(dplist, dpl_get_index_of_element(dplist, (void *)(temp.sensor.id)));
    if (temp == NULL)
    {
        fprintf(stderr, "Cannot get avg of the sensor node with %" PRIu16 "\n", sensor_id);
        fflush(stderr);
        return -1;
    }
    return node->sensor.value;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id)
{
    assert(dplist != NULL);
    sensor_node_t temp;
    temp.room_id = 0;
    temp.sensor.id = sensor_id;
    sensor_node_t *node = dpl_get_element_of_index(dplist, dpl_get_index_of_element(dplist, (void *)(temp.sensor.id)));
    if (temp == NULL)
    {
        fprintf(stderr, "Cannot get the last time modified of the sensor node with %" PRIu16 "\n", sensor_id);
        fflush(stderr);
        return -1;
    }
    return node->sensor.ts;
}

int datamgr_get_total_sensors()
{
    assert(dplist != NULL);
    return (dpl_size(dplist));
}

static void * copy_sensor(void *element)
{
    sensor_node_t *temp = (sensor_node_t *) malloc(sizeof(sensor_node_t));
    *temp = *((sensor_node_t *)element);
    return temp;
}

static void free_sensor(void ** element)
{
    free((sensor_node_t *) *element);
}

static int compare_sensor(void * x, void * y)
{
    uint16_t id_x,id_y;
    id_x = ((sensor_node_t *)x)->room_id;
    id_y = ((sensor_node_t *)y)->room_id;
    return (x==y) ? 0 : -1;
}