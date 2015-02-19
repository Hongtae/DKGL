import _dk_core as core


class Screen(core.Screen):
    '''Screen controller class
    '''

    def __init__(self, window, frame):
        assert isinstance(window, core.Window)
        assert isinstance(frame, core.Frame)
        super().__init__(window=window, frame=frame)

    def terminate(self, wait=False):
        ''' destroy screen.
        Once destroyed, object no longer usable.
        this is helper method to destroy screen and unload frame properly.
        Call this method when application begin terminated.
        '''
        super().terminate(wait)

    def postOperation(self, callable, args, kwargs={}, delay=0.0, waitUntilDone=False):
        '''post operation to call once after delay'''
        return super().postOperation(callable, args, kwargs, delay, waitUntilDone)

    def scheduleOperation(self, callable, args, kwargs={}, interval=0.01):
        '''run operation periodic intervals.
        if key is not None, a handler will be stored inside this object.
        to release handler call unscheduleOperation(key)
        '''
        return super().scheduleOperation(callable, args, kwargs, interval)

    def isWorkingThread(self):
        '''determine working thread'''
        return super().isWorkingThread()


#core.setDefaultClass(core.Screen, Screen)
