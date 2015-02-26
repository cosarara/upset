#include "upset.hpp"
Application application;

void Application::initialize() {
  create(128, 128, 256, 256, "upset v04");
  #if defined(PLATFORM_WIN)
  font.create("Tahoma", 8);
  #else
  font.create("Sans", 8);
  #endif
  setDefaultFont(font);

  unsigned x = 5, y = 5, labelHeight = 15, buttonHeight = 25;

  modeLabel.create(*this, x, y, 80, labelHeight, "Patch mode:");
  modeApply.create(*this, x + 85, y, 385, labelHeight, "Apply an existing source UPS patch to target file"); y += labelHeight + 5;
  modeCreate.create(modeApply, x + 85, y, 385, labelHeight, "Create a new UPS patch from source and target files"); y += labelHeight + 5;

  sourceLabel.create(*this, x, y, 80, buttonHeight, "Source file:");
  sourcePath.create(*this, x + 85, y, 300, buttonHeight);
  sourceBrowse.create(*this, x + 390, y, 80, buttonHeight, "Browse ..."); y += buttonHeight + 5;

  targetLabel.create(*this, x, y, 80, buttonHeight, "Target file:");
  targetPath.create(*this, x + 85, y, 300, buttonHeight);
  targetBrowse.create(*this, x + 390, y, 80, buttonHeight, "Browse ..."); y += buttonHeight + 5;

  progressBar.create(*this, x, y, 385, buttonHeight);
  progressBar.setVisible(false);
  okButton.create(*this, x + 390, y, 80, buttonHeight, "Ok"); y += buttonHeight + 5;
  okButton.setEnabled(false);

  setGeometry(128, 128, 480, y);

  sourcePath.onChange = targetPath.onChange = { &Application::synchronize, this };

  sourceBrowse.onTick = []() {
    if(application.modeApply.checked()) {
      string filename = OS::fileOpen(application, "UPS patches\t*.ups");
      if(filename == "") return;
      application.sourcePath.setText(filename);
    } else {
      string filename = OS::fileOpen(application, "All files\t*");
      if(filename == "") return;
      application.sourcePath.setText(filename);
    }
    application.synchronize();
  };

  targetBrowse.onTick = []() {
    if(application.modeApply.checked()) {
      string filename = OS::fileSave(application, "All files\t*");
      if(filename == "") return;
      application.targetPath.setText(filename);
    } else {
      string filename = OS::fileOpen(application, "All files\t*");
      if(filename == "") return;
      application.targetPath.setText(filename);
    }
    application.synchronize();
  };

  okButton.onTick = []() {
    application.prepare();
    if(application.modeApply.checked()) {
      application.applyPatch(application.sourcePath.text(), application.targetPath.text());
    } else {
      application.createPatch(application.sourcePath.text(), application.targetPath.text());
    }
    OS::quit();
  };

  onClose = []() {
    OS::quit();
    return true;
  };
}

void Application::synchronize() {
  if(sourcePath.text() == "" || targetPath.text() == "") {
    okButton.setEnabled(false);
  } else {
    okButton.setEnabled(true);
  }
}

void Application::prepare() {
  sourcePath.setEnabled(false);
  sourceBrowse.setEnabled(false);
  targetPath.setEnabled(false);
  targetBrowse.setEnabled(false);
  okButton.setEnabled(false);
  progressValue = 0;
  progressBar.setPosition(0);
  progressBar.setVisible(true);
}

void Application::progress(unsigned offset, unsigned length) {
  unsigned currentProgress = (double)offset / (double)length * 100.0;
  if(currentProgress > progressValue) {
    progressBar.setPosition(progressValue = currentProgress);
    OS::run();
  }
}

void Application::applyPatch(string sourceFilename, string targetFilename) {
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
  string outputFilename = string(nall::basename(targetFilename), ".bak");
  file targetExpand;
  if(targetExpand.open(outputFilename, file::mode::write) == false) {
    MessageWindow::warning(application, "Cannot open target file for writing"); return; }

  unsigned outputSize = 0;

  ups patcher;
  filemap sm(sourceFilename, filemap::mode::read);
  filemap tm(targetFilename, filemap::mode::read);
  patcher.apply(sm.data(), sm.size(), tm.data(), tm.size(), 0, outputSize);

  targetExpand.seek(outputSize);
  targetExpand.close();

  patcher.progress = { &Application::progress, this };
  filemap om(outputFilename, filemap::mode::readwrite);
  ups::result result = patcher.apply(sm.data(), sm.size(), tm.data(), tm.size(), om.data(), outputSize = om.size());

  sm.close();
  tm.close();
  om.close();

  if(result != ups::result::success) {
    unlink(outputFilename);
    MessageWindow::warning(application, "Patching failed");
    return;
  }

  string tempFilename = string(nall::basename(targetFilename), ".tmp");
  rename(targetFilename, tempFilename);
  rename(outputFilename, targetFilename);
  rename(tempFilename, outputFilename);

  MessageWindow::information(application, string(
    "Patching was successful!\n"
    "Backup file name: ", notdir(outputFilename)
  ));
}

void Application::createPatch(string sourceFilename, string targetFilename) {
  if(file::exists(sourceFilename) == false) { MessageWindow::warning(application, "Source file does not exist"); return; }
  if(file::exists(targetFilename) == false) { MessageWindow::warning(application, "Target file does not exist"); return; }
  if(striend(sourceFilename, ".ups") == true) { MessageWindow::warning(application, "Source file extension must not be .ups"); return; }
  if(striend(targetFilename, ".ups") == true) { MessageWindow::warning(application, "Target file extension must not be .ups"); return; }
  string outputFilename = string(nall::basename(sourceFilename), ".ups");

  ups patcher;
  filemap sm(sourceFilename, filemap::mode::read);
  filemap tm(targetFilename, filemap::mode::read);
  patcher.progress = { &Application::progress, this };
  ups::result result = patcher.create(sm.data(), sm.size(), tm.data(), tm.size(), outputFilename);

  if(result != ups::result::success) {
    unlink(outputFilename);
    MessageWindow::warning(application, "Patch creation failed");
    return;
  }

  MessageWindow::information(application, string(
    "Patch creation was successful!\n"
    "Output file name: ", notdir(outputFilename)
  ));
}

void Application::main(int argc, char **argv) {
  initialize();
  setVisible(true);
  OS::main();
}

int main(int argc, char **argv) {
  application.main(argc, argv);
  return 0;
}
