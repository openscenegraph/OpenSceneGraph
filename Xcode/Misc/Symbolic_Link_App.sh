#!/bin/bash

for file in *.app; do
	echo ${file}
	(cd "$file/Contents"; \
	 	ln -s ../../../Frameworks/ Frameworks; \
		ln -s ../../../PlugIns/ PlugIns; \
		ln -s ../../../Resources/ Resources;
	)
done


