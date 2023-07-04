## Assets Target Configuration ##

# For prepending the current source path, later
FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

set(SRB2_ASSET_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/installer"
	CACHE STRING "Path to directory that contains all asset files for the installer.")

set(SRB2_ASSET_HASHED
"srb2.srb;\
gfx.kart;\
textures.kart;\
chars.kart;\
maps.kart"
	CACHE STRING "Asset filenames to apply MD5 checks. No spaces between entries!"
)

set(SRB2_ASSET_DOCS
"README.txt;\
HISTORY.txt;\
LICENSE.txt;\
LICENSE-3RD-PARTY.txt"
	CACHE STRING "Documentation filenames. In OS X, these are packaged separately from other assets. No spaces between entries!"
)

PREPEND(SRB2_ASSET_DOCS ${SRB2_ASSET_DIRECTORY} ${SRB2_ASSET_DOCS})

foreach(SRB2_ASSET ${SRB2_ASSET_HASHED})
	file(MD5 ${SRB2_ASSET_DIRECTORY}/${SRB2_ASSET} "SRB2_ASSET_${SRB2_ASSET}_HASH")
	set(SRB2_ASSET_${SRB2_ASSET}_HASH ${SRB2_ASSET_${SRB2_ASSET}_HASH} PARENT_SCOPE)
endforeach()

# Installation

file(COPY "${SRB2_ASSET_DIRECTORY}/" DESTINATION out FILES_MATCHING PATTERN "*.kart")
file(COPY "${SRB2_ASSET_DIRECTORY}/" DESTINATION out FILES_MATCHING PATTERN "*.srb")
