/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */

#include "Python.h"

extern PyObject* PyInit_array(void);

extern PyObject* PyInit_binascii(void);
extern PyObject* PyInit_cmath(void);
extern PyObject* PyInit_errno(void);
extern PyObject* PyInit_faulthandler(void);
extern PyObject* PyInit__tracemalloc(void);
extern PyObject* PyInit_gc(void);
extern PyObject* PyInit_math(void);
extern PyObject* PyInit__md5(void);

extern PyObject* PyInit__operator(void);
extern PyObject* PyInit_signal(void);
extern PyObject* PyInit__sha1(void);
extern PyObject* PyInit__sha256(void);
extern PyObject* PyInit__sha512(void);
extern PyObject* PyInit_time(void);
extern PyObject* PyInit__thread(void);

extern PyObject* PyInit__codecs(void);
extern PyObject* PyInit__weakref(void);
extern PyObject* PyInit_xxsubtype(void);
extern PyObject* PyInit_zipimport(void);
extern PyObject* PyInit__random(void);
extern PyObject* PyInit_itertools(void);
extern PyObject* PyInit__collections(void);
extern PyObject* PyInit__heapq(void);
extern PyObject* PyInit__bisect(void);
extern PyObject* PyInit__symtable(void);
extern PyObject* PyInit_mmap(void);
extern PyObject* PyInit__csv(void);
extern PyObject* PyInit__sre(void);
extern PyObject* PyInit_parser(void);

extern PyObject* PyInit__struct(void);
extern PyObject* PyInit__datetime(void);
extern PyObject* PyInit__functools(void);
extern PyObject* PyInit__json(void);
extern PyObject* PyInit_zlib(void);

/* 2015-01-09 by tiff2766 - BEGIN */
extern PyObject* PyInit_pyexpat(void);
extern PyObject* PyInit_unicodedata(void);
extern PyObject* PyInit_select(void);
extern PyObject* PyInit__elementtree(void);
extern PyObject* PyInit__socket(void);
extern PyObject* PyInit__bz2(void);
extern PyObject* PyInit__sqlite3(void);
extern PyObject* PyInit__ssl(void);
extern PyObject* PyInit__hashlib(void);
extern PyObject* PyInit__decimal(void);
extern PyObject* PyInit__multiprocessing(void);
extern PyObject* PyInit__testcapi(void);
extern PyObject* PyInit__testbuffer(void);
extern PyObject* PyInit__ctypes(void);
extern PyObject* PyInit__ctypes_test(void);
extern PyObject* PyInit_audioop(void);
#ifdef WIN32
extern PyObject* PyInit_nt(void);
extern PyObject* PyInit__locale(void);
extern PyObject* PyInit_msvcrt(void);
extern PyObject* PyInit_winreg(void);
extern PyObject* PyInit__winapi(void);
extern PyObject* PyInit__overlapped(void);
#else
extern PyObject* PyInit_posix(void);
extern PyObject* PyInit__posixsubprocess(void);
#endif
/* 2015-01-09 by tiff2766 - END */

extern PyObject* PyInit__multibytecodec(void);
extern PyObject* PyInit__codecs_cn(void);
extern PyObject* PyInit__codecs_hk(void);
extern PyObject* PyInit__codecs_iso2022(void);
extern PyObject* PyInit__codecs_jp(void);
extern PyObject* PyInit__codecs_kr(void);
extern PyObject* PyInit__codecs_tw(void);
extern PyObject* PyInit__lsprof(void);
extern PyObject* PyInit__ast(void);
extern PyObject* PyInit__io(void);
extern PyObject* PyInit__pickle(void);
extern PyObject* PyInit_atexit(void);
extern PyObject* _PyWarnings_Init(void);
extern PyObject* PyInit__string(void);
extern PyObject* PyInit__stat(void);
extern PyObject* PyInit__opcode(void);

/* tools/freeze/makeconfig.py marker for additional "extern" */
/* -- ADDMODULE MARKER 1 -- */

extern PyObject* PyMarshal_Init(void);
extern PyObject* PyInit_imp(void);

struct _inittab _PyImport_Inittab[] = {

    {"array", PyInit_array},
    {"_ast", PyInit__ast},

    {"binascii", PyInit_binascii},
    {"cmath", PyInit_cmath},
    {"errno", PyInit_errno},
    {"faulthandler", PyInit_faulthandler},
    {"gc", PyInit_gc},
    {"math", PyInit_math},

    {"_operator", PyInit__operator},
    {"signal", PyInit_signal},
    {"_md5", PyInit__md5},
    {"_sha1", PyInit__sha1},
    {"_sha256", PyInit__sha256},
    {"_sha512", PyInit__sha512},
    {"time", PyInit_time},
#ifdef WITH_THREAD
    {"_thread", PyInit__thread},
#endif
	{"_tracemalloc", PyInit__tracemalloc},

	{"_codecs", PyInit__codecs},
    {"_weakref", PyInit__weakref},
    {"_random", PyInit__random},
    {"_bisect", PyInit__bisect},
    {"_heapq", PyInit__heapq},
    {"_lsprof", PyInit__lsprof},
    {"itertools", PyInit_itertools},
    {"_collections", PyInit__collections},
    {"_symtable", PyInit__symtable},
    {"mmap", PyInit_mmap},
    {"_csv", PyInit__csv},
    {"_sre", PyInit__sre},
    {"parser", PyInit_parser},
    {"_struct", PyInit__struct},
    {"_datetime", PyInit__datetime},
    {"_functools", PyInit__functools},
    {"_json", PyInit__json},

    {"xxsubtype", PyInit_xxsubtype},
    {"zipimport", PyInit_zipimport},
    {"zlib", PyInit_zlib},

	/* 2013-01-18 by tiff - BEGIN */
	{"pyexpat", PyInit_pyexpat},
	{"unicodedata", PyInit_unicodedata},
	{"select", PyInit_select},
	{"_elementtree", PyInit__elementtree},
	{"_socket", PyInit__socket},
	{"_bz2", PyInit__bz2},
	{"_sqlite3", PyInit__sqlite3},
	{"_ssl", PyInit__ssl},
	{"_hashlib", PyInit__hashlib},
	{"_decimal", PyInit__decimal},
	{"_multiprocessing", PyInit__multiprocessing},
	{"_testcapi", PyInit__testcapi},
	{"_testbuffer", PyInit__testbuffer},
	{"_ctypes", PyInit__ctypes},
	{"_ctypes_test", PyInit__ctypes_test},    
    {"audioop", PyInit_audioop},
    
#ifdef WIN32
	{"nt", PyInit_nt}, /* Use the NT os functions, not posix */
	{"_winapi", PyInit__winapi},
	{"msvcrt", PyInit_msvcrt},
	{"winreg", PyInit_winreg},
	{"_locale", PyInit__locale},
	{"_overlapped", PyInit__overlapped},
#else
	{"posix", PyInit_posix},
    {"_posixsubprocess", PyInit__posixsubprocess},
#endif
	/* 2013-01-18 by tiff - END */

    /* CJK codecs */
    {"_multibytecodec", PyInit__multibytecodec},
    {"_codecs_cn", PyInit__codecs_cn},
    {"_codecs_hk", PyInit__codecs_hk},
    {"_codecs_iso2022", PyInit__codecs_iso2022},
    {"_codecs_jp", PyInit__codecs_jp},
    {"_codecs_kr", PyInit__codecs_kr},
    {"_codecs_tw", PyInit__codecs_tw},

/* tools/freeze/makeconfig.py marker for additional "_inittab" entries */
/* -- ADDMODULE MARKER 2 -- */

    /* This module "lives in" with marshal.c */
    {"marshal", PyMarshal_Init},

    /* This lives it with import.c */
    {"_imp", PyInit_imp},

    /* These entries are here for sys.builtin_module_names */
    {"builtins", NULL},
    {"sys", NULL},
    {"_warnings", _PyWarnings_Init},
    {"_string", PyInit__string},

    {"_io", PyInit__io},
    {"_pickle", PyInit__pickle},
    {"atexit", PyInit_atexit},
	{"_stat", PyInit__stat},
	{"_opcode", PyInit__opcode},

    /* Sentinel */
    {0, 0}
};
