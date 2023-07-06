{ pkgs, lib }:

pkgs.mkShell rec {
  cmakeFlags = [ "-DNEWSIGNALHANDLER=0" "-DCMAKE_INSTALL_PREFIX=build" ];
  packages = with pkgs; [
    cmake
    emscripten
    nodejs
    elmPackages.elm-language-server
    elmPackages.elm-format
    elmPackages.elm-review
    nodePackages.prettier
  ];
  shellHook = ''
    export EM_CACHE=$(pwd)/.emscriptencache
    export CC="${pkgs.emscripten}/bin/emcc"
    export CXX="${pkgs.emscripten}/bin/emcc"
    export cmakeFlags="${toString cmakeFlags}"

    echo "./build.sh"
  '';
}
