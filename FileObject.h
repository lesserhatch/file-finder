#include <filesystem>
#include <string>

#if !defined(FILEOBJECT_H)
#define FILEOBJECT_H

class FileObject {
public:
  std::filesystem::directory_entry mDirEntry;

  FileObject(std::filesystem::directory_entry de);

  std::string getFilename();
  std::string getFilepath();
};

#endif