{ stdenv, lib, buildNpmPackage, writeShellApplication, nodejs, python3, chromium, nodePackages, work_dir,
  dev-build ? false }:

let
  fix-web = buildNpmPackage rec {
    pname = "fix-web";
    version = "1.0.0";

    src = ./.;

    npmDepsHash = "sha256-wmBtpN2Zn1AinsGA3pEFfzWK8sgGty7hJjJNQHOu6JY=";

    # The prepack script runs the build script, which we'd rather do in the build phase.
    npmPackFlags = [ "--ignore-scripts" ];

    nativeBuildInputs = [ python3 ];

    NODE_ENV= if dev-build then "dev" else "production";
    #NODE_ENV = if dev-build then "dev" else "production";

  };
  nodeEnvStr = if dev-build then "NODE_ENV=dev" else "NODE_ENV=production";
  envSuffix = if dev-build then "dev" else "production";
  dotenvcli = "${nodePackages.dotenv-cli}/bin/dotenv";

  fix-web-starter = writeShellApplication {
    name = "fix-web-starter";
    runtimeInputs = [ nodejs ];
    text = ''
    export NODE_PATH="${fix-web}/lib/node_modules/web/node_modules"
    #export DOTENV_CONFIG_PATH="${fix-web}/lib/node_modules/portal2/.env.${envSuffix}"
    export LOGIC_FILE="${work_dir}/logic.py"
    ${nodeEnvStr} ${dotenvcli} -c ${envSuffix} -e ${fix-web}/lib/node_modules/web/.env node ${fix-web}/lib/node_modules/web/build/index.js
  '';
  };

in {
  inherit fix-web fix-web-starter;
}
