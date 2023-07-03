#!/bin/bash

#Script d'installation du package scientifique.
#A exécuter en mode superutilisateur (sudo su au préalable)

#Description du script dans le rapport de fin de contrat de Thibault XAVIER (Mai 2021)

#Auteurs : Thibault XAVIER, Vincent ROBIN


#-1 PREREQUIS

apt-get -y install make
apt-get -y install tar gzip bzip2
apt-get -y install zstd 
apt-get -y install patch 
apt-get -y install git curl 
apt-get -y install gpg gnupg2
apt-get -y install gcc
apt-get -y install cmake pkg-config build-essential
apt-get -y install mlocate
apt-get install libz-dev
apt-get install rpm
apt install flex
apt install paraview

apt-get -y install openssh-server openssh-client
systemctl start ssh 

#-2 INSTALLATION DE SPACK 
cd /opt
git clone https://github.com/spack/spack.git
. spack/share/spack/setup-env.sh

#-5 INSTALLATION DES COMPILATEURS GCC 5.5 VIEILLE VERSION COMPATIBLE AVEC SOMAFOAM
spack install gcc@5.5.0
spack load gcc@5.5.0
spack compiler find

#-6 EDITION DES BASHRC DES UTILISATEURS (ACTUELS OU FUTURS)
for NomUser in $(ls /home)
do
  echo ". /opt/spack/share/spack/setup-env.sh" >> /home/$NomUser/.bashrc
  echo "spack load gcc@5.5.0" >> /home/$NomUser/.bashrc
  echo "spack compiler find" >> /home/$NomUser/.bashrc
done

cp /etc/skel/.bashrc /etc/skel/.bashrc_backup
echo ". /opt/spack/share/spack/setup-env.sh" >> /etc/skel/.bashrc
echo "spack load gcc@5.5.0" >> /etc/skel/.bashrc
echo "spack compiler find" >> /etc/skel/.bashrc

#-7 - INSTALLATION DES PACKAGES SCIENTIFIQUES

#Utilitaires
apt-get -y install geany
apt-get -y install libreoffice
apt-get -y install gnuplot grace
apt-get -y install vlc

#Installation des package SPACK compilés via GNU
spack install openmpi@4.0.5 %gcc@5.5.0

#Suppression des packages inutiles 
spack gc -y









