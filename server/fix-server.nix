# server.nix
{ stdenv, cmake, quickfix, lib, pkg-config, libxml2, makeWrapper, boost, writeShellScriptBin, fmt, python3, clang-tools, websocketpp, nlohmann_json }:
let
  pname = "fix-server";
  server = stdenv.mkDerivation rec {
      inherit pname;
      version = "0.1.0";

      src = ./src;

      nativeBuildInputs = [ cmake pkg-config makeWrapper clang-tools ];
      buildInputs = [ quickfix libxml2 fmt python3 websocketpp nlohmann_json
                      (boost.override { enablePython = true; python = python3; })
                    ];

      QUICKFIX_DIR=quickfix;
      QUICKFIX_STATE="/tmp/fix-server";
      cmakeFlags = [
        "-DCMAKE_BUILD_TYPE=Release"
      ];

      installPhase = ''
        mkdir -p $out/bin
        cp fix-server $out/bin/
      '';
    };

    config = ./server_settings.cfg;

    logic = ./logic.py;

    work_dir = "/var/tmp/${pname}";
in
{
  fix-server-bin = server;
  fix-server = writeShellScriptBin "fix-server" ''
        mkdir -p ${work_dir}
        mkdir -p ${work_dir}/state

        cp ${config} ${work_dir}/server_settings.cfg
        cp ${logic} ${work_dir}/logic.py
        chmod u+w ${work_dir}/logic.py
        export QUICKFIX_DIR=${quickfix}
        export QUICKFIX_STATE=${work_dir}/state
        ${server}/bin/fix-server \
            --settings ${work_dir}/server_settings.cfg \
            --logic ${work_dir}/logic.py \
            --state ${work_dir}/state \
            --quickfix ${quickfix}
  '';
  inherit work_dir;
}
