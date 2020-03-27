/*
 * Volume.cpp
 *
 *  Created on: 19 Mar 2020
 *      Author: richard
 */

#include "Volume.h"

#include <fstream>
#include <sstream>
#include <stdio.h>

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



Volume::Volume(std::wstring file, const bool isMountable) :
	file_(file),
	isMountable_(isMountable),
	isMounted_(false),
	isSelected_(false)
{
	// if file is not a mountable volume then take the
	// file to be a directory containing files that a mounted volume would
	if (!isMountable_) mount_ = file_;
}

Volume::~Volume()
{
}

std::wstring Volume::getMount() const
{
	return mount_;
}

bool Volume::getIsMountable() const
{
	return isMountable_;
}

bool Volume::getIsMounted() const
{
	return isMounted_;
}

std::wstring Volume::getPropertiesFile() const
{
	return mount_ + Constants::propertiesCache;
}

std::wstring Volume::getFilesDirectory() const
{
	return mount_ + Constants::filesDir;
}

void Volume::addFileSet(FileSetT &fileset)
{
	filesets_.push_back(fileset);
}

void Volume::readProperties()
{
	// read the properties cache into a map of file id v properties list string
	std::string pf = SU::wStrToStr(getPropertiesFile());
	std::ifstream pcache(pf);
	std::string sl;
	while (std::getline(pcache, sl))
	{
		std::wstring wl = SU::strToWStr(sl);
		int n = wl.find(L";");

		if (n != std::wstring::npos)
		{
			std::wstring fn = wl.substr(0, n);
			std::wstring id = FileSet::filenameToId(fn);
			std::wstring ps = wl.substr(n + 1);

			FileSetCollT::const_iterator it =
				std::find_if(filesets_.begin(), filesets_.end(),
						[id](const FileSetT &fs) { return fs->getId().compare(id) == 0; });

			if (it != filesets_.end())
				(*it)->properties().fromString(ps);
		}
	}
	pcache.close();
}

void Volume::writeProperties()
{
	if (isMountable_ && !isMounted_) return;

	std::string pf = SU::wStrToStr(getPropertiesFile());
	std::ofstream pcache(pf);
	for (auto fs : filesets_)
	{
		if (fs->properties().getSize() > 0)
		{
			pcache  << SU::wStrToStr(fs->getId())
					<< ";"
					<< SU::wStrToStr(fs->properties().toString())
					<< std::endl;
		}
	}
	pcache.close();
}

bool Volume::mount(const std::wstring &m, const std::wstring &password)
{
	if (isMounted_)
	{
		Logger::error(L"Volume::mount(), volume already mounted %ls as %ls, %ls !!",
				file_.c_str(), m.c_str(), m.c_str());
		return false;
	}

#ifdef WINDOWS_BUILD
#elif LINUX_BUILD

	// /usr/bin/veracrypt --password=... --slot=n --hash=sha512 file.hc /media/...
	// /usr/bin/veracrypt --password=dummypassword --slot=3 --hash=sha512 /media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/Encrypted/TestVol1.hc /media/veracrypt1

	std::wstringstream cmd;
	cmd 	<< Constants::veracrypt
			<< L" --password=" << password
			<< " --slot=1 --hash=sha512 "
			<< '\"' << file_ << "\" "
			<< m;

	std::wstring s = cmd.str();

	// mount must be in sudo mode
	SudoMode sudo;
	ShellExecuteResult result;

	// if running in the IDE, i.e. not at sudo level, then bigger timeout
	// will allow the user to type in their password
	int tout = sudo.inSudoMode() ? 10000 : 60000;
	ShellExecute::shellSync(cmd.str(), result, tout);
	sudo.lower();

	Logger::info(L"%ls", result.toString().c_str());

	if (result.getSuccess())
	{
		Logger::info(L"Volume %ls mounted ok as %ls", file_.c_str(), m.c_str());
		mount_ = m;
		isMounted_ = true;
		isDirty_ = false;
	}
	else
	{
		Logger::error(L"Volume %ls failed to mount as %ls", file_.c_str(), m.c_str());
		return false;
	}

#endif
}

bool Volume::unmount()
{
	if (!isMounted_)
	{
		Logger::error(L"Volume::unmount(), volume not mounted %ls", file_.c_str());
		return false;
	}
#ifdef WINDOWS_BUILD
#elif LINUX_BUILD
	std::wstringstream cmd;
	cmd << Constants::veracrypt << L" -d " << " " << mount_;

	SudoMode sudo;
	ShellExecuteResult result;
	ShellExecute::shellSync(cmd.str(), result, 5000);
	sudo.lower();

	Logger::info(L"%ls", result.toString().c_str());


	if (result.getSuccess())
	{
		Logger::info(L"Volume::unmount(), Volume %ls unmounted ok, %ls", file_.c_str(), mount_.c_str());
		if (isDirty_)
			Logger::warning(L"Volume::unmount(), Dirty volume %ls unmounted, %ls", file_.c_str(), mount_.c_str());
		mount_.clear();
		isMounted_ = false;
		isDirty_ = false;
		return true;
	}
	else
	{
		Logger::error(L"Volume::unmount(), Volume %ls failed to unmount, %ls", file_.c_str(), mount_.c_str());
		return false;
	}
#endif
}

void Volume::toLogger() const
{
	Logger::info(L"Volume");
	if (isMounted_)
	{
		Logger::info(L"\tmounted");
		Logger::info(L"\mount %ls", mount_.c_str());
	}
	else
		Logger::info(L"\tnot mounted");

	for (auto fs : filesets_)
		fs->toLogger();
}





