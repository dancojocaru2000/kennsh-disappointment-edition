#!/usr/bin/env sh

for lib in lib/*
do
	if [ -d "$lib" ]
	then
		C_INCLUDE_PATH="$(pwd)/$lib:$C_INCLUDE_PATH"
	fi
done
export C_INCLUDE_PATH

if [ ! -d obj ]
then
	mkdir obj
fi

echo "Compiling C files"
for file in src/*.c
do
	basefile=$(basename "$file" .c)
	echo "  - $basefile.c"
	gcc "$file" -c -o obj/"$basefile".o
	if [ $? -ne 0 ]
	then
		exit $?
	fi
done

echo

echo "Compiling libraries"
for lib in lib/*.compile
do
	libname=$(basename "$lib" .compile)
	echo "  - $libname"
	if [ ! -d "obj/$libname" ]
	then
		mkdir "obj/$libname"
	fi
	for file in $(cat "$lib")
	do
		file="lib/$libname/$file"
		basefile=$(basename "$file" .c)
		echo "    - $basefile.c"
		gcc "$file" -c -o obj/"$libname"/"$basefile".o
		if [ $? -ne 0 ]
		then
			exit $?
		fi
	done
done

echo

echo "Linking object files"
for file in obj/**.o
do
	echo "  - $file" | cut -c 1-4,9-
done
gcc obj/**.o -o client
