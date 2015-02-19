import _dk_core as core

# file open mode
FILE_OPEN_NEW = 0
FILE_OPEN_CREATE = 1
FILE_OPEN_EXISTING = 2
FILE_OPEN_READONLY = 3
FILE_OPEN_ALWAYS = 4

# file share mode
FILE_SHARE_ALL = 0
FILE_SHARE_READ = 1
FILE_SHARE_EXCLUSIVE = 2 


class Stream(core.Stream):

    @classmethod
    def createFromFile(cls, file, mode, share):
        """
        :param file: file absolute-path to open
        :type file: str
        :param mode: file mode
        :type mode: int
        :param share: share mode
        :type share: int
        :return: new Stream object
        :rtype: Stream
        """
        return cls(file, mode, share)

