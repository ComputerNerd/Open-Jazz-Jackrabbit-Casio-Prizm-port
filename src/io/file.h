
/**
 *
 * @file file.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 3rd February 2009: Created file.h from parts of OpenJazz.h
 *
 * @section Licence
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#ifndef _FILE_H
#define _FILE_H


#include "OpenJazz.h"
#ifndef CASIO
#include <SDL/SDL.h>
#endif
#include <stdio.h>
#include "surface.h"

// Classes

/// File i/o
class File {

	private:
		char* filePath;
		bool open (const char* path, const char* name, bool write);
	public:
		#ifdef CASIO
		int file=-1;
		#else
		FILE* file;
		#endif
		File                           (const char* name, bool write);
		~File                          ();

		int                getSize     ();
		void               seek        (int offset, bool reset);
		int                tell        ();
		unsigned char      loadChar    ();
		void               storeChar   (unsigned char val);
		unsigned short int loadShort   ();
		unsigned short int loadShort   (unsigned short int max);
		void               storeShort  (unsigned short int val);
		signed long int    loadInt     ();
		void               storeInt    (signed long int val);
		unsigned char*		loadBlock   (int length);
		void				loadBlock   (int length,unsigned char * buf);
		unsigned char*		loadRLE     (int length);
		void				loadRLE     (int length,unsigned char * buffer);
		void               skipRLE     ();
		//unsigned char*     loadLZ      (int compressedLength, int length);
		char*				loadString();
		void				skipString();
		void				loadMiniSurface (int width, int height,unsigned char * pixels,struct miniSurface * surf);
		unsigned char*		loadPixels		(int length);
		void				loadPixels		(int length,unsigned char * sorted);
		void				loadPixels		(int length, int key,unsigned char * sorted);
		unsigned char*		loadPixels		(int length, int key);
		void				loadPalette		(unsigned short* palette, bool rle = true);

};

/// Directory path
class Path {

	public:
		Path* next; ///< Next path to check
		const char* path; ///< Path

		Path  (Path* newNext,const char* newPath);
		~Path ();

};


// Variable

// Paths to files
EXTERN Path* firstPath;

#endif

