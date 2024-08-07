{ pkgs ? import <nixpkgs> {} }:

{
  fix-server = pkgs.callPackage ./server/fix-server.nix {};
  fix-client = pkgs.callPackage ./client/fix-clinet.nix {};
}
