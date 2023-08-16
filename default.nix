{ lib, stdenv, srb2kart-wasm, srb2kart-web }:

stdenv.mkDerivation {
  pname = "srb2kart";
  version = "1.6.0";

  buildInputs = [ srb2kart-wasm srb2kart-web ];

  src = null;

  dontUnpack = true;

  installPhase = ''
    mkdir -p $out
    cp ${srb2kart-wasm}/* $out
    cp ${srb2kart-web}/* $out
  '';

  meta = with lib; {
    description = "SRB2Kart is a classic styled kart racer";
    homepage = "https://mb.srb2.org/threads/srb2kart.25868/";
    platforms = platforms.linux;
    license = licenses.gpl2Plus;
    maintainers = [ ];
  };
}
