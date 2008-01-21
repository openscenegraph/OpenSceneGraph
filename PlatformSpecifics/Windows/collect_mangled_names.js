/* collect_mangled_names - Copyright (C) 2006 Joran Jessurun 

 *

 * This library is open source and may be redistributed and/or modified under  

 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 

 * (at your option) any later version.  The full license is in LICENSE file

 * included with this distribution, and on the openscenegraph.org website.

 * 

 * This library is distributed in the hope that it will be useful,

 * but WITHOUT ANY WARRANTY; without even the implied warranty of

 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 

 * OpenSceneGraph Public License for more details.

 *

 * Made by Joran Jessurun (A.J.Jessurun@tue.nl)

 */ 

var dumpbin="dumpbin";

var fso=WScript.createObject("Scripting.FileSystemObject");

var ForReading=1;

var ForWriting=2;

var shell=WScript.createObject("WScript.Shell");



function process(file) {

  WScript.echo("Processing: "+file);

  var txt="";

  var exec=shell.exec(dumpbin+' /linkermember:1 "'+file+'"');

  while(!exec.stdOut.atEndOfStream)

  {

     var line=exec.stdOut.readLine();

     if(/3V\?\$RegisterReaderWriterProxy/.test(line)

     || /3VRegisterDotOsgWrapperProxy/.test(line)) 

     {

       txt+=line.substr(10)+"\n";

     }

  }

  while(exec.status!=1) WScript.sleep(100);

  

  if(txt!="") {

    file=file.replace(/\.lib$/m,".sym");

    var f=fso.openTextFile(file,ForWriting,true);

    f.write(txt);

    f.close();    

    WScript.echo("Created:    "+file);

  }

}



WScript.echo("Collecting mangled names");

var files=new Enumerator(fso.getFolder("..\\lib\\win32").Files);

for(;!files.atEnd();files.moveNext()) {

  if(/_s\.lib$/.test(files.item())) {

    process(""+files.item());

  }

}

