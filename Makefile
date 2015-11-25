# OpenJazz makefile

objects = src/game/game.o src/game/gamemode.o \
	src/game/localgame.o \
	src/mem.o src/surface.o \
	src/io/gfx/anim.o src/io/gfx/font.o src/io/gfx/paletteeffects.o \
	src/io/gfx/sprite.o src/io/gfx/video.o \
	src/io/controls.o src/io/file.o \
	src/jj1bonuslevel/jj1bonuslevelplayer/jj1bonuslevelplayer.o \
	src/jj1bonuslevel/jj1bonuslevel.o \
	src/jj1level/jj1event/jj1bridge.o src/jj1level/jj1event/jj1event.o \
	src/jj1level/jj1event/jj1guardians.o \
	src/jj1level/jj1event/jj1standardevent.o \
	src/jj1level/jj1levelplayer/jj1bird.o \
	src/jj1level/jj1levelplayer/jj1levelplayer.o \
	src/jj1level/jj1levelplayer/jj1levelplayerframe.o \
	src/jj1level/jj1bullet.o src/jj1level/jj1demolevel.o \
	src/jj1level/jj1level.o src/jj1level/jj1levelframe.o \
	src/jj1level/jj1levelload.o \
	src/jj1planet/jj1planet.o \
	src/jj1scene/jj1scene.o src/jj1scene/jj1sceneload.o \
	src/level/level.o src/level/movable.o src/level/levelplayer.o \
	src/menu/gamemenu.o src/menu/mainmenu.o src/menu/menu.o \
	src/menu/setupmenu.o \
	src/player/player.o \
	src/main.o src/setup.o src/util.o


OpenJazz: $(objects)
	cc -Wall -o OpenJazz -lSDL -lstdc++ -flto $(objects)

%.o: %.cpp
	cc -Wall -Wextra -Isrc -O2 -fmerge-all-constants -s -fdata-sections -ffunction-sections -fomit-frame-pointer -Wl,--gc-sections -Wl,--strip-all -pipe -flto -c $< -o $@
%.o: %.c
	cc -Wall -Wextra -Isrc -O2 -fmerge-all-constants -s -fdata-sections -ffunction-sections -fomit-frame-pointer -Wl,--gc-sections -Wl,--strip-all -pipe -flto -c $< -o $@
clean:
	rm -f OpenJazz $(objects)
