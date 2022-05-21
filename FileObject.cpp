#include <stdio.h>

#include "FileObject.h"

FileObject::FileObject(std::string str) : filename(str) {
  printf("<%s> FileObject::FileObject()\n", filename.c_str());
}

FileObject::~FileObject() {
  printf("<%s> ~FileObject::FileObject()\n", filename.c_str());
}