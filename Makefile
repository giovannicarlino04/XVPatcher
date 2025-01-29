CC := i686-w64-mingw32-gcc
CXX := i686-w64-mingw32-g++

OUTPUT := xinput1_3.dll
OUTPUT_COPY_UNIX := "/home/giovanni/.local/share/Steam/steamapps/common/DB Xenoverse/xinput1_3.dll"
OUTPUT_COPY_WINDOWS = "D:\SteamLibrary\steamapps\common\DB Xenoverse"

OBJS := main.o debug.o CpkFile.o CpkDef.hpp BaseFile.o patch.o patches.hpp PatchUtils.o Utils.o MemoryStream.o FixedMemoryStream.o Stream.o IggyFile.o
OBJS += ./crypto/sha1.o ./crypto/md5.o ./crypto/rijndael.o
OBJS += ./tinyxml/tinystr.o ./tinyxml/tinyxml.o ./tinyxml/tinyxmlerror.o ./tinyxml/tinyxmlparser.o
CFLAGS := -Wall -I. -I./ -std=gnu99 -mms-bitfields -s -O2 -masm=intel -shared -Wl,--subsystem,windows,--kill-at,--enable-stdcall-fixup -DNO_ZLIB -fpermissive
CPPFLAGS := -Wall -O2 -mno-ms-bitfields -std=c++11 -DTIXML_USE_STL -DNO_ZLIB
CXXFLAGS := -Wall -Wno-strict-aliasing -I./ -O2 -std=c++11 -mms-bitfields -DTIXML_USE_STL -fpermissive
LDFLAGS := -static xinput.def
LIBS := -lxinput1_3 -lstdc++ -lversion -lpthread -lMinHook

.PHONY: all clean

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) *.o

$(OUTPUT): $(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	cp $(OUTPUT) $(OUTPUT_COPY_WINDOWS)