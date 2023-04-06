#include <string.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "sensor_db.h"

static pthread_mutex_t *pipe_mutex;
static int *retval;
static int *pfd;

void storagemgr_init(storagrmgr_init_t *arg)
{
    retval = arg->status;
    pipe_mutex = arg->pipe_mutex;
    pfd = arg->pipe_fd;
}

void storagemgr_parse_sensor_data(DBCONN *conn, sbuffer_t *buffer)
{
    sensor_data_t data;
    while(sbuffer_remove(*buffer, &data) != SBUFFER_FAILURE) //remove a node and return data
    {
        insert_sensor(conn, data.id, data.value, data.ts); //insert to sql database
    }
}

DBCONN *init_connection(char clear_up_flag)
{
    sqlite3 *database;
    char * error_msg;
    char *buf;
    int rval = sqlite3_open(TO_STRING(DB_NAME)), &database)

    if(rval != SQLITE_OPEN)
    {
        fprintf(stderr,"Cannot open database %s\n", TO_STRING(DB_NAME));
        fflush(stderr);
        asprintf(&buf,"%ld Storagemgr: Cannot connect to SQL database", time(NULL));
        sqlite3_close(database);
        database = NULL;
    } else {
        asprintf(&buf,"%ld Storagemgr: Connect successfully to SQL database", time(NULL));
    }
    write_pipe(pipe_mutex, pfd, buf);

    if(database != NULL)
    {
        char *sql;
        if(clear_up_flag == 1)
        {
            sql = "DROP TABLE IF EXISTS "TO_STRING(TABLE_NAME)";"
                    "CREATE TABLE "TO_STRING(TABLE_NAME)"(id INTEGER PRIMARY KEY ASC AUTOINCREMENT, sensor_id INTEGER, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);";
        } else {
            sql =  "CREATE TABLE IF NOT EXISTS "TO_STRING(TABLE_NAME)"(id INTEGER PRIMARY KEY ASC AUTOINCREMENT, sensor_id INTEGER, sensor_value DECIMAL(4,2), timestamp TIMESTAMP);";
        }

        rval = sqlite3_exec(database, sql, NULL, NULL, &error_msg);
        if(rval != SQLITE_OPEN)
        {
            fprintf(stderr,"Cannot exec SQL %s\n", TO_STRING(DB_NAME));
            fflush(stderr);
            asprintf(&buf,"%ld Storagemgr: ", time(NULL), error_msg);
            sqlite3_free(error_msg);
            database = NULL:
        } else {
            asprintf(&buf,"%ld Storagemgr: New table "TO_STRING(TABLE_NAME)" created", time(NULL));
        }
        write_pipe(pipe_mutex, pfd, buf);
    }
    return database;
}

void disconnect(DBCONN *conn)
{
    char *buf;
    int rval = sqlite3_close(conn);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "Failed to close. Database still busy\n");
        fflush(stderr);
        asprintf(&buf, "%ld Storage Manager: Unable to disconnect from SQL server - server busy", time(NULL));
    } else {
        asprintf(&buf, "%ld Storage Manager: Disconnected from SQL server", time(NULL));
    }

    write_pipe(pipe_mutex, pfd, buf);
}

int insert_sensor(DBCONN *conn, sensor_id_t id, sensor_value_t value, sensor_ts_t ts)
{
    char *error_msg;
    char *buf;
    char *sql;

    asprintf(&sql, "INSERT INTO "TO_STRING(TABLE_NAME)"(sensor_id, sensor_value, timestamp) VALUES(%hu, %e, %ld);", id, value, ts);
    
    int rval = sqlite3_exec(conn, sql, NULL, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL excec error: %s\n", error_msg);
        fflush(stderr);
        asprintf(buf, "%ld Storagemgr: Inserted data fail %s", ts, error_msg);
        sqlite3_free(error_msg);
    } else {
        asprintf(&buf, "%ld Storagemgr: Inserted new reading in %s", ts, TO_STRING(TABLE_NAME));
    }

    write_pipe(pipe_mutex, pfd, buf);
    return rval;
}

int insert_sensor_from_file(DBCONN *conn, FILE *sensor_data)
{
    sensor_id_t id;
	sensor_value_t value;
 	sensor_ts_t ts;
	int length;

	assert(sensor_data);
	while (!feof(sensor_data))
	{
		length = fread(&id, sizeof(sensor_id_t), 1, sensor_data);
		if(length == 0)
			break;
		
		length = fread(&value, sizeof(sensor_value_t), 1, sensor_data);
		assert(length);
		
		length = fread(&ts, sizeof(sensor_ts_t), 1, sensor_data);
		assert(length);
		
		insert_sensor(conn, id, value, ts);
    }
	return 0;
}

int find_sensor_all(DBCONN * conn, callback_t f)
{
    char *buf;
    char *error_msg;
    char *sql = "SELECT * FROM "TO_STRING(TABLE_NAME)" ORDER BY id ASC;";
    int rval = sqlite3_exec(conn, sql, f, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL exec error: %s\n", error_msg);
        fflush(stderr);
        asprintf(&buf, "%ld Storagemgr: Select all sensor failed::%s", time(NULL), error_msg);
        sqlite3_free(error_msg);
    } else {
        asprintf(&buf, "%ld Storagemgr: Select all sensor complete", time(NULL));
    }
    write_pipe(pipe_mutex, pfd, buf);
    return rval;
}

int find_sensor_by_value(DBCONN * conn, sensor_value_t value, callback_t f)
{
    char *buf;
    char *error_msg;
    char *sql;

    asprintf(&sql, "SELECT * FROM "TO_STRING(TABLE_NAME)" WHERE sensor_value = %g ORDER BY id ASC;", value);
    int rval = sqlite3_exec(conn, sql, f, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL exec error: %s\n", error_msg);
        fflush(stderr);
        asprintf(&buf, "%ld Storagemgr: Select Sensor query by value failed::%s", time(NULL), error_msg);
        sqlite3_free(error_msg);
    } else
    {
        asprintf(&buf, "%ld Storagemgr: Select Sensor query by value complete", time(NULL)); 
    }
    write_pipe(pipe_mutex, pfd, buf);
    free(sql);
    return rval;
}

int find_sensor_exceed_value(DBCONN * conn, sensor_value_t value, callback_t f)
{

    char *buf;
    char *error_msg;
    char *sql;
    
    asprintf(&sql, "SELECT * FROM "TO_STRING(TABLE_NAME)" WHERE sensor_value > %g ORDER BY id ASC;", value);
    int rval = sqlite3_exec(conn, sql, f, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL exec error: %s\n", errmsg);
        fflush(stderr);
        asprintf(&buf, "%ld Storagemgr: Sensor query GT value failed::%s", time(NULL), error_msg);
        sqlite3_free(error_msg);
    } else 
    {
        asprintf(&buf, "%ld Storagemgr: Sensor query GT value complete", time(NULL));
    }
    write_pipe(pipe_mutex, pfd, buf);
    free(sql);
    return rval;
}

int find_sensor_by_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
    char *buf;
    char *error_msg;
    char *sql;

    asprintf(&sql, "SELECT * FROM "TO_STRING(TABLE_NAME)" WHERE timestamp = %ld ORDER BY id ASC;", ts);
    int rval = sqlite3_exec(conn, sql, f, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", error_msg);
        fflush(stderr);
        asprintf(&buf, "%ld Storagemgr: Sensor query by timestamp failed::%s", time(NULL), error_msg);
        sqlite3_free(error_msg);
    } else {
        asprintf(&buf, "%ld Storagemgr: Sensor query by timestamp complete", time(NULL));    
    }
    write_pipe(pipe_mutex, pfd, buf);
    free(sql);
    return rval;
}

int find_sensor_after_timestamp(DBCONN * conn, sensor_ts_t ts, callback_t f)
{
    char *buf;
    char *error_msg;
    char *sql;

    asprintf(&sql, "SELECT * FROM "TO_STRING(TABLE_NAME)" WHERE timestamp > %ld ORDER BY id ASC;", ts);
    int rval = sqlite3_exec(conn, sql, f, NULL, &error_msg);
    
    if(rval != SQLITE_OK)
    {
        fprintf(stderr, "SQL exec error: %s\n", error_msg);
        fflush(stderr);
        asprintf(&buf, "%ld Storagemgr: Sensor query GT timestamp failed::%s", time(NULL), error_msg);
        sqlite3_free(error_msg);
    } else {
        asprintf(&buf, "%ld Storagemgr: Sensor query GT timestamp complete", time(NULL));
    }
    write_pipe(pipe_mutex, pfd, buf);
    free(sql);
    return rval;
}