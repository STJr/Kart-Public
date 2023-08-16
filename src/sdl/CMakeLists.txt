# Declare SDL2 interface sources
set(CMAKE_SYSTEM_NAME "Linux")

add_compile_options("SHELL: -s USE_SDL=2")
add_compile_options("SHELL: -s USE_SDL_MIXER=2")
add_compile_options("SHELL: -s USE_ZLIB=1")

add_link_options("SHELL: -lidbfs.js")
add_link_options("SHELL: -s EXPORTED_RUNTIME_METHODS=ccall,cwrap")
add_link_options("SHELL: -s EXPORTED_FUNCTIONS=_main,_Command_ListWADS_f,_COM_ExecuteString,_P_AddWadFile")
add_link_options("SHELL: -s ASYNCIFY")
add_link_options("SHELL: -sUSE_SDL=2")
add_link_options("SHELL: -sUSE_ZLIB=1")
add_link_options("SHELL: -sUSE_SDL_MIXER=2")
add_link_options("SHELL: -lwebsocket.js")
add_link_options("SHELL: -s ALLOW_MEMORY_GROWTH=1")
add_link_options("SHELL: -s INITIAL_MEMORY=655360000")
add_link_options("SHELL: -s STACK_SIZE=65536000")
add_link_options("--preload-file=${CMAKE_BINARY_DIR}/assets/out@/")
add_link_options("SHELL: --use-preload-plugins")

if("${CMAKE_BUILD_TYPE}" EQUAL "Release")
	add_compile_options("SHELL: -O3")
endif()

if("${CMAKE_BUILD_TYPE}" EQUAL "Debug")
	add_link_options("SHELL: -g3")
	add_compile_options("SHELL: -g3")
endif()

set(SRB2_SDL2_SOURCES
	dosstr.c
	endtxt.c
	hwsym_sdl.c
	i_cdmus.c
	i_main.c
	i_net.c
	i_system.c
	i_ttf.c
	i_video.c
	i_threads.c
	#IMG_xpm.c
	ogl_sdl.c
	mixer_sound.c
)

set(SRB2_SDL2_HEADERS
	endtxt.h
	hwsym_sdl.h
	i_ttf.h
	ogl_sdl.h
	sdlmain.h
)

source_group("Interface Code" FILES ${SRB2_SDL2_SOURCES} ${SRB2_SDL2_HEADERS})

set(SRB2_SDL2_TOTAL_SOURCES
	${SRB2_CORE_SOURCES}
	${SRB2_CORE_HEADERS}
	${SRB2_DISCORDRPC_SOURCES}
	${SRB2_DISCORDRPC_HEADERS}
	${SRB2_PNG_SOURCES}
	${SRB2_PNG_HEADERS}
	${SRB2_CORE_RENDER_SOURCES}
	${SRB2_CORE_GAME_SOURCES}
	${SRB2_LUA_SOURCES}
	${SRB2_LUA_HEADERS}
	${SRB2_BLUA_SOURCES}
	${SRB2_BLUA_HEADERS}
	${SRB2_SDL2_SOURCES}
	${SRB2_SDL2_HEADERS}
)

source_group("Main" FILES ${SRB2_CORE_SOURCES} ${SRB2_CORE_HEADERS}
	${SRB2_PNG_SOURCES} ${SRB2_PNG_HEADERS})
source_group("Renderer" FILES ${SRB2_CORE_RENDER_SOURCES})
source_group("Game" FILES ${SRB2_CORE_GAME_SOURCES})
source_group("Discord Rich Presence" FILES ${SRB2_DISCORDRPC_SOURCES} ${SRB2_DISCORDRPC_HEADERS})
source_group("Assembly" FILES ${SRB2_ASM_SOURCES} ${SRB2_NASM_SOURCES})
source_group("LUA" FILES ${SRB2_LUA_SOURCES} ${SRB2_LUA_HEADERS})
source_group("LUA\\Interpreter" FILES ${SRB2_BLUA_SOURCES} ${SRB2_BLUA_HEADERS})


if(${SRB2_CONFIG_HWRENDER})
	set(SRB2_SDL2_TOTAL_SOURCES ${SRB2_SDL2_TOTAL_SOURCES}
		${SRB2_HWRENDER_SOURCES}
		${SRB2_HWRENDER_HEADERS}
		${SRB2_R_OPENGL_SOURCES}
		${SRB2_R_OPENGL_HEADERS}
	)

	source_group("Hardware" FILES ${SRB2_HWRENDER_SOURCES} ${SRB2_HWRENDER_HEADERS})
	source_group("Hardware\\OpenGL Renderer" FILES ${SRB2_R_OPENGL_SOURCES} ${SRB2_R_OPENGL_HEADERS})
endif()

add_executable(SRB2SDL2 ${SRB2_SDL2_TOTAL_SOURCES})
set_target_properties(SRB2SDL2 PROPERTIES OUTPUT_NAME ${SRB2_SDL2_EXE_NAME} SUFFIX ".js")

target_link_libraries(SRB2SDL2 PRIVATE
	${SDL2_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${GME_LIBRARIES}
	${OPENMPT_LIBRARIES}
	${MIXERX_LIBRARIES}
	${PNG_LIBRARIES}
	${ZLIB_LIBRARIES}
	${OPENGL_LIBRARIES}
	${CURL_LIBRARIES}
	${DISCORDRPC_LIBRARIES}
)

if(${CMAKE_SYSTEM} MATCHES Linux)
	target_link_libraries(SRB2SDL2 PRIVATE
		m
		rt
	)
endif()

target_include_directories(SRB2SDL2 PRIVATE
	${SDL2_INCLUDE_DIRS}
	${SDL2_MIXER_INCLUDE_DIRS}
	${GME_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${DISCORDRPC_INCLUDE_DIRS}
	${LIBELF_INCLUDE_DIRS}
)

target_compile_definitions(SRB2SDL2 PRIVATE -DHAVE_MIXER -DSOUND=SOUND_MIXER)

target_compile_definitions(SRB2SDL2 PRIVATE
	-DHAVE_SDL
	-DHAVE_THREADS
)


#### Installation ####
# install(TARGETS SRB2SDL2 SRB2SDL2
# 	RUNTIME DESTINATION .
# )
install(FILES
	"$<TARGET_FILE_DIR:SRB2SDL2>/${SRB2_SDL2_EXE_NAME}.js"
	"$<TARGET_FILE_DIR:SRB2SDL2>/${SRB2_SDL2_EXE_NAME}.wasm"
	"$<TARGET_FILE_DIR:SRB2SDL2>/${SRB2_SDL2_EXE_NAME}.data"
	DESTINATION .)

set(SRB2_SDL2_AVAILABLE YES PARENT_SCOPE)
