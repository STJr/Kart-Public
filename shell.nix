{ pkgs, lib }:

pkgs.mkShell rec {
  cmakeFlags = [ "-DNEWSIGNALHANDLER=0" "-DCMAKE_INSTALL_PREFIX=build" ];
  packages = with pkgs; [
    cmake
    emscripten
    nodejs
    # ccls
    clang-tools
    cmake-language-server
    rtags
    elmPackages.elm
    elmPackages.elm-language-server
    elmPackages.elm-format
    elmPackages.elm-review
    nodePackages.prettier

    # For development
    SDL2
    SDL_net
    SDL2_mixer
    (python38.withPackages (ps:
      with ps; [
        websockets
        (buildPythonPackage rec {
          pname = "asyncudp";
          version = "0.11.0";
          src = fetchPypi {
            inherit pname version;
            sha256 = "sha256-yKtkWfTcjrxC9AHTvXLCpsaAjRDkIjHzngxDOsZGq/k=";
          };
          doCheck = false;
        })
      ]))
  ];
  shellHook = ''
    export EM_CACHE=$(pwd)/.emscriptencache
    export CC="${pkgs.emscripten}/bin/emcc"
    export CXX="${pkgs.emscripten}/bin/emcc"
    export cmakeFlags="${toString cmakeFlags}"

    echo "./build.sh"
  '';
}
