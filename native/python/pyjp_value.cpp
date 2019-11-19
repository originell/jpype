/*****************************************************************************
   Copyright 2004-2008 Steve Ménard

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 *****************************************************************************/
#include <pyjp.h>
#include <structmember.h>

static PyMethodDef valueMethods[] = {
	{"_toUnicode", (PyCFunction) (&PyJPValue::toUnicode), METH_NOARGS, ""},
	{NULL},
};

static PyMemberDef valueMembers[] = {
	{"__jvm__", T_OBJECT, offsetof(PyJPValue, m_Context), READONLY},
	{0}
};

PyTypeObject PyJPValue::Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	/* tp_name           */ "_jpype.PyJPValue",
	/* tp_basicsize      */ sizeof (PyJPValue),
	/* tp_itemsize       */ 0,
	/* tp_dealloc        */ (destructor) PyJPValue::__dealloc__,
	/* tp_print          */ 0,
	/* tp_getattr        */ 0,
	/* tp_setattr        */ 0,
	/* tp_compare        */ 0,
	/* tp_repr           */ (reprfunc) PyJPValue::__repr__,
	/* tp_as_number      */ 0,
	/* tp_as_sequence    */ 0,
	/* tp_as_mapping     */ 0,
	/* tp_hash           */ 0,
	/* tp_call           */ 0,
	/* tp_str            */ (reprfunc) PyJPValue::__repr__,
	/* tp_getattro       */ 0,
	/* tp_setattro       */ 0,
	/* tp_as_buffer      */ 0,
	/* tp_flags          */ Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC | Py_TPFLAGS_BASETYPE,
	/* tp_doc            */
	"Wrapper of a java value which holds a class and instance of an object \n"
	"or a primitive.  Anything inheriting from this will be\n"
	"considered a java object wrapper.",
	/* tp_traverse       */ (traverseproc) PyJPValue::traverse,
	/* tp_clear          */ (inquiry) PyJPValue::clear,
	/* tp_richcompare    */ 0,
	/* tp_weaklistoffset */ 0,
	/* tp_iter           */ 0,
	/* tp_iternext       */ 0,
	/* tp_methods        */ valueMethods,
	/* tp_members        */ valueMembers,
	/* tp_getset         */ 0,
	/* tp_base           */ 0,
	/* tp_dict           */ 0,
	/* tp_descr_get      */ 0,
	/* tp_descr_set      */ 0,
	/* tp_dictoffset     */ 0,
	/* tp_init           */ (initproc) PyJPValue::__init__,
	/* tp_alloc          */ 0,
	/* tp_new            */ PyJPValue::__new__
};

// Static methods

void PyJPValue::initType(PyObject *module)
{
	PyType_Ready(&PyJPValue::Type);
	Py_INCREF(&PyJPValue::Type);
	PyModule_AddObject(module, "PyJPValue", (PyObject*) (&PyJPValue::Type));
}

// These are from the internal methods when we already have the jvalue

/**
 * Create a JPValue wrapper with the appropriate type.
 *
 * This method dodges the __new__ method, but does involve
 * __init__.  It is called when returning a Java object back to
 * Python.
 *
 * @param type
 * @param context
 * @param cls
 * @param value
 * @return
 */
JPPyObject PyJPValue::create(PyTypeObject *type, JPContext *context, JPClass *cls, jvalue value)
{
	JPPyObject out;
	if (cls == context->_java_lang_Class)
		out = PyJPClass::alloc(type, context,
			context->getTypeManager()->findClass((jclass) value.l)
			);
	if (dynamic_cast<JPArrayClass*> (cls) != 0)
		out = PyJPArray::alloc(type, context, cls, value);
	else
		out = PyJPValue::alloc(type, context, cls, value);

	// We may have custom __init__ methods on the Python wrappers
	type->tp_init(out.get(), NULL, NULL);
	return out;
}

JPPyObject PyJPValue::alloc(PyTypeObject *wrapper, JPContext *context, JPClass *cls, jvalue value)
{
	JPJavaFrame frame(context);
	JP_TRACE_IN_C("PyJPValue::alloc");

	PyJPValue *self = (PyJPValue*) ((PyTypeObject*) wrapper)->tp_alloc(wrapper, 0);
	JP_PY_CHECK();

	// If it is not a primitive we need to reference it
	if (!cls->isPrimitive())
	{
		value.l = frame.NewGlobalRef(value.l);
		JP_TRACE("type", cls->getCanonicalName());
	}

	// New value instance
	self->m_Value = JPValue(cls, value);
	self->m_Cache = NULL;
	self->m_Context = (PyJPContext*) (context->getHost());
	Py_INCREF(self->m_Context);
	return JPPyObject(JPPyRef::_claim, (PyObject*) self);
	JP_TRACE_OUT_C;
}

PyObject *PyJPValue::__new__(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	PyJPValue *self = (PyJPValue*) type->tp_alloc(type, 0);
	jvalue v;
	self->m_Value = JPValue(NULL, v);
	self->m_Cache = NULL;
	self->m_Context = NULL;
	return (PyObject*) self;
}

// Replacement for convertToJava.
//   (class, object)

int PyJPValue::__init__(PyJPValue *self, PyObject *pyargs, PyObject *kwargs)
{
	JP_TRACE_IN_C("PyJPValue::__init__", self);
	JP_TRACE("init", self);
	try
	{
		// Check if we are already initialized.
		if (self->m_Value.getClass() != 0)
			return 0;

		// Get the Java class from the type.
		PyObject *obj = PyObject_GetAttrString((PyObject*) Py_TYPE(self), "__javaclass__");
		JP_PY_CHECK();
		if (!PyObject_IsInstance(obj, (PyObject*) & PyJPValue::Type))
		{
			Py_DECREF(obj);
			JP_RAISE_TYPE_ERROR("__javaclass__ type is incorrect");
		}
		JPClass *cls = ((PyJPClass*) obj)->m_Class;
		Py_DECREF(obj);

		if (dynamic_cast<JPArrayClass*> (cls) != NULL)
		{
			int sz;
			if (!PyArg_ParseTuple(pyargs, "i", &sz))
			{
				return NULL;
			}
			self->m_Value = ((JPArrayClass*) cls)->newInstance(sz);
			return 0;
		}

		JPPyObjectVector args(pyargs);
		// DEBUG
		for (size_t i = 0; i < args.size(); ++i)
		{
			ASSERT_NOT_NULL(args[i]);
		}
		self->m_Value =  cls->newInstance(args);
		return 0;

	}
	PY_STANDARD_CATCH(-1);
	JP_TRACE_OUT_C;
}

void PyJPValue::__dealloc__(PyJPValue *self)
{
	// We have to handle partially constructed objects that result from
	// fails in __init__, thus lots of inits
	JP_TRACE_IN_C("PyJPValue::__dealloc__", self);
	JPValue& value = self->m_Value;
	JPClass *cls = value.getClass();
	if (self->m_Context != NULL && cls != NULL)
	{
		JPContext *context = self->m_Context->m_Context;
		if (context->isRunning() && !cls->isPrimitive())
		{
			// If the JVM has shutdown then we don't need to free the resource
			// FIXME there is a problem with initializing the system twice.
			// Once we shut down the cls type goes away so this will fail.  If
			// we then reinitialize we will access this bad resource.  Not sure
			// of an easy solution.
			JP_TRACE("Dereference object", cls->getCanonicalName());
			context->ReleaseGlobalRef(value.getValue().l);
		}
	}

	PyTypeObject *type = Py_TYPE(self);
	PyObject_GC_UnTrack(self);
	type->tp_clear((PyObject*) self);
	type->tp_free((PyObject*) self);
	JP_TRACE_OUT_C;
}

int PyJPValue::traverse(PyJPValue *self, visitproc visit, void *arg)
{
	JP_TRACE_IN_C("PyJPValue::traverse", self);
	Py_VISIT(self->m_Cache);
	Py_VISIT(self->m_Context);
	return 0;
	JP_TRACE_OUT_C;
}

int PyJPValue::clear(PyJPValue *self)
{
	JP_TRACE_IN_C("PyJPValue::clear", self);
	Py_CLEAR(self->m_Cache);
	Py_CLEAR(self->m_Context);
	return 0;
	JP_TRACE_OUT_C;
}

PyObject *PyJPValue::__repr__(PyJPValue *self)
{
	JP_TRACE_IN_C("PyJPValue::__repr__", self);
	try
	{
		if (self->m_Context == NULL)
			JP_RAISE_RUNTIME_ERROR("Null context");

		JPContext * context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		stringstream sout;
		sout << "<java value " << self->m_Value.getClass()->toString();

		// FIXME Remove these extra diagnostic values
		if (dynamic_cast<JPPrimitiveType*> (self->m_Value.getClass()) != NULL)
			sout << endl << "  value = primitive";
		else
		{
			jobject jo = self->m_Value.getJavaObject();
			sout << "  value = " << jo << " " << context->toString(jo);
		}

		sout << ">";
		return JPPyString::fromStringUTF8(sout.str()).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

void ensureCache(PyJPValue *self)
{
	if (self->m_Cache != NULL)
		return;
	self->m_Cache = PyDict_New();
}

/* *This is the way to convert an object into a python string. */
PyObject *PyJPValue::toString(PyJPValue *self)
{
	JP_TRACE_IN_C("PyJPValue::toString", self);
	try
	{
		if (self->m_Context == NULL)
			JP_RAISE_RUNTIME_ERROR("Null context");

		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPClass *cls = self->m_Value.getClass();
		JPJavaFrame frame(context);
		if (cls == context->_java_lang_String)
		{
			// Java strings are immutable so we will cache them.
			ensureCache(self);
			PyObject *out;
			out = PyDict_GetItemString(self->m_Cache, "str"); // Borrowed reference
			if (out == NULL)
			{
				jstring str = (jstring) self->m_Value.getValue().l;
				if (str == NULL)
					JP_RAISE_VALUE_ERROR("null string");
				string cstring = context->toStringUTF8(str);
				PyDict_SetItemString(self->m_Cache, "str", out = JPPyString::fromStringUTF8(cstring).keep());
			}
			Py_INCREF(out);
			return out;

		}
		if (cls->isPrimitive())
			JP_RAISE_VALUE_ERROR("toString requires a java object");
		if (cls == NULL)
			JP_RAISE_VALUE_ERROR("toString called with null class");

		// In general toString is not immutable, so we won't cache it.
		return JPPyString::fromStringUTF8(context->toString(self->m_Value.getValue().l)).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

// FIXME Gut Python 2 stuff

/* *This is the way to convert an object into a python string. */
PyObject *PyJPValue::toUnicode(PyJPValue *self)
{
	JP_TRACE_IN_C("PyJPValue::toUnicode", self);
	try
	{
		if (self->m_Context == NULL)
			JP_RAISE_RUNTIME_ERROR("Null context");

		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPClass *cls = self->m_Value.getClass();
		JPJavaFrame frame(context);
		if (cls == context->_java_lang_String)
		{
			// Java strings are immutable so we will cache them.
			ensureCache(self);
			PyObject *out;
			out = PyDict_GetItemString(self->m_Cache, "unicode"); // Borrowed reference
			if (out == NULL)
			{
				jstring str = (jstring) self->m_Value.getValue().l;
				if (str == NULL)
					JP_RAISE_VALUE_ERROR("null string");
				string cstring = context->toStringUTF8(str);
				PyDict_SetItemString(self->m_Cache, "unicode", out = JPPyString::fromStringUTF8(cstring, true).keep());
			}
			Py_INCREF(out);
			return out;

		}
		if (cls->isPrimitive())
			JP_RAISE_VALUE_ERROR("toUnicode requires a java object");
		if (cls == NULL)
			JP_RAISE_VALUE_ERROR("toUnicode called with null class");

		// In general toString is not immutable, so we won't cache it.
		return JPPyString::fromStringUTF8(context->toString(self->m_Value.getValue().l), true).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}
