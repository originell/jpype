/*****************************************************************************
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   See NOTICE file for details.
 *****************************************************************************/
#include "jpype.h"
#include "pyjp.h"
#include "jp_boxedtype.h"

static bool isNull(PyObject *self)
{
	JPValue *javaSlot = PyJPValue_getJavaSlot(self);
	if (javaSlot != NULL
			&& !javaSlot->getClass()->isPrimitive()
			&& javaSlot->getValue().l == 0)
		return true;
	return false;
}

#ifdef __cplusplus
extern "C"
{
#endif

static PyObject *PyJPNumber_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	JP_PY_TRY("PyJPNumber_new", type);
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	JPClass *cls = (JPClass*) PyJPClass_getJPClass((PyObject*) type);
	if (cls == NULL)
		JP_RAISE(PyExc_TypeError, "Class type incorrect");

	jvalue val;
	// One argument tries Java conversion first
	if (PyTuple_Size(args) == 1)
	{
		PyObject *arg = PyTuple_GetItem(args, 0);
		JPMatch match(&frame, arg);
		cls->findJavaConversion(match);
		if (match.type >= JPMatch::_implicit)
		{
			// Disable OverrangeError
			match.type = JPMatch::_exact;
			val = match.convert();
			PyObject *obj = cls->convertToPythonObject(frame, val, true).keep();
			return obj;
		}
	}

	if (PyObject_IsSubclass((PyObject*) type, (PyObject*) & PyLong_Type))
	{
		JPPyObject self = JPPyObject::call(PyLong_Type.tp_new(&PyLong_Type, args, kwargs));
		JPMatch match(&frame, self.get());
		cls->findJavaConversion(match);
		match.type = JPMatch::_exact;
		val = match.convert();
		return cls->convertToPythonObject(frame, val, true).keep();
	} else if (PyObject_IsSubclass((PyObject*) type, (PyObject*) & PyFloat_Type))
	{
		JPPyObject self = JPPyObject::call(PyFloat_Type.tp_new(&PyFloat_Type, args, kwargs));
		JPMatch match(&frame, self.get());
		cls->findJavaConversion(match);
		match.type = JPMatch::_exact;
		val = match.convert();
		return cls->convertToPythonObject(frame, val, true).keep();
	} else
	{
		PyErr_Format(PyExc_TypeError, "Type '%s' is not a number class", type->tp_name);
		return NULL;
	}
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberLong_int(PyObject *self)
{
	JP_PY_TRY("PyJPNumberLong_int");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (!isNull(self))
		return PyLong_Type.tp_as_number->nb_int(self);
	PyErr_SetString(PyExc_TypeError, "cast of null pointer would return non-int");
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberLong_float(PyObject *self)
{
	JP_PY_TRY("PyJPNumberLong_float");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (!isNull(self))
		return PyLong_Type.tp_as_number->nb_float(self);
	PyErr_SetString(PyExc_TypeError, "cast of null pointer would return non-float");
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberFloat_int(PyObject *self)
{
	JP_PY_TRY("PyJPNumberFloat_int");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (!isNull(self))
		return PyFloat_Type.tp_as_number->nb_int(self);
	PyErr_SetString(PyExc_TypeError, "cast of null pointer would return non-int");
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberFloat_float(PyObject *self)
{
	JP_PY_TRY("PyJPNumberFloat_float");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (!isNull(self))
		return PyFloat_Type.tp_as_number->nb_float(self);
	PyErr_SetString(PyExc_TypeError, "cast of null pointer would return non-float");
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberLong_str(PyObject *self)
{
	JP_PY_TRY("PyJPNumberLong_str");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
		return Py_TYPE(Py_None)->tp_str(Py_None);
	return PyLong_Type.tp_str(self);
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberFloat_str(PyObject *self)
{
	JP_PY_TRY("PyJPNumberFloat_str");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
		return Py_TYPE(Py_None)->tp_str(Py_None);
	return PyFloat_Type.tp_str(self);
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberLong_repr(PyObject *self)
{
	JP_PY_TRY("PyJPNumberLong_repr");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
		return Py_TYPE(Py_None)->tp_str(Py_None);
	return PyLong_Type.tp_repr(self);
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberFloat_repr(PyObject *self)
{
	JP_PY_TRY("PyJPNumberFloat_repr");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
		return Py_TYPE(Py_None)->tp_str(Py_None);
	return PyFloat_Type.tp_repr(self);
	JP_PY_CATCH(NULL);
}

static const char* op_names[] = {
	"<", "<=", "==", "!=", ">", ">="
};

static PyObject *PyJPNumberLong_compare(PyObject *self, PyObject *other, int op)
{
	JP_PY_TRY("PyJPNumberLong_compare");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
	{
		if (op == Py_EQ)
			return PyBool_FromLong(other == Py_None);
		if (op == Py_NE)
			return PyBool_FromLong(other != Py_None);
		PyErr_Format(PyExc_TypeError, "'%s' not supported with null pointer", op_names[op]);
		JP_RAISE_PYTHON();
	}
	if (!PyNumber_Check(other))
	{
		PyObject *out = Py_NotImplemented;
		Py_INCREF(out);
		return out;
	}
	return PyLong_Type.tp_richcompare(self, other, op);
	JP_PY_CATCH(NULL);
}

static PyObject *PyJPNumberFloat_compare(PyObject *self, PyObject *other, int op)
{
	JP_PY_TRY("PyJPNumberFloat_compare");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	if (isNull(self))
	{
		if (op == Py_EQ)
			return PyBool_FromLong(other == Py_None);
		if (op == Py_NE)
			return PyBool_FromLong(other != Py_None);
		PyErr_Format(PyExc_TypeError, "'%s' not supported with null pointer", op_names[op]);
		JP_RAISE_PYTHON();
	}
	if (!PyNumber_Check(other)) // || Py_TYPE(other) == (PyTypeObject*) _JChar)
	{
		PyObject *out = Py_NotImplemented;
		Py_INCREF(out);
		return out;
	}
	return PyFloat_Type.tp_richcompare(self, other, op);
	JP_PY_CATCH(NULL);
}

static Py_hash_t PyJPNumberLong_hash(PyObject *self)
{
	JP_PY_TRY("PyJPNumberLong_hash");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	JPValue *javaSlot = PyJPValue_getJavaSlot(self);
	if (javaSlot == NULL)
		return Py_TYPE(Py_None)->tp_hash(Py_None);
	if (!javaSlot->getClass()->isPrimitive())
	{
		jobject o = javaSlot->getJavaObject();
		if (o == NULL)
			return Py_TYPE(Py_None)->tp_hash(Py_None);
	}
	return PyLong_Type.tp_hash(self);
	JP_PY_CATCH(0);
}

static Py_hash_t PyJPNumberFloat_hash(PyObject *self)
{
	JP_PY_TRY("PyJPNumberFloat_hash");
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	JPValue *javaSlot = PyJPValue_getJavaSlot(self);
	if (javaSlot == NULL)
		return Py_TYPE(Py_None)->tp_hash(Py_None);
	if (!javaSlot->getClass()->isPrimitive())
	{
		jobject o = javaSlot->getJavaObject();
		if (o == NULL)
			return Py_TYPE(Py_None)->tp_hash(Py_None);
	}
	return PyFloat_Type.tp_hash(self);
	JP_PY_CATCH(0);
}

static PyObject *PyJPBoolean_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
	JP_PY_TRY("PyJPBoolean_new", type);
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	JPPyObject self;
	if (PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Requires one argument");
		return NULL;
	}
	int i = PyObject_IsTrue(PyTuple_GetItem(args, 0));
	PyObject *args2 = PyTuple_Pack(1, PyLong_FromLong(i));
	self = JPPyObject::call(PyLong_Type.tp_new(type, args2, kwargs));
	Py_DECREF(args2);
	JPClass *cls = PyJPClass_getJPClass((PyObject*) type);
	if (cls == NULL)
	{
		PyErr_SetString(PyExc_TypeError, "Class type incorrect");
		return NULL;
	}
	JPMatch match(&frame, self.get());
	cls->findJavaConversion(match);
	jvalue val = match.convert();
	PyJPValue_assignJavaSlot(frame, self.get(), JPValue(cls, val));
	JP_TRACE("new", self.get());
	return self.keep();
	JP_PY_CATCH(NULL);
}

static PyObject* PyJPBoolean_str(PyObject* self)
{
	JP_PY_TRY("PyJPBoolean_str", self);
	if (isNull(self))
		return Py_TYPE(Py_None)->tp_str(Py_None);
	if (PyLong_AsLong(self) == 0)
		return Py_TYPE(Py_False)->tp_str(Py_False);
	return Py_TYPE(Py_True)->tp_str(Py_True);
	JP_PY_CATCH(NULL);
}

static PyType_Slot numberLongSlots[] = {
	{Py_tp_new,      (void*) &PyJPNumber_new},
	{Py_tp_getattro, (void*) &PyJPValue_getattro},
	{Py_tp_setattro, (void*) &PyJPValue_setattro},
	{Py_nb_int,      (void*) &PyJPNumberLong_int},
	{Py_nb_float,    (void*) &PyJPNumberLong_float},
	{Py_tp_str,      (void*) &PyJPNumberLong_str},
	{Py_tp_repr,     (void*) &PyJPNumberLong_repr},
	{Py_tp_hash,     (void*) &PyJPNumberLong_hash},
	{Py_tp_richcompare, (void*) &PyJPNumberLong_compare},
	{0}
};

PyTypeObject *PyJPNumberLong_Type = NULL;
PyType_Spec numberLongSpec = {
	"_jpype._JNumberLong",
	0,
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	numberLongSlots
};

static PyType_Slot numberFloatSlots[] = {
	{Py_tp_new,      (void*) &PyJPNumber_new},
	{Py_tp_getattro, (void*) &PyJPValue_getattro},
	{Py_tp_setattro, (void*) &PyJPValue_setattro},
	{Py_nb_int,      (void*) &PyJPNumberFloat_int},
	{Py_nb_float,    (void*) &PyJPNumberFloat_float},
	{Py_tp_str,      (void*) &PyJPNumberFloat_str},
	{Py_tp_repr,     (void*) &PyJPNumberFloat_repr},
	{Py_tp_hash,     (void*) &PyJPNumberFloat_hash},
	{Py_tp_richcompare, (void*) &PyJPNumberFloat_compare},
	{0}
};

PyTypeObject *PyJPNumberFloat_Type = NULL;
PyType_Spec numberFloatSpec = {
	"_jpype._JNumberFloat",
	0,
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	numberFloatSlots
};

static PyType_Slot numberBooleanSlots[] = {
	{Py_tp_new,      (void*) PyJPBoolean_new},
	{Py_tp_getattro, (void*) PyJPValue_getattro},
	{Py_tp_setattro, (void*) PyJPValue_setattro},
	{Py_tp_str,      (void*) PyJPBoolean_str},
	{Py_tp_repr,     (void*) PyJPBoolean_str},
	{Py_nb_int,      (void*) PyJPNumberLong_int},
	{Py_nb_float,    (void*) PyJPNumberLong_float},
	{Py_tp_hash,     (void*) PyJPNumberLong_hash},
	{Py_tp_richcompare, (void*) PyJPNumberLong_compare},
	{0}
};

PyTypeObject *PyJPNumberBool_Type = NULL;
PyType_Spec numberBooleanSpec = {
	"_jpype._JBoolean",
	0,
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	numberBooleanSlots
};

#ifdef __cplusplus
}
#endif

void PyJPNumber_initType(PyObject* module)
{
	PyObject *bases;

	bases = PyTuple_Pack(2, &PyLong_Type, PyJPObject_Type);
	PyJPNumberLong_Type = (PyTypeObject*) PyJPClass_FromSpecWithBases(&numberLongSpec, bases);
	Py_DECREF(bases);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	PyModule_AddObject(module, "_JNumberLong", (PyObject*) PyJPNumberLong_Type);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE

	bases = PyTuple_Pack(2, &PyFloat_Type, PyJPObject_Type);
	PyJPNumberFloat_Type = (PyTypeObject*) PyJPClass_FromSpecWithBases(&numberFloatSpec, bases);
	Py_DECREF(bases);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	PyModule_AddObject(module, "_JNumberFloat", (PyObject*) PyJPNumberFloat_Type);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE

	bases = PyTuple_Pack(2, &PyLong_Type, PyJPObject_Type);
	PyJPNumberBool_Type = (PyTypeObject*) PyJPClass_FromSpecWithBases(&numberBooleanSpec, bases);
	Py_DECREF(bases);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	PyModule_AddObject(module, "_JBoolean", (PyObject*) PyJPNumberBool_Type);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
}

JPPyObject PyJPNumber_create(JPJavaFrame &frame, JPPyObject& wrapper, const JPValue& value)
{
	JPContext *context = frame.getContext();
	// Bools are not numbers in Java
	if (value.getClass() == context->_java_lang_Boolean)
	{
		jlong l = 0;
		if (value.getValue().l != 0)
			l = frame.CallBooleanMethodA(value.getJavaObject(), context->_java_lang_Boolean->m_BooleanValueID, 0);
		PyObject *args = PyTuple_Pack(1, PyLong_FromLongLong(l));
		return JPPyObject::call(PyLong_Type.tp_new((PyTypeObject*) wrapper.get(), args, NULL));
	}
	if (PyObject_IsSubclass(wrapper.get(), (PyObject*) & PyLong_Type))
	{
		jlong l = 0;
		if (value.getValue().l != 0)
		{
			JPBoxedType* jb = (JPBoxedType*) value.getClass();
			l = frame.CallLongMethodA(value.getJavaObject(), jb->m_LongValueID, 0);
		}
		PyObject *args = PyTuple_Pack(1, PyLong_FromLongLong(l));
		return JPPyObject::call(PyLong_Type.tp_new((PyTypeObject*) wrapper.get(), args, NULL));
	}
	if (PyObject_IsSubclass(wrapper.get(), (PyObject*) & PyFloat_Type))
	{
		jdouble l = 0;
		if (value.getValue().l != 0)
		{
			JPBoxedType* jb = (JPBoxedType*) value.getClass();
			l = frame.CallDoubleMethodA(value.getJavaObject(), jb->m_DoubleValueID, 0);
		}
		PyObject *args = PyTuple_Pack(1, PyFloat_FromDouble(l));
		return JPPyObject::call(PyFloat_Type.tp_new((PyTypeObject*) wrapper.get(), args, NULL));
	}
	JP_RAISE(PyExc_TypeError, "unable to convert");  //GCOVR_EXCL_LINE
}
