This information is a draft for the user guide.

Java Extensions
===============

JPype can extend Java classes from within Python.  Java Extensions are defining
using Python class definitions but they have a number of restrictions.  Here is
a summary of the key differences.

- The JVM must be running to define a Java extension type.

- Java extensions use decorators to mark up Python elements for use by Java.
  Annotations include ``@JPublic``, ``@JProtected``, ``@JPrivate``,
  ``@JOverride`` and ``@JThrows``.  The concepts of ``final`` and ``static``
  are not currently supported.

- ``@JPublic``, ``@JProtected``, or ``@JPrivate`` are considered accessors
  decorators.  They are used without the ``@`` when declaring fields.

- Java extensions must have all Java base types.

- Java base types must not be final.

- Java bases can have at most one concrete or abstract base type, but they can
  have multiple interfaces.

- If no concrete type is included then JObject will be added as the concrete
  base.

- All Java extensions are private classes without a package.  Thus, they cannot
  be annotated with ``@JPublic``, ``@JProtected``, or ``@JPrivate``.

- Java extensions will be Java classes and therefore must have their strongly
  types methods and fields.
  
- All methods must be annotated with an accessor decorator.

- Methods can be annotated with ``@JOverride``.  Methods with ``@JOverride``
  must have a signature that matches a method in the parent.

- Java methods must take ``this`` as the first argument as opposed to the usual
  Python convention ``self``.  ``this`` is a Java handle with privilaged access
  to private methods and fields.

- Java methods must have a return annotation.  The return type can be None if
  the method returns ``void``.  The return type must be a Java class with the 
  exception of None.

- Java methods must have all parameters specified with the exception of the
  ``this``.  ``this`` should not have a type specification as the type
  of this will be changed to the resulting extension class.

- Java methods may not accept keyword arguments.

- Java methods may not have default values.

- Python decorators ``@staticmethod`` and ``@classmethod`` are not currently supported.

- Java methods can be overloaded so long as the signatures are not conflicting.

- Variadic arguements are not currently supported.

- Java classes are closed so all fields must be defined in advance before being
  used.  Use ``JPublic``, ``JProtected``, ``JPrivate`` to declare those slots
  in advance.  

- Java fields are specified using an accessor with arguments of the type
  followed by a list of arguments with variable names with the default value.
  The default value must be ``None`` with the exception of Java primitives.

- Field names must not be Java keywords.

- The constructor is specified with ``__init__``.  Overloading of constructors
  is allowed.

- The first call to the Java class must be to the base class initializer.
  Accessing any other field or method will produce a ``ValueError``.


Mechanism
---------

WHen a Java class is used as a base type, control is transfered a class builder
by the meta class JClassMeta.  The meta class probes all elements in the Python
prototype for decorated elements.  Those decoarated elements are turned into a
class description which is passed to Java.  Java ASM is then used to construct
the requested classes which is loaded dynamically.  The resulting class is then
passed back to Python which calles JClass to produce a Python class wrapper.

The majority of the errors are should be caught when building the class
description.  Errors detected at this stage will produce the using
``TypeError`` exceptions.  However, in some cases errors in the class
description may result in errors in the class generation or loading phase.
The exceptions from these staget currently produce Java exceptions.

