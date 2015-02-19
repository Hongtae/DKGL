import sys
import os
import dk
import gc
from .mainframe import MainFrame

class App(dk.App):
    ''' 응용 프로그램 인스턴스 (dk.App)'''
    def onInit(self):

        self.resourceDir = os.path.abspath(os.path.join( os.path.dirname(__file__), 'resources' ))

        print('resourceDir: ', self.resourceDir)

        displayBounds = self.displayBounds(0)
        contentBounds = self.screenContentBounds(0)
        print('displayBounds: ', displayBounds)
        print('contentBounds: ', contentBounds)

        platform = dk.platform()
        contentSize = dk.Size(1024, 768)
        if 'ios' in platform or 'android' in platform:
            contentSize = dk.Size(contentBounds.size)
        else:
            if contentSize.width > contentBounds.width:
                contentSize.width = contentBounds.width
            if contentSize.height > contentBounds.height:
                contentSize.height = contentBounds.height

        print('main screen resolution: ', contentSize)

        window = dk.Window('editor', contentSize)
        screen = dk.Screen(window, MainFrame())
        if not screen:
            print('screen error!?')
            self.terminate(2)
        else:
            self.screen = screen
            self.screen.activeFrameLatency = 0
            self.screen.inactiveFrameLatency = 0
            self.screen.window.activate()
        pass

    def onExit(self):
        if hasattr(self, 'screen'):
            self.screen.terminate(True)
        self.screen = None
        gc.collect()

if __name__ == '__main__':
    App().run()
