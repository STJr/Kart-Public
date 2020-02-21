# SRB2Kart

[SRB2Kart](https://srb2.org/mods/) is a kart racing mod based on the 3D Sonic the Hedgehog fangame [Sonic Robo Blast 2](https://srb2.org/), based on a modified version of [Doom Legacy](http://doomlegacy.sourceforge.net/).

## Installing on Linux
See the [releases page](https://github.com/GoDzM4TT3O/Kart-Public) to download srb2kart for linux (pre-compiled binary)
***
# Building
## Dependencies
- NASM (x86 builds only)
- SDL2 (Linux/OS X only)
- SDL2-Mixer (Linux/OS X only)
- libupnp (Linux/OS X only)
- libgme (Linux/OS X only)

## Installing dependencies, cloning, compiling and running (Linux)
### Installing dependencies
Ubuntu/Debian/Mint/ElementaryOS/Pop!\_OS (APT):

```bash
sudo apt install git p7zip gcc build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```

Fedora/RHEL/RedHat (DNF):

```bash
sudo dnf install gcc git p7zip build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```

Gentoo (EMERGE):

```bash
sudo emerge git p7zip gcc build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```

Arch/Manjaro/EndeavourOS/ (PACMAN):

```bash
sudo pacman -S gcc p7zip git build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```
(Also available on the AUR: https://aur.archlinux.org/packages/srb2kart/)

CentOS (YUM):

```bash
sudo yum install gcc p7zip git build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```

OpenSUSE (ZYPPER):

```bash
sudo zypper install gcc p7zip git build-essential nasm libpng-dev zlib1g-dev libsdl2-dev libsdl2-mixer-dev libgme-dev libopenmpt-dev
```


### Cloning
NOTE: As of 20/02/2020 the Kart-Public repo size is around `73 MB`, precisely `69.56 MiB`

`cd $HOME; git clone https://github.com/STJr/Kart-Public; cd Kart-Public`

### Compiling
Known bug: when compiling, you get the following message:

```
No package 'libgme' found
Package libgme was not found in the pkg-config search path.
Perhaps you should add the directory containing libgme.pc'
```

How to fix:
type these two strings in your terminal before compiling:

```bash
export LIBGME_CFLAGS=
export LIBGME_LDFLAGS=-lgme
```

How to read the table:

- x86_64/i686: Architecture name. By default, this is only Linux.
- `make -C src/ LINUX=1`: the command to run to compile srb2kart.
- `bin/Linux/Release`: where the binary is located after compilation, inside the Kart-Public directory.

|          i686          |   x86_64 (64 bit Linux)  |                     Mac OSX                    |          FreeBSD         |          Solaris         |
|:----------------------:|:------------------------:|:----------------------------------------------:|:------------------------:|:------------------------:|
| `make -C src/ LINUX=1` | `make -C src/ LINUX64=1` | `make -C src/ MACOSX=1 NONX86=1 SDL=1 NOASM=1` | `make -C src/ FREEBSD=1` | `make -C src/ SOLARIS=1` |
| `bin/Linux/Release`    | `bin/Linux64/Release`    | `bin/SDL/Release`                              | `bin/FreeBSD/Release`    | `bin/Solaris/Release`    |

If you're unsure, please select 64 bit.

NOTE FOR MACOSX USERS: you will not get a .dmg file! You will only get the binary file!

Please add `ECHO=1` if you want verbosity. (For example `make -C src/ LINUX64=1 ECHO=1`)

[EXAMPLE] Compiling on a 64 bit host:
(make sure you are in `$HOME/Kart-Public/`)
```bash
export LIBGME_CFLAGS=
export LIBGME_LDFLAGS=-lgme
make -C src/ LINUX64=1
```

### Running
Depending on how you built srb2kart, the path may vary.

If you built it for 64 bit, the binary is located in `$HOME/Kart-Public/bin/Linux64/Release` under the name `lsdl2srb2kart`

Here's how to download the missing assets. I will include them in [my releases tab](https://github.com/GoDzM4TT3O/Kart-Public/releases/latest)
```bash
cd $HOME/Downloads # the directory where the installer will be downloaded
wget https://github.com/STJr/Kart-Public/releases/download/v1.1/srb2kart-v11-x64-Installer.exe # download the installer
mv srb2kart{-v11-x64-Installer,}.exe # this shortens the name of the exe
mkdir kart; mv srb2kart.exe kart/ # create the directory "kart" and move the installer there
cd kart; 7z x srb2kart.exe # go into directory and extract the installer
cp *.kart srb2.srb -t $HOME/Kart-Public/bin/Linux64/Release/
# this last command will copy the necessary files into the directory where srb2kart was compiled
```

### One liner to clone, compile and download the assets
For Linux 64-Bit only!

```bash
cd $HOME; git clone https://github.com/STJr/Kart-Public; \
cd Kart-Public; export LIBGME_CFLAGS=; export LIBGME_LDFLAGS=-lgme; \
make -C src/ LINUX64=1; cd $HOME/Downloads; \
wget https://github.com/STJr/Kart-Public/releases/download/v1.1/srb2kart-v11-x64-Installer.exe; \
mv srb2kart{-v11-x64-Installer,}.exe; mkdir kart; mv srb2kart.exe kart/; cd kart/; \
7z x srb2kart.exe; cp *.kart srb2.srb -t $HOME/Kart-Public/bin/Linux64/Release/
```

## Compiling on Windows/OSX

See [SRB2 Wiki/Source code compiling](http://wiki.srb2.org/wiki/Source_code_compiling). The compiling process for SRB2Kart is largely identical to SRB2.

***

## Disclaimer
Kart Krew is in no way affiliated with SEGA or Sonic Team. We do not claim ownership of any of SEGA's intellectual property used in SRB2.
