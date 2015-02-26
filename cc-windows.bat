windres phoenix/windows/phoenix.rc phoenix-resource.o
g++ -std=gnu++0x -I. -O3 -fomit-frame-pointer -c phoenix/phoenix.cpp -DPHOENIX_WINDOWS
g++ -std=gnu++0x -I. -O3 -fomit-frame-pointer -c upset.cpp -DPHOENIX_WINDOWS
g++ -mwindows -s -o upset.exe upset.o phoenix.o phoenix-resource.o -lkernel32 -luser32 -lgdi32 -ladvapi32 -lcomctl32 -lcomdlg32 -lshell32 -lole32
upx --best upset.exe
@pause
@del *.o
