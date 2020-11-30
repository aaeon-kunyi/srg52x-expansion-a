
#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef	CONSUMER
#define	CONSUMER	"SetDO_Consumer"
#endif

/*
    DO0 --> gpio2.10
    DO1 --> gpio2.11
    DO2 --> gpio2.12
    DO3 --> gpio0.26
    setdo 0 0
*/

static void print_usage()
{
	printf("setdo -- set digital output state\n");
	printf("\n");
	printf("\tUsagee: setdo pin stats, pin in [0-3]\n");
	printf("\t                       , state in [0, 1], 0 for lower\n");
}

int main(int argc, char **argv)
{
    char *gpio2 = "gpiochip2";
	char *gpio0 = "gpiochip0";
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
	if (i == 3)
		chip = gpiod_chip_open_by_name(gpio0);
	else
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
