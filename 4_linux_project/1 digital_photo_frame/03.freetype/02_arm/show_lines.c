
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <linux/fb.h>

#include <math.h>
#include <wchar.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#define MAX_GLYPHS 100

//定义TGlyph_ 结构体类型
typedef struct TGlyph_{
    FT_UInt    index;  /* glyph index                  */
    FT_Vector  pos;    /* glyph origin on the baseline */
    FT_Glyph   image;  /* glyph image                  */
} TGlyph, *PGlyph;


int fd_fb;

struct fb_var_screeninfo var;
struct fb_fix_screeninfo fix;
int screen_size;
unsigned char *fbmem;
unsigned int line_width;
unsigned int pixel_width;

/* color :  0x00RRGGBB */
void lcd_put_pixel(int x, int y, unsigned int color)
{
	unsigned char  *pen_8 = fbmem + y * line_width + x * pixel_width;
	unsigned short *pen_16;
	unsigned int   *pen_32;

	unsigned int red, green, blue;
	
	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch(var.bits_per_pixel){
		case 8:
		{
			*pen_8 = color;
			break;
		}
		case 16:
		{
			red   = (color >> 16)& 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:
		{
			*pen_32 = color;
			break;
		}
		default:
		{
			break;
		}
	}
	
}

/* Replace this function with something useful. */
void
draw_bitmap( FT_Bitmap*  bitmap,
             FT_Int      x,
             FT_Int      y)
{
  FT_Int  i, j, p, q;
  FT_Int  x_max = x + bitmap->width;
  FT_Int  y_max = y + bitmap->rows;

//  printf("x = %d, y = %d\n", x, y);
  
  for ( i = x, p = 0; i < x_max; i++, p++ ){
    for ( j = y, q = 0; j < y_max; j++, q++ ){
      if ( i < 0      || j < 0       ||
           i >= var.xres || j >= var.yres )
        continue;

//     	      image[j][i] |= bitmap->buffer[q * bitmap->width + p];
	  /************************************************************************************
	     因为buffer点阵里面只有一个字节，对于0xRRGGBB格式来
	     说相当于0x0000BB， 所以只显示为字显示为蓝色。
  	     ************************************************************************************/
	  lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
    }
  }
}

//将一行宽字符字符串转换成glyph
int Get_Glyphs_Frm_Wstr(FT_Face face, wchar_t *wstr, TGlyph glyphs[])
{
	int n;
	PGlyph glyph = glyphs;
	int pen_x = 0;
	int pen_y = 0;
	int error;
	FT_GlyphSlot slot = face->glyph;  /* a small shortcut */

	for(n = 0; n < wcslen(wstr); n++){
		//先根据字符的Unicode码找到glyph的索引。
		glyph->index = FT_Get_Char_Index(face, wstr[n]);
		
		/* store current pen position */
		glyph->pos.x = pen_x;
		glyph->pos.y = pen_y;

		error = FT_Load_Glyph(face, glyph->index, FT_LOAD_DEFAULT);
		if(error)
			continue;

		/*
		   从face->glyph这个插槽里把glyph image
		    数据提取出来存在glyph变量里
		 */
		/* extract glyph image and store it in our table */
	    error = FT_Get_Glyph( face->glyph, &glyph->image);
	    if ( error ){
		  printf("FT_Get_Glyph error!\n");
		  continue;
	    }

		/* translate the glyph image now */
		/* 这使得glyph->image里含有位置信息 */
		FT_Glyph_Transform( glyph->image, 0, &glyph->pos );
		
		//一行只更新x 坐标
		pen_x += slot->advance.x; /* 1 /64 point */
		
		/* increment number of glyphs */
		glyph++;
	}
	
	/* count number of glyphs loaded */
	return (glyph - glyphs);	
}

/*
计算一行字符串所占的最值
*/
void compute_string_bbox(TGlyph glyphs[], FT_UInt num_glyphs, FT_BBox *abbox)
{
	FT_BBox bbox;
	int n;

	bbox.xMin = bbox.yMin = 3200;
	bbox.xMax = bbox.yMax = -3200;

	for(n = 0; n < num_glyphs; n++){
		FT_BBox glyph_bbox;
		FT_Glyph_Get_CBox(glyphs[n].image, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

		if(glyph_bbox.xMin < bbox.xMin)
			bbox.xMin = glyph_bbox.xMin;

		if(glyph_bbox.yMin < bbox.yMin)
			bbox.yMin = glyph_bbox.yMin;

		if (glyph_bbox.xMax > bbox.xMax)
			bbox.xMax = glyph_bbox.xMax;

		if (glyph_bbox.yMax > bbox.yMax)
			bbox.yMax = glyph_bbox.yMax;
	}

	*abbox = bbox;
}

void Draw_Glyphs(TGlyph glyphs[], FT_UInt num_glyphs, FT_Vector pen)
{
	int n;
	FT_Error      error;
	
	for(n = 0; n < num_glyphs; n++){
		
		FT_Glyph_Transform(glyphs[n].image, 0, &pen);
		
		/* convert glyph image to bitmap (destroy the glyph copy!) */
		error = FT_Glyph_To_Bitmap(
				  &glyphs[n].image,
				  FT_RENDER_MODE_NORMAL,
				  0,				  /* no additional translation */
				  1);				  /* destroy copy in "image" */
		if ( !error ){
		  FT_BitmapGlyph bit = (FT_BitmapGlyph)glyphs[n].image;	
		  draw_bitmap(&bit->bitmap, bit->left, var.yres - bit->top);
		  FT_Done_Glyph(glyphs[n].image);
		}
	}
}

int main(int argc, char *argv[])
{
	wchar_t *wstr1 = L"深圳市";
	wchar_t *wstr2 = L"www.shenzhen.com";

	FT_Library    library;
	FT_Face       face;
	FT_Error      error;
    FT_Vector     pen;                    /* untransformed origin  */
	FT_GlyphSlot  slot;
	int i;
	
	FT_BBox  bbox;
	
	int line_box_ymin = 10000;
	int line_box_ymax = 0;

	int line_box_width;
	int line_box_height;
	
	TGlyph  glyphs[MAX_GLYPHS];  /* glyphs table */
	FT_UInt	num_glyphs;

	//用法提示
	if (argc != 2){
		printf("Usage : %s <font_file>\n", argv[0]);
		return -1;
	}
	
	//打开fb
	fd_fb = open("/dev/fb0", O_RDWR);
	if(fd_fb < 0){
		printf("can't open /dev/fb0\n");
		return -1;
	}

	if(ioctl(fd_fb, FBIOGET_VSCREENINFO, &var)){
		printf("can't get var\n");
		return -1;
	}
	
	if(ioctl(fd_fb, FBIOGET_FSCREENINFO, &fix)){
		printf("can't get fix\n");
		return -1;
	}
	
	line_width  = var.xres * var.bits_per_pixel / 8;//一行的字节数
	pixel_width = var.bits_per_pixel / 8; 		    //一个像素的字节数
	
	//LCD  framebuffer内存映射
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, \
								MAP_SHARED, fd_fb, 0);
	if(fbmem == (unsigned char *)-1){
		printf("can't mmap framebuffer\n");
		return -1;
	}
	
	/* 清屏 全部设置为黑色*/
	memset(fbmem, 0, screen_size);
	
	error = FT_Init_FreeType( &library );              /* initialize library */
  	/* error handling omitted */

  	error = FT_New_Face( library, argv[1], 0, &face ); /* create face object */
  	/* error handling omitted */

	slot = face->glyph;

	FT_Set_Pixel_Sizes(face, 24, 0);//set 24x24                /* set character size */

	//居中显示第1 行
	num_glyphs = Get_Glyphs_Frm_Wstr(face, wstr1, glyphs);
	compute_string_bbox(glyphs, num_glyphs, &bbox);

	line_box_width  = bbox.xMax - bbox.xMin;
	line_box_height = bbox.yMax - bbox.yMin;

	pen.x = ((var.xres - line_box_width) / 2) * 64;/* 设置原点坐标 */
	pen.y = ((var.yres - line_box_height) / 2) * 64;

	Draw_Glyphs(glyphs, num_glyphs, pen);
	
	
	//居中显示第2 行
	num_glyphs = Get_Glyphs_Frm_Wstr(face, wstr2, glyphs);
	compute_string_bbox(glyphs, num_glyphs, &bbox);

	line_box_width  = bbox.xMax - bbox.xMin;
	line_box_height = bbox.yMax - bbox.yMin;

	pen.x = ((var.xres - line_box_width) / 2) * 64;/* 设置原点坐标 */
	pen.y = pen.y - 24 * 64;

	Draw_Glyphs(glyphs, num_glyphs, pen);
	
	return 0;
}

