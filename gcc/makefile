#PiFan: PiFan.o CPiUtilities.o
#       g++ -o PiFan PiFan.o CPiUtilities.o -I.

CXX=g++
INCLUDES=-I. -I/usr/local/wxwidgets/debug/shared/lib/wx/include/gtk3-unicode-3.1 -I/usr/local/wxwidgets/debug/shared/include/wx-3.1
CXXFLAGS=-std=c++1y -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -DLINUX_BUILD -Wall -O0 -g3 -Wall -c -fmessage-length=0
LIBS=-L/usr/local/wxwidgets/debug/shared/lib -lpthread -lwx_baseu-3.1 -lwx_gtk3u_core-3.1 -lwx_gtk3u_xrc-3.1 -lwx_gtk3u_html-3.1 -lwx_gtk3u_qa-3.1 -lwx_baseu_xml-3.1 -lwx_baseu_net-3.1 -lwx_gtk3u_media-3.1 -lX11


#g++ -std=c++1y -D_FILE_OFFSET_BITS=64
#-DWXUSINGDLL -D__WXGTK__ -DLINUX_BUILD
#-I/usr/local/wxwidgets/debug/shared/lib/wx/include/gtk3-unicode-3.1
#-I/usr/local/wxwidgets/debug/shared/include/wx-3.1
#-O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP
#-MF"Common/MediaPreviewPlayer.d" -MT"Common/MediaPreviewPlayer.o"
#-o "Common/MediaPreviewPlayer.o"
#"/media/nas_share/Top/Data/Projects/WxWidgets/YipPreview/CommonSource/MediaPreviewPlayer.cpp"



#g++ -L/usr/local/wxwidgets/debug/shared/lib -o "YipPreview"
#./Common/CThread.o ./Common/Constants.o ./Common/FileSet.o ./Common/FileSetManager.o ./Common/GridEx.o ./Common/GridTable.o ./Common/GridTableTest.o ./Common/ImagePanel.o ./Common/ImagesBrowser.o ./Common/Logger.o ./Common/Main.o ./Common/MainFunctions.o ./Common/MediaPreviewPlayer.o ./Common/ShellExecute.o ./Common/Tryout.o ./Common/Utilities.o
#-lpthread -lwx_baseu-3.1 -lwx_gtk3u_core-3.1 -lwx_gtk3u_xrc-3.1 -lwx_gtk3u_html-3.1 -lwx_gtk3u_qa-3.1 -lwx_baseu_xml-3.1 -lwx_baseu_net-3.1 -lwx_gtk3u_media-3.1 -lX11


#DEPS= 

SOURCEDIR= ../CommonSource
_SOURCES= \
	Constants.cpp \
	CThread.cpp \
	FileSet.cpp \
	FileSetManager.cpp \
	GridEx.cpp \
	GridTable.cpp \
	GridTableTest.cpp \
	ImagePanel.cpp \
	ImagesBrowser.cpp \
	Logger.cpp \
	Main.cpp \
	MainFunctions.cpp \
	MediaPreviewPlayer.cpp \
	ShellExecute.cpp \
	Tryout.cpp \
	Utilities.cpp \

SOURCES=$(patsubst %,$(SOURCEDIR)/%,$(_SOURCES))

BUILDDIR=./Release

_OBJECTS= \
	Constants.o \
	CThread.o \
	FileSet.o \
	FileSetManager.o \
	GridEx.o \
	GridTable.o \
	GridTableTest.o \
	ImagePanel.o \
	ImagesBrowser.o \
	Logger.o \
	Main.o \
	MainFunctions.o \
	MediaPreviewPlayer.o \
	ShellExecute.o \
	Tryout.o \
	Utilities.o \

OBJECTS=$(patsubst %,$(BUILDDIR)/%,$(_OBJECTS))

EXECUTABLE=$(BUILDDIR)/YipPreview

# following link is useful
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html#zz-2.1

# target : pre-requisites
# <TAB>command
#
#

# runing make with no argment generates the executeable target
all: $(EXECUTABLE)

# release build
#--------------

# object file is dependent on its cpp file and dependencies
$(BUILDDIR)/%.o: $(SOURCEDIR)/%.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(INCLUDES)

# $@ = EXECUTABLE, LHS :
# $^ = OBJECTS, RHS :
$(EXECUTABLE) : $(OBJECTS)
	$(CXX) -o $@ $^ $(LIBS) 

.PHONY: clean

clean:
	rm $(EXECUTABLE) $(OBJECTS)




