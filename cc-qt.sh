moc -i -o phoenix/qt/qt.moc phoenix/qt/qt.moc.hpp
g++-4.5 -std=gnu++0x -I. -O3 -fomit-frame-pointer -c phoenix/phoenix.cpp `pkg-config --cflags QtCore QtGui` -DPHOENIX_QT
g++-4.5 -std=gnu++0x -I. -O3 -fomit-frame-pointer -c upset.cpp -DPHOENIX_QT
g++-4.5 -s -o upset upset.o phoenix.o `pkg-config --libs QtCore QtGui`
rm *.o
rm phoenix/qt/qt.moc
