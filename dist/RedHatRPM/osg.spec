
Summary: Open Scene Graph
Name: OpenSceneGraph
Version: 0.8.39
Release: 1
Copyright: LGPL
Group: Graphics
Source: osg-0.8.39.tar.gz
URL: http://www.openscenegraph.org 
Packager: Robert Osfield

%description

Open Scene Graph is an open-source scene graph API.

%prep
%setup
%build
%install





# ---------------------
# FILES Sections
%files

%attr(755, root, root) /usr/bin/*
%attr(755, root, root) /usr/lib/*
%attr(444, root, root) /usr/include/osg/*

