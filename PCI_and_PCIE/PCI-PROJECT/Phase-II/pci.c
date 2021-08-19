/* 
 * Realtek RTL8139 PCI Ethernet Wired card Driver template.
 * 
 * Implement PCI portion of rtl8139 Network Card driver
 * Use this as a template. Add code in areas matching string "CODE HERE".  
 * In this phase of the project we will be writing PCI routines. 
 * Net Device and ISR routines will be implemented in PHASE 2.
 * Compile the driver as module. Use the makefile provided earlier
 * Unload the production module or blacklist it: 
 * # lspci -v  -- This will show any module or driver bound to this card
 * # rmmod 8139too -- Unload the production driver
 * # lsmod - list loaded modules
 * Load the driver after adding required code: 
 * # insmod pci.ko
 * Run "ifconfig -a", You should see: 
 * - MAC Address read from the device memory
 * - IRQ number read from the device memory
 * - Device IO memory base address read from the device memory
 *
 * Guidelines are provided to assist you with writing the pci portion of the 
 * driver. Do not limit yourself to it. You are encouraged to review source 
 * code of production driver. 
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/cache.h>
#include <linux/pci.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/iomap.h>
#include <linux/ioport.h>
#include <linux/dma-mapping.h>
#include "rtl8139.h"


#define NETDEV_NAME 					"rtl8319"
#define BAR0                            0
#define BAR1                            1
#define CP_REGS_SIZE		        	(0xff + 1)
#define privr32(reg)	                ioread32(priv->mmio_addr + (reg))
#define privw32(reg,val)	        	iowrite32((val), priv->mmio_addr + (reg))


/**
  * PCI DEVICE REGISTRATION.  
  * pci_device_id describes to the PCI layer the devices that this driver can handle
  * PCI_ANY_ID means anything matches (Wild card), PCI layer automatically
  * calls your probe function for any matching device. 
  * You can find the device and vendor ID using lspci -n
  * Array is zero-terminated
  */

static struct pci_device_id rtl8139_table[] = {
        { PCI_DEVICE(VENDOR_ID,   DEVICE_ID), },
        { 0, }
};

/** 
  * This marks the pci_device_id table in the module image. This 
  * information loads the module on demand when the PCI card is 
  * hot plugged into the PCMCIA slot. It is part of module autoload 
  * mechanism supported as part of Linux Device Model 
  *
  * Production ready Network modules are located in direcotory:
  * /lib/modules/2.6.../kernel/drivers/net
  * rtl8139 production driver: 8139cp.ko, 8139too.ko
  *
  * File where driver and device mapping is defined: 
  * /lib/modules/2.6../modules.pcimap
  * 
  * Entry for rtl8139 production driver: 
  * 
  *   pci module         vendor     device     subvendor  subdevice .. 
  *    8139cp            0x000010ec 0x00008139 0xffffffff 0xffffffff ..
  *    8139too           0x000010ec 0x00008139 0xffffffff 0xffffffff ..
  * 
  * module.alias file identifies what driver to use for the device. 
  * File is generated by depmod utility: 
  * alias pci:v000010ECd00008139sv*sd*bc*sc*i* 8139cp
  * alias pci:v000010ECd00008129sv*sd*bc*sc*i* 8139too
  *
  * modules.dep file - Module dependencies and tells where the 
  * required binary is located 
  * rtl8139 production driver:  8139cp.ko and 8139too.ko 
  * depend on mii.ko module
  * kernel/drivers/net/8139cp.ko: kernel/drivers/net/mii.ko
  * kernel/drivers/net/8139too.ko: kernel/drivers/net/mii.ko
  * 
  * See discussion in lecture: Linux Device Model - Module 9
  */

MODULE_DEVICE_TABLE(pci, rtl8139_table);



// static int __devinit 
// rtl8139_probe(struct pci_dev *pdev, const struct pci_device_id *id);
// static void __devexit rtl8139_remove( struct pci_dev *pdev );
static int rtl8139_probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void rtl8139_remove( struct pci_dev *pdev );
static int rtl8139_open(struct net_device *dev);
static int rtl8139_stop(struct net_device *dev);
static int rtl8139_start_xmit(struct sk_buff *skb, struct net_device *dev);
static struct net_device_stats* rtl8139_get_stats(struct net_device *dev);
static void rtl8139_hardware_start(struct net_device *netdev);
static irqreturn_t rtl8139_interrupt (int irq, void *dev_instance);



/** 
  * PCI driver hooks and supported devices table 
  * pci_register_driver and pci_unregister_driver use struct pci_driver
  * as arguments to register and unregister the pci driver.
  */

static struct pci_driver rtl8139_driver = {
        .name           = DRV_NAME,
        .id_table       = rtl8139_table,
        .probe          = rtl8139_probe,
        .remove         = rtl8139_remove,
 	// .remove         = __devexit_p (rtl8139_remove),

	/* We won't be implementing PCI suspend and resume routines  */
      //.suspent	= rtl8139_suspend,  
      //.resume		= rtl8139_resume, 
};

/**
  *  2.6.29 and above: 
  *  netdevice functions are moved out from netdevice structure 
  *  into net_device_ops 
  */

// #ifdef HAVE_NET_DEVICE_OPS
// static struct net_device_ops rtl8139_netdev_ops = {
//         .ndo_open               = rtl8139_open,
//         .ndo_stop               = rtl8139_stop,
//         .ndo_get_stats          = rtl8139_get_stats,
//         .ndo_start_xmit         = rtl8139_start_xmit
// };
// #endif

static struct net_device_ops rtl8139_netdev_ops = {
        .ndo_open               = rtl8139_open,
        .ndo_stop               = rtl8139_stop,
        .ndo_get_stats          = rtl8139_get_stats,
        .ndo_start_xmit         = rtl8139_start_xmit
};

/***************** PCI ROUTINES*********************/
/**
  * 1- Enable the device
  * 2- Extract the physical address where the device IO memory 
  *    is mapped from the config space 
  * 3- Claim the device IO memory region. It takes start address and 
  *    length of IOMEM region
  * 4- Remap the device IO Memory region. Routine takes start and length 
  *    of IOMEM region
  * 5- Enable DMA processing engine
  * 6- Allocate net_device structure of type ether
  * 7- sysfs hooks 
  * 8- save IO memory region address into the device private and 
  *    netdevice struct 
  */

// static int __devinit rtl8139_probe(struct pci_dev *pdev, const struct pci_device_id *id)
static int rtl8139_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    struct net_device *dev;
    rtl8139_t *priv;
	int i;
    int err;
	unsigned long mmio_start, mmio_end, mmio_len, mmio_flags;
    void __iomem *ioaddr;

    printk(KERN_INFO "pci_template: %d\n",__LINE__);
	
	/* Enable the device first. This wakes up the device if suspended. */
	
	/* CODE HERE */
	i = pci_enable_device(pdev);
	if(i){
			dev_err(&pdev->dev, "Failed to enable pci device\n");
			return -1;
	}
			
	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	i = pci_set_mwi(pdev);
	if (i){
			dev_err(&pdev->dev, "Failed to enable PCI Memory-Write-Invalidate\n");
			goto disable;
	}

	printk(KERN_INFO "rtl8139_probe() is called: %d\n",__LINE__);
		
    /** 
	  * Enable bus mastering of the device. This will set bus master bit 
	  * in the PCI_COMMAND register. Device now can act as a master on 
	  * the address bus 
	  */

    /* CODE HERE */ 
    pci_set_master(pdev);

	/**
	  * PCI has API to access PCI configuration space such as:
	  *   - pci_resource_start
          *   - pci_resource_end
          *   - pci_resource_len  
          *   - pci_resource_flag
	  *
          * These routines can help you to extract physical address 
          * where device MMIO region mapped: start, end, length of the 
          * region and flag. Flag provides a hint to driver about the 
          * resource being cachable or not. 
	  * Start and size of device MMIO region is required in 
          * order for ioremap() to create page table entries 
          * (virtual addresses) to access device IOMEM area.
	  * 
	  * Once the device memory is mapped, drivers can read and 
          * write to PCI  device's  MMIO region using bus independent 
	  * IO API read(b|w|l), write(b|w|l) or wrapper routines: 
	  * ioread/iowrite.
          * PCI BAR: PCI BASE ADDRESS REGISTER.
          * Where: BAR 0 is IOAR, BAR 1 is MEMAR. 
          * Since we will be using memory-mapped I/O (MMIO), we will pass 
	  * the second argument as 1 to pci routines mentioned above.  
	  */

	/* CODE HERE */
	mmio_start = pci_resource_start(pdev, BAR1);

	mmio_len = pci_resource_len(pdev, BAR1); 
	if (mmio_len < CP_REGS_SIZE) {
		i = -EIO;
		dev_err(&pdev->dev, "MMIO resource (%llx) too small\n",
				(unsigned long long)mmio_len);
		goto mwi;
	}

    /**
	* Test if pci BAR 1 is really device MMIO region 
	* otherwise, goto disable the device
	*/

	/* CODE HERE */
	if(!mmio_start || ((pci_resource_flags(pdev, BAR1) & IORESOURCE_MEM) == 0)){
			dev_err(&pdev->dev, "no Memory resource at PCI BAR #1\n");
			goto mwi;
	}
        
    /** claim or take ownership of the IO Memory region. If fail goto 
 	*  disable the device
 	*/

	/* CODE HERE */
	if(request_mem_region(mmio_start, mmio_len, DRV_NAME) == NULL){
			dev_err(&pdev->dev, "I/O resource 0x%lx @ 0x%lx busy\n",
			mmio_len, mmio_start);
			goto mwi;
	}

	printk(KERN_INFO "pci_template: %d\n",__LINE__);

    /** 
	  * ioremap Device MMIO region: ioremap (address, size) function must 
	  * be called to map the device memory into a virtual memory address. 
	  * This function does not allocate any memory, it sets up the 
	  * page tables. Very simply, it adds the needed entries into the page 
	  * tables for device MMIO range (size). If you forget to do it and 
	  * access the memory, then MMU will indicate a request for an invalid 
	  * memory address, causing an invalid page fault and kernel may panic.
	  * Once the memory is remapped with ioremap, a bit of care should be 
	  * taken and avoid the temptation of de-referencing the address.
          * Use ioread/iowrite|b|w|l() instead.
          * goto release in case of ioremap() failure to provide valid 
	  * virtual address
    */

	 /* CODE HERE */
	ioaddr = ioremap(mmio_start, mmio_len);
	if(ioaddr == NULL){
		goto release;
	}
	
	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	/**
    	* Check if 32-bit DMA capability is supported on this platform
        * Use for declaring any device with more (or less) than 32-bit 
		* bus master capability
	  	* goto unmap if unable to set DMA 32 bit mask
    */

	/* CODE HERE */
	err = pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
	if(!err)
			err = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	if(err){
			printk(KERN_WARNING DRV_NAME ": No suitable DMA available.\n");
			goto release;
	}

	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	/** 
	  * Linux Network Stack works with network device not the PCI device. 
	  * We need to allocate an ethernet network device. alloc_etherdev() 
	  * allocates net device structure with memory allocated for 
	  * device private structure. You can access rtl839 private struct 
	  * by using netdev_priv(dev). unmap, in case of failure
	  */

	 /* CODE HERE */
	dev = alloc_etherdev(sizeof(struct rtl8139));
	if(!dev){
		dev_err(&pdev->dev, "Failed to allocate an ethernet network device.\n");
		goto unmap;
	}
		
	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	/** 
 	 * Set the device name to DRV_NAME instead of eth via memcpy 
 	 */ 

	/* CODE HERE */
    memcpy(dev->name, NETDEV_NAME, strlen(NETDEV_NAME));


 	/* sysfs stuff. Sets up device link in /sys/class/net/interface_name */
	SET_NETDEV_DEV(dev, &pdev->dev);

	/**
	  *  Set up information in the device private structure such as 
	  *  mmio_addr, regs_len, pci_dev and initialize spinlock
	  */

	/* CODE HERE */
	priv = netdev_priv(dev);

	spin_lock_init(&priv->lock);
	priv->mmio_addr = ioaddr;
	priv->regs_len = mmio_len;
	priv->pci_dev = pdev;  
	priv->stats = dev->stats;

	/**
	  * Fill in the net device with MMIO address and irq obtained 
	  * from the PCI configuration space 
      * These fields are base_addr and irq field in net_device structure. 
	  * Ifconfig reads these values from net device structure and print it
	  * PCI device gets IRQ assigned automatically - No poking, probing 
	  * guessing needed. 
	  * 
      * Assigned IRQ number is placed in pci pdev structure (pdev->irq) 
      * passed as argument to PCI device probe.
    */

	/* CODE HERE */
	dev->irq = pdev->irq;
	dev->base_addr = mmio_start;

	/**
          * Interface address: MAC and Broadcast Address
          * RealTek8139 datasheet states that the first 6 bytes of ioaddr 
          * (offset 0x0) contain the hardware address of the device. We need to 
          * fill the net device with MAC and broadcast address. We need to 
          * fill net device dev->dev_addr[6] and dev->broadcast[6] arrays.
          * For broadcast address fill all octets with 0xff. Use readb (or 
          * ioreadb) to read from IO memory address, ioaddr
	  */

	// addr_len = read_eeprom (regs, 0, 8) == 0x8129 ? 8 : 6;
	// for (i = 0; i < 3; i++)
	// 	((__le16 *) (dev->dev_addr))[i] =
	// 	    cpu_to_le16(read_eeprom (regs, i + 7, addr_len));

	/* CODE HERE */
	memset(dev->broadcast, 0xff, 6);
	
	for(i=0; i<3; i++){
			((__le16 *) (dev->dev_addr))[i] =
		cpu_to_le16(ioread16(ioaddr + i));

	}

	printk(KERN_INFO "pci_template: %d\n",__LINE__);

    /* Length of Ethernet frame. It is a "hardware header length", number 
 	 * of octets that lead the transmitted packet before IP header, or 
 	 * other protocol information.  Value is 14 for Ethernet interfaces.
    */

    dev->hard_header_len = 14;

    /** 
	 *  fill in the net device with our device methods that we will write 
	 *  for our driver in phase 2. 
	 *  Required methods are: open, stop, hard_start_xmit, getstats
         *  2.6.29 and above: netdev_ops in netdevice structure points to 
	 *  net_device_ops containing netdevice methods
	 */

// #ifdef HAVE_NET_DEVICE_OPS
// 	priv->net_dev->netdev_ops = &rtl8139_netdev_ops;
// #else
// 	priv->net_dev->netdev_ops->ndo_open = rtl8139_open;
// 	priv->net_dev->netdev_ops->ndo_stop = rtl8139_stop;
// 	priv->net_dev->netdev_ops->ndo_start_xmit = rtl8139_start_xmit;
// 	priv->net_dev->netdev_ops->ndo_get_stats = rtl8139_get_stats;
// #endif

    dev->netdev_ops = &rtl8139_netdev_ops;

	/* Finally register the net device. An unused ethernet interface 
	* is alloted
	*/

	/* CODE HERE */
	if(register_netdev(dev)){
			dev_err(&pdev->dev, "Failed to register net device\n");
			goto freedev;
	}

	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	/**
	 * You can stuff net device structure into pci_driver structure
	 * using pci_set_drvdata( struct pci_driver *, void *)  That can 
	 * be retrieved later using pci_get_drvdata(struct pci_driver *)
	 * for example, in remove or other pci functions
	*/

	/* CODE HERE */	
	pci_set_drvdata(pdev, (void*)dev);

	printk(KERN_INFO "pci_template: %d\n",__LINE__);

	return 0;

	/**
	 * cleanup on failure. goto is a better way to deal with serious 
	 * error conditions
	 */

freedev:
    free_netdev(dev);

unmap:
	pci_iounmap(pdev, ioaddr);

release:
	pci_release_regions(pdev);

mwi:
    pci_clear_mwi(pdev);

disable:
	pci_disable_device(pdev);
	return (-ENODEV);
}


/**************** Net device routines ******************************/

static irqreturn_t rtl8139_interrupt (int irq, void *dev_instance)
{
    struct net_device *netdev = (struct net_device*)dev_instance;
    rtl8139_t *priv = netdev_priv(netdev);
    void *__iomem ioaddr = priv->mmio_addr;
    unsigned short isr;
    unsigned int txstatus;
	int handled = 0;

	printk("Entering %s\n", __FUNCTION__);

    /**
	* Read ISR register value in isr variable
	* Clear all interrupts. Reading the ISR register should do it.
	* In some case it does n't work. To be sure, write 0xfff on
	* the ISR register to clear the interrupts
	*/

	/* CODE HERE */
	isr = ioread16(ioaddr + ISR);
	WRITEW_F(0xfff, ioaddr + ISR);

	/* unknown Interrupt Type */
	if (unlikely((isr & INT_MASK) == 0))
		goto out;

	handled = 1;
		
	// Hardware no longer present (hotplug?) or major error, bail out
	if (unlikely(isr == 0xFFFF))
		goto out;	

	printk("%s: interrupt status=%4.4x.\n", netdev->name, isr);

	/* Check type of interrupt received. TxOK and TxErr */
	if((isr & TxOK) || (isr & TxErr))
	{
		/**
		 *  Continue processing if there are transmit buffers and 
		 *  queue is not flow controlled 
		 */

		while((priv->dirty_tx != priv->cur_tx) || netif_queue_stopped(netdev))  // why use this condition ?
		{
			/**
			 * TSD0-3 contains the status of transmit packet
			 * TSAD0-3 contains physical address of the packet in memory
			 */

			txstatus = readl(ioaddr + TSD0 + priv->dirty_tx * sizeof(int));

			// Update the stats counter for TX collisions */
			priv->stats.collisions += (txstatus >> 24)&0xF;
	
			//check whether transmission has been concluded
			if(!(txstatus & (TxStatOK | TxAborted | TxUnderrun)))
				break; /*That means the packet is still sitting there */

			if(txstatus & TxStatOK) { /* Successfully transmitted */
				printk("Packet is transmitted, TxStatOK bit is set\n");

				/**
				* Bits 0-12 of TSD0-3 contains total bytes in the this 
				* descriptor. We are setting 0x1fff mask that tests 0-12 
				* bits of 32 bit value. Update the stats tx_bytes and 
				* tx_packets 
				*/
				
				/* CODE HERE */
				priv->stats.tx_bytes += txstatus & 0x1fff;
				priv->stats.tx_packets ++;

			}
			else {
					printk ("%s: Transmit error\n");
					/* Update tx_errors stat */

					/* CODE HERE */
					priv->stats.tx_errors ++;

					/* update detailed TX error-counters */
					if ( txstatus & TxAborted )
						priv->stats.tx_aborted_errors++;
					if ( txstatus & TxUnderrun )
						priv->stats.tx_fifo_errors++;
					if ( txstatus & TxOutOfWindow )
						priv->stats.tx_window_errors++;
					if ( txstatus & TxCarrierLost )
						priv->stats.tx_carrier_errors++;
				}
                 
			// Point dirty_tx to next transmit descriptor 
		    priv->dirty_tx = (priv->dirty_tx + 1) % NUM_TX_DESC;

			if((priv->dirty_tx == priv->cur_tx) 
			& netif_queue_stopped(netdev))
			{
				printk("wake up queue\n");
				netif_wake_queue(netdev);
			}
        }
    }

    /**
      * Data is moved from device to receive buffer. After the whole 
	  * packet is transferred to receive Buffer, the receive packet 
	  * header (receive status and packet length) is written in front 
	  * of the packet.
    */

   	/**
   	 * The following code is based on RxInterruptHandler( )
   	 *  in REALTEK Chip design & System design manual.
   	 */

    if(isr & RxErr) {
        printk("\nReceive error \n");
	 	/* Update rx_errors stats */
	 
	  	/* CODE HERE */
		priv->stats.rx_errors ++;
    }

    if(isr & RxOK) {
        printk("Interrupt of type receive\n");

	 	/* Test CR register against RxBufEmty to see if driver buffer is empty */

		while((readb(ioaddr + CR) & RxBufEmpty) == 0)
		{
			unsigned int rx_status;
			unsigned short rx_size;
			unsigned short pkt_size;
			struct sk_buff *skb;

			//perform address-wrap if at end-of-buffer
			if(priv->cur_rx > RX_BUF_LEN)
				priv->cur_rx = priv->cur_rx % RX_BUF_LEN;

			/**
			 * Receive status and packet length is stored in the 
			 * beginning of the packet. It should be converted to 
			 * host (little/big) endian.
			 * 
			 * First two bytes are received status and next two
			 * bytes are the frame length.
			 */

			rx_status = *(u32 *)(priv->rx_ring + priv->cur_rx);
			rx_size = rx_status >> 16;

			/* first two bytes are receive status register
			* and next two bytes are frame length
			* Packet follows these initial four bytes
			*/

			pkt_size = rx_size - 4;

			/* allocate skb buffer to copy the packet */

			/* CODE HERE */
			skb = dev_alloc_skb(pkt_size + NET_IP_ALIGN);
			if (skb) {

					skb->dev = netdev;

					/** 
					 * Reserve the necessary bytes at the head of the 
					 * buffer to land the IP header on a long word 
					 * boundary. The existing ethernet drivers thus
					 * reserve 2 bytes extra to land IP headers on a 16 
					 * byte boundary, which is  also the start of a cache 
					 * line and help improve performance on some platform
					*/

					/* CODE HERE */
					skb_reserve(skb, NET_IP_ALIGN);

					/**
					* copy receive buffer into skb via memcpy 
					* Packet is located after 4 bytes of status
					* register and frame length:
					* priv->rx_ring + priv->cur_rx + 4
					* Size of the packet is pkt_size = rx_size -4 to exclude 4 byte CRC
					*/
						
					/* CODE HERE */
					memcpy(skb->data, priv->rx_ring + priv->cur_rx + 4, rx_size - 4);
					skb_put(skb, pkt_size);

					skb->protocol = eth_type_trans(skb, netdev);

					/* hand skb to the protocol layer */

					/* CODE HERE */
					netif_rx(skb);

					/* Update stats: rx_bytes and rx_packets; */

					/* CODE HERE */
					priv->stats.rx_bytes += rx_size - 4;
					priv->stats.rx_packets++;
			}
			else {
					printk (KERN_WARNING "%s: dropping packet.\n", netdev->name);
					
					/* CODE HERE */
					priv->stats.rx_dropped++;

					/* Update detailed RX error-counters */
					if ( rx_status & (1 << 15) )
							priv->stats.multicast++;
					if ( rx_status & ((1 << 4)|(1 << 3)) )
							priv->stats.rx_length_errors++;
					if ( rx_status & (1 << 2) )
							priv->stats.rx_crc_errors++;
					if ( rx_status & (1 << 1) )
							priv->stats.rx_frame_errors++;
				}

			/**
			 * update priv->cur_rx to next writing location.
			 * 4: for header length, packet length includes 4 bytes CRC
			 * 3: for dword alignment
			 */

			priv->cur_rx = (priv->cur_rx + rx_size + 4 + 3) & ~3;

			/* update CAPR. CAPR register keeps track of data driver has read */
			WRITEW_F((u16)(priv->cur_rx-16), ioaddr + CAPR); //-16: avoid overflow --> ? 
		}
    }

	if(isr & CableLen)
			printk("cable length change interrupt\n");
	if(isr & TimeOut)
			printk("time interrupt\n");
	if(isr & SysErr)
			printk("system err interrupt\n");

	printk("Exiting %s\n", __FUNCTION__);

out:
	/* What is the last thing that Interrupt handler does when returning */
	printk ("ISR:%s: exiting interrupt, intr_status=%#4.4x.\n",
                 netdev->name, readw(ioaddr + ISR));
	
    /* CODE HERE */
	return IRQ_RETVAL(handled);
}


// static int rtl8139_open(struct net_device *dev) 
// { 
// 	printk("rtl8139_open: Add code later\n"); 
// 	netif_start_queue(dev); /* transmission queue start */
// 	return 0; 
// }

static int rtl8139_open(struct net_device *netdev)
{
    printk("rtl8139_open:\n");

	/* Get address of private structure using netdev_priv(netdev) */

	/* CODE HERE */
	struct rtl8139 *priv; 
	int rc;
	int i;
	priv = netdev_priv(netdev);
	const int IRQ = priv->pci_dev->irq;
    
	/**
	* Request IRQ. 
	* Device initialization is NOT a good place to 
	* request an IRQ because they can hog a valuable resource 
	* (limited IRQ lines on x86 PC). Therefore best place to request 
	* is when application open the device and free it when application 
	* closes it
	* 
	* Arguments are:
	*    1- irq number
	*    2- Name of the device interrupt routine
	*    3- PCI devices support shared IRQ
	*    4- Name of the device, in our case it is rtl8139
	*    5- If shared IRQ is set, then last argument identify the device 
	*       sharing the IRQ
	*/

	/* CODE HERE */
	rc = request_irq(IRQ, rtl8139_interrupt, IRQF_SHARED, netdev->name, netdev);
	if (rc)
		return -1;

    /**
    	* Allocate consistent DMA buffers for trasmit 
		* and receive. pci_alloc_consistent guarantees data 
		* coherency when DMA is performed. Coherent mapping allows 
        * simultaneously access to buffer by both cpu and device. 
		* Can be expensive to setup and use. Usually allocated for the whole 
		* time module is loaded. Function generates a bus address 
		* pointed by tx_bufs_dma and rx_ring_dma in rtl8139_private 
		* structure. These addresses are used by the PCI device. tx_bufs and 
		* rx_ring are used by driver for DMA.
        * Other arguments are: 
          *  -  PCI device structure 
          *  -  Size of DMA - TOTAL_TX_BUF_SIZE, TOTAL_RX_BUF_SIZE
	  	* if failed free coherent buffer by calling dm_free_coherent()
        * and return -ENOMEM
    */
	
	/* CODE HERE */
	priv->tx_bufs = pci_alloc_consistent(priv->pci_dev,TOTAL_TX_BUF_SIZE, &priv->tx_bufs_dma);
	if(!priv->tx_bufs){
		dev_err(&priv->pci_dev->dev, "no Memory resource allocated for tx buffer\n");
		goto irq;
	}

	priv->rx_ring = pci_alloc_consistent(priv->pci_dev, TOTAL_RX_BUF_SIZE, &priv->rx_ring_dma);
	if(!priv->rx_ring){
		dev_err(&priv->pci_dev->dev, "no Memory resource allocated for rx buffer\n");
		goto dma_tx;
	}

    /**
	* Initialize the receive (cur_rx) and trasmit (dirty_tx) descriptors
	* in rtl8139 private structure. These keep track of receive buffer 
	* pointer and transmit descriptors  
    */

	priv->cur_rx = 0;
    priv->dirty_tx = 0;
	priv->cur_tx = 0;

	/* There are total of four transmit buffers that will be 
	 * using a single DMA buffer tx_bufs allocated above. 
	 */ 

	for (i = 0; i < NUM_TX_DESC; i++)
            priv->tx_buf[i] = &priv->tx_bufs[i * TX_BUF_SIZE];

	/* Initialize the hardware to make sure it is ready*/

    rtl8139_hardware_start(netdev);

	/* Notify the protocol layer so that it can start sending packet */

	/* CODE HERE */
	netif_start_queue(netdev);

	printk("Exiting %s\n", __FUNCTION__);

    return 0;


dma_tx:
	pci_free_consistent(priv->pci_dev, TOTAL_TX_BUF_SIZE, priv->tx_bufs, priv->tx_bufs_dma);
	return -ENOMEM;

irq:
	free_irq(IRQ,(void *)(rtl8139_interrupt));

	return -1;
}

// static int rtl8139_stop(struct net_device *dev) 
// {
//     printk("rtl8139_stop: Add code later \n");
// 	netif_stop_queue(dev); /* transmission queue stop */
//     return 0;
// }

static int rtl8139_stop(struct net_device *netdev)
{
	/* Get address of private structure and ioaddr */

	/* CODE HERE */
    unsigned long flags;
	struct rtl8139 *priv; 
	priv = netdev_priv(netdev);
	void *__iomem ioaddr = priv->mmio_addr;

	/* Notify protocol layer not to send any more packet to this interface */

	/* CODE HERE */
	netif_stop_queue(netdev);

	printk("Entering %s\n", __FUNCTION__);

    printk ("\nrtl8139_stop: shuting down the interface");

	/* Serialize access by calling spin_lock_irqsave */

	/* CODE HERE */
	spin_lock_irqsave(&priv->lock, flags);

    /* Stop the chip's Tx and Rx DMA */
	
	/* CODE HERE */
	WRITEB_F(0, ioaddr + CR);

    /* Disable all interrupts by clearing the interrupt mask. */

    /* CODE HERE */
	WRITEW_F(0,ioaddr + IMR);

	/* Update the error counts. */
	netdev->stats.rx_missed_errors += readl(ioaddr+MPC);
	writel(0, ioaddr + MPC);

	/* Release spin lock */

	/* CODE HERE */
	spin_unlock_irqrestore(&priv->lock, flags);

	/* Free irq */

    /* CODE HERE */
	free_irq(priv->pci_dev->irq, netdev);

	/* Free transmit and recieve consistent buffers */

	/* CODE HERE */
	pci_free_consistent(priv->pci_dev, TOTAL_TX_BUF_SIZE, priv->tx_bufs, priv->tx_bufs_dma);
	pci_free_consistent(priv->pci_dev, TOTAL_RX_BUF_SIZE, priv->rx_ring, priv->rx_ring_dma);
	priv->rx_ring = NULL;
	priv->tx_bufs = NULL;

	printk("Exiting %s\n", __FUNCTION__);

    return 0;
}


// static int rtl8139_start_xmit(struct sk_buff *skb, struct net_device *dev) 
// {
//     printk("rtl8139_start_xmit: Add code later\n");
// 	dev_kfree_skb(skb); /* Just free it for now */
//     return 0;
// }

static int rtl8139_start_xmit(struct sk_buff *skb, struct net_device *netdev)
{
	/* Get address of private struct and ioaddr */
	
	/* CODE HERE */
	struct rtl8139 *priv; 
	int flags;

    priv = netdev_priv(netdev);
    unsigned int entry; /* entry points to transmit descriptor used */
	unsigned int len;
	
	len  = skb->len;

    printk("Entering %s\n", __FUNCTION__);

	/* Use the next available transmit descriptor. */
	entry = (priv->cur_tx) % NUM_TX_DESC;

    if (skb->len < TX_BUF_SIZE) {
		/* 
		* Copy skb->data to DMA buffer of the current transmit descriptor
		*  and then free the skb  
		*/

		/* CODE HERE */
		if(len < ETH_MIN_LEN)
				memset(priv->tx_buf[entry], 0, ETH_MIN_LEN); /* do the padding */
		skb_copy_and_csum_dev(skb, priv->tx_buf[entry]);
		dev_kfree_skb(skb);

	} else {
		// Discard oversized packet by freeing it and updating tx_dropped

		/* CODE HERE */
		printk("%s: Warning, skb-> (%d) > %d!\n", __FUNCTION__, 
				skb->len, TX_BUF_SIZE);
		dev_kfree_skb(skb);
		netdev->stats.tx_dropped++;
		return 0;
	}

	spin_lock_irqsave(&priv->lock, flags);

	/*
	 * Writing to TxStatus triggers a DMA transfer of the data
	 * copied to tp->tx_buf[entry] above. Use a memory barrier
	 * to make sure that the device sees the updated data.
	 */

	wmb();
	
	/* Fill the size of the packet in the status register TSD0-3 */
	WRITEL_F(max(len, (unsigned int)ETH_MIN_LEN),
                       priv->mmio_addr + TSD0 + (entry * sizeof (u32)));

    // Adjust the cur_tx pointer to next tranmit descriptor  

	/* CODE HERE */
	priv->cur_tx = (priv->cur_tx + 1) % NUM_TX_DESC;

	printk("%s cur_tx = %lu\n",__func__,(long unsigned int) priv->cur_tx);

	/**
	 * If hardware is unable to accept the packet (no transmit descriptor),
	 * it should call netif_stop_queue() to cause the protocol layer to stop
	 * sending packets.
	 */
	if(priv->cur_tx == priv->dirty_tx) {
		printk("%s: cur_tx == dirty_tx\n", __FUNCTION__);
		netif_stop_queue(netdev);
	}
	
	spin_unlock_irqrestore(&priv->lock, flags);

	printk("%s: Queued Tx packet at %p size %u to slot %d.\n",
           netdev->name, skb->data, skb->len, (entry));

    return 0;
}


static void __rtl8139_get_stats(struct rtl8139 *priv)
{
	
}


static struct net_device_stats * rtl8139_get_stats(struct net_device *dev) 
{
    printk("rtl8139_get_stats: Add code later\n");

    /**
	 * You cannot return NULL, make sure to return the address 
	 * of net_dev_stat that is in device private structure
	 */

    struct rtl8139 *priv; 
	unsigned long flags;

	/* The chip only need report frame silently dropped. */
    priv = netdev_priv(dev);
	spin_lock_irqsave(&priv->lock, flags);
 	if (netif_running(dev) && netif_device_present(dev))
 		__rtl8139_get_stats(priv);
	spin_unlock_irqrestore(&priv->lock, flags);

	return &dev->stats;
}

static void rtl8139_hardware_start(struct net_device *netdev)
{
	/* Get address of device private structure and ioaddr */

	/* CODE HERE */
	struct rtl8139 *priv; 
	void __iomem *ioaddr;
	int i;

	priv = netdev_priv(netdev);
	ioaddr = priv->mmio_addr;


	printk("Entering %s\n", __FUNCTION__);

    /********* Start of hardware specific code ****************/

    /* Reset the chip. Make sure to wait for chip to reset */

	/* CODE HERE */
	WRITEB_F(CmdReset, ioaddr + CR);
	/* wait for chip to finish the reset */
	udelay(100);

	for(i = 1000; i > 0; i--) {
		if((readb(ioaddr + CR) & CmdReset) == 0) /* chip reset successfully */
			break; 
	}

	/* Enable Tx/Rx with flush*/
	WRITEB_F(( readb(ioaddr + CR) & ChipCmdClear ) | CmdTxEnb | CmdRxEnb, ioaddr + CR);
	 
	/**
	* Tx config. Update Transmit Configuration Register TCR
	* Maximum PCI burst is '6' and that is equal to 1024
	*/

	WRITEL_F( 6 << TCR_DMA_BURST_SHIFT , ioaddr + TCR ); 

    /* Rx config. update receive configuration register RCR */
	WRITEL_F(((1 << RCR_RBLEN_SHIFT) | (7 << RCR_MXDMA_SHIFT) | 
	 	(1 << RCR_WRAP_SHIFT) | (1 << RCR_AB_SHIFT) | 
		(1 << RCR_AM_SHIFT) | (1 << RCR_AAP_SHIFT)), ioaddr + RCR);

	printk("RCR=0x%x!\n", readl(ioaddr + RCR));

    /* init Tx buffer DMA addresses. Write tx_bufs_dma (bus address) in TSAD0-3 */
    for (i = 0; i < NUM_TX_DESC; i++) {
	  WRITEL_F( priv->tx_bufs_dma + (priv->tx_buf[i] - priv->tx_bufs), 
		&(((u32*)(ioaddr + TSAD0))[i]) );
	 }
       
    /* Do the same for receive DMA address by writing into RBSTART - receive buffer start address */ 

	/* CODE HERE */	
	WRITEL_F(priv->rx_ring_dma, ioaddr + RBSTART);

    /* initialize missed packet counter */

	/* CODE HERE */
	WRITEL_F(0, ioaddr + MPC);

    /* no early-rx interrupts */
    writew((readw(ioaddr + MULINT) & 0xF000), ioaddr + MULINT);

    /* Enable all known interrupts by setting the interrupt mask and flush it.*/

	/* CODE HERE */
	WRITEW_F(INT_MASK, ioaddr + IMR);

    /******* End of Hardware Specific code ************/
}

/* PCI remove routine - required else can't rmmod */
// static void __devexit rtl8139_remove( struct pci_dev *pdev )
static void rtl8139_remove( struct pci_dev *pdev )
{
	struct net_device *dev;
	struct rtl8139 *priv;
	/**
	 * Get address of netdevice, device private structures and ioaddr 
	 * Unregister and free netdevice
	 * Unmap the device MMIO region. Also set: priv->mmio_addr = NULL 
	 * Release the ownership of IO memory region
	 * call: pci_set_drvdata(pdev, NULL)
	 * Disable PCI device
	 */

	/* CODE HERE */
	dev = pci_get_drvdata(pdev);
	priv = netdev_priv(dev);

	/* Unregister and free netdevice */
	unregister_netdev(dev);
	free_netdev(dev);
	/* Unmap the device MMIO region */
	pci_iounmap(pdev, priv->mmio_addr);
	priv->mmio_addr = NULL;
	/* Release the ownership of IO memory region */
	pci_release_regions(pdev);	
	/* clear priv_data */
	pci_set_drvdata(pdev, NULL);
	/* Disable MWI */
	pci_clear_mwi(pdev);
	/* Disable PCI device */
	pci_disable_device(pdev);
}


/**************** PCI init and exit routines ***********************/
static int __init pci_rtl8139_init(void)
{
	/* CODE HERE */
	if(pci_register_driver(&rtl8139_driver)){
			printk(KERN_ERR DRV_NAME ": Failed to register the device.\n");
			return -1;
	}
	return 0;
}

static void __exit pci_rtl8139_exit(void)
{
	/* CODE HERE */
    pci_unregister_driver(&rtl8139_driver);
}

module_init(pci_rtl8139_init);
module_exit(pci_rtl8139_exit);

MODULE_AUTHOR("Shuran Xu");
MODULE_DESCRIPTION("PCI Driver for Realtek rtl8139 PCI Ethernet Wired card");
MODULE_LICENSE("Dual BSD/GPL");