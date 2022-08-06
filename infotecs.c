#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alisa V <werewolf.luner@gmail.com>" );

static int __init infotecs_init( void )
{
	printk( "Hello, world!" );
	return 0;
}
static void __exit infotecs_exit( void )
{
	printk( "Goodbye, world!" );
}

module_init( infotecs_init );
module_exit( infotecs_exit ); 
