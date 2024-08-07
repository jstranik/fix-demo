# server.nix
{ stdenv, cmake, quickfix, lib, pkg-config }:

stdenv.mkDerivation {
  pname = "fix-server";
  version = "0.1.0";

  src = ./.;  # Assumes the Server.cpp is in the same directory as this nix file

  nativeBuildInputs = [ cmake pkg-config ];
  buildInputs = [ quickfix ];

  QuickFIX_DIR=quickfix;
  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  installPhase = ''
    mkdir -p $out/bin
    cp fix-server $out/bin/
    cp ${./server_settings.cfg} $out/bin/server_settings.cfg
  '';

  meta = with lib; {
    description = "FIX protocol server using QuickFIX";
    license = licenses.mit;
  };
}
