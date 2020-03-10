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

	static void initialise();
	static void uninitialise();

	static FileSetManager& get();
	virtual ~FileSetManager();

	static bool addFiles(const std::wstring directory);
	static std::wstring toString();
	static int getNoOfFileSets();
	static FileSetT getFileSet(const int n);

private:

	using IdFileSetPairT = std::pair<std::wstring, FileSetT>;
	using FileSetsT = std::vector<IdFileSetPairT>;

	FileSetManager();
	FileSetManager(const FileSetManager&);

	void initialiseImpl();
	void uninitialiseImpl();
	std::wstring toStringImpl();
	bool addFilesImpl(const std::wstring directory);
	int getNoOfFileSetsImpl() const;
	FileSetT getFileSetImpl(const int n) const;

	FileSetsT fileSets_;
};

#endif /* COMMON_FILESETMANAGER_H_ */
