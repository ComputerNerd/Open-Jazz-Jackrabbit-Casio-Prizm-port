
/**
 *
 * @file util.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 23rd August 2005: Created OpenJazz.h
 * 30th April 2010: Created util.h from parts of OpenJazz.h
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


#ifndef _UTIL_H
#define _UTIL_H


#include "OpenJazz.h"


// Macros

#ifdef VERBOSE
	#define LOG(message, detail) log(message, detail)
	#define LOGRESULT(message, expression) log(message, expression)
#else
	#define LOG(message, detail)
	#define LOGRESULT(message, expression) expression
#endif


// Variable

/// Trigonometric function look-up table
extern const fixed sinLut[1024];


// Functions
unsigned char nearestIndex(unsigned char r,unsigned char g,unsigned char b,unsigned char * pal,unsigned short amt);

EXTERN bool               fileExists           (const char *fileName);
EXTERN unsigned short int createShort          (unsigned char* data);
EXTERN int                createInt            (unsigned char* data);
EXTERN char*              createString         (const char *string);
EXTERN char*              createString         (const char *first, const char *second);
EXTERN char*              createFileName       (const char *type, int extension);
EXTERN char*              createFileName       (const char *type, const char *extension);
EXTERN char*              createFileName       (const char *type, int level, int extension);
EXTERN char*              createEditableString (const char *string);
EXTERN void               log                  (const char *message);
EXTERN void               log                  (const char *message, const char *detail);
EXTERN void               log                  (const char *message, int number);
EXTERN void               logError             (const char *message, const char *detail);
//EXTERN fixed              fSin                 (fixed angle);
//EXTERN fixed              fCos                 (fixed angle);//Replace these with macros to reduce function overhead
//#define fSin(angle) (sinLut[angle & 1023])
static fixed fSin(const fixed angle){return sinLut[angle & 1023];}
//#define fCos(angle) (sinLut[(angle + 256) & 1023])
static fixed fCos(const fixed angle){return sinLut[(angle + 256) & 1023];}
#endif

