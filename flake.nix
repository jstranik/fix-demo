{
  description = "FIX protocol server and client";

  inputs = {
    nixpkgs.url = "/Users/janstranik/project/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        fix-server = pkgs.callPackage ./server/fix-server.nix {};
        fix-client = pkgs.callPackage ./client/fix-client.nix {};

      in {
        packages = {
          inherit fix-server fix-client;
          default = fix-server;
        };

        apps = {
          server = {
            type = "app";
            program = "${fix-server}/bin/fix-server";
          };
          client = {
            type = "app";
            program = "${fix-client}/bin/fix-client";
          };
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            cmake
            quickfix
            (python3.withPackages (ps: with ps; [ quickfix ]))
          ];
        };
      }
    );
}
