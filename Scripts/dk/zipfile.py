import _dk_core as core


METHOD_UNKNOWN = 0
METHOD_STORED = 1
METHOD_DEFLATED = 2
METHOD_BZIP2ED = 3


class ZipArchiver(core.ZipArchiver):
    def __init__(self, file, append=False):
        core.ZipArchiver.__init__(self, file, append)

    def write(self, file, source, compressionLevel = 6, password = None):
        if password:
            return core.ZipArchiver(self, file, source, compressionLevel, password)
        else:
            return core.ZipArchiver(self, file, source, compressionLevel)
 

class ZipUnarchiver(core.ZipUnarchiver):
    def __init__(self, file):
        core.ZipUnarchiver.__init__(self, file)
        self.files = core.ZipUnarchiver.allFiles(self)

    def allFiles(self):
        return self.files

    def fileInfo(self, file):
        info = core.ZipUnarchiver.fileInfo(self, file)
        info.crc32 = hex(info.crc32)
        return info

    def openFileStream(self, file, password=None):
        if password:
            return core.ZipUnarchiver.openFileStream(self, file, password)
        else:
            return core.ZipUnarchiver.openFileStream(self, file)
