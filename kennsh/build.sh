#!/usr/bin/env sh

for lib in lib/*
do
	if [ -d "$lib" ]
	then
		C_INCLUDE_PATH="$(pwd)/$lib:$C_INCLUDE_PATH"
		CPLUS_INCLUDE_PATH="$(pwd)/$lib:$CPLUS_INCLUDE_PATH"
	fi
done
export C_INCLUDE_PATH
export CPLUS_INCLUDE_PATH

if [ ! -d obj ]
then
	mkdir obj
fi

echo "Compiling C files"
for file in src/*.c
do
	if [ ! -e $file ]
	then
		break
	fi
	
	basefile=$(basename "$file" .c)
	echo "  - $basefile.c"
	gcc "$file" -c -o obj/"$basefile".o
	if [ $? -ne 0 ]
	then
		exit $?
	fi
done

echo "Compiling C++ files"
for file in src/*.cpp
do
	if [ ! -e $file ]
	then
		break
	fi
	
	basefile=$(basename "$file" .cpp)
	echo "  - $basefile.cpp"
	g++ -std=c++11 "$file" -c -o obj/"$basefile".o
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
		case "$file" in
		*.c )
			# C file
			basefile=$(basename "$file" .c)
			echo "    - $basefile.c"
			gcc "$file" -c -o obj/"$libname"/"$basefile".o
			;;
		*.cpp )
			# C++ file
			basefile=$(basename "$file" .cpp)
			echo "    - $basefile.cpp"
			g++ "$file" -c -o obj/"$libname"/"$basefile".o
			;;
		* )
			# Other file
			;;
		esac
		if [ $? -ne 0 ]
		then
			exit $?
		fi
	done
done

echo

echo "Linking object files"
for file in $(find obj -name '*.o')
do
	echo "  - $file" | cut -c 1-4,9-
done
g++ $(find obj -name '*.o') -o kennsh
