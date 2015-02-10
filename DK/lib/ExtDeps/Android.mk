LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := ExtDeps 
LOCAL_CFLAGS := -DFT2_BUILD_LIBRARY -fvisibility=hidden -DNDEBUG=1 -D_NDEBUG=1
LOCAL_CPPFLAGS := -std=c++11 -fvisibility=hidden -mfpu=neon -DNDEBUG=1 -D_NDEBUG=1
LOCAL_CPP_FEATURES := rtti exceptions
LOCAL_CPP_EXTENSION := .cxx .cpp
LOCAL_ARM_NEON := true

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/FreeType/include \
	$(LOCAL_PATH)/libxml2/include \
	$(LOCAL_PATH)/libogg/include \
	$(LOCAL_PATH)/libvorbis/include \
	$(LOCAL_PATH)/libFLAC/include \
	$(LOCAL_PATH)/zlib \
	$(LOCAL_PATH)/bzip2

BZIP2_SRC := \
	bzip2/blocksort.c \
	bzip2/bzlib.c \
	bzip2/compress.c \
	bzip2/crctable.c \
	bzip2/decompress.c \
	bzip2/huffman.c \
	bzip2/randtable.c

CXIMAGE_SRC := \
	CxImage/tif_xfile.cpp \
	CxImage/ximabmp.cpp \
	CxImage/ximadsp.cpp \
	CxImage/ximaenc.cpp \
	CxImage/ximaexif.cpp \
	CxImage/ximage.cpp \
	CxImage/ximagif.cpp \
	CxImage/ximahist.cpp \
	CxImage/ximaico.cpp \
	CxImage/ximainfo.cpp \
	CxImage/ximaint.cpp \
	CxImage/ximajas.cpp \
	CxImage/ximajbg.cpp \
	CxImage/ximajpg.cpp \
	CxImage/ximalpha.cpp \
	CxImage/ximalyr.cpp \
	CxImage/ximamng.cpp \
	CxImage/ximapal.cpp \
	CxImage/ximapcx.cpp \
	CxImage/ximapng.cpp \
	CxImage/ximapsd.cpp \
	CxImage/ximaraw.cpp \
	CxImage/ximasel.cpp \
	CxImage/ximaska.cpp \
	CxImage/ximatga.cpp \
	CxImage/ximath.cpp \
	CxImage/ximatif.cpp \
	CxImage/ximatran.cpp \
	CxImage/ximawbmp.cpp \
	CxImage/ximawmf.cpp \
	CxImage/ximawnd.cpp \
	CxImage/xmemfile.cpp

FREETYPE_SRC := \
	FreeType/src/autofit/autofit.c \
	FreeType/src/bdf/bdf.c \
	FreeType/src/cff/cff.c \
	FreeType/src/base/ftbase.c \
	FreeType/src/base/ftbitmap.c \
	FreeType/src/cache/ftccache.c \
	FreeType/src/base/ftfstype.c \
	FreeType/src/base/ftgasp.c \
	FreeType/src/base/ftglyph.c \
	FreeType/src/gzip/ftgzip.c \
	FreeType/src/base/ftinit.c \
	FreeType/src/lzw/ftlzw.c \
	FreeType/src/base/ftstroke.c \
	FreeType/src/base/ftsystem.c \
	FreeType/src/smooth/smooth.c \
	FreeType/src/base/ftbbox.c \
	FreeType/src/base/ftgxval.c \
	FreeType/src/base/ftlcdfil.c \
	FreeType/src/base/ftmm.c \
	FreeType/src/base/ftotval.c \
	FreeType/src/base/ftpatent.c \
	FreeType/src/base/ftpfr.c \
	FreeType/src/base/ftsynth.c \
	FreeType/src/base/fttype1.c \
	FreeType/src/base/ftwinfnt.c \
	FreeType/src/base/ftxf86.c \
	FreeType/src/pcf/pcf.c \
	FreeType/src/pfr/pfr.c \
	FreeType/src/psaux/psaux.c \
	FreeType/src/pshinter/pshinter.c \
	FreeType/src/psnames/psmodule.c \
	FreeType/src/raster/raster.c \
	FreeType/src/sfnt/sfnt.c \
	FreeType/src/truetype/truetype.c \
	FreeType/src/type1/type1.c \
	FreeType/src/cid/type1cid.c \
	FreeType/src/type42/type42.c \
	FreeType/src/winfonts/winfnt.c

JPEG_SRC := \
	jpeg/jaricom.c \
	jpeg/jcapimin.c \
	jpeg/jcapistd.c \
	jpeg/jcarith.c \
	jpeg/jccoefct.c \
	jpeg/jccolor.c \
	jpeg/jcdctmgr.c \
	jpeg/jchuff.c \
	jpeg/jcinit.c \
	jpeg/jcmainct.c \
	jpeg/jcmarker.c \
	jpeg/jcmaster.c \
	jpeg/jcomapi.c \
	jpeg/jcparam.c \
	jpeg/jcprepct.c \
	jpeg/jcsample.c \
	jpeg/jctrans.c \
	jpeg/jdapimin.c \
	jpeg/jdapistd.c \
	jpeg/jdarith.c \
	jpeg/jdatadst.c \
	jpeg/jdatasrc.c \
	jpeg/jdcoefct.c \
	jpeg/jdcolor.c \
	jpeg/jddctmgr.c \
	jpeg/jdhuff.c \
	jpeg/jdinput.c \
	jpeg/jdmainct.c \
	jpeg/jdmarker.c \
	jpeg/jdmaster.c \
	jpeg/jdmerge.c \
	jpeg/jdpostct.c \
	jpeg/jdsample.c \
	jpeg/jdtrans.c \
	jpeg/jerror.c \
	jpeg/jfdctflt.c \
	jpeg/jfdctfst.c \
	jpeg/jfdctint.c \
	jpeg/jidctflt.c \
	jpeg/jidctfst.c \
	jpeg/jidctint.c \
	jpeg/jmemmgr.c \
	jpeg/jmemnobs.c \
	jpeg/jquant1.c \
	jpeg/jquant2.c \
	jpeg/jutils.c

LIBFLAC_SRC := \
	libFLAC/src/libFLAC/bitmath.c \
	libFLAC/src/libFLAC/bitreader.c \
	libFLAC/src/libFLAC/bitwriter.c \
	libFLAC/src/libFLAC/cpu.c \
	libFLAC/src/libFLAC/crc.c \
	libFLAC/src/libFLAC/fixed.c \
	libFLAC/src/libFLAC/float.c \
	libFLAC/src/libFLAC/format.c \
	libFLAC/src/libFLAC/lpc.c \
	libFLAC/src/libFLAC/md5.c \
	libFLAC/src/libFLAC/memory.c \
	libFLAC/src/libFLAC/metadata_iterators.c \
	libFLAC/src/libFLAC/metadata_object.c \
	libFLAC/src/libFLAC/ogg_decoder_aspect.c \
	libFLAC/src/libFLAC/ogg_encoder_aspect.c \
	libFLAC/src/libFLAC/ogg_helper.c \
	libFLAC/src/libFLAC/ogg_mapping.c \
	libFLAC/src/libFLAC/stream_decoder.c \
	libFLAC/src/libFLAC/stream_encoder.c \
	libFLAC/src/libFLAC/stream_encoder_framing.c \
	libFLAC/src/libFLAC/window.c

LIBOGG_SRC := \
	libogg/src/bitwise.c \
	libogg/src/framing.c

LIBPNG_SRC := \
	libpng/png.c \
	libpng/pngerror.c \
	libpng/pngget.c \
	libpng/pngmem.c \
	libpng/pngpread.c \
	libpng/pngread.c \
	libpng/pngrio.c \
	libpng/pngrtran.c \
	libpng/pngrutil.c \
	libpng/pngset.c \
	libpng/pngtrans.c \
	libpng/pngwio.c \
	libpng/pngwrite.c \
	libpng/pngwtran.c \
	libpng/pngwutil.c

LIBTIFF_SRC := \
	libtiff/mkg3states.c \
	libtiff/tif_aux.c \
	libtiff/tif_close.c \
	libtiff/tif_codec.c \
	libtiff/tif_color.c \
	libtiff/tif_compress.c \
	libtiff/tif_dir.c \
	libtiff/tif_dirinfo.c \
	libtiff/tif_dirread.c \
	libtiff/tif_dirwrite.c \
	libtiff/tif_dumpmode.c \
	libtiff/tif_error.c \
	libtiff/tif_extension.c \
	libtiff/tif_fax3.c \
	libtiff/tif_fax3sm.c \
	libtiff/tif_flush.c \
	libtiff/tif_getimage.c \
	libtiff/tif_jbig.c \
	libtiff/tif_jpeg.c \
	libtiff/tif_jpeg_12.c \
	libtiff/tif_luv.c \
	libtiff/tif_lzma.c \
	libtiff/tif_lzw.c \
	libtiff/tif_next.c \
	libtiff/tif_ojpeg.c \
	libtiff/tif_open.c \
	libtiff/tif_packbits.c \
	libtiff/tif_pixarlog.c \
	libtiff/tif_predict.c \
	libtiff/tif_print.c \
	libtiff/tif_read.c \
	libtiff/tif_strip.c \
	libtiff/tif_swab.c \
	libtiff/tif_thunder.c \
	libtiff/tif_tile.c \
	libtiff/tif_unix.c \
	libtiff/tif_version.c \
	libtiff/tif_warning.c \
	libtiff/tif_write.c \
	libtiff/tif_zip.c \
	libtiff/tif_stream.cxx

LIBVORBIS_SRC := \
	libvorbis/src/analysis.c \
	libvorbis/src/bitrate.c \
	libvorbis/src/block.c \
	libvorbis/src/codebook.c \
	libvorbis/src/envelope.c \
	libvorbis/src/floor0.c \
	libvorbis/src/floor1.c \
	libvorbis/src/info.c \
	libvorbis/src/lookup.c \
	libvorbis/src/lpc.c \
	libvorbis/src/lsp.c \
	libvorbis/src/mapping0.c \
	libvorbis/src/mdct.c \
	libvorbis/src/psy.c \
	libvorbis/src/registry.c \
	libvorbis/src/res0.c \
	libvorbis/src/sharedbook.c \
	libvorbis/src/smallft.c \
	libvorbis/src/synthesis.c \
	libvorbis/src/vorbisenc.c \
	libvorbis/src/vorbisfile.c \
	libvorbis/src/window.c

LIBXML2_SRC := \
	libxml2/src/buf.c \
	libxml2/src/c14n.c \
	libxml2/src/catalog.c \
	libxml2/src/chvalid.c \
	libxml2/src/debugXML.c \
	libxml2/src/dict.c \
	libxml2/src/DOCBparser.c \
	libxml2/src/encoding.c \
	libxml2/src/entities.c \
	libxml2/src/error.c \
	libxml2/src/globals.c \
	libxml2/src/hash.c \
	libxml2/src/HTMLparser.c \
	libxml2/src/HTMLtree.c \
	libxml2/src/legacy.c \
	libxml2/src/list.c \
	libxml2/src/nanoftp.c \
	libxml2/src/nanohttp.c \
	libxml2/src/parser.c \
	libxml2/src/parserInternals.c \
	libxml2/src/pattern.c \
	libxml2/src/relaxng.c \
	libxml2/src/SAX.c \
	libxml2/src/SAX2.c \
	libxml2/src/schematron.c \
	libxml2/src/threads.c \
	libxml2/src/tree.c \
	libxml2/src/uri.c \
	libxml2/src/valid.c \
	libxml2/src/xinclude.c \
	libxml2/src/xlink.c \
	libxml2/src/xmlIO.c \
	libxml2/src/xmlmemory.c \
	libxml2/src/xmlmodule.c \
	libxml2/src/xmlreader.c \
	libxml2/src/xmlregexp.c \
	libxml2/src/xmlsave.c \
	libxml2/src/xmlschemas.c \
	libxml2/src/xmlschemastypes.c \
	libxml2/src/xmlstring.c \
	libxml2/src/xmlunicode.c \
	libxml2/src/xmlwriter.c \
	libxml2/src/xpath.c \
	libxml2/src/xpointer.c \
	libxml2/src/xzlib.c

SQLITE_SRC := \
	sqlite/sqlite3.c

ZLIB_SRC := \
	zlib/adler32.c \
	zlib/compress.c \
	zlib/crc32.c \
	zlib/deflate.c \
	zlib/gzclose.c \
	zlib/gzlib.c \
	zlib/gzread.c \
	zlib/gzwrite.c \
	zlib/infback.c \
	zlib/inffast.c \
	zlib/inflate.c \
	zlib/inftrees.c \
	zlib/ioapi.c \
	zlib/mztools.c \
	zlib/trees.c \
	zlib/uncompr.c \
	zlib/unzip.c \
	zlib/zip.c \
	zlib/zutil.c


LOCAL_SRC_FILES := $(BZIP2_SRC)
LOCAL_SRC_FILES += $(CXIMAGE_SRC)
LOCAL_SRC_FILES += $(FREETYPE_SRC)
LOCAL_SRC_FILES += $(JPEG_SRC)
LOCAL_SRC_FILES += $(LIBFLAC_SRC)
LOCAL_SRC_FILES += $(LIBOGG_SRC)
LOCAL_SRC_FILES += $(LIBPNG_SRC)
LOCAL_SRC_FILES += $(LIBTIFF_SRC)
LOCAL_SRC_FILES += $(LIBVORBIS_SRC)
LOCAL_SRC_FILES += $(LIBXML2_SRC)
LOCAL_SRC_FILES += $(SQLITE_SRC)
LOCAL_SRC_FILES += $(ZLIB_SRC)

include $(BUILD_STATIC_LIBRARY)
