 #include <stdlib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <linux/fb.h>
 #include <sys/mman.h>
 #include <sys/ioctl.h>
 #include <math.h>
 #include <pthread.h>
#include "conio.h"
#define PI 3.14159265

int fbfd = 0;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
long int screensize = 0;
char *fbp = 0;
int x = 0, y = 0;
long int location = 0;
int ufo = 800;
int destroy = 0;
int tx = 600;
pthread_t t_ufo;
pthread_t t_bullet;
int command = 0;

//Untuk menentukan apakah peluru meledak atau tidak
int startX = 0;
int endX = 0;
int startY = 0;
int endY = 0;

int rotatex(int x, int y, int angle, int cx, int cy);
int rotatey(int x, int y, int angle, int cx, int cy);



typedef int OutCode;

int ymin = 100;
int ymax = 600;
int xmin = 300;
int xmax = 1000;


const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

// Compute the bit code for a point (x, y) using the clip rectangle
// bounded diagonally by (xmin, ymin), and (xmax, ymax)

// ASSUME THAT xmax, xmin, ymax and ymin are global constants.

OutCode ComputeOutCode(double x, double y)
{
	OutCode code;

	code = INSIDE;          // initialised as being inside of [[clip window]]

	if (x < xmin)           // to the left of clip window
		code |= LEFT;
	else if (x > xmax)      // to the right of clip window
		code |= RIGHT;
	if (y < ymin)           // below the clip window
		code |= BOTTOM;
	else if (y > ymax)      // above the clip window
		code |= TOP;

	return code;
}



//-----------------------------------------------------------------
//-----------------------------------------------------------------

int check_color(int x, int y, int color) {
	int same = 0;
	location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
		(y+vinfo.yoffset) * finfo.line_length;
	if(x >= vinfo.xres_virtual || x <= 0 || y <= 0 || y >=vinfo.yres_virtual){
				return 0;
			}
	if (vinfo.bits_per_pixel == 32) {
		if (*(fbp + location) == color) {
			same = 1;
		}
	} else  { //assume 16bpp
		int b = color;
		int g = color;
		int r = color;
		unsigned short int t = r<<11 | g << 5 | b;
		if (*((unsigned short int*)(fbp + location)) == t) {
			same = 1;
		}
	}
	return same;
}

void set_color(int x, int y, int color) {
	if(x > vinfo.xres_virtual || x < 0 || y < 0 || y > vinfo.yres_virtual){
				return;
			}
	location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
		(y+vinfo.yoffset) * finfo.line_length;
	if (vinfo.bits_per_pixel == 32) {
		*(fbp + location) = color;
		*(fbp + location + 1) = 15+(color-100)/2;
		*(fbp + location + 2) = 200-(color-100)/5;
		*(fbp + location + 3) = 0;
	} else  { //assume 16bpp
		int b = color;
		int g = color;
		int r = color;
		unsigned short int t = r<<11 | g << 5 | b;
		*((unsigned short int*)(fbp + location)) = t;
	}
}

int flood_fill(int x, int y, int target_color, int replacement_color) {


	// if (target_color == replacement_color) {
	// 	return 0;
	// } else if (check_color(x, y, target_color) == 0) {
	// 	return 0;
	// }

	// set_color(x, y, replacement_color);
	// flood_fill(x+1, y, target_color, replacement_color);
	// flood_fill(x-1, y, target_color, replacement_color);
	// flood_fill(x, y+1, target_color, replacement_color);
	// flood_fill(x, y-1, target_color, replacement_color);
	return 0;
}


//------------------------------------------------------------------------------



void drawPoint(int xn, int yn, int color){
 for (y = yn; y < yn+4; y++)
         for (x = xn; x < xn+4; x++) {
             location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
                        (y+vinfo.yoffset) * finfo.line_length;
			if(x >= vinfo.xres_virtual || x <= 0 || y <= 0 || y >=vinfo.yres_virtual){
				break;
			}
			if (color!=0){
				if (vinfo.bits_per_pixel == 32) {
					*(fbp + location) = color;        // Some blue
					*(fbp + location + 1) = 15+(color-100)/2;     // A little green
					*(fbp + location + 2) = 200-(color-100)/5;    // A lot of red
					*(fbp + location + 3) = 0;      // No transparency
				} else  { //assume 16bpp
					int b = color;//10;
					int g = color;//(x-100)/6;     // A little green
					int r = color;//31-(y-100)/16;    // A lot of red
					unsigned short int t = r<<11 | g << 5 | b;
					*((unsigned short int*)(fbp + location)) = t;
				}
			}else{
				*(fbp + location) = 0;        // Some blue
				*(fbp + location + 1) = 0;     // A little green
				*(fbp + location + 2) = 0;    // A lot of red
				*(fbp + location + 3) = 0;      // No transparency
			}	
	 }
}

void drawLine(int x1, int y1, int x2, int y2, int color){
/* x1 = koordinat x titik pertama; y1 = koordinat y titik pertama;
 * x2 = koordinat x titik kedua; y3 = koordinat y titik kedua;
 * color = warna garis
 * Prosedur untuk ngegambar garis Bresenham*/ 
	int xn=x1, yn=y1;
	int dx=abs(x2-x1);
	int dy=abs(y2-y1);
	int p = 2*dy-dx;
	
	if(dx!=0){
		if (dx>dy){
			for (int i=0;i<abs(x2-x1);i++){
				if(x2 > x1){
					xn=xn+1;
				} else {
					xn=xn-1;
				}
				p=p+2*dy;
				if (p>=0){
					if (y2-y1>0)
						yn++;
					else
						yn--;
					p-=2*dx;
				}
				drawPoint(xn,yn,color);
			}
		} else {
			for (int i=0;i<abs(y2-y1);i++){
				if(y2 > y1){
					yn=yn+1;
				} else {
					yn=yn-1;
				}
				p=p+2*dx;
				if (p>=0){
					if (x2-x1>0)
						xn++;
					else
						xn--;
					p-=2*dy;
				}
				drawPoint(xn,yn,color);
			}
		}
	} else {
		if (y2 > y1) {
			for (int i=0; i < dy; i++){
				drawPoint(xn,yn,color);
				yn++;
			}
		} else {
			for (int i=0; i < abs(dy); i++){
				drawPoint(xn,yn,color);
				yn--;
			}
		}
	}
	
}

void DrawRectangle(int xmin, int ymin, int xmax, int ymax, int color) {
  drawLine(xmin, ymin, xmin, ymax, color);
  drawLine(xmin, ymax, xmax, ymax, color);
  drawLine(xmax, ymin, xmin, ymin, color);
  drawLine(xmax, ymax, xmax, ymin, color);
}



void drawGaris(int x0, int y0, int x1, int y1, int color){

	// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
	OutCode outcode0 = ComputeOutCode(x0, y0);
	OutCode outcode1 = ComputeOutCode(x1, y1);
	bool accept = false;

	while (true) {
		if (!(outcode0 | outcode1)) { // Bitwise OR is 0. Trivially accept and get out of loop
			accept = true;
			break;
		} else if (outcode0 & outcode1) { // Bitwise AND is not 0. (implies both end points are in the same region outside the window). Reject and get out of loop
			break;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			double x, y;

			// At least one endpoint is outside the clip rectangle; pick it.
			OutCode outcodeOut = outcode0 ? outcode0 : outcode1;

			// Now find the intersection point;
			// use formulas y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
			if (outcodeOut & TOP) {           // point is above the clip rectangle
				x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
				y = ymax;
			} else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
				x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
				y = ymin;
			} else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
				y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
				x = xmax;
			} else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
				y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
				x = xmin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = ComputeOutCode(x0, y0);
			} else {
				x1 = x;
				y1 = y;
				outcode1 = ComputeOutCode(x1, y1);
			}
		}
	}
	if (accept) {
               // Following functions are left for implementation by user based on
               // their platform (OpenGL/graphics.h etc.)
               drawLine(x0, y0, x1, y1, color);
	}

}
  



void drawCheck(int x, int y, int color) {
	if (ComputeOutCode(x,y) == 0) {
		drawPoint(x,y,color);
	}
}


void drawCircle(int r, int xc, int yc, int full, int color){
/* r = jari-jari; xc = koordinar X pusat lingkaran;
 * yc= koordinat Y pusat lingkaran;
 * full = 1: lingkaran penuh, 0: setengah lingkaran bgn atas;
 * color= warna garis lingkaran
 * Prosedur untuk ngegambar lingkaran Bresenham*/
 
	int xn=0, yn=r;
	int p = 3-2*r; 

	while (xn<yn){
		xn++;
		if (p>=0){
			yn--;
			p+=4*(xn-yn)+10;
		}else{
			p+=4*(xn)+6;
		}
		int x3=xn;
		int y3=yn*(-1)+0.5*r;
				
		//Sisi kanan atas

		drawCheck(x3+xc,y3+yc,color);
		drawCheck(-1*y3+xc+0.5*r,-1*x3+yc+0.5*r,color);
		
		//Sisi kiri atas
		drawCheck(x3*(-1)+xc,y3+yc,color);
		drawCheck(y3+xc-0.5*r,-1*x3+yc+0.5*r,color);
		
		if (full==1){
			//sisi kanan bawah
			drawCheck(xn+xc,yn+yc+0.5*r,color);
			drawCheck(yn+xc,xn+yc+0.5*r,color);
		
			//sisi kiri bawah
			drawCheck(-1*x3+xc,-1*y3+yc+r,color);
			drawCheck(y3+xc-0.5*r,x3+yc+0.5*r,color);
		}
	}
}




void drawPolygon(int x, int y, int color) {
  drawGaris(x, y, x+50, y, color);
  drawGaris(x+50, y, x+50, y+50, color);
  drawGaris(x, y+50, x+50, y+50, color);
  drawGaris(x, y, x, y+50, color);

  flood_fill(x+10, y+10, 0, 100);
}

void printBackground (){
	for (y = 0; y < vinfo.yres_virtual-30; y++) {
		for(x = 0; x < 1366; x++) {
			location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) +
					(y+vinfo.yoffset) * finfo.line_length;
			 *(fbp + location) = 0;        // Some blue
			 *(fbp + location + 1) = 0;     // A little green
			 *(fbp + location + 2) = 0;    // A lot of red
			 *(fbp + location + 3) = 0; 
		}
	 }

	 //drawPolygon(tx,700, 300);
	  for (int i = 0; i < 50; i++){
		  int x = rand() % (xmax-xmin); 
		  int y = rand() % (ymax-ymin);
		  x += xmin;
		  y += ymin;
		  //printf("(%d, %d)\n", x, y);
		  drawPoint(x, y, 50);
	  }
}


void drawExplosion(int xc, int yc, int color){
	for (int i=1;i<3;i++) {
		//reset layar
		
		//Inner explosion
		drawGaris(xc+15,yc+20,xc+50,yc+40,color); //1
		drawGaris(xc+50,yc+40,xc+20,yc+10,color); //2
		drawGaris(xc+20,yc+10,xc+40,yc,color); //3
		drawGaris(xc+40,yc,xc+20,yc-10,color); //4
		drawGaris(xc+20,yc-10,xc+25,yc-40,color); //5
		drawGaris(xc+25,yc-40,xc,yc-15,color); //6
		drawGaris(xc,yc-15,xc-10,yc-30,color); //7
		drawGaris(xc-10,yc-30,xc-20,yc-5,color); //8
		drawGaris(xc-20,yc-5,xc-45,yc-13,color); //9
		drawGaris(xc-45,yc-13,xc-30,yc+7,color); //10
		drawGaris(xc-30,yc+7,xc-55,yc+15,color); //11
		drawGaris(xc-55,yc+15,xc-14,yc+23,color); //12
		drawGaris(xc-14,yc+23,xc-2,yc+38,color); //13
		drawGaris(xc-2,yc+38,xc+15,yc+20,color); //14
		
		flood_fill(xc, yc, 0, color);
	}
}

void rotateantena(int rj,int xc, int y, int angle, int color){
	int xl1 = rotatex(xc-105, y-25, angle, xc, y);
	int yl1 = rotatey(xc-105, y-25, angle, xc, y);
	int xl2 = rotatex(xc-90, y-15, angle, xc, y);
	int yl2 = rotatey(xc-90, y-15, angle, xc, y);
	drawGaris(xl1,yl1,xl2,yl2,color);
	drawGaris(xl1+105,yl1,xl2+105,yl2,color);
	drawCircle(rj/2, xl1, yl1, 1, color);
	drawCircle(rj/2, xl1+105, yl1, 1, color);

}
void rotateatap(int rj,int xc, int y, int angle, int color){
	int xl1 = rotatex(xc-105, y-25, angle, xc, y);
	int yl1 = rotatey(xc-105, y-25, angle, xc, y);
	int xl2 = rotatex(xc-90, y-15, angle, xc, y);
	int yl2 = rotatey(xc-90, y-15, angle, xc, y);
	drawGaris(xl1,yl1,xl2,yl2,color);
	drawGaris(xl1+105,yl1,xl2+105,yl2,color);
	drawCircle(rj/2, xl1, yl1, 1, color);
	drawCircle(rj/2, xl1+105, yl1, 1, color);

}

void rotateBadan(int rc,int xc, int y, int angle, int color){
	int xl0 = rotatex(xc, y+40, angle, xc, y+rc);
	int yl0 = rotatey(xc, y+40, angle, xc, y+rc);
	int xl1 = rotatex(xc-rc, y+0.5*rc, angle, xc, y+rc);
	int yl1 = rotatey(xc-rc, y+0.5*rc, angle, xc, y+rc);
	int xl2 = rotatex(xc+rc, y+0.5*rc, angle, xc, y+rc);
	int yl2 = rotatey(xc+rc, y+0.5*rc, angle, xc, y+rc);
	int xl3 = rotatex(xc-1.5*rc,y+50, angle, xc, y+rc);
	int yl3 = rotatey(xc-1.5*rc,y+50, angle, xc, y+rc);
	int xl4 = rotatex(xc+1.5*rc,y+50, angle, xc, y+rc);
	int yl4 = rotatey(xc+1.5*rc,y+50, angle, xc, y+rc);
	int xl5 = rotatex(xc+75,y+50, angle, xc, y+rc);
	int yl5 = rotatey(xc+75,y+50, angle, xc, y+rc);
	int xl6 = rotatex(xc-75,y+50, angle, xc, y+rc);
	int yl6 = rotatey(xc-75,y+50, angle, xc, y+rc);
	int xl7 = rotatex(xc+rc,y+50, angle, xc, y+rc);
	int yl7 = rotatey(xc+rc,y+50, angle, xc, y+rc);
	int xl8 = rotatex(xc+60,y+65, angle, xc, y+rc);
	int yl8 = rotatey(xc+60,y+65, angle, xc, y+rc);
	int xl9 = rotatex(xc-rc,y+50, angle, xc, y+rc);
	int yl9 = rotatey(xc-rc,y+50, angle, xc, y+rc);
	int xl10 = rotatex(xc-60,y+65, angle, xc, y+rc);
	int yl10 = rotatey(xc-60,y+65, angle, xc, y+rc);

	drawGaris(xl1,yl1,xl2,yl2,color);
	drawGaris(xl3,yl3,xl4, yl4,color);
	drawGaris(xl2,yl2,xl5, yl5,color);
	drawGaris(xl1,yl1,xl6, yl6,color);
	drawGaris(xl7, yl7,xl8,yl8,color);	
	drawGaris(xl9, yl9, xl10, yl10,color);
	flood_fill(xl0, yl0, 0, color);	

}

void drawAntena(int xc, int rc,int yc, int rj, int color){
		//Antena UFO
	//drawGaris(xc-45,xc-30,yc-25,yc-15,color);
	drawCircle(rj/2,xc-45,yc-25,1,color);
	drawCircle(rj/2,xc+45,yc-25,1,color);
}

void drawAtap(int xc, int rc, int yc, int rj, int color){
	//Atap UFO
	drawCircle(rc,xc,yc,0,color);
	drawCircle(rj,xc,yc+10,1,color);
	drawCircle(rj,xc+30,yc+10,1,color);
	drawCircle(rj,xc-30,yc+10,1,color);
	drawGaris(xc-rc,yc+0.5*rc,xc+rc,yc+0.5*rc,color);
	flood_fill(xc, yc, 0, color);
}

void drawBadan(int xc, int rc, int yc, int rj, int color){
	//Badan UFO
	drawGaris(xc-rc,yc+0.5*rc,xc+rc,yc+0.5*rc,color);
	drawGaris(xc-1.5*rc,yc+50,xc+1.5*rc,yc+50,color);
	drawGaris(xc+rc,yc+0.5*rc,xc+75,yc+50,color);
	drawGaris(xc-rc,yc+0.5*rc,xc-75,yc+50,color);
	drawGaris(xc+rc,yc+50,xc+60,yc+65,color);	
	drawGaris(xc-rc,yc+50,xc-60,yc+65,color);	
	flood_fill(xc, yc+40, 0, color);
}

void drawUFO(int xc, int rc, int yc,int rj, int color){
/* xc = koordinat X pusat lingkaran UFO
 * rc = jari-jari atap UFO; yc = koordinat Y pusat lingkaran atap UFO;
 * rj = jar-jari jendela UFO; 
 * Prosedur untuk bikin gambar UFO*/
  
	//Antena UFO
	drawAntena(xc,rc,yc,rj,color-1000);
	drawAtap(xc,rc,yc,rj,color);
	drawBadan(xc,rc,yc,rj,color-2000);
}

void explosionMove(int xc, int yc, int yf, int color, int rc, int rj){
	int angle = 0; 
	int xant = xc;
	int yant = yc;
	int xbody = xc;
	int ybody = yc;
	int ysum = 0;
	int yzig = 0;
	int xsum = 0;
	int xzig = 1;
	
	int explode_init_range = 0;
	int bounce_naturally_setter  = 0;
	for (int y = yc; y < yf; y++){
		if (explode_init_range < 100){
			drawExplosion(xc, y, color);
			drawExplosion(xc-explode_init_range, y, color);
			drawExplosion(xc+explode_init_range, y, color);
			explode_init_range++;
		}
		rotateantena(rj, xant,yant,angle,color+300);
		xant--;
		//int tempx = rotatex(xc,y,angle,xc-10,y-15);
		//int tempy = rotatey(xc,y,angle,xc-10,y-15);
		if(yant < yf-10){
			yant++;
		}
		xbody++;
		if(ybody < yf-10){
			ybody++;
		}
		
		drawAtap(xc+200+xsum,rc,y+ysum,rj,color+100);
		if(y+ysum > vinfo.yres/2){
			yzig = -3;
			if (bounce_naturally_setter < 0) bounce_naturally_setter = 0;
			bounce_naturally_setter += 4;
		}
		else if(y+ysum < vinfo.yres/3){
			if (bounce_naturally_setter > 0) bounce_naturally_setter = 0;
			bounce_naturally_setter = 1;
			yzig = 1;
		}
		ysum += yzig;
		xsum += xzig;
		rotateBadan(rc, xbody, ybody, 45, color+200);
		usleep(100*bounce_naturally_setter);
		printBackground();
		if(angle >=360){
			angle = 0;
		}else{
			angle++;
		}
	}
}



void drawBullet(int xf, int yf){
	int x = tx + 10;
	int y = 690;
	int deltax = abs(xf - x)/70;

	if (xf < x) {
		deltax *= -1;
	}
	int tempy = y-10;
	int tempx = x + (deltax*-1);
	
	drawGaris(x,y,tempx,tempy, 50);
	usleep(100000);
	drawGaris(x,y,tempx,tempy,0);
	
	while(tempy > yf && x > 0 && x < 1362) {
		//not_collision = check_color(tempx+deltax, tempy-15, 100);
		if ((x > startX)&&(x < endX)&&(y > startY)&&(y < endY)) {
			destroy = 1;
			break;
		}
		drawGaris(x+deltax,y-10,tempx+deltax,tempy-10, 50);
		usleep(10000);
		drawGaris(x+deltax,y-10,tempx+deltax,tempy-10,0);
		x = x+deltax;
		tempx = tempx+deltax;
		y = y -10;
		tempy = tempy-10;
	}
}

void *controller(void *args){
	int tempx = 600;
	while(1){
		if (getch() == '\033') { // if the first value is esc
			getch(); // skip the [
			switch(getch()) { // the real value
				case 'C':
					// code for arrow right
					tx += 10;
					break;
				case 'D':
					// code for arrow left
					tx -= 10;
					break;
				case 'A':
					// code for arrow up
					drawBullet(tx, 50);
					break;
				case 'B':
					// code for arrow down
					command = 1;
					break;				
			}
		}

		char c;
 		c = getchar();
 		if (c == 'a'){
 			xmin -= 40;
 			xmax -= 40;
 		}
 		if (c == 'd'){
 			xmin += 40;
 			xmax += 40;
 		}
 		if (c == 'w'){
 			ymin -= 40;
 			ymax -= 40;
 		}
 		if (c == 's'){
 			ymin += 40;
 			ymax += 40;
 		}
		if(command ==1||destroy==1){
			break;
		}
	}
}

int rotatex(int x, int y, int angle, int cx, int cy){
	double val = PI / 180;
	double rad = (double)angle *val;
	double s = sin(rad);
	double c = cos(rad);
	x -=cx ;
	y -= cy;
	return(x*c - y*s)+cx;
}

int rotatey(int x, int y, int angle, int cx, int cy){
	double val = PI / 180;
	double rad = (double)angle *val;
	double s = sin(rad);
	double c = cos(rad);
	x -=cx ;
	y -= cy;
	return(x*s + y*c)+cy;
}

void moveUFO(int rc, int yc, int rj, int sX, int fX, int color){
/* rc = jari-jari atap UFO; yc = pusat lingkaran atap UFO di sumbu Y;
 * rj = jari-jari jendela UFO; 
 * sX = titik awal pergerakan UFO (koordinat X jari-jari atap UFO di posisi pertama);
 * fX = titik terakhir pergerakan UFO (koordinat X jari-jari atap UFO di posisi terakhir);
 * Prosedur untuk menggerakkan UFO*/
 
	int xc=sX;



	bool arah = true;
	int ax = xc;
	int ay = yc;
	int angle = 0;

	while (xc!=fX){

		//Menentukan lokasi ufo untuk ledakan
		startX = xc - rc;
		endX= xc + rc;
		startY = yc - rc;
		endY = yc + rc;
		//==================================

		//sleep(10);
		if(angle > -180 && arah){
			angle--;
		}else{
			angle++;
			arah = false;
			if(angle>0){
				arah = true;
			}
		}
		
		int x = vinfo.xres/2;
		int y = vinfo.yres/2;
		drawUFO(xc,rc,yc,rj,color);

		xc = rotatex(ax, ay, angle, x, y);
		yc = rotatey(ax, ay, angle, x, y);
		ufo = xc;
		//drawPoint(xc, yc, 50);
		//drawPoint(ax, ay, 50);
		//drawPoint(x, y, 100);
		
		DrawRectangle(xmin, ymin, xmax, ymax, color);
		usleep(10000);
		printBackground();

		if (destroy == 1 || command == 1){
			drawUFO(xc,rc,yc,rj,color);
			// printBackground();
			explosionMove(xc,yc+50, 660, 30, rc, rj);
			DrawRectangle(xmin, ymin, xmax, ymax, color);
			command = 1;
			break;
		}	
			
	}   
}

int main()
{
     // Open the file for reading and writing
     fbfd = open("/dev/fb0", O_RDWR);
     if (fbfd == -1) {
         perror("Error: cannot open framebuffer device");
         exit(1);
     }
     printf("The framebuffer device was opened successfully.\n");

     // Get fixed screen information
     if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
         perror("Error reading fixed information");
         exit(2);
     }

     // Get variable screen information
     if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
         perror("Error reading variable information");
         exit(3);
     }

     printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

     // Figure out the size of the screen in bytes
     screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

     // Map the device to memory
     fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                        fbfd, 0);
     if ((intptr_t)fbp == -1) {
         perror("Error: failed to map framebuffer device to memory");
         exit(4);
     }
     printf("The framebuffer device was mapped to memory successfully.\n");
	
//-----UFO

	 int xc=700; 
	 int rc = 50;
	 int yc= 400;
	 int rj=8, yj=60;
	 
	 printf("Before Thread\n");
	 pthread_create(&t_bullet, NULL, controller, NULL);
	 printBackground();
	
	 while (destroy != 1){
		moveUFO(rc,(vinfo.yres/2),rj,(vinfo.xres/2)+350,0,100);
		
		if(command == 1) {
			break;
		}
	 }
	 
	 
     pthread_join(t_bullet, NULL);
     sleep(1);
	 munmap(fbp, screensize);
     close(fbfd);
     return 0;
 }
 