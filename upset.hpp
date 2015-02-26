#include <nall/detect.hpp>
#include <nall/file.hpp>
#include <nall/filemap.hpp>
#include <nall/foreach.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/ups.hpp>
using namespace nall;

#include <phoenix/phoenix.hpp>
using namespace phoenix;

struct Application : Window {
  Font font;
  Label modeLabel;
  RadioBox modeApply;
  RadioBox modeCreate;
  Label sourceLabel;
  TextBox sourcePath;
  Button sourceBrowse;
  Label targetLabel;
  TextBox targetPath;
  Button targetBrowse;
  ProgressBar progressBar;
  Button okButton;
  unsigned progressValue;

  void initialize();
  void synchronize();
  void prepare();
  void progress(unsigned offset, unsigned length);
  void applyPatch(string sourceFilename, string targetFilename);
  void createPatch(string sourceFilename, string targetFilename);
  void main(int argc, char **argv);
};
