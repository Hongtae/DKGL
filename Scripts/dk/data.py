import _dk_core as core


class Data(core.Data):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    @classmethod
    def fromSource(cls, source):
        """
        download from URL or copy from buffer.

        :param source: URL or source Buffer-Object
        :type source: str or Buffer-Object
        :return: new Data object
        :rtype: Data
        """
        return cls(source=source)

    @classmethod
    def openFileMap(cls, path, writable=False):
        """
        mapping existing file as data-buffer.

        :param path: file path. file must be exist!
        :type path: str
        :param writable: open file as writable
        :type writable: bool
        :return: new Data object
        :rtype: Data
        """
        return cls(filemap=path, writable=writable)

    @classmethod
    def createFileMap(cls, path, length, overwrite=False):
        """
        create new file with length, then map as data-buffer.

        :param path: file path
        :type path: str
        :param length: file size
        :type length: int
        :param overwrite: False if file should not be overwritten else True
        :type overwrite: bool
        :return: new Data object
        :rtype: Data
        """
        return cls(filemap=path, length=length, overwrite=overwrite)

    @classmethod
    def createData(cls, length):
        """
        create new buffer object with length sized.

        :param length: buffer length
        :type length: int
        :return: new Data object
        :rtype: Data
        """
        return cls(length=length)

