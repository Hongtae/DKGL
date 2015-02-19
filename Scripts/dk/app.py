import _dk_core as core
import weakref

def _sharedInstance():
    return None

EVN_PATH_SYSTEM_ROOT = 'SystemRoot'
ENV_PATH_APP_ROOT = 'AppRoot'
ENV_PATH_APP_RESOURCE = 'AppResource'
ENV_PATH_APP_EXECUTABLE = 'AppExecutable'
ENV_PATH_APP_DATA = 'AppData'
ENV_PATH_USER_HOME = 'UserHome'
ENV_PATH_USER_DOCUMENTS = 'UserDocuments'
ENV_PATH_USER_PREFERENCES = 'UserPreferences'
ENV_PATH_USER_CACHE = 'UserCache'
ENV_PATH_USER_TEMP = 'UserTemp'

APP_INFO_HOST_NAME = 'HostName'
APP_INFO_OS_NAME = 'OSName'
APP_INFO_USER_NAME = 'UserName'


class App(core.App):
    """This is singleton object, and only one object should be exist
    """
    def __init__(self):
        core.App.__init__(self)
        global _sharedInstance
        _sharedInstance = weakref.ref(self)

    @staticmethod
    def instance():
        return _sharedInstance()

    def onInit(self):
        pass
    
    def onExit(self):
        pass

    def onActivated(self):
        pass

    def onDeactivated(self):
        pass

    def onHidden(self):
        pass

    def onRestore(self):
        pass

    def run(self):
        return super().run()

    def terminate(self, exitCode):
        return super().terminate(exitCode)
