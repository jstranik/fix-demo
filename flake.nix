{
  description = "FIX protocol server and client";

  inputs = {
    nixpkgs.url = "github:jstranik/nixpkgs?ref=quickfix-python3-bindings";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};

        server = pkgs.callPackage ./server/fix-server.nix {};
        fix-client = pkgs.callPackage ./client/fix-client.nix {};
      in {
        packages = rec {
          inherit fix-client;
          inherit (server) fix-server fix-server-bin;
          default = fix-server;
        };

        apps = {
          server = {
            type = "app";
            program = "${server.fix-server}/bin/fix-server";
          };
          client = {
            type = "app";
            program = "${fix-client}/bin/fix-client";
          };
        };

        # devShells.default = pkgs.mkShell {
        #   buildInputs =
        #     fix-server.buildInputs ++
        #     [
        #       (pkgs.python3.withPackages (ps: with ps; [ setuptools ]))

        #     ];

        #   nativeBuildInputs = fix-server.nativeBuildInputs;
        # };
      }
    );
}
