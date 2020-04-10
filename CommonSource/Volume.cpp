/*
 * Volume.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "Volume.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdio.h>

#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	#include <sys/statvfs.h>
#endif


#include "_Types.h"
#include "Constants.h"
#include "Logger.h"
#include "ShellExecute.h"
#include "Utilities.h"


//          // mount:
//            //      "C:\Program Files\VeraCrypt\VeraCrypt.exe" /q /a /nowaitdlg y /hash sha512 /v VolAccounts.hc /l x /p password
//            string cmd = Constants.VeracryptExe;
//            string args = "/q /a /nowaitdlg y /hash sha512" + " /v \"" + Filename + "\" /l " + drive.Substring(0, 1).ToLower() + " /p " + Constants.Password;
//            string wdir = Constants.WorkingFolder;
//

// sudo /usr/bin/veracrypt /q /a /nowaitdlg y /hash sha512 /v TestVol1.hc /l x /p dummypassword


// windows
// "C:\Program Files\VeraCrypt\VeraCrypt.exe" /q /a /nowaitdlg y /hash sha512 /v VolAccounts.hc /l x /p password
// 	/q = background mode
//	/a = automatically mount specified volume
//  /nowaitdlg y = no wait dialog displayed
//  /hash sha512 = hashing alogrithm
//  /v file = volume file
//  /l x = drive letter x
//  /p password

// linux
// /usr/bin/veracrypt --hash=sha512 --mount=file --password=dummypassword --slot=n

// /usr/bin/veracrypt --hash=sha512 --mount=TestVol1.hc --password=dummypassword --slot=1
//
// /usr/bin/veracrypt --hash=sha512 --password=dummypassword --slot=1 --mount=/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc


/*
 *
 * /usr/bin/veracrypt -t /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc /media/veracrypt1
 * /usr/bin/veracrypt --password=dummypassword --slot=3 --hash=sha512 /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc /media/veracrypt1
 *
 * dismount = /usr/bin/veracrypt -d /media/veracrypt1
 *
 *
 *
 */

Volume::Volume(std::string file, const bool isMountable) :
	file_(file),
	isMountable_(isMountable),
	isMounted_(false),
	isDirty_(false),
	isSelected_(false)
{
	// if file is not a mountable volume then take the
	// file to be a directory containing files that a mounted volume would
	if (!isMountable_)
		mount_ = file;

	short_ = FU::abbreviateFilename(file, 30);
}

Volume::~Volume()
{
}

std::string Volume::getFile() const
{
	return file_;
}

std::string Volume::getMount() const
{
	return mount_;
}

std::string Volume::getShortName() const
{
	return short_;
}

bool Volume::getIsMountable() const
{
	return isMountable_;
}

bool Volume::getIsMounted() const
{
	return isMounted_;
}

bool Volume::getIsSelected() const
{
	return isSelected_;
}

void Volume::setIsSelected(const bool isSelected)
{
	isSelected_ = isSelected;
}

std::string Volume::getPropertiesFile() const
{
	return mount_ + Constants::propertiesCache;
}

std::string Volume::getFilesDirectory() const
{
	return mount_ + Constants::filesDir;
}

void Volume::readProperties()
{
	// read the properties cache into a map of file id v properties list string
	std::ifstream pcache(getPropertiesFile());
	std::string sl;
	while (std::getline(pcache, sl))
	{
		size_t n = sl.find(";");

		if (n != std::string::npos)
		{
			std::string fn = sl.substr(0, n);
			std::string id = FileSet::filenameToId(fn);
			std::string ps = sl.substr(n + 1);

			FileSetCollT::const_iterator it =
				std::find_if(fileSets_.begin(), fileSets_.end(),
						[id](const FileSetT &fs) { return fs->getId().compare(id) == 0; });

			if (it != fileSets_.end())
				(*it)->properties().fromString(ps);
		}
	}
	pcache.close();
}

void Volume::writeProperties()
{
	if (isMountable_ && !isMounted_) return;

//bug check
	int pc = 0;
	for (auto fs : fileSets_)
		pc += fs->properties().getSize();

	if (pc == 0)
	{
		int ret = US::messageBox("No properties for volume {}\n Cancel = skip writing properties",
				"write properties", wxOK | wxCANCEL, nullptr);
		if (ret == wxID_CANCEL) return;
	}
// bug check


	std::ofstream pcache(getPropertiesFile());
	for (auto fs : fileSets_)
	{
		if (fs->properties().getSize() > 0)
		{
			pcache  << fs->getId()
					<< ";"
					<< fs->properties().toString()
					<< std::endl;
		}
	}
	pcache.close();
}

bool Volume::mount(const std::string &m, const std::string &password)
{
	if (isMounted_)
	{
		Logger::error("Volume::mount(), volume already mounted {} as {}, {} !!", file_, m, m);
		return false;
	}
	std::stringstream cmd;

#ifdef WINDOWS_BUILD
//  "C:\Program Files\VeraCrypt\VeraCrypt.exe" /q /a /nowaitdlg y /hash sha512 /v VolAccounts.hc /l x /p password
	cmd << Constants::veracrypt
		<< R"( /q /a /nowaitdlg y /hash sha512)"
		<< R"( /v )" << SU::doubleQuotes(file_)
		<< R"( /l )" << m[0]
		<< R"( /p )" << password;
#elif LINUX_BUILD
// /usr/bin/veracrypt --password=... --slot=n --hash=sha512 file.hc /media/...
// /usr/bin/veracrypt --password=dummypassword --slot=3 --hash=sha512 /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc /media/veracrypt1
	cmd 	<< Constants::veracrypt
			<< " --password=" << password
			<< " --slot=" << m.substr(m.size() - 2, 2) << " --hash=sha512 "
			<< SU::doubleQuotes(file_) << " "
			<< m;
	Logger::error("Volume::mount() TODO, sort out slot number");
#endif
	// mount must be in sudo mode
#if LINUX_BUILD
	SudoMode sudo;
#endif
	ShellExecuteResult result;

	// if running in the IDE, i.e. not at sudo level, then bigger timeout
	// will allow the user to type in their password
	ShellExecute::shellSync(cmd.str(), result, 15000);

	Logger::info("{}", result.toString());

	if (result.getSuccess())
	{
		Logger::info("Volume {} mounted ok as {}", file_, m);
		mount_ = m;
		isMounted_ = true;
		isDirty_ = false;
		return true;
	}
	else
	{
		Logger::error("Volume {} failed to mount as {}", file_, m);
		return false;
	}
}

bool Volume::unmount()
{
	if (!isMounted_)
	{
		Logger::error("Volume::unmount(), volume not mounted {}", file_);
		return false;
	}

	std::stringstream cmd;
#ifdef WINDOWS_BUILD
//  "C:\Program Files\VeraCrypt\VeraCrypt.exe" /q /nowaitdlg y /force /d x
	cmd << Constants::veracrypt << R"( /q /nowaitdlg y /force /d )" << mount_[0];
#elif LINUX_BUILD
	cmd << Constants::veracrypt << " -d " << " " << mount_;
	SudoMode sudo;
#endif
	ShellExecuteResult result;
	ShellExecute::shellSync(cmd.str(), result, 5000);

	Logger::info("{}", result.toString());

	if (result.getSuccess())
	{
		Logger::info("Volume::unmount(), Volume {} unmounted ok, {}", file_, mount_);
		if (isDirty_)
			Logger::warning("Volume::unmount(), Dirty volume {} unmounted, {}", file_, mount_);
		mount_.clear();
		isMounted_ = false;
		isDirty_ = false;
		return true;
	}
	else
	{
		Logger::error("Volume::unmount(), Volume {} failed to unmount, {}", file_, mount_);
		return false;
	}
}

void Volume::loadFiles()
{
	fileSets_.clear();

	StringCollT files;
	if (FU::findMatchingFiles(getFilesDirectory(), files, "*"))
	{
		for (auto f : files)
		{
			if (FileSet::isValidType(f))
			{
				// search for existing file set entry
				std::string id = FileSet::filenameToId(f);

				FileSetCollT::const_iterator it =
					std::find_if(fileSets_.begin(), fileSets_.end(),
							[id](const FileSetT &fs) { return fs->getId().compare(id) == 0; });

				if (it != fileSets_.end())
				{
					(*it)->set(f);
				}
				else
				{
					// create file set object and add to collection
					FileSetT fs = std::make_shared<FileSet>(this, f);
					fileSets_.push_back(fs);
				}
			}
		}
	}
	else
		Logger::warning(
				"Volume::loadFiles() empty directory {}",
				getFilesDirectory());
}

void Volume::clearFiles()
{
	fileSets_.clear();
}

FileSetCollT Volume::getFileSets() const
{
	return fileSets_;
}

bool Volume::hasFileSets() const
{
	return fileSets_.size() > 0;
}

void Volume::setIsDirty(const bool dirty)
{
	isDirty_ = dirty;
}

bool Volume::getIsdirty() const
{
	return isDirty_;
}

void Volume::addFileSet(FileSetT &fs)
{
	fileSets_.push_back(fs);
}

void Volume::removeFileSet(FileSetT& fs)
{
	FileSetCollT::const_iterator n = std::find_if(
		fileSets_.cbegin(), fileSets_.cend(),
		[fs](const FileSetT &fs1) { return fs->getId().compare(fs1->getId()) == 0; });

	if (n != fileSets_.cend())
		fileSets_.erase(n);
}

bool Volume::hasFreeSpace(const long long size)
{
	if (!isMounted_) return false;

	FU::DiskSpaceT space;
	if (FU::getVolumeFreeSpace(mount_, space))
		return space.free > (Constants::minDiskFreeSpace + size);
	else return false;
}

std::string Volume::toString() const
{
	std::stringstream s;
	s << file_;
	if (isMountable_ && isMounted_) s << ", mount=" << mount_;
	if (isDirty_) s << " *";
	if (isSelected_) s << " X";

	if (isMounted_)
	{
		FU::DiskSpaceT space;
		if (FU::getVolumeFreeSpace(mount_, space))
			s << " " << US::bytesToString(space.free);
	}

	return s.str();
}

void Volume::toLogger() const
{
	Logger::info("Volume");
	if (isMounted_)
	{
		Logger::info("\tmounted");
		Logger::info("\tmount {}", mount_);
	}
	else
		Logger::info("\tnot mounted");

	for (auto fs : fileSets_)
		fs->toLogger();
}





