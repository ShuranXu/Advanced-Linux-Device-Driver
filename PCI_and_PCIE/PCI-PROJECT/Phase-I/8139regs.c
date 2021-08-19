//-------------------------------------------------------------------
//	8139regs.c
//
//	This module creates a pseudo-file (named '/proc/8139regs')
//	which allows users to view the contents of the significant
//	read/write registers in the RTL8139 programming interface.
//
//	NOTE: Written and tested with Linux kernel version 2.6.36.
//
//	programmer: ALLAN CRUSE
//	written on: 09 NOV 2010 
//-------------------------------------------------------------------

#include <linux/module.h>	// for init_module() 
#include <linux/proc_fs.h>	// for create_proc_read_entry()
#include <linux/pci.h>		// for pci_get_device() 
#include <linux/version.h> //for kernel version comparison
#include <linux/uaccess.h>

#define VENDOR_ID  0x10EC	// RealTek Semiconductor Corp
#define	DEVICE_ID  0x8139 	// RTL-8139 Network Processor

char modname[] = "8139regs";	// for displaying module name
unsigned long mmio_base;	// address-space of registers
unsigned long mmio_size;	// size of the register-space
void	*io;			// virtual address of ioremap
char legend[] = "RealTek-8139 Network Controller register-values";


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
static int eof[1];
static char sbuf[512];

static ssize_t read_proc(struct file *file, char *buf,
size_t len, loff_t *ppos)
#else
static int read_proc (char *buf, char **start, off_t offset, 
		int len, int *eof, void *unused) 
#endif
{
	if (*eof!=0) { 
		*eof=0; 
		printk("read_proc reached here\n");
		return 0; 
	}

	int	i, buf_len = 0;
	*eof = 1;

	buf_len += sprintf( buf+buf_len, "\n\n %13s %s \n", " ", legend );
	buf_len += sprintf( buf+buf_len, "\n      " );
	buf_len += sprintf( buf+buf_len, " CR=%02X   ",  readb( io + 0x37 ) );
	buf_len += sprintf( buf+buf_len, " TCR=%08X  ", readl( io + 0x40 ) );
	buf_len += sprintf( buf+buf_len, " RCR=%08X  ", readl( io + 0x44 ) );
	buf_len += sprintf( buf+buf_len, "   MAC=" );
	for (i = 0; i < 6; i++) 
	{
		buf_len += sprintf( buf+buf_len, "%02X", readb( io + i ) );
		buf_len += sprintf( buf+buf_len, "%c", (i<5) ? ':' : ' ' );
	}
	buf_len += sprintf( buf+buf_len, "\n" );

	buf_len += sprintf( buf+buf_len, "\n      " );
	buf_len += sprintf( buf+buf_len, "  TSD0=%08X ", readl( io + 0x10 ) );
	buf_len += sprintf( buf+buf_len, "  TSD1=%08X ", readl( io + 0x14 ) );
	buf_len += sprintf( buf+buf_len, "  TSD2=%08X ", readl( io + 0x18 ) );
	buf_len += sprintf( buf+buf_len, "  TSD3=%08X ", readl( io + 0x1C ) );

	buf_len += sprintf( buf+buf_len, "\n      " );
	buf_len += sprintf( buf+buf_len, " TSAD0=%08X ", readl( io + 0x20 ) );
	buf_len += sprintf( buf+buf_len, " TSAD1=%08X ", readl( io + 0x24 ) );
	buf_len += sprintf( buf+buf_len, " TSAD2=%08X ", readl( io + 0x28 ) );
	buf_len += sprintf( buf+buf_len, " TSAD3=%08X ", readl( io + 0x2C ) );
	buf_len += sprintf( buf+buf_len, "\n" );

	buf_len += sprintf( buf+buf_len, "\n      " );
	buf_len += sprintf( buf+buf_len, " RBSTART=%08X ", readl( io + 0x30 ) );
	buf_len += sprintf( buf+buf_len, " CAPR=%04X ", readw( io + 0x38 ) );
	buf_len += sprintf( buf+buf_len, " CABR=%04X ", readw( io + 0x3A ) );
	buf_len += sprintf( buf+buf_len, "   " );
	buf_len += sprintf( buf+buf_len, " ERBCR=%04X ", readw( io + 0x34 ) );
	buf_len += sprintf( buf+buf_len, " ERSR=%02X ",  readb( io + 0x36 ) );
	buf_len += sprintf( buf+buf_len, "\n" );

	buf_len += sprintf( buf+buf_len, "\n      " );
	buf_len += sprintf( buf+buf_len, " IMR=%04X  ", readw( io + 0x3C ) );
	buf_len += sprintf( buf+buf_len, " MULINT=%04X ", readw( io + 0x5C ) );
	buf_len += sprintf( buf+buf_len, "   TSAD=%04X   ", readw( io + 0x60 ) );
	buf_len += sprintf( buf+buf_len, " REC=%02X ",  readb( io + 0x72 ) );
	buf_len += sprintf( buf+buf_len, " DIS=%02X ",  readb( io + 0x6C ) );
	buf_len += sprintf( buf+buf_len, " FCSC=%02X ",  readb( io + 0x6E ) );
	buf_len += sprintf( buf+buf_len, "\n" );

	buf_len += sprintf( buf+buf_len, "\n\n" );
	
	if (copy_to_user(buf,sbuf, buf_len)) 
		return -EFAULT;

	return(strlen(buf));
}


static struct proc_dir_entry *procent;
static const struct file_operations proc_fops = {
	.read = read_proc,
	.owner = THIS_MODULE
};


static int _8139rega_init( void )
{
	struct pci_dev	*devp = NULL; 

	printk( "<1>\nInstalling \'%s\' module\n", modname );

	devp = pci_get_device( VENDOR_ID, DEVICE_ID, devp );
	if ( !devp ) return -ENODEV;

	mmio_base = pci_resource_start( devp, 1 );

	if(!mmio_base || ((pci_resource_flags(devp, 1) & IORESOURCE_MEM) == 0)){
		dev_err(&devp->dev, "no Memory resource at PCI BAR #1\n");
		return -1;
    }

	mmio_size = pci_resource_len( devp, 1 );
	printk( "  RealTek 8139 Network Interface:" ); 
	printk( "  %08lX-%08lX \n", mmio_base, mmio_base+mmio_size );

	io = ioremap( mmio_base, mmio_size );
	if ( !io ){
		dev_err(&devp->dev, "iomap failed\n");
		return -ENOSPC;
	} 

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
	create_proc_read_entry( modname, 0, NULL, my_proc, NULL );
#else
	procent = proc_create(modname, 0777 , NULL, &proc_fops);
#endif 

	if (procent == NULL) {
		printk("proc_create failure\n");
		return -1;
	}
	printk(KERN_ALERT "8139regs initialized!\n");
	return	0;  // SUCCESS
}

static void _8139rega_exit( void )
{
	remove_proc_entry( modname, NULL );
	// iounmap( io );
	printk( "<1>Removing \'%s\' module\n", modname );
}

MODULE_LICENSE("GPL");
module_init(_8139rega_init);
module_exit(_8139rega_exit);