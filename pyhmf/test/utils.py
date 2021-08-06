import inspect
import functools
import warnings

class FailureWarning(Warning):
    pass

def get_parent_class(m):
    for cls in inspect.getmro(m.__self__.__class__):
        if m.__name__ in cls.__dict__: return cls


def repeat(n=1):
    def repeat_test(func):
        @functools.wraps(func)
        def dummy(*args, **kwargs):
            for i in range(n):
                func(*args, **kwargs)
        return dummy
    return repeat_test

def fails(message):
    def fails_test(func):
        @functools.wraps(func)
        def dummy(self, *args, **kwargs):
            location = ' (in ' + self.__class__.__name__ + '.' + func.__name__ + ')'
            try:
                func(*args, **kwargs)
                raise "This should fail. Now I'm sad!"
            except:
                warnings.warn(message + location, FailureWarning)
        return dummy
    return fails_test

def parametrize(params):
    def decorator(method):
        for param in params:
            def wrapper(self, parameter=param):
                method(self, parameter)

            functools.update_wrapper(wrapper, method)

            new_method_name = '{}_{}'.format(method.__name__, param)
            wrapper.__name__ = new_method_name

            frame = inspect.currentframe().f_back
            frame.f_locals[new_method_name] = wrapper
        return None
    return decorator
