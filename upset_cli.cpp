#include "nall/ups.hpp"
#include "nall/file.hpp"
#include "nall/filemap.hpp"

#include <stdio.h>
using namespace std;
using namespace nall;

int diff(char* sourceFilename, char* targetFilename) {
  /*
  if(file::exists(sourceFilename) == false) { MessageWindow::warning(application, "Source file does not exist"); return; }
  if(file::exists(targetFilename) == false) { MessageWindow::warning(application, "Target file does not exist"); return; }
  if(striend(sourceFilename, ".ups") == true) { MessageWindow::warning(application, "Source file extension must not be .ups"); return; }
  if(striend(targetFilename, ".ups") == true) { MessageWindow::warning(application, "Target file extension must not be .ups"); return; }
  */
  string outputFilename = string(nall::basename(sourceFilename), ".ups");

  ups patcher;
  filemap sm(sourceFilename, filemap::mode::read);
  filemap tm(targetFilename, filemap::mode::read);
  ups::result result = patcher.create(sm.data(), sm.size(), tm.data(), tm.size(), outputFilename);

  if(result != ups::result::success) {
    unlink(outputFilename);
    puts("Patch creation failed");
    return 1;
  }

  puts("Patch creation was successful!");
  printf("Output file name: %s\n", notdir(outputFilename)());
}

int patch(char* sourceFilename, char* targetFilename) {
  /*
  if(file::exists(sourceFilename) == false) {
    MessageWindow::warning(application, "UPS patch does not exist"); return; }
  if(file::exists(targetFilename) == false) {
    MessageWindow::warning(application, "Target file does not exist"); return; }
  if(sourceFilename == targetFilename) {
    MessageWindow::warning(application, "Source file and target file cannot be the same"); return; }
  if(striend(sourceFilename, ".ups") == false) {
    MessageWindow::warning(application, "UPS patch extension must be .ups"); return; }
  if(striend(targetFilename, ".ups") == true) {
    MessageWindow::warning(application, "Target file extension must not be .ups"); return; }
  */
  string outputFilename = string(nall::basename(targetFilename), ".bak");
  file targetExpand;
  if(targetExpand.open(outputFilename, file::mode::write) == false) {
    puts("Cannot open target file for writing");
    return 1;
  }

  unsigned outputSize = 0;

  ups patcher;
  filemap sm(sourceFilename, filemap::mode::read);
  filemap tm(targetFilename, filemap::mode::read);
  patcher.apply(sm.data(), sm.size(), tm.data(), tm.size(), 0, outputSize);

  targetExpand.seek(outputSize);
  targetExpand.close();

  //patcher.progress = { &Application::progress, this };
  filemap om(outputFilename, filemap::mode::readwrite);
  ups::result result = patcher.apply(sm.data(), sm.size(), tm.data(), tm.size(), om.data(), outputSize = om.size());

  sm.close();
  tm.close();
  om.close();

  if(result != ups::result::success) {
    unlink(outputFilename);
    puts("Patching failed");
    return 1;
  }

  string tempFilename = string(nall::basename(targetFilename), ".tmp");
  rename(targetFilename, tempFilename);
  rename(outputFilename, targetFilename);
  rename(tempFilename, outputFilename);

  puts("Patching was successful!");
  printf("Backup file name: %s\n", notdir(outputFilename)());
}

int main(int argc, char** argv) {
  if (argc < 3) {
    puts("Syntax: upset_cli <apply|create> [source] [dest]");
    puts("When creating a patch, source means original, dest means the hack.");
    return 1;
  }
  char* mode = argv[1];
  char* sourceFilename = argv[2];
  char* targetFilename = argv[3];
  // apply
  if (mode[0] == 'a') return patch(sourceFilename, targetFilename);
  // create
  if (mode[0] == 'c') return diff(sourceFilename, targetFilename);
}
