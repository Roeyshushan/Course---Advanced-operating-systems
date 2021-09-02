/*
 * Roey Shushan - 312577919
 * Bar Pinian - 313347585
*/

#include <stdio.h> // For the printing.
#include <unistd.h> // For reading and closing the file.
#include <fcntl.h> // For the file opening.
#include <string.h> // For the strcpy.

#define MAX_PKT_SIZE 1024 // Constant size for the path of the file.

/* Our user program main function. */
int main(int argc, char** argv)
{
	int fd; // Integer representation for our file opening.
	char buff[MAX_PKT_SIZE]; // Our buffer.
	char devicePath[100]; // The path of the created file by our kobject within the driver.

	strcpy(devicePath, "/sys/kernel/keylogger/data_attr"); // Initialization of the file path.
	fd = open(devicePath, O_RDONLY); // Opening the file (our keylogger file).
	
	if(fd == -1) // If error was returned by the file opening.
	{
		printf("error file open"); // Printing error message.
		return -1; // Returning the integer representation of the error.
	}

	read(fd,buff,sizeof(buff)); // Reading from the file (our keylogger file).
	printf("The keylogger file content: \n%s\n", buff); // Printing the content within the keylogger file.
	close(fd); // Closing the opened file.
}
	
