#include <string>

#if !defined(FILEOBJECT_H)
#define FILEOBJECT_H

class FileObject {
public:
  std::string filename;

  FileObject(std::string str);
  ~FileObject();
};

#endif