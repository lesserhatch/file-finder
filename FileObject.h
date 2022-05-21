#include <filesystem>
#include <string>

#if !defined(FILEOBJECT_H)
#define FILEOBJECT_H

class FileObject {
public:
  std::string filename;
  std::filesystem::directory_entry mDirEntry;

  FileObject(std::string str);
  FileObject(std::filesystem::directory_entry de);

  ~FileObject();

  std::string getFilename();
  std::string getFilepath();
};

#endif