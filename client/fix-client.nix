# client.nix
{ stdenv, lib, python3Packages, quickfix, makeWrapper, pyright }:

python3Packages.buildPythonApplication rec {
  pname = "fix-client";
  version = "0.1.0";

  src = ./.;  # Assumes the client.py is in the same directory as this nix file

  nativeBuildInputs = [ makeWrapper pyright ];
  propagatedBuildInputs = with python3Packages; [
    quickfix
  ];

  #PYTHON_PATH="${quickfix}/lib/python3.12/site-packages";
  QUICKFIX_DIR=quickfix;
  QUICKFIX_STATE="/tmp/fix-client";

  build-system = with python3Packages; [ setuptools-scm ];

  postInstall = ''
    cp client_settings.cfg $out/bin/client_settings.cfg
  '';

  makeWrapperArgs = ["--set-default QUICKFIX_STATE ${QUICKFIX_STATE}"
                     "--set-default QUICKFIX_DIR ${QUICKFIX_DIR}"
                    ];
  # installPhase = ''
  #   mkdir -p $out/bin
  #   cp client.py $out/bin/fix-client
  #   cp client_settings.cfg $out/bin/client_settings.cfg
  #   chmod +x $out/bin/fix-client
  #   wrapProgram $out/bin/fix-client \
  #     --set-default QUICKFIX_DIR ''${QUICKFIX_DIR} \
  #     --set-default QUICKFIX_STATE ''${QUICKFIX_STATE} \
  #     --chdir $out/bin   \
  #     --run "echo Starting server"

  # '';

  meta = with lib; {
    description = "FIX protocol client using QuickFIX Python bindings";
    license = licenses.mit;
  };
}
