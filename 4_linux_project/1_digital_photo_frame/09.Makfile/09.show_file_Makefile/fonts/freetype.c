
#include <config.h>
#include <fonts_manager.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

static FT_Library   g_tLibrary;
static FT_Face      g_tFace;
static FT_GlyphSlot g_tSlot;

static int 
FreeTypeFontInit(char *pcFontFile, unsigned int dwFontSize)
{
	FT_Error tError;
		
	tError = FT_Init_FreeType(&g_tLibrary);			   /* initialize library */
	/* error handling omitted */
	if(tError){
		DBG_PRINTF("FT_Init_FreeType failed!\n");
		return -1;
	}

	tError = FT_New_Face(g_tLibrary, pcFontFile, 0, &g_tFace); /* create face object */
	/* error handling omitted */
	if(tError){
		DBG_PRINTF("FT_New_Face failed!\n");
		return -1;
	}
	g_tSlot = g_tFace->glyph;

	tError = FT_Set_Pixel_Sizes(g_tFace, dwFontSize, 0);//set dwFontSize x dwFontSize  /* set character size */
	if(tError){
		DBG_PRINTF("FT_Set_Pixel_Sizes failed : %d \n", dwFontSize);
		return -1;
	}
	
	return 0;
}

static int 
FreeTypeGetFontBitmap(unsigned int dwCode, PT_FontBitMap ptFontBitMap)
{
	FT_Error tError;
	
	//把当前的坐标读出来
	int iPenX = ptFontBitMap->iCurOriginX;
	int iPenY = ptFontBitMap->iCurOriginY;
	
	/* load glyph image into the slot (erase previous one) */
	//FT_LOAD_MONOCHROME标志转换为单色位图
	tError = FT_Load_Char( g_tFace, dwCode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME );
	if ( tError ){
		DBG_PRINTF("FT_Load_Char error for code : 0x%x !\n", dwCode);
		return -1;
	}

	ptFontBitMap->iXLeft       = iPenX + g_tSlot->bitmap_left;
	ptFontBitMap->iYTop        = iPenY - g_tSlot->bitmap_top;
	ptFontBitMap->iXMax        = ptFontBitMap->iXLeft + g_tSlot->bitmap.width;
	ptFontBitMap->iYMax        = ptFontBitMap->iYTop  + g_tSlot->bitmap.rows;
	ptFontBitMap->iBpp 		   = 1;//什么时候设为8，什么时候设为1 ?
	ptFontBitMap->iPitch 	   = g_tSlot->bitmap.pitch;
//	ptFontBitMap->iCurOriginX = ;
//	ptFontBitMap->iCurOriginY = ;
	ptFontBitMap->iNextOriginX = iPenX + g_tSlot->advance.x / 64;
	ptFontBitMap->iNextOriginY = iPenY;
	ptFontBitMap->pucBuffer    = g_tSlot->bitmap.buffer;

	return 0;
	
}
	
static T_FontOpr g_tFreeTypeFontOpr = {
	.name          = "freetype", 
	.FontInit      = FreeTypeFontInit,
	.GetFontBitmap = FreeTypeGetFontBitmap,
};

/* 注册freetype 结构体 */
int 
FreeTypeInit(void)
{
	return RegisterFontOpr(&g_tFreeTypeFontOpr);
}
	


