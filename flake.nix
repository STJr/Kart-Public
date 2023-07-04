{
  description = "A very basic flake";

  outputs = { self, nixpkgs }:
    let pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in {

      packages.x86_64-linux.hello = nixpkgs.legacyPackages.x86_64-linux.hello;

      packages.x86_64-linux.default = pkgs.callPackage ./emscripten.nix { };
      devShells.x86_64-linux.default = pkgs.callPackage ./shell.nix { };
    };
}
