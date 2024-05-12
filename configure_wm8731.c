#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define WM8731_ADDR 0x1a // I2C address for WM8731 (check!)
void write_register(int file, int reg, int value) {
    // WM8731 uses 9-bit register addresses
    unsigned char buffer[2];
    buffer[0] = (reg << 1) | ((value >> 8) & 0x01); // Register address and MSB of value
    buffer[1] = value & 0xFF; // LSB of value
    if (write(file, buffer, 2) != 2) {
        perror("Failed to write to the i2c bus");
    }
}

int main() {
    int file;
    char *filename = "/dev/i2c-1";

    // Open I2C bus
    if ((file = open(filename, O_RDWR)) < 0) {
        perror("Failed to open the I2C bus");
        return -1;
    }

    // Configure the I2C client
    if (ioctl(file, I2C_SLAVE, WM8731_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        close(file);
        return -1;
    }

    // Initialize WM8731
    write_register(file, 0x0F, 0x000); // Reset device
    write_register(file, 0x07, 0x001); // Enable DAC
    write_register(file, 0x0A, 0x001); // Enable the outputs
    write_register(file, 0x0C, 0x079); // Set volume for headphone
    write_register(file, 0x0E, 0x001); // Enable I2S, MCLK input
    write_register(file, 0x12, 0x008); // Clocking settings (do we need it?)
    write_register(file, 0x02, 0x017); // Audio path

    close(file);
    return 0;
}
//RUN: arm-linux-gnueabihf-gcc -o configure_wm8731 configure_wm8731.c