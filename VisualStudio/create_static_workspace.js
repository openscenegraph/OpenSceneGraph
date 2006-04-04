/* create_static_workspace - Copyright (C) 2006 Joran Jessurun 
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
var fso=WScript.createObject("Scripting.FileSystemObject");
var ForReading=1;
var ForWriting=2;

WScript.echo("Creating OpenSceneGraphStatic.dsw from OpenSceneGraph.dsw");

var source=fso.openTextFile("OpenSceneGraph.dsw",ForReading);
var destin=fso.openTextFile("OpenSceneGraphStatic.dsw",ForWriting,true);
var state=0; // 0=copy, 1=skip
while(!source.atEndOfStream) {
  line=source.readLine();
  if(/Begin Project Dependency/.test(line)) state=1;
  if(state==0) destin.writeLine(line);
  if(/End Project Dependency/.test(line)) state=0;  
}
source.close();
destin.close();

WScript.echo("Done");
