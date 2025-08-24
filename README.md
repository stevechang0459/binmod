# binmod

## Usage

```sh
./bin/binmod.exe -i"C:\Workspace\MAP1001\FW_1001_STD\AllBin_MP\REIJCDHJ\CPU1\BEAGLE.bin\INIT.bin" -o"C:\Workspace\MAP1001\FW_1001_STD\AllBin_MP\REIJCDHJ.bin" -s0x1c0,0x40 -d0x30,0x40
```

```sh
cp ./AllBin_MP/"$name".bin ./AllBin_MP/"$name"_org.bin
binmod.exe -i ./AllBin_MP/"$name"/CPU1/BEAGLE.bin/INIT.bin -o ./AllBin_MP/"$name".bin -s 0x1c0,0x40 -d 0x30,0x40
```

## Format the source code with Linux coding style

```sh
# AStyle
$(pwd)/Astyle/bin/astyle.exe --options=$(pwd)/AStyle/file/linux.ini -R ./*.c,*.h --exclude=AStyle --formatted
```
