/***************************************************************************************
Similarly, the rough outline of a JPEG decompression operation is:

Allocate and initialize a JPEG decompression object    // ����ͳ�ʼ��һ��decompression�ṹ��
Specify the source of the compressed data (eg, a file) // ָ��Դ�ļ�
Call jpeg_read_header() to obtain image info		      // ��jpeg_read_header���jpgͼ����Ϣ
Set parameters for decompression		               // ���ý�ѹ����,����Ŵ���С
jpeg_start_decompress(...); 			               // ������ѹ��jpeg_start_decompress
while (scan lines remain to be read)
	jpeg_read_scanlines(...);		                   // ѭ������jpeg_read_scanlines
jpeg_finish_decompress(...);			               // jpeg_finish_decompress������ѹ
Release the JPEG decompression object                  // �ͷ�decompression�ṹ��
***************************************************************************************/
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>

/* Uage : jpg2rgb <jpg_file>
 */
int main(int argc, char *argv[])
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE *infile;
	
	//����ͳ�ʼ��һ��decompression�ṹ��
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//ָ��.jpgԴ�ļ�
	if ((infile = fopen(argv[1], "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", argv[1]);	    
		return -1;	
	}	
	jpeg_stdio_src(&cinfo, infile);

	// ��jpeg_read_header���jpg�ļ���Ϣͷ
	jpeg_read_header(&cinfo, TRUE);
	
	//jpgԴ�ļ���Ϣ
	printf("image_width = %d\n", cinfo.image_width);
	printf("image_height = %d\n", cinfo.image_height);
	printf("num_components = %d\n", cinfo.num_components);
	
	// ���ý�ѹ����,����Ŵ���С
	printf("enter M/N:\n");
	scanf("%d/%d", &cinfo.scale_num,  &cinfo.scale_denom);
	printf("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);
	//cinfo.scale_num    = 1;
	//cinfo.scale_denom  = 2;////����Ϊ1/2
	
	// ������ѹ��jpeg_start_decompress	
	jpeg_start_decompress(&cinfo);

	//�����ͼ���ļ���Ϣ
	printf("output_width = %d\n", cinfo.output_width);
	printf("output_height = %d\n", cinfo.output_height);
	printf("output_components = %d\n", cinfo.output_components);

	// ѭ������jpeg_read_scanlines��һ��һ�еػ�ý�ѹ������

	jpeg_finish_decompress(&cinfo);//������ѹ
	jpeg_destroy_decompress(&cinfo);//�ͷŽ�ѹʹ�ýṹ��	
}

