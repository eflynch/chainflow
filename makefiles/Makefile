# Modified from github.com/Ledoux/Makefile-for-MaxMSP-externals-Building
# presumably with permission

##############################################################
#
#  Makefile Template for MaxMSP Object on Mac systems written by Erwan Ledoux
#  derived from Makefile Template for wxMac Applications written by Mark Wilkinson
# 
#  This makefile is designed for experienced UNIX
#  developers who wish to learn how to build Mac OS X
#  packages (.app) and MaxMSP packages (.mxo) 
#  using Makefiles rather than Apple's
#  Xcode IDE.
#
#  Developed with the assistance of Kevin Hock, CHB,
#    and DialogBlocks examples.
#  Distribution of this file is subject to the terms of
#    the wxWindows License, see www.wxwidgets.org
#
##############################################################


##############################################################
#
#  A Brief Tutorial on Packages : .mxo MaxMSP objects as Bundle Mac .app
#
#  Rather than use resource forks on application
#    files, as Apple did in their Classic operating
#    system, Mac OS X uses application packages.
#  Application packages have a .app extension and
#    are actually directories which contain the
#    true executable as well as several resource
#    files.  The structure of an application
#    package is:
#
#    (program).app
#     |
#     +-- Contents
#          |
#          +--  Info.plist
#          |
#          +--  MacOS
#          |     |
#          |     +--  (program)
#          |
#          +--  PkgInfo
#          |
#          +--  Resources
#                |
#                +-- (icon file)
#
#    In this diagram "(program)" is the name of
#    the executable program.  "Contents," "MacOS,"
#    and "Resouces," are directories.  "Info.plist"
#    is an XML DTD file.  And "PkgInfo" is a text
#    file containing data.
#  This makefile generates the package directory
#    structure, and it compiles and places the
#    executabel in the MacOS directory.  This
#    makefile also creates the "PkgInfo" and
#    "Info.plist" files.  The PkgInfo file is very
#    simple.  Info.plist is an XML DTD and contains
#    several variables, which can be examined in
#    this makefile.
#  You need to supply the source code and the icon
#    file.  The icon file is copied into the
#    Resources directory of the package.
#
##############################################################

##############################################################
#
#  For the Max Objects specificity, you need to some stuff before compiling :
#
#  1. A version of the MaxSDK library is given here, but you can 
#   of course update it on : http://cycling74.com/products/sdk/
#
#   2. On Mac Systems, a call of an external .mxo object needs to use the MaxAPI
#   framework given here, in the ./MaxSDK-6.0.4/c74support/max-includes/ folder
#   The Makefile will copy this folder into your Library/Frameworks directory if
#   you don't have yet this. 
#
#   3. Note that if you want to build Max Object from C++ class, you may need to use 
#   already written encapsulation functions for C++ class, like max5cpp.h that we can find 
#   for example here : http://code.google.com/p/maxcpp/, but also here attached in the lib folder
#   3-bis. max5cpp.h needs to use the common_symbols_init() function that is not defined into max-includes. 
#   Note that you can to download somewhere its definition like here https://github.com/Cycling74/max6-sdk/blob/master/c74support/max-includes/
#   common/, but the file is also here in the lib folder and included into the compilation.
#
#   4. Last but not Least, where the .mxo object will be installed ?? In /Applications/Max5/Cycling74/max-eternals/
#   Normally, MaxMSP looks automatically to this folder once you create an object on a patch…So it should work hopefully.   
#
#   5. If COMPILATION WORKS, then you can try the example.maxhelp in order to see if Max loads in a good manner the new example object. 
#   Then try the object by using the bang or messages connected to the object just to check that each function is working.
#
#
##############################################################

##############################################################
#
#    These variables are used to compile the program.  You
#    will need to provide this information.  You may need
#    to specify the full path to the wx-config executable.
#    The output path may be left blank.  The program name
#    and object names are essential.  You may use the icon
#    file included with the wxMac distribution
#    (wxMac-2.6.1/src/mac/wxmac.icns) or create your own
#    icon using the Apple Developer Tools
#    (/Developer/Applications/Utilities/Icon Composer).
#
#     Usual problems are : 
#   A. What is the language for my object…If it is in C only, well you may better 
#   change LANGUAGE into c.
#
#   B. What is the architecture of my computer ? Type arch on a shell to see.
#   Depending on the damned diversity of Mac distributions…You should or not change the 
#   ARCH option.
#
#   C. If you need to recompile, because you change something in the Makefile (because it didn't work…)
#   don't forget to do a "make clean" to erase old files in order to let the Makefile doing new ones.
#
##############################################################

PATCHERS=chain.test chain.example
NAME=maxchain

OUTPUTPATH=./package/$(NAME)/patchers

CXX=clang
LANGUAGE=c++

ARCH= -arch i386
OSVERSION=10.10

PROGVER=1.0
COMPANY=edu.mit.media.resenv
BUNDLEPACKAGE=APPL
BUNDLESIGNATURE=????
ICONFILE=maxchain.icns

FRAMEWORKS=$(HOME)/Library/Frameworks/
MAXAPI=/Library/Frameworks/MaxAPI.framework
MAXAUDIOAPI=/Library/Frameworks/MaxAudioAPI.framework
MAXMSPSDK=./max6-sdk
c74SUPPORT=$(MAXMSPSDK)/c74support
MAXINCLUDES=$(c74SUPPORT)/max-includes
MSPINCLUDES=$(c74SUPPORT)/msp-includes
JITINCLUDES=$(c74SUPPORT)/jit-includes

WXCONFIG=wx-config


BUNDLES=$(addprefix $(OUTPUTPATH)/,$(addsuffix .mxo,$(PATCHERS)))
OBJECTS=$(addsuffix .o,$(addprefix ./src/,$(PATCHERS)))


DEVELOPERFOLDER=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer

LIBS=`$(WXCONFIG) --inplace  --libs`
LINKERFLAGS= -lstdc++
WARNINGFLAGS=
OPTFLAGS=
DEBUGFLAGS=

CPPINC=`$(WXCONFIG) --inplace --cxxflags`
LDFLAGS=$(LIBS) $(LINKERFLAGS)
CPPFLAGS=$(CPPINC) $(GCCFLAGS) $(DEBUGFLAGS) $(OPTFLAGS) $(WARNINGFLAGS)

all: $(MAXAPI) $(MAXAUDIOAPI)
all: $(BUNDLES)

clean:
	rm -rf $(OUTPUTPATH)/*
	rm -f ./src/*.o

help:
   @echo "Usage: make -f Makefile [all|clean|help]"

#  This builds the bundle's directory structure.
$(BUNDLES): %.mxo: $(OBJECTS)

	mkdir -p $(OUTPUTPATH)
	mkdir -p $@/Contents
	mkdir -p $@/Contents/MacOS
	mkdir -p $@/Contents/Resources

	# Build Executable
	${CXX} $(ARCH) -bundle \
	-isysroot $(DEVELOPERFOLDER)/SDKs/MacOSX$(OSVERSION).sdk \
	-F$(MAXINCLUDES) \
	-F$(MSPINCLUDES) \
	-F$(JITINCLUDES) \
	-mmacosx-version-min=$(OSVERSION)  \
	-framework MaxAPI \
	-framework MaxAudioAPI \
	-o $@/Contents/MacOS/$(notdir $(basename $@)) ./src/$(basename $(notdir $@)).o

	# Add icon file
	cp -f $(ICONFILE) $@/Contents/Resources/$(ICONFILE)

	# Add pkginfo
	touch $@/Contents/PkgInfo
	@echo -n "$(BUNDLEPACKAGE)$(BUNDLESIGNATURE)" > $@/Contents/PkgInfo

	# Add Info.plist
	@echo "==== Creating Info.plist ===="
	touch $@/Contents/Info.plist
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" >> $@/Contents/Info.plist
	@echo -n "<!DOCTYPE plist PUBLIC " >> $@/Contents/Info.plist
	@echo -n "\"-//Apple Computer//DTD PLIST 1.0//EN\" " >> $@/Contents/Info.plist
	@echo "\"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> $@/Contents/Info.plist
	@echo "<plist version=\"1.0\">" >> $@/Contents/Info.plist
	@echo "<dict>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleDevelopmentRegion</key>" >> $@/Contents/Info.plist
	@echo "   <string>English</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleExecutable</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(MAXOBJECT)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleIconFile</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(ICONFILE)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleName</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(MAXOBJECT)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleIdentifier</key>" >> $@/Contents/Info.plist
	@echo "   <string>com.$(COMPANY).$(PROGRAM)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleInfoDictionaryVersion</key>" >> $@/Contents/Info.plist
	@echo "   <string>6.0</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundlePackageType</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(BUNDLEPACKAGE)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleSignature</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(BUNDLESIGNATURE)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleVersion</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(PROGVER)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleShortVersionString</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(PROGVER)</string>" >> $@/Contents/Info.plist
	@echo "   <key>CFBundleGetInfoString</key>" >> $@/Contents/Info.plist
	@echo "   <string>$(MAXOBJECT), Version $(PROGVER), $(COMPANY)</string>" >> $@/Contents/Info.plist
	@echo "</dict>" >> $@/Contents/Info.plist
	@echo "</plist>" >> $@/Contents/Info.plist
	@echo "\n"


#  This compiles all the .cpp files into .o objects to be linked after
$(OBJECTS): %.o: %.cpp
	@echo "==== Building objects  ===="
	${CXX} $(ARCH) $(CPPFLAGS) -x ${LANGUAGE} \
	-Wmost -Wno-four-char-constants -Wno-unknown-pragmas \
	-isysroot $(DEVELOPERFOLDER)/SDKs/MacOSX$(OSVERSION).sdk \
	-fmessage-length=0 -pipe -Wno-trigraphs -fpascal-strings -O0 \
	-I/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers/ \
	-I$(MAXINCLUDES) \
	-I$(MSPINCLUDES) \
	-I$(JITINCLUDES) \
	-I./lib/ \
	-include "$(MAXINCLUDES)/macho-prefix.pch" \
	-I$(DEVELOPERFOLDER)/SDKs/MacOSX$(OSVERSION).sdk \
	-I$(DEVELOPERFOLDER)/Headers/FlatCarbon \
	-fasm-blocks -mmacosx-version-min=$(OSVERSION) -gdwarf-2 \
	"-DDENORM_WANT_FIX = 1" \
	-c -o $@  $<
	@echo "\n"

.PHONY:    all clean

# This copy the attached MaxAPI framework into the Library if it doesn't yet exist
$(MAXAPI): $(MAXINCLUDES)/MaxAPI.framework
	test -d $(MAXAPI) || cp -r $(MAXINCLUDES)/MaxAPI.framework $(FRAMEWORKS)

# This copy the attached MaxAudioAPI framework into the Library if it doesn't yet exist
$(MAXAUDIOAPI): $(MSPINCLUDES)/MaxAudioAPI.framework
	test -d $(MAXAUDIOAPI) || cp -r $(MSPINCLUDES)/MaxAudioAPI.framework $(FRAMEWORKS)
