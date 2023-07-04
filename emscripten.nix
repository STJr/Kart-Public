{ lib, stdenv, fetchurl, fetchFromGitHub, substituteAll, cmake, curl, nasm
, unzip, game-music-emu, libpng, SDL2, SDL2_mixer, zlib, buildEmscriptenPackage
, libelf, emscripten, openssl, emscriptenPackages, libogg, emscriptenStdenv }:

let

  release_tag = "v1.6";

  assets = fetchurl {
    url =
      "https://github.com/STJr/Kart-Public/releases/download/${release_tag}/AssetsLinuxOnly.zip";
    sha256 = "sha256-ejhPuZ1C8M9B0S4+2HN1T5pbormT1eVL3nlivqOszdE=";
  };

  sdl2Port = fetchurl {
    url = "https://github.com/libsdl-org/SDL/archive/release-2.28.1.zip";
    sha256 = "sha256-FKwMsBatjzIqSA9p/ng6WrCnyAJTQpVHJZ7MNV/Kp0c=";
  };

  zlibPort = fetchurl {
    url = "https://github.com/madler/zlib/archive/refs/tags/v1.2.13.tar.gz";
    sha256 = "sha256-FSWVKgpWdYF5JhOpcjMz1/jMILh6gfkg+4vH4/IlFCg=";
  };

in stdenv.mkDerivation rec {
  pname = "srb2kart";
  version = "1.6.0";

  src = ./.;

  nativeBuildInputs = [ cmake nasm unzip emscripten ];

  buildInputs = [
    curl
    game-music-emu
    libpng
    SDL2
    SDL2_mixer
    emscriptenPackages.zlib
    libogg
  ];

  cmakeFlags = [
    "-DGME_INCLUDE_DIR=${game-music-emu}/include"
    # "-DGME_LIBRARY=${game-music-emu}/lib/libgme.so"
    "-DPNG_PNG_INCLUDE_DIR=${lib.getDev libpng}/include"
    # "-DPNG_LIBRARY=${lib.getDev libpng}"
    "-DSDL2_MIXER_INCLUDE_DIR=${lib.getDev SDL2_mixer}/include/SDL2"
    "-DSDL2_INCLUDE_DIR=${lib.getDev SDL2}/include/SDL2"
    # "-DSDL2_LIBRARY=${lib.getDev SDL2}"
    "-DCURL_INCLUDE_DIR=${lib.getDev curl}/include"
    # "-DCURL_LIBRARY=${lib.getDev curl}"
    "-DZLIB_INCLUDE_DIR=${lib.getDev emscriptenPackages.zlib}/include"
    "-DLIBELF_INCLUDE_DIR=${lib.getDev libelf}/include"
    "-DLIBELF_INCLUDE_DIRS=${lib.getDev libelf}/include"
    # "-DZLIB_LIBRARY=${lib.getDev zlib}"
    "-DLINUX64=1"
    "-DMACOSX=0"
    "-DCMAKE_INSTALL_PREFIX=$out"
  ];

  patches = [ ./wadlocation.patch ];

  configurePhase = ''
    # FIXME: Some tests require writing at $HOME
    HOME=$TMPDIR
    export LINUX=1
    BUILDDIR=$(pwd)

    echo $SSL_CERT_FILE
    # export SSL_CERT_FILE="/etc/ssl/certs/ca-bundle.crt"
    unset SSL_CERT_FILE
    echo $SSL_CERT_FILE

    echo $SSL_CERT_DIR
    unset $SSL_CERT_DIR
    echo $SSL_CERT_DIR

    export SSL_CERT_FILE="/etc/ssl/certs/ca-bundle.crt"
    export SSL_CERT_DIR="/etc/ssl/certs"

    mkdir -p .emscriptencache
    export EM_CACHE=$(pwd)/.emscriptencache

    echo "LSING ports"
    # ls -la ports/zlib-1.2.13
    # ls -la $SSL_CERT_FILE

    export CC="${emscripten}/bin/emcc"
    export CXX="${emscripten}/bin/emcc"
    runHook preConfigure

    mkdir -p $out
    cd $out

    echo "$CC"
    echo "$CXX"
    emcmake cmake ${toString cmakeFlags} "$BUILDDIR"


    runHook postConfigure
  '';

  postPatch = ''
    substituteInPlace src/sdl/i_system.c \
        --replace '@wadlocation@' $out
  '';

  preConfigure = ''
    mkdir ports
    pushd ports
    unzip ${sdl2Port}
    tar xzfv ${zlibPort}
    popd

    mkdir assets/installer
    pushd assets/installer
    unzip ${assets} "*.kart" srb2.srb
    popd
  '';

  buildPhase = ''
    export SSL_CERT_FILE="/etc/ssl/certs/ca-bundle.crt"
    export SSL_CERT_DIR="/etc/ssl/certs"

    emmake make
  '';

  installPhase = ''
    emmake make install
  '';

  postInstall = ''
    mkdir -p $out/bin $out/share/games/SRB2Kart

    ls -la $out/bin

    # mv $out/srb2kart* $out/bin/
    mv $out/*.kart $out/share/games/SRB2Kart
  '';

  checkPhase = "";

  meta = with lib; {
    description = "SRB2Kart is a classic styled kart racer";
    homepage = "https://mb.srb2.org/threads/srb2kart.25868/";
    platforms = platforms.linux;
    license = licenses.gpl2Plus;
    maintainers = with maintainers; [ viric ];
  };
}
