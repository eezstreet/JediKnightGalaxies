# Makefile	-- Thatcher Ulrich <tu@tulrich.com> 2003

# This source code has been donated to the Public Domain.  Do
# whatever you want with it.

# Makefile for gameswf library & tests.

TOP = ..
include $(TOP)/config

INCLUDE_PATHS := $(ENGINE_INCLUDE) $(ZLIB_INCLUDE) $(SDL_MIXER_INCLUDE) $(LIBMAD_INCLUDE)

CFLAGS := $(CFLAGS) $(INCLUDE_PATHS) $(CC_DEBUG_FLAGS)
LDFLAGS := $(SDL_LDFLAGS) $(LIB_DEBUG_FLAGS) $(LDFLAGS)

LIB_OUT = $(LIB_PRE)gameswf.$(LIB_EXT)
EXE_OUT = gameswf_test_ogl$(EXE_EXT)
PARSER_OUT = gameswf_parser$(EXE_EXT)
PROCESSOR_OUT = gameswf_processor$(EXE_EXT)

LIBS := $(LIB_OUT) $(BASE_LIB) $(LIBS) $(JPEGLIB) $(ZLIB) $(SDL_MIXER_LIB) $(LIBMAD_LIB)

all: base_lib $(LIB_OUT) $(EXE_OUT) $(PARSER_OUT) $(PROCESSOR_OUT)


LIB_OBJS = \
	gameswf_action.$(OBJ_EXT)	\
	gameswf_button.$(OBJ_EXT)	\
	gameswf_dlist.$(OBJ_EXT)	\
	gameswf_font.$(OBJ_EXT)	\
	gameswf_fontlib.$(OBJ_EXT)	\
	gameswf_impl.$(OBJ_EXT)	\
	gameswf_log.$(OBJ_EXT)	\
	gameswf_morph.$(OBJ_EXT) \
	gameswf_render.$(OBJ_EXT)	\
	gameswf_shape.$(OBJ_EXT)	\
	gameswf_sound.$(OBJ_EXT)	\
	gameswf_stream.$(OBJ_EXT)	\
	gameswf_styles.$(OBJ_EXT)	\
	gameswf_tesselate.$(OBJ_EXT)	\
	gameswf_text.$(OBJ_EXT)		\
	gameswf_types.$(OBJ_EXT)

TEST_PROGRAM_OBJS = \
	gameswf_test_ogl.$(OBJ_EXT) \
	gameswf_render_handler_ogl.$(OBJ_EXT)	\
	gameswf_sound_handler_sdl.$(OBJ_EXT)	\
	gameswf_sound_handler_mp3.$(OBJ_EXT)

PARSER_OBJS = \
	gameswf_parser.$(OBJ_EXT) 

PROCESSOR_OBJS = \
	gameswf_processor.$(OBJ_EXT)

OBJS = $(LIB_OBJS) $(TEST_PROGRAM_OBJS) $(PARSER_OBJS) $(PROCESSOR_OBJS)

gameswf_impl.$(OBJ_EXT): gameswf.h gameswf_impl.h gameswf_types.h

$(LIB_OUT): $(LIB_OBJS)
	$(AR) $(LIB_OUT_FLAG)$@ $^


$(EXE_OUT): $(TEST_PROGRAM_OBJS) $(LIB_OUT) $(BASE_LIB)
	$(CC) -o $@ $(TEST_PROGRAM_OBJS) $(LIBS) $(LDFLAGS)


$(PARSER_OUT): $(PARSER_OBJS) $(LIB_OUT) $(BASE_LIB)
	$(CC) -o $@ $(PARSER_OBJS) $(LIBS) $(LDFLAGS)


$(PROCESSOR_OUT): $(PROCESSOR_OBJS) $(LIB_OUT) $(BASE_LIB)
	$(CC) -o $@ $(PROCESSOR_OBJS) $(LIBS) $(LDFLAGS)


base_lib:
	make -C $(TOP)/base

clean:
	make -C $(TOP)/base clean
	-rm $(OBJS) $(LIB_OUT) $(EXE_OUT) $(PARSER_OUT) $(PROCESSOR_OUT)

fresh: clean all
