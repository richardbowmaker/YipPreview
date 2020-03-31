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

#include "_Types.h"
#include "FileSet.h"
#include "Volume.h"

class FileSetManager
{
public:

	static void initialise();
	static void uninitialise();

	static FileSetManager& get();
	virtual ~FileSetManager();

//	static bool addFiles(VolumeT volume);
	static std::wstring toString();
	static int getNoOfFileSets();
	static FileSetT getFileSet(const int n);
	static void setFileSets(const FileSetCollT &fileSets);
	static void sort(const ColT col);
	static void toLogger();

private:

	FileSetManager();
	FileSetManager(const FileSetManager&);

	void initialiseImpl();
	void uninitialiseImpl();
	std::wstring toStringImpl();
//	bool addFilesImpl(VolumeT volume);
	int getNoOfFileSetsImpl() const;
	FileSetT getFileSetImpl(const int n) const;
	void setFileSetsImpl(const FileSetCollT &fileSets);
	void sortImpl(const ColT col);
	void toLoggerImpl() const;

	FileSetCollT fileSets_;
	ColT sortCol_;
	bool sortAscend_;
};

#endif /* COMMON_FILESETMANAGER_H_ */
