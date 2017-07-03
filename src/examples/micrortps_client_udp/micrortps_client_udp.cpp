#include <px4_config.h>
#include <px4_tasks.h>
#include <px4_posix.h>
#include <unistd.h>
#include <stdio.h>
#include <poll.h>
#include <string.h>
#include <termios.h>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <microcdr/microCdr.h>
#include <uORB/uORB.h>

#include "UDP_node.h"

#include <uORB/topics/sensor_combined.h>

#define BUFFER_SIZE 1024
#define POLL_TIME_MS 0
#define UPDATE_TIME_MS 0
#define LOOPS 10000
#define USLEEP_MS 0


extern "C" __EXPORT int micrortps_client_udp_main(int argc, char *argv[]);
void* send(void*);

static bool _should_exit_task = false;
static uint32_t _received = 0;
static int _started = 0;

UDP_node udp_node;

void* send(void*)
{
    char data_buffer[BUFFER_SIZE] = {};

    /* subscribe to topics */
     px4_pollfd_struct_t fds[1];

    // orb_set_interval statblish an update interval period in milliseconds.
    fds[0].fd = orb_subscribe(ORB_ID(sensor_combined));
    orb_set_interval(fds[0].fd, 1000);
    fds[0].events = POLLIN;

    // microBuffer to serialized using the user defined buffer
    struct microBuffer microBufferWriter;
    initStaticAlignedBuffer(data_buffer, BUFFER_SIZE, &microBufferWriter);
    // microCDR structs for managing the microBuffer
    struct microCDR microCDRWriter;
    initMicroCDR(&microCDRWriter, &microBufferWriter);

    _started++;
    uint32_t length = 0;
    while (!_should_exit_task)
    {
        int poll_ret = px4_poll(fds, 1, 1000);
        if (poll_ret >= 0)
        {
            if (fds[0].revents & POLLIN)
            {
                // obtained data for the file descriptor
                struct sensor_combined_s data;
                // copy raw data into local buffer
                orb_copy(ORB_ID(sensor_combined), fds[0].fd, &data);
                serialize_sensor_combined(&data, data_buffer, &length, &microCDRWriter);
                udp_node.write((char)58, data_buffer, length);
                printf("[%d]>> %f\n", 58, (double)data.baro_temp_celcius);
            }
        }
    }

    return 0;
}

int micrortps_client_udp_main(int argc, char *argv[])
{

    if (0 != udp_node.init(2020, 2019))
    {
        printf("ERROR UDP INIT, EXITING...\n");
        return -1;
    }


    // create a thread for sending data to the simulator
    pthread_t sender_thread;
    // initialize threads
    pthread_attr_t sender_thread_attr;
    pthread_attr_init(&sender_thread_attr);
    pthread_attr_setstacksize(&sender_thread_attr, PX4_STACK_ADJUSTED(4000));
    struct sched_param param;
    (void)pthread_attr_getschedparam(&sender_thread_attr, &param);
    // low priority
    param.sched_priority = SCHED_PRIORITY_DEFAULT + 40;
    (void)pthread_attr_setschedparam(&sender_thread_attr, &param);
    // got data from simulator, now activate the sending thread
    _should_exit_task = false;
    pthread_create(&sender_thread, &sender_thread_attr, send, nullptr);
    pthread_attr_destroy(&sender_thread_attr);

    // microBuffer to deserialize
    char data_buffer[BUFFER_SIZE] = {};
    struct microBuffer microBufferReader;
    initDeserializedAlignedBuffer(data_buffer, BUFFER_SIZE, &microBufferReader);
    // microCDR structs for managing the microBuffer
    struct microCDR microCDRReader;
    initMicroCDR(&microCDRReader, &microBufferReader);

    struct sensor_combined_s sensor_combined_data {};
    orb_advert_t sensor_combined_pub = 0;

    _received = 0;
    while (!_should_exit_task)
    {
        char topic_ID = 255;
        if (0 < udp_node.read(&topic_ID, data_buffer))
        {
            switch (topic_ID)
            {
                case 58:
                    deserialize_sensor_combined(&sensor_combined_data, data_buffer, &microCDRReader);
                    if (!sensor_combined_pub)
                        sensor_combined_pub = orb_advertise(ORB_ID(sensor_combined), &sensor_combined_data);
                    else
                        orb_publish(ORB_ID(sensor_combined), sensor_combined_pub, &sensor_combined_data);
                    printf("                >>[%d] %f\n", 58, (double)sensor_combined_data.baro_temp_celcius);
                    ++_received;
                break;
            }
        }
    }

    pthread_join(sender_thread, nullptr);
    udp_node.close();
    PX4_INFO("exiting");
    fflush(stdout);
    return 0;
}
