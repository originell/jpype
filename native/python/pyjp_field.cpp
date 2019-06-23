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

static PyMethodDef fieldMethods[] = {
	{"getName", (PyCFunction) (&PyJPField::getName), METH_NOARGS, ""},
	{"isFinal", (PyCFunction) (&PyJPField::isFinal), METH_NOARGS, ""},
	{"isStatic", (PyCFunction) (&PyJPField::isStatic), METH_NOARGS, ""},
	{NULL},
};

PyTypeObject PyJPField::Type = {
	PyVarObject_HEAD_INIT(&PyType_Type, 0)
	/* tp_name           */ "_jpype.PyJPField",
	/* tp_basicsize      */ sizeof (PyJPField),
	/* tp_itemsize       */ 0,
	/* tp_dealloc        */ (destructor) PyJPField::__dealloc__,
	/* tp_print          */ 0,
	/* tp_getattr        */ 0,
	/* tp_setattr        */ 0,
	/* tp_compare        */ 0,
	/* tp_repr           */ 0,
	/* tp_as_number      */ 0,
	/* tp_as_sequence    */ 0,
	/* tp_as_mapping     */ 0,
	/* tp_hash           */ 0,
	/* tp_call           */ 0,
	/* tp_str            */ 0,
	/* tp_getattro       */ 0,
	/* tp_setattro       */ 0,
	/* tp_as_buffer      */ 0,
	/* tp_flags          */ Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	/* tp_doc            */ "Java Field",
	/* tp_traverse       */ (traverseproc) PyJPField::traverse,
	/* tp_clear          */ (inquiry) PyJPField::clear,
	/* tp_richcompare    */ 0,
	/* tp_weaklistoffset */ 0,
	/* tp_iter           */ 0,
	/* tp_iternext       */ 0,
	/* tp_methods        */ fieldMethods,
	/* tp_members        */ 0,
	/* tp_getset         */ 0,
	/* tp_base           */ 0,
	/* tp_dict           */ 0,
	/* tp_descr_get      */ (descrgetfunc) PyJPField::__get__,
	/* tp_descr_set      */ (descrsetfunc) PyJPField::__set__,
	/* tp_dictoffset     */ 0,
	/* tp_init           */ 0,
	/* tp_alloc          */ 0,
	/* tp_new            */ PyType_GenericNew

};

// Static methods

void PyJPField::initType(PyObject* module)
{
	PyType_Ready(&PyJPField::Type);
	Py_INCREF(&PyJPField::Type);
	PyModule_AddObject(module, "PyJPField", (PyObject*) (&PyJPField::Type));
}

JPPyObject PyJPField::alloc(JPField* m)
{
	JP_TRACE_IN_C("PyJPField::alloc");
	PyJPField *self = (PyJPField*) PyJPField::Type.tp_alloc(&PyJPField::Type, 0);
	JP_PY_CHECK();
	self->m_Field = m;
	self->m_Context = (PyJPContext*) (m->getContext()->getHost());
	Py_INCREF(self->m_Context);
	JP_TRACE("self", self);
	return JPPyObject(JPPyRef::_claim, (PyObject*) self);
	JP_TRACE_OUT;
}

void PyJPField::__dealloc__(PyJPField* self)
{
	JP_TRACE_IN_C("PyJPField::__dealloc__", self);
	PyObject_GC_UnTrack(self);
	clear(self);
	Py_TYPE(self)->tp_free(self);
	JP_TRACE_OUT_C;
}

int PyJPField::traverse(PyJPField *self, visitproc visit, void *arg)
{
	JP_TRACE_IN_C("PyJPField::traverse", self);
	Py_VISIT(self->m_Context);
	return 0;
	JP_TRACE_OUT_C;
}

int PyJPField::clear(PyJPField *self)
{
	JP_TRACE_IN_C("PyJPField::clear", self);
	Py_CLEAR(self->m_Context);
	return 0;
	JP_TRACE_OUT_C;
}

PyObject* PyJPField::getName(PyJPField* self, PyObject* arg)
{
	JP_TRACE_IN_C("PyJPField::getName", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		return JPPyString::fromStringUTF8(self->m_Field->getName()).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

PyObject* PyJPField::__get__(PyJPField *self, PyObject *obj, PyObject *type)
{
	JP_TRACE_IN_C("PyJPField::__get__", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		if (self->m_Field->isStatic())
			return self->m_Field->getStaticField().keep();
		if (obj == NULL)
			JP_RAISE_ATTRIBUTE_ERROR("Field is not static");
		JPValue *jval = JPPythonEnv::getJavaValue(obj);
		if (jval == NULL)
			JP_RAISE_ATTRIBUTE_ERROR("Field requires instance value");

		return self->m_Field->getField(jval->getValue().l).keep();
	}
	PY_STANDARD_CATCH(NULL);
	JP_TRACE_OUT_C;
}

int PyJPField::__set__(PyJPField *self, PyObject *obj, PyObject *pyvalue)
{
	JP_TRACE_IN_C("PyJPField::__set__", self);
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		if (self->m_Field->isFinal())
			JP_RAISE_ATTRIBUTE_ERROR("Field is final");
		if (self->m_Field->isStatic())
		{
			self->m_Field->setStaticField(pyvalue);
			return 0;
		}
		if (obj == Py_None)
			JP_RAISE_ATTRIBUTE_ERROR("Field is not static");
		JPValue *jval = JPPythonEnv::getJavaValue(obj);
		if (jval == NULL)
			JP_RAISE_ATTRIBUTE_ERROR("Field requires instance value");
		self->m_Field->setField(jval->getValue().l, pyvalue);
		return 0;
	}
	PY_STANDARD_CATCH(-1);
	JP_TRACE_OUT_C;
}

PyObject *PyJPField::isStatic(PyJPField *self, PyObject *arg)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		return PyBool_FromLong(self->m_Field->isStatic());
	}
	PY_STANDARD_CATCH(NULL);
}

PyObject* PyJPField::isFinal(PyJPField *self, PyObject *arg)
{
	try
	{
		JPContext *context = self->m_Context->m_Context;
		ASSERT_JVM_RUNNING(context);
		JPJavaFrame frame(context);
		return PyBool_FromLong(self->m_Field->isFinal());
	}
	PY_STANDARD_CATCH(NULL);
}
