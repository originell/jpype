# This is the prototype for forward declarations
import typing
import weakref
import sys
from types import ModuleType
import importlib.abc
import importlib.machinery
from . import _jinit       # lgtm [py/import-own-module]
import _jpype

__all__ = ['JForward']

# Convert a forward symbol from string to an actual symbol


def _lookup(symbol: typing.Optional[str]):
    if symbol is None:
        return None
    if _jpype.isPackage(symbol):
        return jpype.JPackage(symbol)
    parts = symbol.split(".")
    for i in range(len(parts)):
        p = ".".join(parts[:i + 1])
        if _jpype.isPackage(p):
            continue
        obj = _jpype.JClass(p)
        if i + 1 == len(parts):
            return obj
        break
    for j in range(i + 1, len(parts)):
        obj = getattr(obj, parts[j])
    return obj


def _resolve(forward, value=None):
    # Only works on forward declarations
    if not isinstance(forward, JForward):
        raise TypeError("Must be a forward declaration")

    # Find the value using Java reflection
    if value is None:
        value = _lookup(forward._symbol)

    # If it was a null symbol then just skip it as unresolvable
    if value is None:
        return

    # if the memory is compatable mutate the object

    # set the instance
    object.__setattr__(forward, "_instance", value)

    # morph the so that the slots act more like their correct behavior
    #   types we have here
    #     - java classes
    #     - java array classes
    #     - java arrays
    #     - java packages
    #     - java static fields
    #     - java throwables
    #     - java primitives
    #     - other?
    if isinstance(value, _jpype._JPackage):
        object.__setattr__(forward, '__class__', _JResolvedObject)
        # FIXME if this ended up in the import table we should replace it
        # in sys.modules[]
    elif isinstance(value, _jpype._JClass):
        object.__setattr__(forward, '__class__', _JResolvedType)
    elif isinstance(value, _jpype._JArray):
        raise TypeError("Forward declarations of arrays are not supported")
    elif isinstance(value, (_jpype._JBoolean, _jpype._JNumberLong, _jpype._JChar, _jpype._JNumberLong, _jpype._JNumberFloat)):
        raise TypeError("Forward declarations of primitives are not supported")
    elif isinstance(value, object):
        # Bypass the JForward setattr, which always raises a JVM must be started
        # exception.
        object.__setattr__(forward, '__class__', _JResolvedObject)
    else:
        raise TypeError("Forward type not supported")

    # Depending on how the base class is implemented and the amount of reserve space
    # we give it we may be able to mutate it into the actual object in C, but
    # only if we can dispose of its accessing resources (dict) and copy
    # over the new resources without overrunning the space requirements.


# Hook up the resolver for all forward declarations
@_jinit.onJVMStart
def _resolveAll():
    for forward in JForward._INSTANCES.values():
        _resolve(forward)
    JForward._INSTANCES.clear()


###########################################################################


# This will be used for morphed types, everything must have a base tree
#  We need to make sure these are fast lookup so we don't change method resoluton cost
class _JForwardProto:
    __slots__ = ('_instance', '_symbol')

# resolved symbol


class _JResolved(_JForwardProto):
    def __getattr__(self, name):
        return getattr(self._instance, name)

    def __setattr__(self, name, value):
        return setattr(self._instance, name)

    def __str__(self):
        return str(self._instance)

    def __repr__(self):
        return repr(self._instance)

    def __eq__(self, other):
        # We swap the order of the __eq__ here to allow other to influence the
        # result further (e.g. it may be a _JResolved also)
        return other == self._instance

    def __ne__(self, other):
        # We swap the order of the __ne__ here to allow other to influence the
        # result further (e.g. it may be a _JResolved also)
        return other != self._instance


# We need to specialize the forward based on the resolved object
class _JResolvedType(_JResolved):

    def __call__(self, *args):
        return self._instance(*args)

    def __matmul__(self, value):
        return self._instance @ value

    def __getitem__(self, value):
        return self._instance[value]

    def __instancecheck__(self, other):
        return isinstance(other, self._instance)

    def __subclasscheck__(self, other):
        return issubclass(self._instance, other)


class _JResolvedObject(_JResolved):
    pass


def _jvmrequired(*args):
    raise RuntimeError("JVM must be started")


# This is an unresolved symbol
#  Must have every possible slot type.


# Public view of a forward declaration
class JForward(_JForwardProto):
    # Keep track of all instances of JForward so that we can always return the
    # same instance for a given symbol, and so that we can resolve these when the
    # JVM starts up.
    _INSTANCES = weakref.WeakValueDictionary()

    def __new__(cls, symbol: typing.Optional[str]):
        if not _jpype.isStarted():
            return _lookup(symbol)
        if symbol in cls._INSTANCES:
            return cls._INSTANCES[symbol]
        self = _JForwardProto.__new__(cls)
        object.__setattr__(self, '_symbol', symbol)
        # A dictionary of JForward attributes that have been accessed on this instance.
        object.__setattr__(self, '_forward_attrs', weakref.WeakValueDictionary())
        self._INSTANCES[symbol] = self
        return self

    def __getattr__(self, name):
        if name.startswith('__'):
            return object.__getattr__(self, name)
        if name in self._forward_attrs:
            return self._forward_attrs[name]

        forward = self._symbol

        if forward is None:
            # Handle this JForward being the root.
            value = JForward(name)
        elif name.startswith('['):
            value = JForward(forward + name)
        else:
            value = JForward(forward + "." + name)
        # Cache the result for later, giving us faster lookup next time.
        self._forward_attrs[forward] = value
        return value

    def __getitem__(self, value):
        if isinstance(value, slice):
            return self.__getattr__("[]")
        if isinstance(value, tuple):
            depth = 0
            for item in value:
                if isinstance(item, slice):
                    depth += 1
                else:
                    raise RuntimeError("Cannot create array without a JVM")
            return self.__getattr__("[]" * depth)
        raise RuntimeError("Cannot create array without a JVM")

    def __setattr__(self, key, value):
        if key.startswith('__'):
            object.__setattr__(self, key, value)
        elif isinstance(value, JForward):
            # Comes from the import machinery, which puts imported submodules
            # onto a package when imported.
            self._forward_attrs[value._symbol] = value
        else:
            raise AttributeError("JVM Must be running")

    __call__ = _jvmrequired
    __matmul__ = _jvmrequired
    __instancecheck__ = _jvmrequired
    __subclasscheck__ = _jvmrequired


class JRootFinder(importlib.abc.MetaPathFinder):
    def __init__(self, module_root_name: str):
        self._root_name = module_root_name
        self._loader = JRootLoader(module_root_name)

    def find_spec(self, fullname, path, target=None):
        if fullname.startswith(f'{self._root_name}.') or fullname == self._root_name:
            spec = importlib.machinery.ModuleSpec(fullname, self._loader, is_package=True)
            return spec


class JRootLoader(importlib.abc.Loader):
    def __init__(self, root_name):
        self._root_name = root_name

    def create_module(self, spec):
        mod = self._load_module(spec.name)
        return mod

    def exec_module(self, mod):
        # Must be implemented as part of importlib's machinery.
        # We don't need to do anything to execute the module.
        pass

    def _load_module(self, fullname):
        java_name = fullname[len(self._root_name) + 1:]
        if not java_name:
            return JForward(None)
        else:
            return JForward(java_name)


sys.meta_path.append(JRootFinder('jpype.jroot'))
