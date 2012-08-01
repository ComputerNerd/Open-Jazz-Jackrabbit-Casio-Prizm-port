
/**
 *
 * @file level.h
 *
 * Part of the OpenJazz project
 *
 * @section History
 * 31st January 2006: Created level.h from parts of OpenJazz.h
 * 4th February 2009: Created events.h from parts of level.h
 * 19th March 2009: Created sprite.h from parts of level.h
 * 30th March 2010: Created baselevel.h from parts of level.h
 * 29th June 2010: Created jj2level.h from parts of level.h
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

/// JJ1 level grid element
typedef struct {

	unsigned char tile; ///< Indexes the tile set
	unsigned char bg; ///< 0 = Effect background, 1 = Black background
	unsigned char event; ///< Indexes the event set
	unsigned char hits; ///< Number of times the event has been shot
	int           time; ///< Point at which the event will do something, e.g. terminate

} GridElement;

/// JJ1 level event type
typedef struct {

	unsigned char anims[6]; ///< Indices of animations
	signed char   difficulty; ///< The minimum difficulty level at which the event is used
	signed char   reflection; ///< Whether or not to show a reflection
	signed char   movement; ///< Movement type
	signed char   magnitude; ///< Usage depends on event type
	signed char   strength; ///< Number of hits required to destroy the event
	signed char   modifier; ///< Modifier
	unsigned char points; ///< Points obtained by getting/destroying the event
	unsigned char bullet; ///< Type of bullet the event fires
	unsigned char bulletPeriod; ///< The time between successive bullet shots
	unsigned char speed; ///< The speed at which the event moves
	unsigned char animSpeed; ///< The speed of the event's animation
	unsigned char sound; ///< The sound played on the appropriate trigger
	signed char   multiA; ///< Usage depends on event type
	signed char   multiB; ///< Usage depends on event type
	signed char   pieceSize; ///< Size of pieces in bridges, swinging balls chains, etc.
	signed char   pieces; ///< Number of pieces in bridges, swinging ball chains, etc.
	signed char   angle; ///< Initial angle of swinging balls, etc.

} JJ1EventType;

/// Pre-defined JJ1 event movement path
typedef struct {

	short int*    x; ///< X-coordinates for each node
	short int*    y; ///< Y-coordinates for each node
	unsigned char length; ///< Number of nodes

} JJ1EventPath;


// Classes

class Font;
class JJ1Bullet;
class JJ1Event;
class JJ1LevelPlayer;

/// JJ1 level
class JJ1Level : public Level {

	private:
		SDL_Surface*  tileSet; ///< Tile images
		SDL_Surface*  panel; ///< HUD background image
		SDL_Surface*  panelAmmo[5]; ///< HUD ammo type images
		JJ1Event*     events; ///< JJ1Events
		char*         musicFile; ///< Music file name
		char*         sceneFile; ///< File name of cutscene to play when level has been completed
		Sprite*       spriteSet; ///< Sprites
		Anim          animSet[ANIMS]; ///< Animations
		char          miscAnims[4]; ///< Further animations
		signed char   bulletSet[BULLETS][BLENGTH]; ///< Bullet types
		JJ1EventType  eventSet[EVENTS]; ///< Event types
		char          mask[240][64]; ///< Tile masks. At most 240 tiles, all with 8 * 8 masks
		GridElement   grid[LH][LW]; ///< Level grid. All levels are the same size
		int           soundMap[32]; ///< Maps event sound effect numbers to actual sound effect indices
		SDL_Color     skyPalette[256]; ///< Full palette for sky background
		bool          sky; ///< Whether or not to use sky background
		unsigned char skyOrb; ///< The tile to use as the background sun/moon/etc.
		int           levelNum; ///< Number of current level
		int           worldNum; ///< Number of current world
		int           nextLevelNum; ///< Number of next level
		int           nextWorldNum; ///< Number of next world
		int           enemies; ///< Number of enemies to kill
		fixed         waterLevel; ///< Height of water
		fixed         waterLevelTarget; ///< Future height of water
		fixed         waterLevelSpeed; ///< Rate of water level change
		fixed         energyBar; ///< HUD energy bar fullness

		void deletePanel  ();
		int  loadPanel    ();
		void loadSprite   (File* file, Sprite* sprite);
		int  loadSprites  (char* fileName);
		int  loadTiles    (char* fileName);
		int  playBonus    ();

	protected:
		Font* font; ///< On-screen message font

		JJ1Level (Game* owner);

		int  load (char* fileName, bool checkpoint);
		int  step ();
		void draw ();

	public:
		JJ1Bullet*   bullets; ///< Active bullets
		JJ1EventPath path[PATHS]; ///< Pre-defined event movement paths

		JJ1Level          (Game* owner, char* fileName, bool checkpoint, bool multi);
		virtual ~JJ1Level ();

		bool          checkMaskUp   (fixed x, fixed y);
		bool          checkMaskDown (fixed x, fixed y);
		bool          checkSpikes   (fixed x, fixed y);
		int           getWorld      ();
		void          setNext       (int nextLevel, int nextWorld);
		void          setTile       (unsigned char gridX, unsigned char gridY, unsigned char tile);
		JJ1Event*     getEvents     ();
		JJ1EventType* getEvent      (unsigned char gridX, unsigned char gridY);
		unsigned char getEventHits  (unsigned char gridX, unsigned char gridY);
		unsigned int  getEventTime  (unsigned char gridX, unsigned char gridY);
		void          clearEvent    (unsigned char gridX, unsigned char gridY);
		int           hitEvent      (unsigned char gridX, unsigned char gridY, JJ1LevelPlayer* source);
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

/// JJ1 level played as a demo
class JJ1DemoLevel : public JJ1Level {

	private:
		unsigned char* macro; ///< Sequence of player control codes

	public:
		JJ1DemoLevel  (Game* owner, const char* fileName);
		~JJ1DemoLevel ();

		int play   ();

};


// Variables

EXTERN JJ1Level* level; ///< JJ1 level

EXTERN int viewH; ///< Canvas height, minus 33 if the panel obscures the whole of the bottom of the canvas
#define viewW canvasW

#endif

