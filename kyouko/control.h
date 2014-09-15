/*************************************************************************
    > File Name: control.h
    > Subject: Head file for control use 
    > Author: ayumi_Long
    > Mail: ayumilong41@gmail.com 
    > Created Time: Fri 31 Jan 2014 08:55:58 AM EST
 ************************************************************************/
#define DRV_NAME 	"kyouko2_driver"

#define FLOAT_ONE 	0x3F800000
#define FLOAT_ZERO 	0x00000000
#define POINT_FIVE	0x3F000000
#define MINUS_POINT_FIVE	0xBF000000
#define POINT_ONE_TWO_FIVE	0x3E000000

#define KYOUKO2_CONTROL_SIZE 	(65536)
#define DMA_BUFFER_SIZE		(124 * 1024)
#define BUFFER_NUM		(8)

//Pci device id table
#define PCI_VENDOR_ID_CCORSI 	0x1234
#define PCI_DEVICE_ID_KYOUKO2 	0x1113

//Device number
#define MAJOR_NUM 		500
#define MINOR_NUM 		127

//Define of some commands
#define KYOUKO_MAGIC 		41
#define VMODE _IOW(KYOUKO_MAGIC, 0x0, unsigned long) 
#define BIND_DMA _IOW(KYOUKO_MAGIC, 0x1, unsigned long) 
#define START_DMA _IOW(KYOUKO_MAGIC, 0x2, unsigned long) 
#define SYNC _IO(KYOUKO_MAGIC, 0x3)	
#define GRAPHICS_OFF 		0
#define GRAPHICS_ON 		1

//ROM registers, just for some information
#define ROM_VERSION_MAJOR	0x0000
#define ROM_VERSION_MINOR	0x0004
#define ROM_MANUF_ID		0x0008
#define ROM_MANUF_WEEK		0x0010
#define ROM_MANUF_YEAR		0x0014
#define ROM_MANUF_LOT		0x0018
#define ROM_MANUF_LOCATION	0x001C
#define ROM_DEVICE_VRAM		0x0020 //Global VRAM, in Megabyte
#define ROM_DEVICE_FRATURES	0x000C
#define ROM_DEVICE_FRAMES	0x0024 //Number of frame objects supported by device
#define ROM_DEVICE_ENCODERS	0x0028 //Number of Encoder objects supported by device
#define ROM_FIFO_SIZE		0x002C //Number of messages queueable on device FIFO

//Configuratin registers
#define CONFIG_REBOOT 		0x1000 //Any write to this register will immediately reboot the device
#define CONFIG_MODESET 		0x1008 //Any write to this register will flush encoder frame configuration
#define CONFIG_INTERRUPT 	0x100C
#define CONFIG_ACCELERATION 	0x1010
//Stream control
#define STREAM_BUFFERA_ADDR 	0x2000 //Physical source address for stream buffer A
#define STREAM_BUFFERA_CONFIG 	0x2008
#define STREAM_BUFFERB_ADDR 	0x2004
#define STREAM_BUFFERB_CONFIG 	0x200C
//Rasterization
#define RASTER_PRIMITIVE 	0x3000 
#define RASTER_EMIT 		0x3004 //Any write to this register will cause a single vertex to be emitted to the rasterizer subsystem. 
#define RASTER_CLEAR 		0x3008
#define RASTER_TARGETFRAME	0x3100 //Raster target(frame object index)
#define RASTER_FLUSH		0x3FFC //Any write to this register will initiate a flush of the hardware's internal raster queue, ensuring the frame data in VRAM is consistent with all prior FIFO commands
//Info and status
#define INFO_FIFO_DEPTH		0x4004 //Number of items currently waiting in device FIFO
#define INFO_STATUS			0x4008
//Drawing state
#define DRAW_VERTEX_COORD4F	0x5000 //X, Y, Z, W coordinate
#define DRAW_VERTEX_COLOR4F	0x5010 //Red, Green, Blue, Alpha
#define DRAW_VERTEX_TRANS16F	0x5080
#define DRAW_CLEAR_COLOR4F	0x5100 //RGBA
//Frames
#define FRAME_COLUMNS		0x8000
#define FRAME_ROWS			0x8004
#define FRAME_ROW_PITCH		0x8008
#define FRAME_PIX_FORMAT	0x800C
#define FRAME_START_ADDRESS	0x8010
//Encoders
#define ENCODER_WIDTH		0x9000
#define ENCODER_HEIGHT		0x9004
#define ENCODER_OFFSETX		0x9008
#define ENCODER_OFFSETY		0x900C
#define ENCODER_FRAME		0x9010

