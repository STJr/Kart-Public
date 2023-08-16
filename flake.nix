{
  description = "A very basic flake";

  outputs = { self, nixpkgs }:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
      srb2kart-wasm = pkgs.callPackage ./emscripten.nix { };
      srb2kart-web = pkgs.callPackage ./src/web/default.nix { };
      srb2kart =
        pkgs.callPackage ./default.nix { inherit srb2kart-wasm srb2kart-web; };
    in {
      packages.x86_64-linux.wasm = srb2kart-wasm;
      packages.x86_64-linux.web = srb2kart-web;
      packages.x86_64-linux.default = srb2kart;
    };
}
