#include "image.h"


Image::Image() {
	width = 0; height = 0;
	pixels = NULL;
}

Image::Image(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new Color[width*height];
	memset(pixels, 0, width * height * sizeof(Color));
}

//copy constructor
Image::Image(const Image& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
}

//assign operator
Image& Image::operator = (const Image& c)
{
	if(pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if(c.pixels)
	{
		pixels = new Color[width*height*sizeof(Color)];
		memcpy(pixels, c.pixels, width*height*sizeof(Color));
	}
	return *this;
}

Image::~Image()
{
	if(pixels) 
		delete pixels;
}



//change image size (the old one will remain in the top-left corner)
void Image::resize(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for(unsigned int x = 0; x < min_width; ++x)
		for(unsigned int y = 0; y < min_height; ++y)
			new_pixels[ y * width + x ] = getPixel(x,y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

//change image size and scale the content
void Image::scale(unsigned int width, unsigned int height)
{
	Color* new_pixels = new Color[width*height];

	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++y)
			new_pixels[ y * width + x ] = getPixel((unsigned int)(this->width * (x / (float)width)), (unsigned int)(this->height * (y / (float)height)) );

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

Image Image::getArea(unsigned int start_x, unsigned int start_y, unsigned int width, unsigned int height)
{
	Image result(width, height);
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height; ++x)
		{
			if( (x + start_x) < this->width && (y + start_y) < this->height) 
				result.setPixelSafe( x, y, getPixel(x + start_x,y + start_y) );
		}
	return result;
}

void Image::flipX()
{
	for(unsigned int x = 0; x < width * 0.5; ++x)
		for(unsigned int y = 0; y < height; ++y)
		{
			Color temp = getPixel(width - x - 1, y);
			setPixelSafe( width - x - 1, y, getPixel(x,y));
			setPixelSafe( x, y, temp );
		}
}

void Image::flipY()
{
	for(unsigned int x = 0; x < width; ++x)
		for(unsigned int y = 0; y < height * 0.5; ++y)
		{
			Color temp = getPixel(x, height - y - 1);
			setPixelSafe( x, height - y - 1, getPixel(x,y) );
			setPixelSafe( x, y, temp );
		}
}


//Loads an image from a TGA file
bool Image::loadTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char TGAcompare[12];
	unsigned char header[6];
	unsigned int bytesPerPixel;
	unsigned int imageSize;

	FILE * file = fopen(filename, "rb");
   	if ( file == NULL || fread(TGAcompare, 1, sizeof(TGAcompare), file) != sizeof(TGAcompare) ||
		memcmp(TGAheader, TGAcompare, sizeof(TGAheader)) != 0 ||
		fread(header, 1, sizeof(header), file) != sizeof(header))
	{
		std::cerr << "File not found: " << filename << std::endl;
		if (file == NULL)
			return NULL;
		else
		{
			fclose(file);
			return NULL;
		}
	}

	TGAInfo* tgainfo = new TGAInfo;
    
	tgainfo->width = header[1] * 256 + header[0];
	tgainfo->height = header[3] * 256 + header[2];
    
	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (header[4] != 24 && header[4] != 32))
	{
		std::cerr << "TGA file seems to have errors or it is compressed, only uncompressed TGAs supported" << std::endl;
		fclose(file);
		delete tgainfo;
		return NULL;
	}
    
	tgainfo->bpp = header[4];
	bytesPerPixel = tgainfo->bpp / 8;
	imageSize = tgainfo->width * tgainfo->height * bytesPerPixel;
    
	tgainfo->data = new unsigned char[imageSize];
    
	if (tgainfo->data == NULL || fread(tgainfo->data, 1, imageSize, file) != imageSize)
	{
		if (tgainfo->data != NULL)
			delete tgainfo->data;
            
		fclose(file);
		delete tgainfo;
		return false;
	}

	fclose(file);

	//save info in image
	if(pixels)
		delete pixels;

	width = tgainfo->width;
	height = tgainfo->height;
	pixels = new Color[width*height];

	//convert to float all pixels
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			unsigned int pos = y * width * bytesPerPixel + x * bytesPerPixel;
			this->setPixelSafe(x , height - y - 1, Color( tgainfo->data[pos+2], tgainfo->data[pos+1], tgainfo->data[pos]) );
		}

	delete tgainfo->data;
	delete tgainfo;

	return true;
}

// Saves the image to a TGA file
bool Image::saveTGA(const char* filename)
{
	unsigned char TGAheader[12] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	FILE *file = fopen(filename, "wb");
	if ( file == NULL )
	{
		fclose(file);
		return false;
	}

	unsigned short header_short[3];
	header_short[0] = width;
	header_short[1] = height;
	unsigned char* header = (unsigned char*)header_short;
	header[4] = 24;
	header[5] = 0;

	//tgainfo->width = header[1] * 256 + header[0];
	//tgainfo->height = header[3] * 256 + header[2];

	fwrite(TGAheader, 1, sizeof(TGAheader), file);
	fwrite(header, 1, 6, file);

	//convert pixels to unsigned char
	unsigned char* bytes = new unsigned char[width*height*3];
	for(unsigned int y = 0; y < height; ++y)
		for(unsigned int x = 0; x < width; ++x)
		{
			Color c = pixels[(height-y-1)*width+x];
			unsigned int pos = (y*width+x)*3;
			bytes[pos+2] = c.r;
			bytes[pos+1] = c.g;
			bytes[pos] = c.b;
		}

	fwrite(bytes, 1, width*height*3, file);
	fclose(file);
	return true;
}


FloatImage::FloatImage(unsigned int width, unsigned int height)
{
	this->width = width;
	this->height = height;
	pixels = new float[width*height];
	memset(pixels, 0, width * height * sizeof(float));
}

//copy constructor
FloatImage::FloatImage(const FloatImage& c) {
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
}

//assign operator
FloatImage& FloatImage::operator = (const FloatImage& c)
{
	if (pixels) delete pixels;
	pixels = NULL;

	width = c.width;
	height = c.height;
	if (c.pixels)
	{
		pixels = new float[width*height * sizeof(float)];
		memcpy(pixels, c.pixels, width*height * sizeof(float));
	}
	return *this;
}

FloatImage::~FloatImage()
{
	if (pixels)
		delete pixels;
}


//change image size (the old one will remain in the top-left corner)
void FloatImage::resize(unsigned int width, unsigned int height)
{
	float* new_pixels = new float[width*height];
	unsigned int min_width = this->width > width ? width : this->width;
	unsigned int min_height = this->height > height ? height : this->height;

	for (unsigned int x = 0; x < min_width; ++x)
		for (unsigned int y = 0; y < min_height; ++y)
			new_pixels[y * width + x] = getPixel(x, y);

	delete pixels;
	this->width = width;
	this->height = height;
	pixels = new_pixels;
}

void Image::line(int x0, int y0, int x1, int y1, int ** minMax, int minY, bool boolean) {

	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;
	float v;
	for (;;) {
		//setPixelSafe(x0, y0, Color(255,255,255));
		if (boolean)
		{
			if (x0 >= 0 && y0 >= 0) {
				if (x0 <= minMax[y0-minY][0])
				{
					minMax[y0-minY][0] = x0;
				}
				if (x0 >= minMax[y0-minY][1])
				{
					minMax[y0-minY][1] = x0;
				}
			}
		}
		
		if (x0 == x1 && y0 == y1) break;
		e2 = err;
		if (e2 > -dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}



}

double area(int x1, int y1, int x2, int y2, int x3, int y3) {
	return abs((x1*(y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

void Image::drawTriangle(int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3,  Color& color, bool fill, Camera* cam, FloatImage& depthbuffer) {


	if (fill == false)
	{
		line(x1, y1, x2, y2,  NULL, 0, false);
		line(x1, y1, x3, y3,  NULL, 0, false);
		line(x2, y2, x3, y3,  NULL, 0, false);
	}
	else
	{

		int yMax = max(y1, max(y2, y3));
		int yMin = min(y1, min(y2, y3));


		// Creating the array to store the edges of the triangle for each pixel row
		int** minMax = new int*[yMax-yMin+1];
		for (int i = 0; i < yMax-yMin+1; i++)
			minMax[i] = new int[2];

		// MinMax initialization
		for (int i = 0; i < yMax-yMin+1; i++)
		{
			minMax[i][0] = this->width + 1;
			minMax[i][1] = -1;
		}

		bool interpolated = true;

		

		line(x1, y1, x2, y2,  minMax, yMin, true);
		line(x1, y1, x3, y3,  minMax, yMin, true);
		line(x2, y2, x3, y3,  minMax, yMin, true);

		float totalArea = area(x1, y1, x2, y2, x3, y3);
		

		// Filling the triangle with minMax

		if (interpolated)
		{
			/*Going from bottom to top*/
			for (int i = yMin; i <= yMax; i++)
			{
				/*Iterate from minimum to maximum and compute the partial area.
				The color output will be the ratio of each partial area to the total area*/
				if (minMax[i-yMin][0] <= minMax[i-yMin][1])
				{
					for (int j = minMax[i-yMin][0]; j <= minMax[i-yMin][1]; j++)
					{
						float partialArea1 = area(j, i, x2, y2, x3, y3) / totalArea;
						float partialArea2 = area(x1, y1, j, i, x3, y3) / totalArea;
						float partialArea3 = area(x1, y1, x2, y2, j, i) / totalArea;

						float distanceZ = abs(cam->eye.z - (z1*partialArea1 + z2 * partialArea2 + z3*partialArea3));
						if (i >= depthbuffer.height) {
							return;
						}
						if (distanceZ <= depthbuffer.getPixel(j, i))
						{
							depthbuffer.setPixel(j, i, distanceZ);
							setPixelSafe(j, i, Color(255 * partialArea1, 255 * partialArea2, 255 * partialArea3));
						}
					}
				}
			}
			
		}
		else
		{
			for (int i = yMin; i <= yMax; i++) 
			{
				if (minMax[i-yMin][0] <= minMax[i-yMin][1])
				{
					for (int j = minMax[i-yMin][0]; j <= minMax[i-yMin][1]; j++)
					{
						float partialArea1 = area(j, i, x2, y2, x3, y3) / totalArea;
						float partialArea2 = area(x1, y1, j, i, x3, y3) / totalArea;
						float partialArea3 = area(x1, y1, x2, y2, j, i) / totalArea;
						float distanceZ = abs(cam->eye.z - (z1*partialArea1  + z2 * partialArea2 + z3 * partialArea3));
						if (i >= depthbuffer.height) {
							return;
						}
						if (distanceZ <= depthbuffer.getPixel(j, i))
						{
							depthbuffer.setPixel(j, i, distanceZ);
							setPixelSafe(j, i, Color(255,255,255));
						}
					}
				}
			}
		}


		// Deallocating the minMax array
		for (int i = 0; i < yMax-yMin+1; i++)
			delete[] minMax[i];
		delete[] minMax;
	}

}
#ifndef IGNORE_LAMBDAS

//you can apply and algorithm for two images and store the result in the first one
//forEachPixel( img, img2, [](Color a, Color b) { return a + b; } );
template <typename F>
void forEachPixel(Image& img, const Image& img2, F f) {
	for(unsigned int pos = 0; pos < img.width * img.height; ++pos)
		img.pixels[pos] = f( img.pixels[pos], img2.pixels[pos] );
}

#endif