/*
 * FileSetManager.h
 *
 *  Created on: 14 Feb 2020
 *      Author: richard
 */

#ifndef COMMON_FILESETMANAGER_H_
#define COMMON_FILESETMANAGER_H_

#include <string>
#include <utility>
#include <vector>

#include "FileSet.h"

class FileSetManager
{
public:

	using IdFileSetPairT = std::pair<std::wstring, FileSetT>;
	using FileSetsT = std::vector<IdFileSetPairT>;

	static FileSetManager& get();
	virtual ~FileSetManager();

	static bool addFiles(const std::wstring directory);
	static std::wstring toString();
	static int getNoOfFileSets();
	static FileSet& getFileSet(const int n);

private:

	FileSetManager();
	FileSetManager(const FileSetManager&);

	std::wstring toStringImpl();
	bool addFilesImpl(const std::wstring directory);
	int getNoOfFileSetsImpl() const;
	FileSet& getFileSetImpl(const int n) const;

	FileSetsT fileSets_;
};

#endif /* COMMON_FILESETMANAGER_H_ */
