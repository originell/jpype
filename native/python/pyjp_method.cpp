/*****************************************************************************
   Copyright 2004 Steve Ménard

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

PyObject* PyJPMethod_Type = NULL;
PyObject* PyJPMethod_new(PyTypeObject *self, PyObject *args, PyObject *kwargs);
void      PyJPMethod_dealloc(PyJPMethod *o);
int       PyJPMethod_traverse(PyJPMethod *self, visitproc visit, void *arg);
int       PyJPMethod_clear(PyJPMethod *self);
PyObject* PyJPMethod_get(PyJPMethod *self, PyObject *obj, PyObject *type);
PyObject* PyJPMethod_str(PyJPMethod *o);
PyObject* PyJPMethod_repr(PyJPMethod *self);
PyObject* PyJPMethod_call(PyJPMethod* self, PyObject* args, PyObject* kwargs);
PyObject* PyJPMethod_getSelf(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getName(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getQualName(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getDoc(PyJPMethod *self, void *context);
int       PyJPMethod_setDoc(PyJPMethod *self, PyObject* obj, void *context);
PyObject* PyJPMethod_getAnnotations(PyJPMethod *self, void *context);
int       PyJPMethod_setAnnotations(PyJPMethod *self, PyObject* obj, void *context);
PyObject* PyJPMethod_getNone(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getCodeAttr(PyJPMethod *self, void *context, const char* attr);
PyObject* PyJPMethod_getCode(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getClosure(PyJPMethod *self, void *context);
PyObject* PyJPMethod_getGlobals(PyJPMethod *self, void *context);
PyObject* PyJPMethod_isBeanMutator(PyJPMethod* self, PyObject* arg);
PyObject* PyJPMethod_isBeanAccessor(PyJPMethod* self, PyObject* arg);
PyObject* PyJPMethod_matchReport(PyJPMethod* self, PyObject* arg);
PyObject* PyJPMethod_dump(PyJPMethod* self, PyObject* arg);

static PyMethodDef methodMethods[] = {
	{"_isBeanAccessor", (PyCFunction) (&PyJPMethod_isBeanAccessor), METH_NOARGS, ""},
	{"_isBeanMutator", (PyCFunction) (&PyJPMethod_isBeanMutator), METH_NOARGS, ""},
	{"_matchReport", (PyCFunction) (&PyJPMethod_matchReport), METH_VARARGS, ""},
	{"_dump", (PyCFunction) (&PyJPMethod_dump), METH_NOARGS, ""},
	{NULL},
};

struct PyGetSetDef methodGetSet[] = {
	{"__self__", (getter) (&PyJPMethod_getSelf), NULL, NULL, NULL},
	{"__name__", (getter) (&PyJPMethod_getName), NULL, NULL, NULL},
	{"__doc__", (getter) (&PyJPMethod_getDoc), (setter) (&PyJPMethod_setDoc), NULL, NULL},
	{"__annotations__", (getter) (&PyJPMethod_getAnnotations), (setter) (&PyJPMethod_setAnnotations), NULL, NULL},
#if PY_MAJOR_VERSION >= 3
	{"__closure__", (getter) (&PyJPMethod_getClosure), NULL, NULL, NULL},
	{"__code__", (getter) (&PyJPMethod_getCode), NULL, NULL, NULL},
	{"__defaults__", (getter) (&PyJPMethod_getNone), NULL, NULL, NULL},
	{"__kwdefaults__", (getter) (&PyJPMethod_getNone), NULL, NULL, NULL},
	{"__globals__", (getter) (&PyJPMethod_getGlobals), NULL, NULL, NULL},
	{"__qualname__", (getter) (&PyJPMethod_getQualName), NULL, NULL, NULL},
#else
	{"func_closure", (getter) (&PyJPMethod_getClosure), NULL, NULL, NULL},
	{"func_code", (getter) (&PyJPMethod_getCode), NULL, NULL, NULL},
	{"func_defaults", (getter) (&PyJPMethod_getNone), NULL, NULL, NULL},
	{"func_doc", (getter) (&PyJPMethod_getDoc), (setter) (&PyJPMethod::setDoc), NULL, NULL},
	{"func_globals", (getter) (&PyJPMethod_getGlobals), NULL, NULL, NULL},
	{"func_name", (getter) (&PyJPMethod_getName), NULL, NULL, NULL},
#endif
	{NULL},
};

static PyType_Slot methodSlots[] = {
	{Py_tp_dealloc        , (destructor) PyJPMethod_dealloc},
	{Py_tp_repr           , (reprfunc) PyJPMethod_repr},
	{Py_tp_call           , (ternaryfunc) PyJPMethod_call},
	{Py_tp_str            , (reprfunc) PyJPMethod_str},
	{Py_tp_traverse       , (traverseproc) PyJPMethod_traverse},
	{Py_tp_clear          , (inquiry) PyJPMethod_clear},
	{Py_tp_methods        , methodMethods},
	{Py_tp_getset         , methodGetSet},
	{Py_tp_descr_get      , (descrgetfunc) PyJPMethod_get},
	{Py_tp_new            , PyJPMethod_new},
	{0}
};

static PyType_Spec methodSpec = {
	"_jpype.PyJPMethod",
	sizeof (PyJPMethod),
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	methodSlots
};

// Static methods

void PyJPMethod::initType(PyObject *module)
{
	// We inherit from PyFunction_Type just so we are an instance
	// for purposes of inspect and tab completion tools.  But
	// we will just ignore their memory layout as we have our own.
	PyObject *bases = PyTuple_Pack(1, &PyFunction_Type);
	PyModule_AddObject(module, "PyJPMethod",
			PyJPMethod_Type = PyType_FromSpecWithBases(&methodSpec, bases));
	Py_DECREF(bases);
}

JPPyObject PyJPMethod::alloc(JPMethodDispatch *m, PyObject *instance)
{
	JP_TRACE_IN_C("PyJPMethod::alloc");
	PyJPMethod *self = (PyJPMethod*) ((PyTypeObject*) PyJPMethod_Type)->tp_alloc((PyTypeObject*) PyJPMethod_Type, 0);
	JP_PY_CHECK();
	self->m_Method = m;
	self->m_Instance = instance;
	if (instance != NULL)
	{
		JP_TRACE_PY("method alloc (inc)", instance);
		Py_INCREF(instance);
	}
	self->m_Doc = NULL;
	self->m_Annotations = NULL;
	self->m_CodeRep = NULL;
	self->m_Context = (PyJPContext*) (m->getContext()->getHost());
	Py_INCREF(self->m_Context);
	JP_TRACE("self", self);
	return JPPyObject(JPPyRef::_claim, (PyObject*) self);
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	PyJPMethod *self = (PyJPMethod*) type->tp_alloc(type, 0);
	self->m_Method = NULL;
	self->m_Instance = NULL;
	self->m_Context = NULL;
	self->m_Doc = NULL;
	self->m_Annotations = NULL;
	self->m_CodeRep = NULL;
	return (PyObject*) self;
}

void PyJPMethod_dealloc(PyJPMethod *self)
{
	JP_TRACE_IN_C("PyJPMethod::__dealloc__", self);
	PyObject_GC_UnTrack(self);
	PyJPMethod_clear(self);
	Py_TYPE(self)->tp_free(self);
	JP_TRACE_OUT_C;
}

int PyJPMethod_traverse(PyJPMethod *self, visitproc visit, void *arg)
{
	JP_TRACE_IN_C("PyJPMethod::traverse", self);
	Py_VISIT(self->m_Instance);
	Py_VISIT(self->m_Context);
	Py_VISIT(self->m_Doc);
	Py_VISIT(self->m_Annotations);
	Py_VISIT(self->m_CodeRep);
	return 0;
	JP_TRACE_OUT_C;
}

int PyJPMethod_clear(PyJPMethod *self)
{
	JP_TRACE_IN_C("PyJPMethod::clear", self);
	Py_CLEAR(self->m_Instance);
	Py_CLEAR(self->m_Context);
	Py_CLEAR(self->m_Doc);
	Py_CLEAR(self->m_Annotations);
	Py_CLEAR(self->m_CodeRep);
	return 0;
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_get(PyJPMethod *self, PyObject *obj, PyObject *type)
{
	JP_TRACE_IN_C("PyJPMethod::__get__", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		if (obj == NULL)
		{
			Py_INCREF((PyObject*) self);
			JP_TRACE_PY("method get (inc)", (PyObject*) self);
			return (PyObject*) self;
		}
		return PyJPMethod::alloc(self->m_Method, obj).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_call(PyJPMethod *self, PyObject *args, PyObject *kwargs)
{
	JP_TRACE_IN_C("PyJPMethod::__call__", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		JP_TRACE(self->m_Method->getName());
		if (self->m_Instance == NULL)
		{
			JPPyObjectVector vargs(args);
			return self->m_Method->invoke(vargs, false).keep();
		} else
		{
			JPPyObjectVector vargs(self->m_Instance, args);
			return self->m_Method->invoke(vargs, true).keep();
		}
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_str(PyJPMethod *self)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		stringstream sout;
		sout << self->m_Method->getClass()->getCanonicalName() << "." << self->m_Method->getName();
		return JPPyString::fromStringUTF8(sout.str()).keep();
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject *PyJPMethod_repr(PyJPMethod *self)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		stringstream ss;
		if (self->m_Instance == NULL)
			ss << "<java method `";
		else
			ss << "<java bound method `";
		ss << self->m_Method->getName() << "' of '" <<
				self->m_Method->getClass()->getCanonicalName() << "'>";
		return JPPyString::fromStringUTF8(ss.str()).keep();
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject *PyJPMethod_getSelf(PyJPMethod *self, void *context)
{
	JP_TRACE_IN("PyJPMethod::getSelf");
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		if (self->m_Instance == NULL)
			Py_RETURN_NONE;
		Py_INCREF(self->m_Instance);
		return self->m_Instance;
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT;
}

PyObject *PyJPMethod_getNone(PyJPMethod *self, void *context)
{
	Py_RETURN_NONE;
}

PyObject *PyJPMethod_getName(PyJPMethod *self, void *context)
{
	JP_TRACE_IN_C("PyJPMethod::getName", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		return JPPyString::fromStringUTF8(self->m_Method->getName(), false).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_getQualName(PyJPMethod *self, void *context)
{
	JP_TRACE_IN_C("PyJPMethod::getQualName", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		stringstream str;
		str << self->m_Method->getClass()->getCanonicalName() << '.'
				<< self->m_Method->getName();
		return JPPyString::fromStringUTF8(str.str(), false).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_getDoc(PyJPMethod *self, void *context)
{
	JP_TRACE_IN_C("PyJPMethod::getDoc", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		if (self->m_Doc)
		{
			Py_INCREF(self->m_Doc);
			return self->m_Doc;
		}

		// Get the resource
		JPPyObject getMethodDoc(JPPyRef::_claim, PyObject_GetAttrString(PyJPModule::module, "GetMethodDoc"));
		if (getMethodDoc.isNull())
		{
			JP_TRACE("Resource not set.");
			Py_RETURN_NONE;
		}

		// Convert the overloads
		JP_TRACE("Convert overloads");
		const JPMethodList& overloads = self->m_Method->getMethodOverloads();
		JPPyTuple ov(JPPyTuple::newTuple(overloads.size()));
		int i = 0;
		JPClass *methodClass = context->getTypeManager()->findClassByName("java.lang.reflect.Method");
		for (JPMethodList::const_iterator iter = overloads.begin(); iter != overloads.end(); ++iter)
		{
			JP_TRACE("Set overload", i);
			jvalue v;
			v.l = (*iter)->getJava();
			JPPyObject obj(JPPythonEnv::newJavaObject(JPValue(methodClass, v)));
			ov.setItem(i++, obj.get());
		}

		// Pack the arguments

		JP_TRACE("Pack arguments");
		jvalue v;
		v.l = (jobject) self->m_Method->getClass()->getJavaClass();
		JPPyObject obj(JPPythonEnv::newJavaObject(JPValue(context->_java_lang_Class, v)));

		JPPyTuple args(JPPyTuple::newTuple(3));
		args.setItem(0, (PyObject*) self);
		args.setItem(1, obj.get());
		args.setItem(2, ov.get());
		JP_TRACE("Call Python");
		self->m_Doc = getMethodDoc.call(args.get(), NULL).keep();

		Py_XINCREF(self->m_Doc);
		return self->m_Doc;
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT;
}

int PyJPMethod_setDoc(PyJPMethod *self, PyObject *obj, void *context)
{
	JP_TRACE_IN("PyJPMethod::getDoc");
	Py_CLEAR(self->m_Doc);
	self->m_Doc = obj;
	Py_XINCREF(self->m_Doc);
	return 0;
	JP_TRACE_OUT;
}

PyObject *PyJPMethod_getAnnotations(PyJPMethod *self, void *context)
{
	JP_TRACE_IN_C("PyJPMethod::getAnnotations", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		if (self->m_Annotations)
		{
			Py_INCREF(self->m_Annotations);
			return self->m_Annotations;
		}

		// Get the resource
		JPPyObject getAnnotations(JPPyRef::_claim,
				PyObject_GetAttrString(PyJPModule::module, "GetMethodAnnotations"));
		if (getAnnotations.isNull())
		{
			JP_TRACE("Resource not set.");
			Py_RETURN_NONE;
		}

		// Convert the overloads
		JP_TRACE("Convert overloads");
		const JPMethodList& overloads = self->m_Method->getMethodOverloads();
		JPPyTuple ov(JPPyTuple::newTuple(overloads.size()));
		int i = 0;
		JPClass *methodClass = context->getTypeManager()->findClassByName("java.lang.reflect.Method");
		for (JPMethodList::const_iterator iter = overloads.begin(); iter != overloads.end(); ++iter)
		{
			JP_TRACE("Set overload", i);
			jvalue v;
			v.l = (*iter)->getJava();
			JPPyObject obj(JPPythonEnv::newJavaObject(JPValue(methodClass, v)));
			ov.setItem(i++, obj.get());
		}

		// Pack the arguments

		JP_TRACE("Pack arguments");
		jvalue v;
		v.l = (jobject) self->m_Method->getClass()->getJavaClass();
		JPPyObject obj(JPPythonEnv::newJavaObject(JPValue(context->_java_lang_Class, v)));

		JPPyTuple args(JPPyTuple::newTuple(3));
		args.setItem(0, (PyObject*) self);
		args.setItem(1, obj.get());
		args.setItem(2, ov.get());
		JP_TRACE("Call Python");
		self->m_Annotations = getAnnotations.call(args.get(), NULL).keep();

		Py_XINCREF(self->m_Annotations);
		return self->m_Annotations;
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT;
}

int PyJPMethod_setAnnotations(PyJPMethod *self, PyObject *obj, void *context)
{
	JP_TRACE_IN_C("PyJPMethod::getAnnotations", self);
	Py_CLEAR(self->m_Annotations);
	self->m_Annotations = obj;
	Py_XINCREF(self->m_Annotations);
	return 0;
	JP_TRACE_OUT_C;
}

PyObject *PyJPMethod_getCodeAttr(PyJPMethod *self, void *context, const char *attr)
{
	JP_TRACE_IN_C("PyJPMethod::getCode", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		if (self->m_CodeRep == NULL)
		{
			JPPyObject getCode(JPPyRef::_claim,
					PyObject_GetAttrString(PyJPModule::module, "GetMethodCode"));
			if (getCode.isNull())
			{
				JP_TRACE("Resource not set.");
				Py_RETURN_NONE;
			}

			// Pack the arguments
			JP_TRACE("Pack arguments");
			JPPyTuple args(JPPyTuple::newTuple(1));
			args.setItem(0, (PyObject*) self);
			JP_TRACE("Call Python");
			self->m_CodeRep = getCode.call(args.get(), NULL).keep();

			Py_XINCREF(self->m_CodeRep);
		}
		return PyObject_GetAttrString(self->m_CodeRep, attr);
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT;
}

PyObject *PyJPMethod_getCode(PyJPMethod *self, void *context)
{
#if PY_MAJOR_VERSION >= 3
	return PyJPMethod_getCodeAttr(self, context, "__code__");
#else
	return PyJPMethod_getCodeAttr(self, context, "func_code");
#endif
}

PyObject *PyJPMethod_getClosure(PyJPMethod *self, void *context)
{
#if PY_MAJOR_VERSION >= 3
	return PyJPMethod_getCodeAttr(self, context, "__closure__");
#else
	return PyJPMethod_getCodeAttr(self, context, "func_closure");
#endif
}

PyObject *PyJPMethod_getGlobals(PyJPMethod *self, void *context)
{
#if PY_MAJOR_VERSION >= 3
	return PyJPMethod_getCodeAttr(self, context, "__globals__");
#else
	return PyJPMethod_getCodeAttr(self, context, "func_globals");
#endif
}

PyObject *PyJPMethod_isBeanAccessor(PyJPMethod *self, PyObject *arg)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		return PyBool_FromLong(self->m_Method->isBeanAccessor());
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject *PyJPMethod_isBeanMutator(PyJPMethod *self, PyObject *arg)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		return PyBool_FromLong(self->m_Method->isBeanMutator());
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject *PyJPMethod_matchReport(PyJPMethod *self, PyObject *args)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		JPPyObjectVector vargs(args);
		string report = self->m_Method->matchReport(vargs);
		return JPPyString::fromStringUTF8(report).keep();
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject *PyJPMethod_dump(PyJPMethod *self, PyObject *args)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		string report = self->m_Method->dump();
		return JPPyString::fromStringUTF8(report).keep();
	}
	PY_STANDARD_CATCH(NULL);
}
