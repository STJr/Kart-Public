{ lib, mkYarnPackage, elmPackages, stdenv, yarn }:

let
  yarnPkg = mkYarnPackage {
    name = "srb2kart-node-modules";
    dontBuild = true;
    doDist = false;
    src = ./.;
    publishBinsFor = [ "webpack" ];
  };
in stdenv.mkDerivation {
  name = "srb2kart-frontend";
  src = lib.cleanSource ./.;

  buildInputs = [ elmPackages.elm elmPackages.elm-format yarnPkg yarn ];

  patchPhase = ''
    rm -rf elm-stuff
    ln -sf ${yarnPkg}/libexec/**/node_modules .
  '';

  shellHook = ''
    ln -fs ${yarnPkg}/libexec/**/node_modules .
  '';

  configurePhase = elmPackages.fetchElmDeps {
    elmPackages = import ./elm-srcs.nix;
    elmVersion = "0.19.1";
    registryDat = ./registry.dat;
  };

  installPhase = ''
    mkdir -p $out
    export OUTPUT_PATH="$out"
    webpack
  '';
}
