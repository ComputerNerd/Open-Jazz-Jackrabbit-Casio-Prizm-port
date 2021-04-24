
/**
 *
 * @file file.cpp
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created main.c
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 3rd February 2009: Created file.cpp from parts of util.cpp
 *
 * @section Licence
 * Copyright (c) 2005-2012 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * @section Description
 * Deals with files.
 *
 */


#include "file.h"

#include "io/gfx/video.h"
#include "util.h"

#include <string.h>
//#include <zlib.h>
#include "surface.h"
#ifdef CASIO
	#include "platforms/casio.h"
	#include <fxcg/file.h>
	#include <alloca.h>
	#include <fxcg/display.h>
	#include <fxcg/keyboard.h>
	#include <fxcg/misc.h>
#endif
/**
 * Try opening a file from the available paths
 *
 * @param name File name
 * @param write Whether or not the file can be written to
 */
#ifndef CASIO
static int fileCnt;
#endif
File::File (const char* name, bool write) {
	#ifdef CASIO
	file=-1;
	#else
	file=0;
	#endif
	Path* path;

	path = firstPath;

	while (path) {

		if (open(path->path, name, write)) {
#ifndef CASIO
			++fileCnt;
			printf("*** fileCnt: %d\n", fileCnt);
#endif
			return;
		}
		path = path->next;

	}
	#ifndef CASIO
	log("Could not open file", name);
	#endif
	throw E_FILE;
}


/**
 * Delete the file object.
 */
File::~File () {
#ifdef CASIO
	if(file>=0)
		Bfile_CloseFile_OS(file);
#else
	if(file>0)
		fclose(file);
	--fileCnt;
	printf("*** fileCnt: %d\n", fileCnt);
#endif
#ifdef VERBOSE
	log("Closed file", filePath);
#endif
	if(filePath)
		delete[] filePath;
	return;
}
/**
 * Try opening a file from the given path
 *
 * @param path Directory path
 * @param name File name
 * @param write Whether or not the file can be written to
 */
bool File::open (const char* path, const char* name, bool write) {
	if(write){
		#ifdef CASIO
			casioQuit("No write files");
		#else
			puts("No write files");
			exit(-1);
		#endif
		return false;
	}
	// Create the file path for the given directory
	filePath = createString(path, name);
	// Open the file from the path
	#ifdef CASIO
		DmaWaitNext();
		unsigned int flen=strlen(filePath);
		unsigned short * strC=(unsigned short *)alloca(flen*2+2);
		Bfile_StrToName_ncpy(strC,filePath,flen+1);
		file=Bfile_OpenFile_OS(strC,0,0);
	#else
		file = fopen(filePath,"rb");
	#endif
	#ifdef CASIO
	if (file>=0){
	#else
	if (file){
	#endif
#ifdef VERBOSE
		log("Opened file", filePath);
#endif
		return true;
	}
	#ifdef CASIO
		drawStrL(4,"Cannot open:");
		drawStrL(5,filePath);
		char buf[16];
		itoa(file,(unsigned char *)buf);
		drawStrL(6,buf);
	#else
		printf("Cannot open %s\n",filePath);
	#endif
	delete[] filePath;
	filePath=0;
	return false;
}


/**
 * Get the size of the file.
 *
 * @return The size of the file
 */
int File::getSize () {
#ifdef CASIO
	return Bfile_GetFileSize_OS(file);
#else
	int pos, size;
	pos = ftell(file);
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
#endif
}


/**
 * Get the current read/write location within the file.
 *
 * @return The current location
 */
int File::tell () {
	#ifdef CASIO
		return Bfile_TellFile_OS(file);
	#else
		return ftell(file);
	#endif
}


/**
 * Set the read/write location within the file.
 *
 * @param offset The new offset
 * @param reset Whether to offset from the current location or the start of the file
 */
void File::seek (int offset, bool reset) {
	#ifdef CASIO
		if(reset)
			Bfile_SeekFile_OS(file,offset);
		else
			Bfile_SeekFile_OS(file,Bfile_TellFile_OS(file)+offset);
	#else
		fseek(file, offset, reset ? SEEK_SET: SEEK_CUR);
	#endif
}


/**
 * Load an unsigned char from the file.
 *
 * @return The value read
 */
unsigned char File::loadChar () {
	#ifdef CASIO
		unsigned char temp;
		Bfile_ReadFile_OS(file,&temp,1,-1);
		return temp;
	#else
		return fgetc(file);
	#endif
}


void File::storeChar (unsigned char val) {
	#ifndef CASIO
	fputc(val, file);
	#endif
}

/**
 * Load an unsigned short int from the file.
 *
 * @return The value read
 */
unsigned short int File::loadShort(){

	unsigned short val;
	#ifdef CASIO
		Bfile_ReadFile_OS(file,&val,2,-1);
		val=__builtin_bswap16(val);
	#else
		val = fgetc(file);
		val |= fgetc(file) << 8;
	#endif
	return val;
}


/**
 * Load an unsigned short int with an upper limit from the file.
 *
 * @return The value read
 */
unsigned short int File::loadShort (unsigned short int max) {

	unsigned short int val;

	val = loadShort();

	if (val > max) {
		#ifdef CASIO
			drawStrL(1,"Oversized value");
		#else
			logError("Oversized value in file", filePath);
		#endif
		return max;

	}

	return val;

}


void File::storeShort(unsigned short int val){
	#ifndef CASIO
		fputc(val & 255, file);
		fputc(val >> 8, file);
	#endif
}


/**
 * Load a signed long int from the file.
 *
 * @return The value read
 */
signed long int File::loadInt () {
	unsigned long int val;
	#ifdef CASIO
		/*val = fgetc(file)<<24;
		val |= fgetc(file)<<16;
		val |= fgetc(file)<<8;
		val |= fgetc(file);*/
		/*unsigned char * v=(unsigned char *)&val;
		Bfile_ReadFile_OS(file,v+3,1,-1);
		Bfile_ReadFile_OS(file,v+2,1,-1);
		Bfile_ReadFile_OS(file,v+1,1,-1);
		Bfile_ReadFile_OS(file,v,1,-1);*/
		Bfile_ReadFile_OS(file,&val,4,-1);
		/*val = ((val>>24)&0xff) | // move byte 3 to byte 0
                    ((val<<8)&0xff0000) | // move byte 1 to byte 2
                    ((val>>8)&0xff00) | // move byte 2 to byte 1
                    ((val<<24)&0xff000000); // byte 0 to byte 3*/
		val=__builtin_bswap32(val);//Make sure you are using gcc 4.8 or later or other compiler which supports such features this should use the swap instruction
	#else
		val = fgetc(file);
		val |= fgetc(file) << 8;
		val |= fgetc(file) << 16;
		val |= fgetc(file) << 24;
	#endif
	return *((signed long int *)&val);
}


void File::storeInt (signed long int val) {
	unsigned long int uval;
	uval = *((unsigned long int *)&val);
	#ifndef CASIO
		fputc(uval & 255, file);
		fputc((uval >> 8) & 255, file);
		fputc((uval >> 16) & 255, file);
		fputc(uval >> 24, file);
	#endif
	return;
}


/**
 * Load a block of uncompressed data from the file.
 *
 * @param length The length of the block
 *
 * @return Buffer containing the block of data
 */
unsigned char * File::loadBlock (int length) {
	unsigned char *buffer;
	buffer = new unsigned char[length];
	loadBlock(length,buffer);
	return buffer;
}
void File::loadBlock(int length,unsigned char * buf){
	#ifdef CASIO
		Bfile_ReadFile_OS(file,buf,length,-1);
	#else
		fread(buf, 1, length, file);
	#endif
}



/**
 * Load a block of RLE compressed data from the file.
 *
 * @param length The length of the uncompressed block
 *
 * @return Buffer containing the uncompressed data
 */
void File::loadRLE (int length,unsigned char* buffer) {

	int pos, count, next;
	unsigned char byte,rle;
	// Determine the offset that follows the block
	#ifdef CASIO
		{unsigned short tmp;
		Bfile_ReadFile_OS(file,&tmp,2,-1);
		next=__builtin_bswap16(tmp);}
	#else
		next = fgetc(file);
		next |= fgetc(file) << 8;
	#endif
	#ifdef CASIO
		next+=Bfile_TellFile_OS(file);
	#else
		next += ftell(file);
	#endif
	pos = 0;
	/*#ifdef CASIO
	itoa(next,(unsigned char *)bufP);
	clearLine(5);
	PrintXY(1,5,bufP-2,0x20,TEXT_COLOR_WHITE);
	Bdisp_PutDisp_DD();
	#else
		printf("next: %d length: %d\n",next,length);
	#endif*/
	while (pos < length) {
		#ifdef CASIO
			Bfile_ReadFile_OS(file,&rle,1,-1);
			//show percentage
			/*itoa(pos,(unsigned char *)bufP);
			clearLine(7);
			PrintXY(1,7,bufP-2,0x20,TEXT_COLOR_WHITE);
			Bdisp_PutDisp_DD();*/
		#else
			rle = fgetc(file);
		#endif
		if (rle & 128) {
			#ifdef CASIO
				Bfile_ReadFile_OS(file,&byte,1,-1);
			#else
				byte = fgetc(file);
			#endif
			for (count = 0; count < (rle & 127); count++) {
				buffer[pos++] = byte;
				if (pos >= length) break;
			}
			/*#ifdef CASIO
				itoa(pos,(unsigned char *)bufP);
				clearLine(4);
				PrintXY(1,4,bufP-2,0x20,TEXT_COLOR_RED);
				Bdisp_PutDisp_DD();
			#endif*/
		} else if (rle) {

			/*for (count = 0; count < rle; count++) {
				#ifdef CASIO
					Bfile_ReadFile_OS(file,&buffer[pos++],1,-1);
				#else
					buffer[pos++] = fgetc(file);
				#endif
				if (pos >= length) break;

			}*/

			if((pos+rle)>=length){
				//read for length then break
				if((length-pos)>0){
				#ifdef CASIO
					Bfile_ReadFile_OS(file,&buffer[pos],(length-pos),-1);
				#else
					if(fread(&buffer[pos],1,(length-pos),file)!=(length-pos)){
						puts("Errors byte");//punny yes?
					}
				#endif
				pos+=(length-pos);
				}
			}else{
				#ifdef CASIO
					Bfile_ReadFile_OS(file,&buffer[pos],rle,-1);
				#else
					if(fread(&buffer[pos],1,rle,file)!=rle){
						puts("Errors byte");
					}
				#endif
				pos+=rle;
				/*#ifdef CASIO
					itoa(pos,(unsigned char *)bufP);
					clearLine(4);
					PrintXY(1,4,bufP-2,0x20,TEXT_COLOR_BLUE);
					Bdisp_PutDisp_DD();
				#endif*/
			}
		} else{
			#ifdef CASIO
				Bfile_ReadFile_OS(file,&buffer[pos++],1,-1);
				/*itoa(pos,(unsigned char *)bufP);
				clearLine(4);
				PrintXY(1,4,bufP-2,0x20,TEXT_COLOR_GREEN);
				Bdisp_PutDisp_DD();*/
			#else
				buffer[pos++] = fgetc(file);
			#endif
		}
	}
	#ifdef CASIO
		Bfile_SeekFile_OS(file,next);
	#else
		fseek(file, next, SEEK_SET);
	#endif
}

/**
 * Skip past a block of RLE compressed data in the file.
 */
void File::skipRLE(){
	unsigned short next;
	#ifdef CASIO
		//unsigned char * n=(unsigned char *)&next;
		//Bfile_ReadFile_OS(file,n+1,1,-1);
		//Bfile_ReadFile_OS(file,n,1,-1);
		Bfile_ReadFile_OS(file,&next,2,-1);
		next=__builtin_bswap16(next);
	#else
		next = fgetc(file);
		next |= fgetc(file) << 8;
	#endif
	seek(next,false);
}


/**
 * Load a string from the file.
 *
 * @return The new string
 */
char * File::loadString () {
	char *string;
	unsigned char length;
	int count;
	#ifdef CASIO
		Bfile_ReadFile_OS(file,&length,1,-1);
	#else
		length = fgetc(file);
	#endif
	if (length) {
		string = new char[length + 1];
		#ifdef CASIO
			Bfile_ReadFile_OS(file,string,length,-1);
		#else
			fread(string, 1, length, file);
		#endif
	}else {
		// If the length is not given, assume it is an 8.3 file name
		string = new char[13];
		for (count = 0; count < 9; count++) {
			#ifdef CASIO
				Bfile_ReadFile_OS(file,&string[count],1,-1);
			#else
				string[count] = fgetc(file);
			#endif
			if (string[count] == '.') {
				#ifdef CASIO
					Bfile_ReadFile_OS(file,&string[++count],3,-1);
					count+=3;
				#else
					string[++count] = fgetc(file);
					string[++count] = fgetc(file);
					string[++count] = fgetc(file);
					count++;
				#endif
				break;
			}
		}
		length = count;
	}
	string[length] = 0;
	return string;
}


void File::skipString () {
	unsigned char length;
	int count;
	#ifdef CASIO
		Bfile_ReadFile_OS(file,&length,1,-1);
	#else
		length = fgetc(file);
	#endif
	if (length) {
		//string = new char[length + 1];
		seek(length,false);
	}else {
		// If the length is not given, assume it is an 8.3 file name
		char string[14];
		for (count = 0; count < 9; count++) {
			#ifdef CASIO
				Bfile_ReadFile_OS(file,&string[count],1,-1);
			#else
				string[count] = fgetc(file);
			#endif
			if (string[count] == '.') {
				#ifdef CASIO
					//Bfile_ReadFile_OS(file,&string[++count],3,-1);
					//count+=3;
					seek(3,false);
				#else
					string[++count] = fgetc(file);
					string[++count] = fgetc(file);
					string[++count] = fgetc(file);
					count++;
				#endif
				break;
			}
		}
		length = count;
	}
}


void File::loadMiniSurface(int width, int height,unsigned char * pixels,struct miniSurface * surf){
	loadRLE(width * height,pixels);
	initMiniSurface(surf,pixels,width,height);
}

void File::loadPixels(int length,unsigned char * sorted){
	unsigned char* pixels=(unsigned char *)alloca(length);
	//unsigned char* sorted;
	int count;
	//sorted = new unsigned char[length];
	loadBlock(length,pixels);
	// Rearrange pixels in correct order
	for (count = 0; count < length; count++) {
		sorted[count] = pixels[(count >> 2) + ((count & 3) * (length >> 2))];
	}
	//delete[] pixels;
	//return sorted;
}

/**
 * Load a block of scrambled pixel data from the file.
 *
 * @param length The length of the block
 *
 * @return Buffer containing the de-scrambled data
 */
unsigned char* File::loadPixels  (int length) {
	unsigned char * sorted = new unsigned char[length];
	loadPixels(length,sorted);
	return sorted;

}


/**
 * Load a block of scrambled and masked pixel data from the file.
 *
 * @param length The length of the block
 * @param key The transparent pixel value
 *
 * @return Buffer containing the de-scrambled data
 */
void File::loadPixels(int length, int key,unsigned char * sorted) {
	unsigned char* pixels;
	//unsigned char* sorted;
	unsigned char mask;
	int count;
	//sorted = new unsigned char[length];
	pixels = (unsigned char*)alloca(length);//new unsigned char[length];
	// Read the mask
	// Each mask pixel is either 0 or 1
	// Four pixels are packed into the lower end of each byte
	for (count = 0; count < length; count++) {
		#ifdef CASIO
			if (!(count & 3)) Bfile_ReadFile_OS(file,&mask,1,-1);
		#else
			if (!(count & 3)) mask = fgetc(file);
		#endif
		pixels[count] = (mask >> (count & 3)) & 1;
	}
	// Pixels are loaded if the corresponding mask pixel is 1, otherwise
	// the transparent index is used. Pixels are scrambled, so the mask
	// has to be scrambled the same way.
	for (count = 0; count < length;++count) {
		sorted[(count >> 2) + ((count & 3) * (length >> 2))] = pixels[count];
	}
	// Read pixels according to the scrambled mask
	for (count = 0; count < length;++count){
		// Use the transparent pixel
		pixels[count] = key;
		if (sorted[count] == 1) {
			// The unmasked portions are transparent, so no masked
			// portion should be transparent.
			#ifdef CASIO
				while (pixels[count] == key)
					Bfile_ReadFile_OS(file,&pixels[count],1,-1);
			#else
				while (pixels[count] == key) pixels[count] = fgetc(file);
			#endif
		}
	}
	// Rearrange pixels in correct order
	for (count = 0; count < length; count++) {
		sorted[count] = pixels[(count >> 2) + ((count & 3) * (length >> 2))];
	}
	//delete[] pixels;
}
unsigned char* File::loadPixels (int length, int key){
	unsigned char* sorted=new unsigned char[length];
	loadPixels(length,key,sorted);
	return sorted;
}

/**
 * Load a palette from the file.
 *
 * @param palette The palette to be filled with loaded colours
 * @param rle Whether or not the palette data is RLE-encoded
 */
void File::loadPalette (unsigned short* palette, bool rle) {

	unsigned char buffer[768];
	loadPalette6(buffer, rle);
	convertPalette(palette, buffer);
}

void File::convertPalette(unsigned short* palette, unsigned char* buffer) {
	for (int count = 0; count < 256; count++) {
		palette[count] = ((buffer[count * 3]&62)<<10)|(buffer[(count * 3) + 1] << 5)|(buffer[(count * 3) + 2]>>1);
	}
}

void File::loadPalette6(unsigned char* palette, bool rle) {
	if (rle) loadRLE(768, palette);
	else loadBlock(768, palette);
}


/**
 * Create a new directory path object.
 *
 * @param newNext Next path
 * @param newPath The new path
 */
Path::Path (Path* newNext,const char* newPath) {

	next = newNext;
	path = newPath;

	return;

}


/**
 * Delete the directory path object.
 */
Path::~Path () {

	if (next) delete next;
}


