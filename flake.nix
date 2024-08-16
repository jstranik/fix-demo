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
        fix-web-nix = pkgs.callPackage ./web/fix-web.nix { inherit (server) work_dir; };
      in rec {
        packages = rec {
          inherit fix-client;
          inherit (fix-web-nix) fix-web fix-web-starter;
          inherit (server) fix-server fix-server-bin;
          default = fix-server;
        };

        overlays = rec {
          default = final: prev: {
              fix = packages;
          };
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
      }
    );
}
