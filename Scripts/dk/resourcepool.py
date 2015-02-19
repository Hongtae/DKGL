import _dk_core as core
import os
from . import zipfile


class _DirLocator:
    def __init__(self, dir):
        self.dir = dir

    def getSystemPath(self, file):
        file = os.path.normpath(os.path.join(self.dir, file))
        if os.path.isfile(file):
            return file

    def openFile(self, file):
        file = os.path.normpath(os.path.join(self.dir, file))
        if os.path.isfile(file):
            try:
                s = core.Data(filemap=file, writable=False)
                return s
            except:
                pass


class _ZipLocator:
    def __init__(self, zip, prefix):
        assert isinstance(zip, core.ZipUnarchiver)
        self.zip = zip
        if prefix[-1:] == '/':
            self.prefix = prefix[:-1]
        else:
            self.prefix = prefix

    def getSystemPath(self, file):
        pass

    def openFile(self, file):
        files = (self.prefix + '/' + file, self.prefix + '\\' + file)
        for f in files:
            try:
                s = self.zip.openFileStream(f)
                return s
            except:
                pass


_URLPrefix = ('http://', 'ftp://', 'file://')


class ResourcePool(core.ResourceLoader):
    """
    ResourcePool

    Loading core resource from file or URL and keep object alive in a pool.
    """

    def __init__(self):
        super().__init__()
        self.resources = {}
        self.data = {}
        self.locators = {}

    def addResource(self, name, res):
        '''add resource to pool'''
        self.resources[name] = res

    def addResourceData(self, name, data):
        '''add resource-data to pool'''
        self.data[name] = data

    def removeAllResources(self):
        self.resources = {}

    def removeAllResourceData(self):
        self.data = {}

    def removeResource(self, name):
        try:
            del(self.resources[name])
        except:
            pass

    def removeResourceData(self, name):
        try:
            del(self.data[name])
        except KeyError:
            pass

    def findResource(self, name):
        return self.resources.get(name, None)

    def findResourceData(self, name):
        return self.data.get(name, None)

    def findResourcePath(self, name):
        for loc in self.locators.values():
            s = loc.getSystemPath(name)
            if s:
                return s

    def loadResource(self, name):
        res = self.findResource(name)
        if res:
            return res

        print('loading resource:', name)

        if name.startswith(_URLPrefix):
            res = self.resourceFromObject(name)
        else:
            path = self.findResourcePath(name)
            if path:
                res = self.resourceFromObject(path, name)
            else:
                stream = self.openResourceStream(name)
                if stream:
                    res = self.resourceFromObject(stream, name)
        if res:
            self.addResource(name, res)
            return res

    def loadResourceData(self, name):
        data = self.findResourceData(name)
        if data:
            return data

        print('loading resource data:', name)

        if name.startswith(_URLPrefix):
            data = core.Data(source = name)
        else:
            path = self.findResourcePath(name)
            if path:    # file-system file
                try:
                    data = core.Data(filemap=path, writable=False)
                except:
                    pass
            else:       # zip file
                stream = self.openResourceStream(name)
                if stream:
                    data = core.Data(source=stream)

        if data:
            self.addResourceData(name, data)
            return data

    def openResourceStream(self, name):
        '''load file or data to byte-like object(buffer),
        use mmap (or core.Data) for local file.'''
        s = self.findResourceData(name)
        if s:
            return s

        for loc in self.locators.values():
            s = loc.openFile(name)
            if s:
                return s

    def addSearchPath(self, path):
        '''add search path, path must be filesystem directory or zip file.
        a zip file can have zip + prefix style.
        ie, addSearchPath("/myfiles.zip/myPrefix"),
        then just files which name starts with 'myPrefix' will be used.
        '''
        if path in self.locators:
            return True

        abspath = os.path.abspath(path)
        if os.path.isdir(abspath):
            # system directory
            loc = _DirLocator(abspath)
            self.locators[path] = loc
            return True          
        else:
            # zip + prefix
            p = path.replace('\\', '/')

            index = len(path)
            while index >= 0:
                p = p[:index]
                file = os.path.normpath(p)
                rest = path[index+1:]
                if os.path.isfile(file):
                    try:
                        zip = zipfile.ZipUnarchiver(file)
                        loc = _ZipLocator(zip, rest)
                        self.locators[path] = loc
                        return True
                    except Exception as e:
                        print('addSearchPath Error: ', e)
                        break

                index = p.rfind('/')
        return False

    def removeSearchPath(self, path):
        try:
            del(self.locators[path])
        except KeyError:
            pass

    def clone(self):
        obj = type(self)()
        obj.resources = self.resources.copy()
        obj.data = self.data.copy()
        obj.locators = self.locators.copy()
        return obj
