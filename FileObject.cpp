#include "FileObject.h"

FileObject::FileObject(std::filesystem::directory_entry de) : mDirEntry(de) {}

FileObject::~FileObject() {}

std::string FileObject::getFilename() {
  return mDirEntry.path().filename();
}

std::string FileObject::getFilepath() {
  return mDirEntry.path();
}