<p>How to build and compile the third-party libraries for OSG and OSG? </p>

​            **The author called jing_zhong will introduce for you!**

​            If you want to compile the OSG on Windows platform, please ensure you have the compiled third-party dependency. Because I have compiled the third-party library for OpenScenegraph many times, write this tutorial and hope to help developers and engineers!

# 1、Two general compiling methods

In general , we use three compiled methods to compile the source code for obtaining the compiled .lib、.dll  files (etc).

## 1.1  MinGW---gcc--g++ for linux、unix-like、windows

​        configure->make->make install

## 1.2  Visual Studio---MSVC---VC++ for Windows

## ### 2.1 nmake

## ### 2.2 CMake->configure->generate->open project->ALLBUILD->INSTALL



## 2、The compiled third-party libraries includes

​        **Here indrocues 8 libraries:** <kbd>curl</kbd>、<kbd>zlib</kbd>、<kbd>libpng</kbd>、<kbd>jpeg</kbd>、<kbd>freetype</kbd>、<kbd>glut</kbd>、<kbd>tiff</kbd>and <kbd>giflib</kbd>.

## 3、My compiled environment is following:

### 3.1     **Operation System**

###              <u>Windows 11（x64）</u>

### 3.2**Installed Software**

​        <u>CMake</u>、<u>Microsoft Visual Studio 20015</u>、<u>MSYS2</u>

## 4、**Download the source code of third-party libraries and unzip them**

<table>
    <caption>The download_url for the source code of third-party libraries!</caption>
    <tr>
        <td>
            curl-7.83.1.tar.gz
        </td>
        <td>
            https://curl.se/download/curl-7.83.1.tar.gz
        </td>
    </tr>
    <tr>
        <td>
            zlib-1.2.12.tar.gz
        </td>
        <td>
            http://www.zlib.net/zlib-1.2.12.tar.gz
        </td>
    </tr>
    <tr>
        <td>
            libpng-1.6.37.tar.gz
        </td>
        <td>
            https://download.sourceforge.net/libpng/libpng-1.6.37.tar.gz
        </td>
    </tr>
    <tr>
        <td>
            jpegsr9e.zip
        </td>
        <td>
            http://www.ijg.org/files/jpegsr9e.zip
        </td>
    </tr>
    <tr>
        <td>
            freetype-2.12.1.tar.gz
        </td>
        <td>
            https://download.savannah.gnu.org/releases/freetype/freetype-2.12.1.tar.gz
        </td>
    </tr>
    <tr>
        <td>
            glut37.zip
        </td>
        <td>
            https://www.opengl.org/resources/libraries/glut/glut37.zip
        </td>
    </tr>
    <tr>
        <td>
            tiff-4.4.0.tar.gz
        </td>
        <td>
            http://download.osgeo.org/libtiff/tiff-4.4.0.tar.gz
        </td>
    </tr>
    <tr>
        <td>
            giflib-5.2.1.tar.gz
        </td>
        <td>
            https://sourceforge.net/projects/giflib/files/giflib-5.2.1.tar.gz
        </td>
    </tr>
 </table>


​    

## 5、Compile the third-party libraries for OSG(x64、Debug and Release)

### 5.1   **The compiled order of third-party libraries for OSG** 

​        curl  >  zlib  > libpng > freetype > jpeg > glut > tiff > giflib

### 5.2  **The compiled method of third-party libraries for OSG**

<ul>
    <li>curl   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>
        zlib   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>libpng   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>
        freetype   （VS2015---builds/win/2010---build）
    </li>
    <li>jpeg   （configure->make->make install）
    </li>
    <li>
        glut   （VS2015---nmake---glutmake）
    </li>
    <li>tiff   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>
        giflib   （VS2015---New Project---build）
    </li>
</ul>


## 6、Notes(Must read)

## 6.1 Attention for compiling libpng(x64、Debug and Release)

​      When compiling the libpng, you would better compile dependent libraries（**bzip2**、**brotli**、**harfbuzz**).

(1) **Download the source code of three dependent libraries** **for libpng**.

<table>
    <tr>
        <td>
            bzip2-latest.tar.gz 
        </td>
        <td>
        	http://sourceware.org/pub/bzip2/bzip2-latest.tar.gz 
        </td>
    </tr>
    <tr>
        <td>
            brotli
        </td>
        <td>
            git clone https://github.com/google/brotli.git
        </td>
    </tr>
    <tr>
        <td>
            harfbuzz-4.3.0.tar.xz
        </td>
        <td>
            https://github.com/harfbuzz/harfbuzz/releases/download/4.3.0/harfbuzz-4.3.0.tar.xz
        </td>
    </tr>
</table>


(2) **The compiled method for three dependent libraries for libpng**.

<ul>
    <li>
        bzip2   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>
        brotli   （CMake---VS2015---ALLBUILD---INSTALL）
    </li>
    <li>
        harfbuzz   （configure->make->make install）
    </li>
</ul>


## 6.2 Attention for compiling glut(x64、Debug and Release)

(1)  Replace glutwin32.mak and edit it.
(2)  Copy the file->(C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\Include\Win32.Mak) to the directory->(D:\Program Files (x86)\Microsoft Visual Studio 2015\VC\include) and rename this file as win32.mak.

(3)  Replace the file->(..\glut37\glut-3.7\lib\glut\Makefile.win)

(4)  Replace the file->(..\glut37\glut-3.7\progs\demos\particle\particle.c\)

(5)  Open the VS2015 developer command prompt and enter the directory of glut

​      type the command：

```bash
glutmake
```

## 6.3 Attention for compiling giflib(x64、Debug and Release)

(1)  Open VS 2015, New->Visual C++->Win32 Program->Select the static lib, do not include precompiled header

(2)  Open the file **gif_hash.h** and edit, comment the line：

```c++
// #include <unistd.h>
```

(3)  Open the file **gif_lib.h** and edit, in front of the line: 

```c++
#ifdef __cplusplus
```

​      add this code:

```c++
#pragma warning( disable : 4996 )
```

(4)  Open the file **getopt.c** and edit, replace **#include<strings.h>** as **#include<string.h>**

(5)  After build the program, mannually copy the **.lib** and **.h** to the installed giflib directory(**the folder lib、the folder include**)



Finally, congratulations!!!

# 7、Compile the OSG

Download the source code of OSG.

CMake->VS2015->ALLBUILD->INSTALL
