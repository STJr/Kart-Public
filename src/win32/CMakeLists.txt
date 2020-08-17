file(GLOB SRB2_WIN_SOURCES *.c *.h *.rc)

add_executable(SRB2DD EXCLUDE_FROM_ALL WIN32
	${SRB2_WIN_SOURCES}
)

target_compile_definitions(SRB2DD PRIVATE
	-D_WINDOWS
)

set_target_properties(SRB2DD PROPERTIES OUTPUT_NAME ${SRB2_WIN_EXE_NAME})

target_link_libraries(SRB2DD PRIVATE SRB2Core)
