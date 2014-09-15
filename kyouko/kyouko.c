/*************************************************************************
 	> File Name: kyouko.c
	> Subject: kyouko_card_driver
	> Author: yaolinz
	> Created Time: Mon 20 Jan 2014 02:27:58 PM EST
**************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <asm/system.h>

#include "control.h"

struct kyouko2_device_info{
	unsigned long p_control_base; //Bar#1
	unsigned long p_framebuffer_base; //Bar#2
	unsigned int *k_control_base;
	unsigned int *k_framebuffer_base;
	struct pci_dev *pci_dev;
	struct cdev newDevice;
}kyouko2;

void K_WRITE_REG(unsigned int reg, unsigned int value){
	udelay(1);
	rmb();
	*(kyouko2.k_control_base + (reg>>2)) = value;
}

unsigned int K_READ_REG(unsigned int reg){
	unsigned int value;
	udelay(1);
	rmb();
	value = *(kyouko2.k_control_base + (reg>>2));
	return value;
}

inline void sync(void){
	while(K_READ_REG(INFO_FIFO_DEPTH) > 0){
	}
}

static const struct pci_device_id kyouko2_dev_ids[] ={
	{PCI_DEVICE(PCI_VENDOR_ID_CCORSI, PCI_DEVICE_ID_KYOUKO2)},
	{0},
};

static int kyouko2_probe(struct pci_dev *pci_dev, const struct pci_device_id *pci_id){
	kyouko2.p_control_base = pci_resource_start(pci_dev, 1);
	kyouko2.p_framebuffer_base = pci_resource_start(pci_dev, 2);
	kyouko2.pci_dev = pci_dev;
	return 0;
}

void kyouko2_remove(struct pci_dev *dev){
}

static struct pci_driver kyouko2_pci_driver ={
	.name = "kyouko2_pci_driver",
	.id_table = kyouko2_dev_ids,
	.probe = kyouko2_probe,
	.remove = kyouko2_remove
};

static int kyouko2_open(struct inode *inode, struct file *fp){
	int ramSize;
	kyouko2.k_control_base = ioremap_nocache(kyouko2.p_control_base, KYOUKO2_CONTROL_SIZE); 
	ramSize = K_READ_REG(ROM_DEVICE_VRAM);
	ramSize *= (1024 * 1024);
	printk(KERN_ALERT "opened device -- %s:%s\n", __DATE__, __TIME__);
	kyouko2.k_framebuffer_base = ioremap_nocache(kyouko2.p_framebuffer_base, ramSize); 
	return 0;
}

static int kyouko2_release(struct inode* inode, struct file* fp){
	printk(KERN_ALERT "closed device -- %s:%s\n", __DATE__, __TIME__);
	K_WRITE_REG(CONFIG_REBOOT, 1);
	iounmap((void*)kyouko2.k_control_base);
	iounmap((void*)kyouko2.k_framebuffer_base);
	return 0;
}

inline void setFrame(unsigned int columns, unsigned int rows, unsigned int pixFormat, unsigned int startAddr){
 	K_WRITE_REG(FRAME_COLUMNS, columns);
 	K_WRITE_REG(FRAME_ROWS, rows);
 	K_WRITE_REG(FRAME_ROW_PITCH, columns * 4);
 	K_WRITE_REG(FRAME_PIX_FORMAT, pixFormat);
 	K_WRITE_REG(FRAME_START_ADDRESS, startAddr);
}

inline void setEncoder(unsigned int width, unsigned int height, unsigned int offsetX, unsigned int offsetY, unsigned int frame){
 	K_WRITE_REG(ENCODER_WIDTH, width);
 	K_WRITE_REG(ENCODER_HEIGHT, height);
 	K_WRITE_REG(ENCODER_OFFSETX, offsetX);
 	K_WRITE_REG(ENCODER_OFFSETY, offsetY);
 	K_WRITE_REG(ENCODER_FRAME, frame);
}

inline void setColor(unsigned int reg, unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha){
	K_WRITE_REG(reg, blue); //Blue 
	K_WRITE_REG(reg + 4, green); //Green
	K_WRITE_REG(reg + 8, red); //Red
	K_WRITE_REG(reg + 12, alpha); //Alpha
}

static long kyouko2_ioctl(struct file* fp, unsigned int cmd, unsigned long arg){
	switch(cmd){
 		case VMODE:
 			if((int)(arg) == GRAPHICS_ON){
				setFrame(1024, 768, 0xF888, 0x0);
				setEncoder(1024, 768, 0, 0, 0);
 			//	set acceleration  = 0x40000000
 				K_WRITE_REG(CONFIG_ACCELERATION, 0x40000000);
				K_WRITE_REG(CONFIG_MODESET, 0x1); //Set mode
 				sync();
			//	set clear color register: take 4 floats
				setColor(DRAW_CLEAR_COLOR4F, POINT_FIVE, POINT_FIVE, POINT_FIVE, FLOAT_ONE); 
  				K_WRITE_REG(RASTER_FLUSH, 0x1); //Flush
  				sync();
  				K_WRITE_REG(RASTER_CLEAR, 0x1);
  			//	graphics.on = 1
  			}else if(((int)arg) == GRAPHICS_OFF){
  				//set graphics_on = 0;
				sync();
				K_WRITE_REG(CONFIG_REBOOT, 0x1); //Reboot
  			}
			break;
		case SYNC:
			sync();
			break;
		default:
			printk(KERN_ALERT "Invalid command!\n");
			return -EINVAL; //Invalid argument
 	}
	return 0;
}

//vma is created by do_mmap_pgoff()
static int kyouko2_mmap(struct file *fp, struct vm_area_struct *vma){
	int ret = -1;
//The third parameter is the page address that we want to map
//vma->vma_pgoff<<PAGE_SHIFT
	if(vma->vm_pgoff == 0x0){ //For p_control_base
		ret = io_remap_pfn_range(vma, vma->vm_start, kyouko2.p_control_base>>PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
	}else if(vma->vm_pgoff == 0x80000){ //For p_framebuffer_base
		ret = io_remap_pfn_range(vma, vma->vm_start, kyouko2.p_framebuffer_base>>PAGE_SHIFT, vma->vm_end - vma->vm_start, vma->vm_page_prot);
	}
	return ret;
}

static struct file_operations kyouko2_fops ={
	.open = kyouko2_open,
	.release = kyouko2_release,
	.unlocked_ioctl = kyouko2_ioctl,
	.mmap = kyouko2_mmap,
	.owner = THIS_MODULE
};


static int __init init_function(void){
	int flag;
	printk(KERN_ALERT "Loading my device driver -- %s:%s\n", __DATE__, __TIME__);
	cdev_init(&kyouko2.newDevice, &kyouko2_fops);
	flag = cdev_add(&kyouko2.newDevice, MKDEV(MAJOR_NUM, MINOR_NUM), 1);
	if(flag == 0){
		printk(KERN_ALERT "Driver init success!\n");
		flag = pci_register_driver(&kyouko2_pci_driver);
		if(flag == 0){
			printk(KERN_ALERT "Driver register success!\n");
		//	pci_enable_device(kyouko2.pci_dev);
		//	pci_set_master(kyouko2.pci_dev);
		}else{
			printk(KERN_ALERT "Driver register failed!\n");
		}
	}else{
		printk(KERN_ALERT "Driver init failed!\n");
	}
	return 0;
}

static void __exit exit_function(void){
	printk(KERN_ALERT "Unload device driver  -- %s:%s\n", __DATE__, __TIME__);
	pci_unregister_driver(&kyouko2_pci_driver);
	cdev_del(&kyouko2.newDevice);
}

module_init(init_function);
module_exit(exit_function);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yaolinz");
MODULE_DESCRIPTION("Kyouko2 card driver.");
