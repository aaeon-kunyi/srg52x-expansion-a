

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#ifndef	CONSUMER
#define	CONSUMER	"GetDI_Consumer"
#endif

/*
    DI0 --> gpio2.6
    DI1 --> gpio2.7
    DI2 --> gpio2.22
    DI3 --> gpio2.23
    getdi 0
*/

static void print_usage()
{
	printf("getdi -- read digital input state\n");
	printf("\n");
	printf("Usage: getdi pins, pins in [0-3]\n");
	printf("\n");
	printf("will return 0 or 1 for pin state lower/high\n");
}

int main(int argc, char **argv)
{
	char *gpio2 = "gpiochip2";
	unsigned int  pinnums[] = { 6, 7, 22, 23 };
	unsigned int val;
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int i, ret;
	char pin;
	
	if (argc < 2) {
		print_usage();
		return 1;
	}

	i = strlen(argv[1]);
	if (i != 1) {
		print_usage();
		return 2;
	}

	pin = argv[1][0];
	if ( pin < '0' || pin > '3') {
		print_usage();
		return 3;
	}

	i = pin - '0';
	chip = gpiod_chip_open_by_name(gpio2);

	if (!chip) {
		perror("Open DI failed\n");
		goto end;
	}

	line = gpiod_chip_get_line(chip, pinnums[i]);
	if (!line) {
		perror("Get DI failed\n");
		goto close_chip;
	}

	ret = gpiod_line_request_input(line, CONSUMER);
	if (ret < 0) {
		perror("Request DI as input failed\n");
		goto release_line;
	}

	ret = 0;
	val = gpiod_line_get_value(line);
	if (val < 0) {
		perror("Read DI failed\n");
		ret = 8;
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
