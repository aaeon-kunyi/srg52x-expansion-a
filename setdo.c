/*
 * setting digital output for SRG-3352x Expansion Mode A - ADC/DIO
 *
 * the code use sysfs to operation gpio
 * because libgpiod can't keep state of gpio when the process exit
 *
 *
 * DO0 --> gpio2.10, GPIO74
 * DO1 --> gpio2.11, GPIO75
 * DO2 --> gpio2.12, GPIO76
 * DO3 --> gpio0.26, GPIO26
 * ==============================================================
 *  program usage example
 *	setdo 0 0 # for digital output 0, to lower
 *	setdo 0 1 # for digital output 0, to high
 *	setdo 1 0 # for digital output 1, to lower
 *	setdo 1 1 # for digital output 1, to high
 *	setdo 2 0 # for digital output 2, to lower
 *	setdo 2 1 # for digital output 2, to high
 *	setdo 3 0 # for digital output 3, to lower
 *	setdo 3 1 # for digital output 3, to high
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#ifndef	CONSUMER
#define	CONSUMER	"SetDO_Consumer"
#endif

#define  sysfs_gpio	"/sys/class/gpio"
#define  gpio_export "export"

static int writeFile(char* file, char* data, size_t len)
{
	int fd = open(file, O_WRONLY);
	int ret = 0;
	if (fd < 0) {
		perror("open file failed\n");
		return fd;
	}

	ret = write(fd, data, len);
	if (ret < 0) {
		perror("write file failed\n");
		return ret;
	}
	close(fd);
	return 0;
}

static void export_gpio(int i)
{
	char path[PATH_MAX];
	char data[32];

	if (i < 0)
		return;

	snprintf(path, sizeof(path), "%s/export", sysfs_gpio);
	snprintf(data, sizeof(data), "%d", i);
	writeFile(path, data, strlen(data));
}

static void setgpio_output_mode(int i)
{
	char path[PATH_MAX];
	char data[] = "out";
	snprintf(path, sizeof(path), "%s/gpio%d/direction", sysfs_gpio, i);
	writeFile(path, data, strlen(data));
}

static void setgpio_output_value(int i, int value)
{
	char path[PATH_MAX];
	char data[] = "out";
	snprintf(path, sizeof(path), "%s/gpio%d/value", sysfs_gpio, i);
	snprintf(data, sizeof(data), "%d", value);
	writeFile(path, data, strlen(data));
}

static int isgpio_exist(int i)
{
	int ret;
	char path[PATH_MAX];
	snprintf(path, sizeof(path), "%s/gpio%d", sysfs_gpio, i);
	/* printf("FUNC:%s, LINE:%d, debug:%s\n", __FUNCTION__, __LINE__, path); */
	return (0 == access(path, F_OK)) ? 1 : 0;
}


static void print_usage()
{
	printf("setdo -- set digital output state\n");
	printf("\n");
	printf("\tUsage: setdo pin stats, pin in [0-3]\n");
	printf("\t                      , state in [0,1], 0 for lower, 1 for high\n");
}

int main(int argc, char **argv)
{
	unsigned int	pinnums[] = { 74, 75, 76, 26 };
	unsigned int	val;

	int i, s, ret;
	char pin;

	if (argc < 3) {
		print_usage();
		return 1;
	}

	i = strlen(argv[1]);
	if (i != 1) {
		print_usage();
		return 2;
	}

	pin = argv[1][0];
	if (pin < '0' || pin > '3') {
		print_usage();
		return 3;
	}
	i = pin - '0';

	s = strlen(argv[2]);
	if (s != 1) {
		print_usage();
		return 4;
	}

	s = argv[2][0] - '0';

	if (!isgpio_exist(pinnums[i])) {
		export_gpio(pinnums[i]);
		/* need waiting some time for sysfs operation */
		usleep(500*1000);
	}
	ret = 0;
	setgpio_output_mode(pinnums[i]);
	/* need waiting some time for sysfs operation */
	usleep(500*1000);
	setgpio_output_value(pinnums[i], s);
	return ret;
}

#if 0
#include <gpiod.h>

/*
	because libgpiod can't keep state of gpio when the process exit
*/
int main(int argc, char **argv)
{
	char *gpio2 = "gpiochip2";
	unsigned int	pinnums[] = { 6, 7, 22, 23 };
	unsigned int	val;
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int i, s, ret;
	char pin;

	if (argc < 3) {
		print_usage();
		return 1;
	}

	i = strlen(argv[1]);
	if (i != 1) {
		print_usage();
		return 2;
	}
	
	pin = argv[1][0];
	if (pin < '0' || pin > '3') {
		print_usage();
		return 3;
	}
	i = pin - '0';

	s = strlen(argv[2]);
	if (s != 1) {
		print_usage();
		return 4;
	}

	s = argv[2][0] - '0';
	chip = gpiod_chip_open_by_name(gpio2);
	if (!chip) {
		perror("Open chip failed\n");
		goto end;
	}

	line = gpiod_chip_get_line(chip, pinnums[i]);
	if (!line) {
		perror("Get line failed\n");
		goto close_chip;
	}
	ret = 0;
	ret = gpiod_line_request_output(line, CONSUMER, 0);
	if (ret < 0) {
		perror("Request line as output failed\n");
		goto release_line;
	}

	val = (s != 0);
	ret = gpiod_line_set_value(line, val);
	if (ret < 0) {
		perror("Set line output failed\n");
		goto release_line;
	}
	printf("%u\n", val);


release_line:
	gpiod_line_release(line);
close_chip:
	gpiod_chip_close(chip);
end:
	return ret;
}
#endif
