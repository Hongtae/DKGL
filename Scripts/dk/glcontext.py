import _dk_core as core

class OpenGLContext(core.OpenGLContext):
    '''To enable OpenGL functionality in thread
    use this object with with-context-manager.'''

    def __enter__(self): 
        self.bind()
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.unbind()

