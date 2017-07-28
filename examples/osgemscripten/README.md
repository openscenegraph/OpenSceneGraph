How to build osgemscripten example
==================================

### 1. Install Emscripten

  Download and install [Emscripten portable SDK](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html)

### 2. Create build directory

  Run the following commands:

  `mkdir /path/to/build/dir`

  `cd /path/to/build/dir`

### 3. Configure example

  Run the following command:

  `cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/emsdk-portable/emscripten/<version>/cmake/Modules/Platform/Emscripten.cmake /path/to/OpenSceneGraph/examples/osgemscripten`

  This also configures OpenSceneGraph as a dependency

### 3. Build example

  Run the following command:

  `make`

  This also builds OpenSceneGraph under `/path/to/OpenSceneGraph/build/Emscripten`
  if it has not yet been built.

### 4. Launch example

  Firefox only: locate and open `osgemscripten.html` file.

  All browsers:
  * go to the directory with `osgemscripten.html`
  * run `python -m SimpleHTTPServer 8080` from there
  * open `http://localhost:8080/osgemscripten.html` in any browser

  You should now see red cube in your web browser.
 
