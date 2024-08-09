# client.nix
{ stdenv, lib, python3Packages, quickfix, makeWrapper, pyright }:

python3Packages.buildPythonApplication rec {
  pname = "fix-client";
  version = "0.1.0";

  src = ./.;

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

  meta = with lib; {
    description = "FIX protocol client using QuickFIX Python bindings";
    license = licenses.mit;
  };
}
