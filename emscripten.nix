{ lib, stdenv, fetchurl, fetchFromGitHub, substituteAll, cmake, curl, nasm
, unzip, game-music-emu, libpng, SDL2, SDL2_mixer, zlib, buildEmscriptenPackage
, libelf, openssl, emscripten, emscriptenPackages, libogg, emscriptenStdenv
, breakpointHook, debug ? true }:

let

  release_tag = "v1.6";

  patchedEmscripten = emscripten.overrideAttrs
    (prev: { patches = prev.patches ++ [ ./add_nix_local_ports.patch ]; });

  assets = fetchurl {
    url =
      "https://github.com/STJr/Kart-Public/releases/download/${release_tag}/AssetsLinuxOnly.zip";
    sha256 = "sha256-ejhPuZ1C8M9B0S4+2HN1T5pbormT1eVL3nlivqOszdE=";
  };

  sdl2Port = fetchurl {
    url = "https://github.com/libsdl-org/SDL/archive/release-2.24.2.zip";
    sha256 = "sha256-3DPf2yTCi3V/MjReqOymemAwydywhK6MeLwnoh5zeto=";
  };

  zlibPort = fetchurl {
    url = "https://github.com/madler/zlib/archive/refs/tags/v1.2.13.tar.gz";
    sha256 = "sha256-FSWVKgpWdYF5JhOpcjMz1/jMILh6gfkg+4vH4/IlFCg=";
  };

  oggPort = fetchurl {
    url = "https://github.com/emscripten-ports/ogg/archive/version_1.zip";
    sha256 = "sha256-IvWZu+YGX/mPxWXy22Zl2lSBgc+QmCuJ/cQtg/vGNws=";
  };

  vorbisPort = fetchurl {
    url = "https://github.com/emscripten-ports/vorbis/archive/version_1.zip";
    sha256 = "sha256-snyqUYYgCLEJyxN18DnXb2ItYOpzrTCOG1oJh0Vp/n4=";
  };

  sdl2MixerPort = fetchurl {
    url = "https://github.com/libsdl-org/SDL_mixer/archive/release-2.0.4.zip";
    sha256 = "sha256-4Ajep3iVJ89tEz+DiZfTL/t2E3W/pCUCspkUUa7TxFg=";
  };

in stdenv.mkDerivation rec {
  pname = "srb2kart-wasm";
  version = "1.6.0";

  src = ./.;

  nativeBuildInputs = [ cmake nasm unzip patchedEmscripten ];

  buildInputs = [ curl emscriptenPackages.zlib libogg ];

  cmakeFlags = [
    "-DCMAKE_INSTALL_PREFIX=$out"
    "-DCMAKE_BUILD_TYPE=${if debug then "Debug" else "Release"}"
    "-DCMAKE_C_COMPILER=${patchedEmscripten}/bin/emcc"
  ];

  dontStrip = debug;

  NIX_LOCAL_PORTS =
    "zlib=${zlibPort},sdl2=${sdl2Port},ogg=${oggPort},vorbis=${vorbisPort},sdl2_mixer=${sdl2MixerPort}";

  configurePhase = ''
    runHook preConfigure

    SRCDIR=$(pwd)

    mkdir -p $out

    export EM_CACHE=$(pwd)/.emscriptencache
    mkdir -p $EM_CACHE

    mkdir build
    pushd build
    emcmake cmake ${toString cmakeFlags} "$SRCDIR"

    runHook postConfigure
  '';

  preConfigure = ''
    mkdir -p assets/installer
    pushd assets/installer
    unzip ${assets} "*.kart" srb2.srb
    popd
  '';

  buildPhase = ''
    emmake make
  '';

  installPhase = ''
    emmake make install
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
