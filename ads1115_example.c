/*
 * SRG-3352x Expansion Mode A
 * ADC example code
 * 
 * the code modify from
 * 	https://github.com/giobauermeister/ads1115-linux-rpi
 * 
 */
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "ads1115.h"

#define	CONSUMER	"EXADC_Consumer"

static char *gpio2 = "gpiochip2";
static int addr_adc = 0x48;
static char *i2cbus = "/dev/i2c-0";
static int pinnums[4] = { 15, 16, 17, 18 };

int main(void) {
	int i, ret;
	struct gpiod_chip *chip;
	struct gpiod_line *line[4];

	if(openI2CBus(i2cbus) == -1)
	{
		return EXIT_FAILURE;
	}
	setI2CSlave(addr_adc);

	chip = gpiod_chip_open_by_name(gpio2);
	if (!chip) {
		perror("Open gpiochip failed\n");
		ret = EXIT_FAILURE;
		goto end;
	}

	// set ADC_SET0-3 to low for voltage mode
	for(i = 0; i < 4; i++) {
		line[i] = gpiod_chip_get_line(chip, pinnums[i]);
		if (!line[i]) {
			perror("Get line failed\n");
			ret = EXIT_FAILURE;
			goto close_chip;
		}
		ret = gpiod_line_request_output(line[i], CONSUMER, 0);
		if (ret < 0) {
			perror("request line as output failed.\n");
			goto release_lines;
		}
		ret = gpiod_line_set_value(line[i], 0);
		if (ret < 0) {
			perror("set line output failed.\n");
			goto release_lines;
		}
	}
	ret = EXIT_SUCCESS;
	printf("CH_0 = %.2f V | ", readVoltage(0));
	printf("CH_1 = %.2f V | ", readVoltage(1));
	printf("CH_2 = %.2f V | ", readVoltage(2));
	printf("CH_3 = %.2f V \n", readVoltage(3));

release_lines:
	for (i = 0; i < 4; i++)
		gpiod_line_release(line[i]);
close_chip:
	gpiod_chip_close(chip);
end:
	closeI2CBus();
	return ret;
}
