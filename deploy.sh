#!/bin/bash

cd --

git clone https://github.com/sisoputnfrba/so-commons-library.git

cd so-commons-library

make

sudo make install

cd --

echo "Commons Instaladas"

cd /home/utnso/tp-2019-1c-misc/funcionesCompartidas

sudo make install

echo "Funciones Compartidas Instaladas"

cd --

sudo apt-get install libreadline6 libreadline6-dev

echo "ReadLine instalada"

mkdir /home/utnso/lfs-base/

mkdir /home/utnso/lfs-base/Metadata

mkdir /home/utnso/lfs-base/Tables

mkdir /home/utnso/lfs-base/Bloques

echo "Carpetas File seteadas"

cp /home/utnso/tp-2019-1c-misc/FileSystem/Metadata.bin /home/utnso/lfs-base/Metadata/Metadata.bin

echo "Metadata File copiada"

cd /home/utnso/tp-2019-1c-misc/FileSystem/ 

make compiler

cd /home/utnso/tp-2019-1c-misc/kernel/

make compiler

cd /home/utnso/tp-2019-1c-misc/memoria/

make compiler

echo "Si no hubo errores, el ejecutable se escontrara en la carpeta de cada proyecto, ejecutelo con normalidad"
echo "¡ Exitos !"
