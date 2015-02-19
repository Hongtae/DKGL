import _dk_core as core
from .treeview import TreeView
from .view import View
import os
import sys
import fnmatch
import re
import stat


_WIN32_FILE_ATTRIBUTE_NORMAL = 128
_WIN32_FILE_ATTRIBUTE_HIDDEN = 2
_WIN32_FILE_ATTRIBUTE_READONLY = 1
_WIN32_FILE_ATTRIBUTE_SYSTEM = 4
_WIN32_FILE_ATTRIBUTE_DIRECTORY = 16


def _win32DriveLetters():
    from string import ascii_uppercase
    drives = [c+':' for c in ascii_uppercase if os.path.isdir(c+':')]
    print('windows drives: ', drives)
    return drives

# IGPython 에서는 ctypes 지원 안함!!
def _win32IsFileHidden(path):
    try:
        import ctypes
        attrs = ctypes.windll.kernel32.GetFileAttributesW(path)
        print('GetFileAttributesW({}):{}'.format(path, attrs))
        assert attrs != -1
        if attrs & _WIN32_FILE_ATTRIBUTE_DIRECTORY:
            return bool(attrs & _WIN32_FILE_ATTRIBUTE_HIDDEN)
        if attrs & _WIN32_FILE_ATTRIBUTE_NORMAL:
            return False
        if attrs & _WIN32_FILE_ATTRIBUTE_SYSTEM:
            return True
        return bool(attrs & _WIN32_FILE_ATTRIBUTE_HIDDEN)
    except (AttributeError, AssertionError):
        pass
    return False

def _isFileHidden(filepath, filename):
    if sys.platform == 'win32':
        return _win32IsFileHidden(filepath)
    return False


class FileInfo:
    def __init__(self):
        self.isDir = False
        self.isHidden = False
        self.isSelectable = False
        self.isReadOnly = False
        self.path = ''


class FileSelectView(View):

    showHiddenFiles = False

    borderWidth = 1

    def __init__(self, baseDir='/', *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__treeView = TreeView()
        self.__treeView.borderWidth = 0
        self.__baseDir = baseDir
        self.__filter = ''
        self.addChild(self.__treeView)

    def reload(self):
        self.__treeView.removeAllItems()
        folders = []
        files = []
        try:
            contents = os.listdir(self.__baseDir)
            for name in contents:
                path = os.path.abspath(os.path.join(self.__baseDir, name))
                try:
                    mode = os.stat(path).st_mode
                    if not self.showHiddenFiles:
                        if _isFileHidden(path, name):
                            continue

                    if stat.S_ISDIR(mode):
                        folders.append(name)
                    elif stat.S_ISREG(mode):
                        files.append(name)
                    else:
                        print('Unknown file attr:{}'.format(path))
                except PermissionError as e:
                    print(e)

        except FileNotFoundError:
            pass

        for name in folders:
            path = os.path.abspath(os.path.join(self.__baseDir, name))
            item = self.__treeView.addItem(name)
            item.path = path
            tmp = self.__treeView.addItem('loading...', parent=item)
            tmp.enabled = False

        if len(self.__filter):
            pat = os.path.normcase(self.__filter)
            res = fnmatch.translate(pat)
            match = re.compile(res).match

            for name in files:
                if match(os.path.normcase(name)):
                    path = os.path.abspath(os.path.join(self.__baseDir, name))
                    item = self.__treeView.addItem(name)
                    item.path = path
        else:
            for name in files:
                path = os.path.abspath(os.path.join(self.__baseDir, name))
                item = self.__treeView.addItem(name)
                item.path = path


    def _onTreeViewItemChanged(self, tv, event, item):
        if event == tv.EVENT_ITEM_EXPANDED:
            print('EVENT_ITEM_EXPANDED:', item)
        elif event == tv.EVENT_ITEM_COLLAPSED:
            print('EVENT_ITEM_COLLAPSED:', item)
        # elif event == tv.EVENT_ITEM_STATE_CHANGED:
        #     print('EVENT_ITEM_STATE_CHANGED:', item)
        # elif event == tv.EVENT_ITEM_SELECTION_CHANGED:
        #     print('EVENT_ITEM_SELECTION_CHANGED:', item)
        elif event == tv.EVENT_ITEM_SELECTED:
            print('EVENT_ITEM_SELECTED:', item)

    def onResized(self):
        super().onResized()
        self.__treeView.frame = self.contentBounds()

    def onLoaded(self):
        super().onLoaded()
        self.__treeView.addTarget(self, self._onTreeViewItemChanged)
        self.__treeView.borderWidth = 0
        self.reload()

    def onUnload(self):
        self.__treeView.removeTarget(self)
        super().onUnload()
