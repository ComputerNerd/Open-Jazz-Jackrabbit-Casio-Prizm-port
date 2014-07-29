
/*
 *
 * level.h
 *
 * 31st January 2006: Created level.h from parts of OpenJazz.h
 * 4th February 2009: Created events.h from parts of level.h
 * 19th March 2009: Created sprite.h from parts of level.h
 * 30th March 2010: Created baselevel.h from parts of level.h
 * 29th June 2010: Created jj2level.h from parts of level.h
 *
 * Part of the OpenJazz project
 *
 *
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


/* "Tile" is a flexible term. Here it is used to refer specifically to the
   individual elements of the tile set.
   "Tiles" in the context of level units are referred to as grid elements. */


#ifndef _LEVEL_H
#define _LEVEL_H


#include "baselevel.h"
#include "io/gfx/anim.h"
#include "OpenJazz.h"

#include <SDL/SDL.h>


// Constants

// General
#define LW        256 /* Level width */
#define LH         64 /* Level height */
#define EVENTS    127
#define ELENGTH    32 /* Length of events, in bytes */
#define BULLETS    32
#define BLENGTH    20 /* Length of bullets, in bytes */
#define ANIMS     128
#define PATHS      16
#define TKEY      127 /* Tileset colour key */

// Black palette index
#define LEVEL_BLACK 31

// Fade delays
#define T_START 500
#define T_END   1000


// Datatypes

typedef struct {

	unsigned char tile;  // Indexes the tile set
	unsigned char bg;    // 0 = Effect background, 1 = Black background
	unsigned char event; // Indexes the event set
	unsigned char hits;  // Number of times the event has been shot
	int           time;  /* Point at which the event will do something, e.g. terminate */

} GridElement;

typedef struct {

	short int*    x;
	short int*    y;
	unsigned char length;
	unsigned char node;

} EventPath;


// Classes

class Bullet;
class Event;
class Font;
class LevelPlayer;
class Scene;

class Level : public BaseLevel {

	private:
		SDL_Surface*  tileSet;
		SDL_Surface*  panel;
		SDL_Surface*  panelAmmo[5];
		Event*        events;
		char*         musicFile;
		char*         sceneFile;
		Anim          animSet[ANIMS];
		char          miscAnims[4];
		signed char   bulletSet[BULLETS][BLENGTH];
		signed char   eventSet[EVENTS][ELENGTH];
		char          mask[240][64]; // At most 240 tiles, all with 8 * 8 masks
		GridElement   grid[LH][LW]; // All levels are the same size
		int           soundMap[32];
		SDL_Color     skyPalette[256];
		bool          sky;
		unsigned char skyOrb;
		int           levelNum, worldNum, nextLevelNum, nextWorldNum;
		unsigned char difficulty;
		int           enemies;
		fixed         waterLevel;
		fixed         waterLevelTarget;
		fixed         waterLevelSpeed;
		fixed         energyBar;

		void deletePanel  ();
		int  loadPanel    ();
		void loadSprite   (File* file, Sprite* sprite);
		int  loadSprites  (char* fileName);
		int  loadTiles    (char* fileName);
		int  playBonus    ();

	protected:
		Font*         font;

		int  load (char* fileName, unsigned char diff, bool checkpoint);
		int  step ();
		void draw ();

	public:
		Bullet*   bullets;
		EventPath path[PATHS];

		Level          ();
		Level          (char* fileName, unsigned char diff, bool checkpoint);
		virtual ~Level ();

		bool          checkMaskUp   (fixed x, fixed y);
		bool          checkMaskDown (fixed x, fixed y);
		bool          checkSpikes   (fixed x, fixed y);
		int           getWorld      ();
		void          setNext       (int nextLevel, int nextWorld);
		void          setTile       (unsigned char gridX, unsigned char gridY, unsigned char tile);
		Event*        getEvents     ();
		signed char*  getEvent      (unsigned char gridX, unsigned char gridY);
		unsigned char getEventHits  (unsigned char gridX, unsigned char gridY);
		unsigned int  getEventTime  (unsigned char gridX, unsigned char gridY);
		void          clearEvent    (unsigned char gridX, unsigned char gridY);
		int           hitEvent      (unsigned char gridX, unsigned char gridY, LevelPlayer* source);
		void          setEventTime  (unsigned char gridX, unsigned char gridY, unsigned int time);
		signed char*  getBullet     (unsigned char bullet);
		Sprite*       getSprite     (unsigned char sprite);
		Anim*         getAnim       (unsigned char anim);
		Anim*         getMiscAnim   (unsigned char anim);
		void          setWaterLevel (unsigned char gridY);
		fixed         getWaterLevel ();
		void          playSound     (int sound);
		void          flash         (unsigned char red, unsigned char green, unsigned char blue, int duration);
		void          receive       (unsigned char* buffer);
		virtual int   play          ();

};


class DemoLevel : public Level {

	private:
		unsigned char* macro;

	public:
		DemoLevel  (const char* fileName);
		~DemoLevel ();

		int play   ();

};


// Variable

EXTERN Level* level;

#endif

