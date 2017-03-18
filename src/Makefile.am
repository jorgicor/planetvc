.rc.o:
	$(WINDRES) $< -o $@

AM_CFLAGS = $(WARN_CFLAGS)

nobase_dist_doc_DATA = README NEWS COPYING \
		       libxmp-lite/README libxmp-lite/README2

dist_pkgdata_DATA = gcdb/gamecontrollerdb.txt gcdb/README-gcdb

EXTRA_DIST = icon/icon.ico

bin_PROGRAMS = planetvc

planetvc_CPPFLAGS = -DLIBXMP_CORE_PLAYER -DLIBXMP_CORE_DISABLE_IT \
		    -DBUILDING_STATIC

if !USE_SDL_DATADIR
planetvc_CPPFLAGS += -DPP_DATADIR='"$(pkgdatadir)-data"' \
		     -DPP_PROG_DATADIR='"$(pkgdatadir)"'
endif

planetvc_CFLAGS = $(SDL_CFLAGS) $(AM_CFLAGS)

if HAVE_WINDRES
RESOURCE = game/resource.rc
else
RESOURCE = 
endif

planetvc_SOURCES =	$(RESOURCE) \
			cfg/cfg.h \
			\
			cbase/cbase.h cbase/cbase.c \
			cbase/kassert.h cbase/kassert.c \
			\
			kernel/kernel.h kernel/kernel.c \
			kernel/kernel_snd.h kernel/kernel_snd.c \
			kernel/kernel_snd_sdl.h kernel/kernel_snd_sdl.c \
			kernel/kernel_snd_null.h kernel/kernel_snd_null.c \
			\
			gamelib/bmp_load.h gamelib/bmp_load.c \
			gamelib/bmp.h gamelib/bmp_draw.c \
			gamelib/wav.h gamelib/wav_load.c \
			gamelib/vfs.h gamelib/vfs.c \
			gamelib/mixer.h gamelib/mixer.c \
			gamelib/state.h gamelib/state.c \
			gamelib/lang.h gamelib/lang.c \
			gamelib/ngetopt.h gamelib/ngetopt.c \
			\
			game/cmdline.c game/cmdline.h \
			game/crypt.c game/crypt.h \
			game/coroutin.c game/coroutin.h \
			game/cheats.c game/cheats.h \
			game/confpath.h game/confpath.c \
			game/log.c game/log.h \
			game/data.c game/data.h \
			game/bitmaps.c game/bitmaps.h \
			game/sounds.c game/sounds.h \
			game/tilengin.c game/tilengin.h \
			game/path.c game/path.h \
			game/main.c \
			game/modplay.h game/modplay.c \
			game/game.h game/game.c \
			game/input.h game/input.c \
			game/text.h game/text.c \
			game/strdraw.h game/strdraw.c \
			game/load_st.h game/load_st.c \
			game/gplay_st.h game/gplay_st.c \
			game/menu_st.h game/menu_st.c \
			game/buy_st.h game/buy_st.c \
			game/fade_st.h game/fade_st.c \
			game/over_st.h game/over_st.c \
			game/end_st.h game/end_st.c \
			game/win_st.h game/win_st.c \
			game/demo_st.h game/demo_st.c \
			game/cosmonau.h game/cosmonau.c \
			game/oxigen.h game/oxigen.c \
			game/wasp.h game/wasp.c \
			game/caterpil.h game/caterpil.c \
			game/balloon.h game/balloon.c \
			game/spider.h game/spider.c \
			game/platform.h game/platform.c \
			game/drop.h game/drop.c \
			game/moth.h game/moth.c \
			game/grasshop.h game/grasshop.c \
			game/lavatong.h game/lavatong.c \
			game/stargate.h game/stargate.c \
			game/bat.h game/bat.c \
			game/fish.h game/fish.c \
			game/fenix.h game/fenix.c \
			game/draco.h game/draco.c \
			game/lavashot.h game/lavashot.c \
			game/hud.h game/hud.c \
			game/gameover.h game/gameover.c \
			game/hiscore.h game/hiscore.c \
			game/msgbox.h game/msgbox.c \
			game/menu.h game/menu.c \
			game/fade.h game/fade.c \
			game/prefs.h game/prefs.c \
			game/initfile.h game/initfile.c \
			game/readlin.h game/readlin.c \
			game/demosave.h game/demosave.c \
			game/arrow.h game/arrow.c \
			game/pad.h game/pad.c \
			game/google.h game/google.c \
			\
			libxmp-lite/common.h libxmp-lite/control.c \
			libxmp-lite/dataio.c \
			libxmp-lite/effects.c libxmp-lite/effects.h \
			libxmp-lite/envelope.c libxmp-lite/envelope.h \
			libxmp-lite/filter.c \
			libxmp-lite/format.c libxmp-lite/format.h \
			libxmp-lite/hio.c libxmp-lite/hio.h \
			libxmp-lite/lfo.c libxmp-lite/lfo.h \
			libxmp-lite/list.h \
			libxmp-lite/load.c libxmp-lite/load_helpers.c \
			libxmp-lite/mdataio.h \
			libxmp-lite/memio.c libxmp-lite/memio.h \
			libxmp-lite/mix_all.c \
			libxmp-lite/mixer.c libxmp-lite/mixer.h \
			libxmp-lite/period.c libxmp-lite/period.h \
			libxmp-lite/player.c libxmp-lite/player.h \
			libxmp-lite/precomp_lut.h \
			libxmp-lite/read_event.c \
			libxmp-lite/scan.c \
			libxmp-lite/smix.c \
			libxmp-lite/tempfile.h \
			libxmp-lite/virtual.c libxmp-lite/virtual.h \
			libxmp-lite/common.c \
			libxmp-lite/it.h \
			libxmp-lite/it_load.c \
			libxmp-lite/itsex.c \
			libxmp-lite/loader.h \
			libxmp-lite/mod.h \
			libxmp-lite/mod_load.c \
			libxmp-lite/s3m.h \
			libxmp-lite/s3m_load.c \
			libxmp-lite/sample.c \
			libxmp-lite/xm.h \
			libxmp-lite/xm_load.c \
			libxmp-lite/xmp.h

