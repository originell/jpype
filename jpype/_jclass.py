# *****************************************************************************
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   See NOTICE file for details.
#
# *****************************************************************************
import _jpype
from ._pykeywords import pysafe
from . import _jcustomizer
import inspect

__all__ = ['JClass', 'JInterface', 'JOverride', 'JPublic', 'JProtected',
        'JPrivate', 'JThrows']


class _JFieldDecl(object):
    def __init__(self, cls, name, value, modifiers):
        self.cls = cls
        self.name = name
        self.value = value
        self.modifiers = modifiers

    def __repr__(self):
        return "Field(%s,%s)" % (self.cls.__name__, self.name)


def _JMemberDecl(nonlocals, target, strict, modifiers, **kwargs):
    """Generic annotation to pass to the code generator.
    """
    if not "__jspec__" in nonlocals:
        nonlocals["__jspec__"] = []
    jspec = nonlocals["__jspec__"]

    if isinstance(target, type):
        if not isinstance(target, _jpype.JClass):
            raise TypeError("Fields must be Java classes")
        prim = issubclass(target, (_jpype._JBoolean, _jpype._JNumberLong, _jpype._JChar, _jpype._JNumberFloat))
        out = []
        for p, v in kwargs.items():
            if not prim and v is not None:
                raise ValueError("Initial value must be None")
            if prim:
                v = _jpype.JObject(v, target)  # box it
            var = _JFieldDecl(target, p, v, modifiers)
            jspec.append(var)
            out.append(var)
        return out

    if isinstance(target, type(_JMemberDecl)):
        spec = inspect.getfullargspec(target)
        args = spec.args

        # Verify the requirements for arguments are met
        # Must have a this argument first
        if strict:
            if len(args) < 1:
                raise TypeError("Methods require this argument")
            if args[0] != "this":
                raise TypeError("Methods first argument must be this")

            # All other arguments must be annotated as JClass types
            for i in range(1, len(args)):
                if not args[i] in spec.annotations:
                    raise TypeError("Methods types must have specifications")
                if not isinstance(spec.annotations[args[i]], _jpype.JClass):
                    raise TypeError("Method arguments must be Java classes")

            if target.__name__ != "__init__":
                if "return" not in spec.annotations:
                    raise TypeError("Return specification required")
                if not isinstance(spec.annotations["return"], (_jpype.JClass, type(None))):
                    raise TypeError("Return type must be Java type")

        # Place in the Java spec list
        for p, v in kwargs.items():
            object.__setattr__(target, p, v)
        if modifiers is not None:
            jspec.append(target)
            object.__setattr__(target, '__jmodifiers__', modifiers)
        return target

    raise TypeError("Unknown Java specification '%s'" % type(target))


def JPublic(target, **kwargs):
    nonlocals = inspect.stack()[1][0].f_locals
    return _JMemberDecl(nonlocals, target, True, 1, **kwargs)


def JProtected(target, **kwargs):
    nonlocals = inspect.stack()[1][0].f_locals
    return _JMemberDecl(nonlocals, target, True, 4, **kwargs)


def JPrivate(target, **kwargs):
    nonlocals = inspect.stack()[1][0].f_locals
    return _JMemberDecl(nonlocals, target, True, 2, **kwargs)


def JThrows(*args):
    for arg in args:
        if not isinstance(arg, _jpype.JException):
            raise TypeError("JThrows requires Java exception arguments")

    def deferred(target):
        object.__setattr__(target, '__jthrows__', args)
        return target
    return deferred


def JOverride(*target, sticky=False, rename=None, **kwargs):
    """Annotation to denote a method as overriding a Java method.

    This annotation applies to customizers, proxies, and extensions
    to Java classes. Apply it to methods to mark them as implementing
    or overriding Java methods.  Keyword arguments are passed to the
    corresponding implementation factory.

    Args:
      sticky=bool: Applies a customizer method to all derived classes.

    """
    nonlocals = inspect.stack()[1][0].f_locals
    if len(target) == 0:
        overrides = {}
        if kwargs:
            overrides.update(kwargs)
        if sticky:
            overrides["sticky"] = True
        if rename is not None:
            overrides["rename"] = rename

        def deferred(method):
            return _JMemberDecl(nonlocals, method, False, None, __joverride__=overrides)
        return deferred
    if len(target) == 1:
        return _JMemberDecl(nonlocals, *target, False, None, __joverride__={})
    raise TypeError("JOverride can only have one argument")


class JClassMeta(type):
    def __instancecheck__(self, other):
        return type(other) == _jpype._JClass


class JClass(_jpype._JClass, metaclass=JClassMeta):
    """Meta class for all Java class instances.

    When called as an object, JClass will contruct a new Java class wrapper.

    All Python wrappers for Java classes derive from this type.
    To test if a Python class is a Java wrapper use
    ``isinstance(obj, jpype.JClass)``.

    Args:
      className (str): name of a Java type.

    Keyword Args:
      loader (java.lang.ClassLoader): specifies a class loader to use
        when creating a class.
      initialize (bool): If true the class will be loaded and initialized.
        Otherwise, static members will be uninitialized.

    Returns:
      JavaClass: a new wrapper for a Java class

    Raises:
      TypeError: if the component class is invalid or could not be found.
    """
    def __new__(cls, jc, loader=None, initialize=True):
        if loader and isinstance(jc, str):
            jc = _jpype._java_lang_Class.forName(jc, initialize, loader)

        # Handle generics
        if isinstance(jc, str) and jc.endswith(">"):
            i = jc.find("<")
            params = jc[i + 1:-1]
            ret = _jpype._getClass(jc[:i])
            acceptParams = len(ret.class_.getTypeParameters())
            if acceptParams == 0:
                raise TypeError(
                    "Java class '%s' does not take parameters" % (ret.__name__))
            if len(params) > 0:
                params = params.split(',')
            if len(params) > 0 and len(params) != len(ret.class_.getTypeParameters()):
                raise TypeError(
                    "Java generic class '%s' length mismatch" % (ret.__name__))
            return ret

        # Pass to class factory to create the type
        return _jpype._getClass(jc)


class JInterface(_jpype._JObject, internal=True):
    """A meta class for all Java Interfaces.

    ``JInterface`` is serves as the base class for any Java class that is
    a pure interface without implementation. It is not possible to create
    a instance of a Java interface.

    Example:

    .. code-block:: python

       if isinstance(java.util.function.Function, jpype.JInterface):
          print("is interface")

    """
    pass


def _jclassPre(name, bases, members):
    # Correct keyword conflicts with Python
    m = list(members.items())
    for k, v in m:
        k2 = pysafe(k)
        if k2 != k:
            del members[k]
            members[k2] = v

    # Apply customizers
    hints = _jcustomizer.getClassHints(name)
    hints.applyCustomizers(name, bases, members)
    return (name, tuple(bases), members)


def _jclassPost(res, *args):
    # Post customizers
    hints = _jcustomizer.getClassHints(res.__name__)
    res._hints = hints
    hints.applyInitializer(res)

    # Attach public inner classes we find
    #   Due to bootstrapping, we must wait until java.lang.Class is defined
    #   before we can access the class structures.
    if _jpype._java_lang_Class:
        for cls in res.class_.getDeclaredClasses():
            if cls.getModifiers() & 1 == 0:
                continue
            wrapper = _jpype.JClass(cls)
            res._customize(str(cls.getSimpleName()), wrapper)


def _jclassDoc(cls):
    """Generator for JClass.__doc__ property

    Parameters:
       cls (JClass): class to document.

    Returns:
      The doc string for the class.
    """
    out = []
    if not hasattr(cls, "__javadoc__"):
        jde = JClass("org.jpype.javadoc.JavadocExtractor")()
        jd = jde.getDocumentation(cls)
        if jd is not None:
            setattr(cls, "__javadoc__", jd)
            if jd.description is not None:
                out.append(str(jd.description))
            if jd.ctors is not None:
                out.append(str(jd.ctors))
            return "".join(out)
    setattr(cls, "__javadoc__", None)
    from textwrap import TextWrapper
    jclass = cls.class_
    out.append("Java class '%s'" % (jclass.getName()))
    out.append("")

    sup = jclass.getSuperclass()
    if sup:
        out.append("    Extends:")
        out.append("        %s" % sup.getName())
        out.append("")

    intfs = jclass.getInterfaces()
    if intfs:
        out.append("    Interfaces:")
        words = ", ".join([str(i.getCanonicalName()) for i in intfs])
        wrapper = TextWrapper(initial_indent=' ' * 8,
                              subsequent_indent=' ' * 8)
        out.extend(wrapper.wrap(words))
        out.append("")

    ctors = jclass.getDeclaredConstructors()
    if ctors:
        exceptions = []
        name = jclass.getSimpleName()
        ctordecl = []
        for ctor in ctors:
            modifiers = ctor.getModifiers()
            if not modifiers & 1:
                continue
            params = ", ".join([str(i.getCanonicalName())
                                for i in ctor.getParameterTypes()])
            ctordecl.append("    * %s(%s)" % (name, params))
            exceptions.extend(ctor.getExceptionTypes())
        if ctordecl:
            out.append("  Constructors:")
            out.extend(ctordecl)
            out.append("")
        if exceptions:
            out.append("  Raises:")
            for exc in set(exceptions):
                out.append("    %s: from java" % exc.getCanonicalName())
            out.append("")

    fields = jclass.getDeclaredFields()
    if fields:
        fielddesc = []
        for field in fields:
            modifiers = field.getModifiers()
            if not modifiers & 1:
                continue
            fieldInfo = []
            if modifiers & 16:
                fieldInfo.append("final")
            if modifiers & 8:
                fieldInfo.append("static")
            if field.isEnumConstant():
                fieldInfo.append("enum constant")
            else:
                fieldInfo.append("field")
            fielddesc.append("    %s (%s): %s" % (field.getName(),
                                                  field.getType().getName(),
                                                  " ".join(fieldInfo)))
        if fielddesc:
            out.append("  Attributes:")
            out.extend(fielddesc)
            out.append("")

    return "\n".join(out)


def _JExtension(name, bases, members):
    if "__jspec__" not in members:
        raise TypeError("Java classes cannot be extended in Python")
    jspec = members['__jspec__']
    Factory = _jpype.JClass('org.jpype.extension.Factory')
    cls = Factory.newClass(name, bases)
    for i in jspec:
        if isinstance(i, _JFieldDecl):
            cls.addField(i.cls, i.name, i.value, i.modifiers)
        elif isinstance(i, type(_JExtension)):
            exceptions = getattr(i, '__jthrows__', None)
            mspec = inspect.getfullargspec(i)
            if i.__name__ == '__init__':
                args = [mspec.annotations[j] for j in mspec.args[1:]]
                cls.addCtor(args, exceptions, i.__jmodifiers__)
            else:
                args = [mspec.annotations[j] for j in mspec.args[1:]]
                ret = mspec.annotations["return"]
                cls.addMethod(i.__name__, ret, args, exceptions, i.__jmodifiers__)
        else:
            raise TypeError("Unknown member %s" % type(i))
    Factory.loadClass(cls)

    raise TypeError("Not implemented")


# Install module hooks
_jpype.JClass = JClass
_jpype.JInterface = JInterface
_jpype._jclassDoc = _jclassDoc
_jpype._jclassPre = _jclassPre
_jpype._jclassPost = _jclassPost
_jpype._JExtension = _JExtension
