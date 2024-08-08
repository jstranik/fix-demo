{ buildPythonPackage, stdenv, automake, autoconf, libtool, python3, quickfix }:

buildPythonPackage rec {
  pname = "quickix-python";
  version = "0.1.0";

  src = "${quickfix}/lib/python3.12/site-packages";

  nativeBuildInputs = [ automake autoconf libtool ];

  postUnpack = ''
    echo $SETUP_SCRIPT > setup.py
  '';

  # configureFlags = [
  #   # Add any necessary configure flags
  # ];

  # buildPhase = ''
  #   make
  # '';

  # installPhase = ''
  #   make install
  # '';

  SETUP_SCRIPT=''
    from setuptools import setup, find_packages

    setup(
      name='${pname}',
      version='${version}',
      packages=find_packages(),
    )
  '';


  # Add any Python-specific metadata
  meta = with stdenv.lib; {
    description = "Your package description";
    homepage = "https://your-package-homepage.com";
    license = licenses.mit; # Adjust as needed
  };
}
