/*
 * Uses SPI bus to measure distance with an HC-SR04 ultrasonic rangefinder.
 * To use, connect the Trig pin to MOSI, and the Echo pin to MISO.
 * Hackily adapted from linux/tools/spi/spidev_test.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static void pabort(const char *s)
{
    perror(s);
    abort();
}

static const char *device = "/dev/spidev32766.0";
static uint8_t mode;
static uint32_t clock_speed = 500000;
static uint32_t num_measurements = 100;
static size_t buf_size = 256;
static double speed_of_sound = 343.0; // meters per second.

static int transfer(int fd)
{
    int ret;
    uint8_t tx[buf_size];
    uint8_t rx[buf_size];

    struct spi_ioc_transfer transfer_info = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = buf_size,
        .delay_usecs = 0,
        .speed_hz = clock_speed,
        .bits_per_word = 8,
    };

    for (int i=0; i<buf_size; i++) {
        tx[i] = 0;
    }
    tx[0] = 255;

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer_info);
    if (ret < 1)
        pabort("can't send spi message");


    int num_bits_high = 0;
    int last_high_bit = 0;
    for (ret = 0; ret < buf_size; ret++) {
        for (int i=0; i<8; i++) {
            num_bits_high += (rx[ret] & 1);
            if (rx[ret] & 1) {
                last_high_bit = ret * 8 + i;
            }
            rx[ret] >>= 1;
        }
    }
    return num_bits_high;

}

static void print_usage(const char *prog)
{
    printf("Usage: %s [-DsbdlHOLC3]\n", prog);
    puts("  -D --device            device to use (default /dev/spidev32766.0)\n"
         "  -s --speed             max speed (Hz)\n"
         "  -n --num-measurements  Number of measurements to take; result is the median of\n"
         "                         these. (Default 100)\n"
         "  -b --buf-size          Number of bytes to read; affects maximum distance\n"
         "                         readable and how much time measurements take.\n"
         "  -c --speed-of-sound    Speed of sound, in (preferred output units)/second.\n"
         "                         Default to 343.0, which is the speed of sound in\n"
         "                         meters/second at 20C. For output in millimeters, use\n"
         "                         343000; for output in feet, use 1125.\n"
    );
    exit(1);
}

static void parse_opts(int argc, char *argv[])
{
    while (1) {
        static const struct option lopts[] = {
            { "device",             1, 0, 'D' },
            { "speed",              1, 0, 's' },
            { "num-measurements",   1, 0, 'n' },
            { "buf-size",           1, 0, 'b' },
            { "speed-of-sound",     1, 0, 'c' },
            { NULL, 0, 0, 0 },
        };
        int c;

        c = getopt_long(argc, argv, "D:s:n:d:b:c:", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'D':
            device = optarg;
            break;
        case 's':
            clock_speed = atoi(optarg);
            break;
        case 'n':
            num_measurements = atoi(optarg);
            break;
        case 'b':
            buf_size = atoi(optarg);
            break;
        case 'c':
            speed_of_sound = atof(optarg);
            break;
        default:
            print_usage(argv[0]);
            break;
        }
    }
}

int compare_ints(const void *pa, const void *pb) {
    const int *a = (const int *)pa;
    const int *b = (const int *)pb;

    return (*a > *b) - (*a < *b);
}


int main(int argc, char *argv[])
{
    int ret = 0;
    int fd;

    parse_opts(argc, argv);

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &clock_speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &clock_speed);
    if (ret == -1)
        pabort("can't get max speed hz");


    int measurements[num_measurements];
    for (int i=0; i<num_measurements; i++) {
        measurements[i] = transfer(fd);
    }

    qsort(measurements, num_measurements, sizeof(measurements[0]), compare_ints);

    printf("%f\n", measurements[num_measurements/2] * speed_of_sound / clock_speed);
    close(fd);

    return ret;
}

