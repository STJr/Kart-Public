{ pkgs, lib }:

pkgs.mkShell rec {

  cmakeFlags = [
    "-DGME_INCLUDE_DIR=${pkgs.game-music-emu}/include"
    # "-DGME_LIBRARY=${game-music-emu}/lib/libgme.so"
    "-DPNG_PNG_INCLUDE_DIR=${lib.getDev pkgs.libpng}/include"
    # "-DPNG_LIBRARY=${lib.getDev libpng}"
    # "-DSDL2_MIXER_INCLUDE_DIR=${lib.getDev pkgs.SDL2_mixer}/include/SDL2"
    # "-DSDL2_INCLUDE_DIR=${lib.getDev pkgs.SDL2}/include/SDL2"
    # "-DSDL2_LIBRARY=${lib.getDev SDL2}"
    "-DCURL_INCLUDE_DIR=${lib.getDev pkgs.curl}/include"
    # "-DCURL_LIBRARY=${lib.getDev curl}"
    "-DZLIB_INCLUDE_DIR=${lib.getDev pkgs.emscriptenPackages.zlib}/include"
    "-DLIBELF_INCLUDE_DIR=${lib.getDev pkgs.libelf}/include"
    "-DLIBELF_INCLUDE_DIRS=${lib.getDev pkgs.libelf}/include"
    # "-DZLIB_LIBRARY=${lib.getDev zlib}"
    "-DLINUX64=1"
    "-DMACOSX=0"
    "-DHAVE_THREADS=0"
    "-DNEWSIGNALHANDLER=0"
    "-DCMAKE_INSTALL_PREFIX=build"
    # "-DCMAKE_CC_FLAGS=--preload-file=assets/Makefile"
    # "-DCMAKE_CXX_FLAGS=--preload-file=assets/Makefile"
  ];

  packages = with pkgs; [
    cmake
    curl
    game-music-emu
    libpng
    libogg
    emscripten
    nodejs
  ];

  EM_CACHE = ".emscriptencache";

  shellHook = ''
    echo "HIASUHDIUASHDUIHAUISHDIUAHSIUDHUIASHDUIS"

    export EM_CACHE=$(pwd)/.emscriptencache
    export CC="${pkgs.emscripten}/bin/emcc"
    export CXX="${pkgs.emscripten}/bin/emcc"
    export cmakeFlags="${toString cmakeFlags}"

    echo "CD into build"
    echo "emcmake cmake $cmakeFlags .."
    echo "emmake make install -j24"
  '';
}
