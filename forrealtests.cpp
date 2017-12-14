/*********************************************************************************/
/* imageproc.c									 */
/* Usage: imageproc in_file_name out_file_name width height			 */
/*********************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "CImg.h"
using namespace std;
using namespace cimg_library;

int main(int argc, char *argv[])
{
	FILE  *in, *out;
	int   j, k, width, height;
	int ** image_in, ** image_out, ** image_medium, ** image_medtwo;
	float sum1, sum2;
	float new_T, old_T, delta_T;
	long count1, count2;
	int value = 0; //this holds the value after the mask has been applied
	int lamda = 3; // big lamda means big sharpness
	double intensities[256];
	int equalized[256];
	int temp;
	double tempprob = 0;
	double middleman[256];
	int groupOneCount = 0;
	int groupTwoCount = 0;
	int groupOneAvg = 0;
	int groupTwoAvg = 0;
	int a = 5;
	int threshold = 150;
	int newThreshold = 0;
	float window[25];
	int n;
	
	if(argc<5) { printf("ERROR: Insufficient parameters!\n"); return(1);}

	width = atoi(argv[3]);
	height = atoi(argv[4]);

	image_in = (int**) calloc(height, sizeof(int*));
	if(!image_in)
	{
		printf("Error: Can't allocate memmory!\n");
		return(1);
	}

	image_medium = (int**)calloc(height, sizeof(int*));
		if (!image_medium)
		{
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}
	image_medtwo = (int**)calloc(height, sizeof(int*));
	        if (!image_medtwo)
		{
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}

	image_out = (int**) calloc(height, sizeof(int*));
	if(!image_out)
	{
		printf("Error: Can't allocate memmory!\n");
		return(1);
	}
	
	for (j=0; j<height; j++)
	{
		image_in[j] = (int *) calloc(width, sizeof(int));
		if(!image_in[j])
		{
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}

		image_medium[j] = (int *)calloc(width, sizeof(int));
		if (!image_medium[j])
		{				
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}
		
		image_medtwo[j] = (int *)calloc(width, sizeof(int));
		if (!image_medtwo[j])
		{
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}
				
		image_out[j] = (int *) calloc(width, sizeof(int));
 		if(!image_out[j])
		{
			printf("Error: Can't allocate memmory!\n");
			return(1);
		}

	}

	if((in=fopen(argv[1],"rb"))==NULL)
	{
		printf("ERROR: Can't open in_file!\n");
		return(1);
	}

	if((out=fopen(argv[2],"wb"))==NULL)
	{
		printf("ERROR: Can't open out_file!\n");
		return(1);
	}

	for (j=0; j<height; j++)
		for (k=0; k<width; k++)
		{
			if((image_in[j][k]=getc(in))==EOF)
			{
				printf("ERROR: Can't read from in_file!\n");
				return(1);
			}
		}
		if(fclose(in)==EOF)
		{
			printf("ERROR: Can't close in_file!\n");
			return(1);
		}

/* display image_in */
	CImg<int> image_disp(width, height,1,1,0);
	/* CImg<type> image_name(width,height,temporal_frame_number,color_plane_number,initial_value) */

	for(j=0; j<height; j++)
		for (k=0; k<width; k++)
		{
			image_disp(k,j,0,0) = image_in[j][k];
		}
	CImgDisplay disp_in(image_disp,"Image_In",0);
	/* CImgDisplay display_name(image_displayed, "window title", normalization_factor) */

/********************************************************************/
/* Image Processing begins                                          */
/********************************************************************/
	//create a binary image
	
	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			image_medium[j][k] = image_in[j][k];
		}
	}
	
	/*while(1)
	{
		groupOneAvg = groupOneCount = groupTwoAvg = groupTwoCount = 0;
		for (j=0;j<height;j++)
			for (k=0; k<width; k++)
			{
				value = image_in[j][k];
				if (value > threshold)
				{
					groupOneCount = groupOneCount + 1;
					groupOneAvg = groupOneAvg + value;
					image_medium[j][k] = 255;
				}
				else
				{
					groupTwoCount = groupTwoCount + 1;
					groupTwoAvg = groupTwoAvg + value;
					image_medium[j][k] = 0;
				}
			}
		groupOneAvg = groupOneAvg/groupOneCount;
		groupTwoAvg = groupTwoAvg/groupTwoCount;
		
		newThreshold = (groupOneAvg + groupTwoAvg)/2;

		if ((newThreshold - threshold) < a)
		{
			threshold = newThreshold;
			for (j=0; j<height; j++)
			{
				for (k=1; k < width; k++)
				{
					value = image_in[j][k];
					if (value > threshold)
					{
						image_medium[j][k] = 255;
					}
					else
					{
						image_medium[j][k] = 0;
					}
					else
					{
						groupTwoCount = groupTwoCount + 1;
						groupTwoAvg = groupTwoAvg + value;
						image_medium[j][k] = 0;
					}
				}
			}
			break;
		}
		else
		{
			threshold = newThreshold;
		}
	}
	
	for (int i = 0; i < 256; i++)
	{
		intensities[i] = 0;
		middleman[i] = 0;
		equalized[i] = 0;
	}
	
	for (j = 1; j < height - 1; j++)
	{
		for (k = 1; k < width - 1; k++)
		{
			//single step laplacian operator
			image_out[j][k] = (4 * image_medium[j][k]) - (image_medium[j - 1][k] + image_medium[j + 1][k] + image_medium[j][k - 1] + image_medium[j][k + 1]);
			if (image_out[j][k] > 255)
				image_out[j][k] = 255; //if too high, make it max intensity
			else if (image_out[j][k] < 0)
				image_out[j][k] = 0; // if too low, make it min intensity
		}

	}
	
	for (j = 0; j < height; j++)
	{
		for(k = 0; k < width; k++)
		{
			for (int i = 0; i < 256; i++)
			{
				if (image_in[j][k] == i)  // is that pixel equal to intensity i?
				{
					intensities[i] = intensities[i] + 1; // increment how many pixels are in that intensity
				}
			}
		}
	}
	for (int i = 0; i < 256; i++)
	{
		middleman[i] = intensities[i] / (height*width);  //gives a probability between 0 and 1 for intensity instead of a pixel count
	}
	
	for (int i = 0; i < 256; i++)
	{
		tempprob = tempprob + middleman[i];  // summation of all intensity values
		equalized[i] = round(255 * tempprob);
	}
	
	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			temp= image_in[j][k];
			image_medium[j][k] = equalized[temp];
		}
	}
	*/
	/*
	for (j=0; j<height; j++)
		for (k=0; k<height; k++)
		{
			if(j==0 || k==0 || k==1 || j==1 || j==height-1 || k==width-1 || j==height-2 || k==width-2)
			image_medium[j][k]=image_in[j][k];
			else
			{
				window[0] = image_medium[j-2][k-2];
				window[1] = image_medium[j-1][k-2];
				window[2] = image_medium[j][k-2];
				window[3] = image_medium[j+1][k-2];
				window[4] = image_medium[j+2][k-2];
				window[5] = image_medium[j-2][k-1];
				window[6] = image_medium[j-1][k-1];
				window[7] = image_medium[j][k-1];
				window[8] = image_medium[j+1][k-1];
				window[9] = image_medium[j+2][k-1];
				window[10] = image_medium[j-2][k];
				window[11] = image_medium[j-1][k];
				window[12] = image_medium[j][k];
				window[13] = image_medium[j+1][k];
				window[14] = image_medium[j+2][k];
				window[15] = image_medium[j-2][k+1];
				window[16] = image_medium[j-1][k+1];
				window[17] = image_medium[j][k+1];
				window[18] = image_medium[j+1][k+1];
				window[19] = image_medium[j+2][k+1];
				window[20] = image_medium[j-2][k+2];
				window[21] = image_medium[j-1][k+2];
				window[22] = image_medium[j][k+2];
				window[23] = image_medium[j+1][k+2];
				window[24] = image_medium[j+2][k+2];
				n = sizeof(window)/sizeof(window[0]);
				sort(window, window+n);
				window[12] = (window[12] > 255 ? 255: window[12]);
				window[12] = (window[12] < 0 ? 0: window[12]);
				image_medtwo[j][k] = (int)window[12];
			}
		}
	
	*/
	  for (j = 0; j < height; j++)
	  {
	  	  for (k = 0; k < width; k++)
		  {
			  image_out[j][k] = image_in[j][k];
		  }					         
	  }
          while(1)
	  {
		  groupOneAvg = groupOneCount = groupTwoAvg = groupTwoCount = 0;
		  for (j=0;j<height;j++)
			  for (k=0; k<width; k++)
			  {
				  value = image_in[j][k];
				  if (value > threshold)
				  {
					  groupOneCount = groupOneCount + 1;
					  groupOneAvg = groupOneAvg + value;
					  image_medium[j][k] = 255;
				  }
				  else
				  {
					 groupTwoCount = groupTwoCount + 1;
					 groupTwoAvg = groupTwoAvg + value;
					 image_medium[j][k] = 0;
				  }
			  }
		 groupOneAvg = groupOneAvg/groupOneCount;
		 groupTwoAvg = groupTwoAvg/groupTwoCount;
		 newThreshold = (groupOneAvg + groupTwoAvg)/2;
	         if ((newThreshold - threshold) < a)
		 {
			threshold = newThreshold;
			for (j=0; j<height; j++)
			{
				for (k=1; k < width; k++)
				{
					value = image_in[j][k];
					if (value > threshold)
					{
						image_medium[j][k] = 255;
					}
					else
					{
						image_medium[j][k] = 0;
					}
				}
			}
			break;
		}
		else
		{
			threshold = newThreshold;
		}
	}




	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			image_out[j][k] = image_in[j][k];
		}
	}
	
	//first filter - horizontal line
	for (j = 1; j < height - 1; j++)
	{
		for (k = 1; k < width - 1; k++)
		{
			value = -1 * image_medium[j - 1][k - 1] + -1 * image_medium[j - 1][k] + -1 * image_medium[j - 1][k + 1] + 2 * image_medium[j][k - 1] + 2 * image_medium[j][k] + 2 * image_medium[j][k + 1] + -1 * image_medium[j + 1][k - 1] + -1 * image_medium[j + 1][k] + -1 * image_medium[j + 1][k + 1];
			if (value >= 100)
			{
				image_out[j][k] = 255;
			}
		}
	}
	// second filter - vertical line
	for (j = 1; j < height - 1; j++)
	{
		for (k = 1; k < width - 1; k++)
		{
			value = -1 * image_medium[j - 1][k - 1] + 2 * image_medium[j - 1][k] + -1 * image_medium[j - 1][k + 1] + -1 * image_medium[j][k - 1] + 2 * image_medium[j][k] + -1 * image_medium[j][k + 1] + -1 * image_medium[j + 1][k - 1] + 2 * image_medium[j + 1][k] + -1 * image_medium[j + 1][k + 1];
			if (value >= 100)
			{
				image_out[j][k] = 255;
			}
		}
	}
	//third filter +45 degrees
	for (j = 1; j < height - 1; j++)
	{
		for (k = 1; k < width - 1; k++)
		{
			value = -1 * image_medium[j - 1][k - 1] + -1 * image_medium[j - 1][k] + 2 * image_medium[j - 1][k + 1] + -1 * image_medium[j][k - 1] + 2 * image_medium[j][k] + -1 * image_medium[j][k + 1] + 2 * image_medium[j + 1][k - 1] + -1 * image_medium[j + 1][k] + -1 * image_medium[j + 1][k + 1];
			if (value >= 100)
			{
				image_out[j][k] = 255;
			}
		}
	}
	// final filter -45 degrees
	for (j = 1; j < height - 1; j++)
	{
		for (k = 1; k < width - 1; k++)
		{
			value = 2 * image_medium[j - 1][k - 1] + -1 * image_medium[j - 1][k] + -1 * image_medium[j - 1][k + 1] + -1 * image_medium[j][k - 1] + 2 * image_medium[j][k] + -1 * image_medium[j][k + 1] + -1 * image_medium[j + 1][k - 1] + -1 * image_medium[j + 1][k] + 2 * image_medium[j + 1][k + 1];
			if (value >= 100)
			{
				image_out[j][k] = 255;
			}
		}
	}
	
	/*
	// sharpening the image
	image_in.blur(2.5);
	for (j = 1; j < height-1; j++)
	{
		for (k = 1; k < width-1; k++)
		{
			value = 8 * image_in[j][k] - image_in[j - 1][k - 1] - image_in[j - 1][k] - image_in[j - 1][k + 1] - image_in[j][k - 1] - image_in[j][k + 1] - image_in[j + 1][k - 1] - image_in[j + 1][k] - image_in[j + 1][k + 1];
			value = lamda*value + image_in[j][k];
			if (value > 255)
			{
				value = 255;
			}
			else if (value < 0)
			{
				value = 0;
			}
			
			image_out[j][k] = value;
		}
	}
	*/
	/*
	//smoothing image (averaging method)
	for (j = 2; j < height - 2; j++)
	{
		for (k = 2; k < width - 2; k++) // 5x5 matrix
		{
			value = 0;
			for (int yshift = -2; yshift <= 2; yshift++)
			{
				for (int xshift = -2; xshift <= 2; xshift++)					{
					value += image_in[j + yshift][k + xshift];
				}
			}
			value = value / 25;
			//value = (image_in[j - 1][k - 1] + image_in[j - 1][k] + image_in[j - 1][k + 1] + image_in[j][k - 1] + image_in[j][k] + image_in[j][k + 1] + image_in[j + 1][k-1] + image_in[j + 1][k] + image_in[j+1][k+1])/9;
			if (value > 255)
			{
				value = 255;
			}
			else if (value < 0)
			{
				value = 0;
			}
			
			image_out[j][k] = value;		
		}
	}
	
	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			image_medium[j][k] = 255 - image_in[j][k];
		}
	}
	*/
	/*
	for (j = 0; j < height; j++)
	{
		for (k = 0; k < width; k++)
		{
			image_out[j][k] = image_medium[j][k];	
		}
	}
	*/
	
	
/********************************************************************/
/* Image Processing ends                                            */
/********************************************************************/

	/* display image_out */
	for (j=0; j<height; j++)
		for (k=0; k<width; k++)
		{
			image_disp(k,j,0,0) = image_out[j][k];
		}
	CImgDisplay disp_out(image_disp,"Image_Out",0);
	
	/* save image_out into out_file in RAW format */
	for (j=0; j<height; j++)
		for (k=0; k<width; k++)
		{
			if((putc(image_out[j][k],out))==EOF)
			{
				printf("ERROR: Can't write to out_file!\n");
				return(1);
			}
		}
	
	if(fclose(out)==EOF)
	{
		printf("ERROR: Can't close out_file!\n");
		return(1);
	}
	
	/* closing */
	
	while (!disp_in.is_closed())
		disp_in.wait();
	while (!disp_out.is_closed())
		disp_out.wait();
	
	for (j=0; j<height; j++)
	{
		free(image_in[j]);
		free(image_out[j]);
	}
	free(image_in);
	free(image_out);
	
	return 0;
}
