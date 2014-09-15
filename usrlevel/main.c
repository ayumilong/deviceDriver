#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <sys/mman.h>
#include <linux/ioctl.h>

#include "control.h"

struct u_kyouko_device{
	unsigned int *u_control_base;
	unsigned int *u_framebuffer_base;
}kyouko2;

unsigned int U_READ_REG(unsigned int reg){
	return (*(kyouko2.u_control_base + (reg>>2)));
}

void U_WRITE_REG(unsigned int reg, unsigned int value){
	*(kyouko2.u_control_base + (reg>>2)) = value;
}

void U_WRITE_FB(unsigned int reg, unsigned int value){
	*(kyouko2.u_framebuffer_base + (reg)) = value;
}

void setColor(unsigned int reg, unsigned int red, unsigned int green, unsigned int blue, unsigned int alpha){
	U_WRITE_REG(reg, blue); //Blue 
	U_WRITE_REG(reg + 4, green); //Green
	U_WRITE_REG(reg + 8, red); //Red
	U_WRITE_REG(reg + 12, alpha); //Alpha
}

void setCoordinate(unsigned int reg, unsigned int x, unsigned int y, unsigned int z, unsigned int w){
	U_WRITE_REG(reg , x);
	U_WRITE_REG(reg + 4, y);
	U_WRITE_REG(reg + 8, z);
	U_WRITE_REG(reg + 12, w);
}

int main(){
	int fd;
	int ramSize;
	fd = open("/dev/kyouko2", O_RDWR);
	//mmap, the last parameer is offset, 0x0 is for control region, ox80000000 is for framebuffer
	//Usercode calls mmap will call sys_mmap() then call do_mmap_pgoff(), then call driver's mmap(), in this program, it's kyouko2_mmap()
	//MAP_SHARED MAP_PRIVATE
	kyouko2.u_control_base = mmap(0, KYOUKO2_CONTROL_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	ramSize = U_READ_REG(ROM_DEVICE_VRAM);
	printf("Ram size in MB is: %d\n", ramSize);
	ramSize *= 1024 * 1024;
	kyouko2.u_framebuffer_base = mmap(0, ramSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x80000000);
 	ioctl(fd, VMODE, GRAPHICS_ON); //Open graphics mode
	ioctl(fd, SYNC);
	int i = 0;
	for(i = 200 * 1024; i < 201 * 1024; ++i){ //Directly write frame buffer
		U_WRITE_FB(i, 0xFF0000);
	}
	U_WRITE_REG(RASTER_FLUSH, 0x1); //Flush
	ioctl(fd, SYNC);

	//Using on-board FIFO command queue
	U_WRITE_REG(RASTER_PRIMITIVE, 0x1); //Start triangles

	setColor(DRAW_VERTEX_COLOR4F, FLOAT_ONE, FLOAT_ZERO, FLOAT_ZERO, FLOAT_ONE);
	setCoordinate(DRAW_VERTEX_COORD4F, MINUS_POINT_FIVE, MINUS_POINT_FIVE, FLOAT_ZERO, FLOAT_ONE);
	U_WRITE_REG(RASTER_EMIT, 0x0); //Emit

	setColor(DRAW_VERTEX_COLOR4F, FLOAT_ZERO, FLOAT_ONE, FLOAT_ZERO, FLOAT_ONE);
	setCoordinate(DRAW_VERTEX_COORD4F, POINT_FIVE, FLOAT_ZERO, FLOAT_ZERO, FLOAT_ONE);
	U_WRITE_REG(RASTER_EMIT, 0x0); //Emit

	setColor(DRAW_VERTEX_COLOR4F, FLOAT_ZERO, FLOAT_ZERO, FLOAT_ONE, FLOAT_ONE);
	setCoordinate(DRAW_VERTEX_COORD4F, POINT_ONE_TWO_FIVE, POINT_FIVE, FLOAT_ZERO, FLOAT_ONE);
	U_WRITE_REG(RASTER_EMIT, 0x0); //Emit

	U_WRITE_REG(RASTER_PRIMITIVE, 0x0); //End triangles
	U_WRITE_REG(RASTER_FLUSH, 0x1); //Flush
	
 	ioctl(fd, SYNC);
	sleep(5);
	
	ioctl(fd, VMODE, GRAPHICS_OFF);
	close(fd);
	return(0);
}
