# server.nix
{ stdenv, cmake, quickfix, lib, pkg-config, libxml2, makeWrapper }:

stdenv.mkDerivation {
  pname = "fix-server";
  version = "0.1.0";

  src = ./.;  # Assumes the Server.cpp is in the same directory as this nix file

  nativeBuildInputs = [ cmake pkg-config makeWrapper ];
  buildInputs = [ quickfix libxml2 ];

  QUICKFIX_DIR=quickfix;
  QUICKFIX_STATE="/tmp/fix-server";
  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"
  ];

  installPhase = ''
    mkdir -p $out/bin
    cp fix-server $out/bin/
    cp ${./server_settings.cfg} $out/bin/server_settings.cfg
    wrapProgram $out/bin/fix-server \
      --set-default QUICKFIX_DIR ''${QUICKFIX_DIR} \
      --set-default QUICKFIX_STATE ''${QUICKFIX_STATE} \
      --chdir $out/bin   \
      --run "echo Starting server"
  '';

  meta = with lib; {
    description = "FIX protocol server using QuickFIX";
    license = licenses.mit;
  };
}
