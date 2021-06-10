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
 **************************************************************************** */

#include "jpype.h"
#include "pyjp.h"
#include "jp_array.h"
#include "structmember.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PyJPForward
{
	PyObject_HEAD
	PyObject *m_Instance;
	PyObject *m_Symbol;
	PyObject *m_Dict;
	PyObject *m_WeakRef;
};

static PyObject *PyJPForward_new(PyTypeObject *type, PyObject *pyargs, PyObject *kwargs)
{
	JP_PY_TRY("PyJPForward_new");
	PyJPForward *self = (PyJPForward*) type->tp_alloc(type, 0);
	JP_PY_CHECK();
	self->m_Instance = NULL;
	self->m_Symbol = NULL;
	self->m_Dict = NULL;
	self->m_WeakRef = NULL;
	return (PyObject*) self;
	JP_PY_CATCH(NULL);
}

static PyObject* PyJPForward_resolve(PyJPForward *self, PyObject* value)
{
	JPContext *context = PyJPModule_getContext();
	JPJavaFrame frame = JPJavaFrame::outer(context);
	
	JP_PY_TRY("PyJPForward_resolve");
	if (self->m_Instance != NULL)
	{
		PyErr_SetString(PyExc_ValueError, "instance has already been resolved");
		return NULL;
	}
	
	PyTypeObject *prior = Py_TYPE(self);
	PyTypeObject *requested = Py_TYPE(value); 

	// Clear any existing Java slot (for safety, may not be necessary)
	PyJPValue_assignJavaSlot(frame, (PyObject*) self, JPValue());

	// We do not need the symbol any longer
	Py_CLEAR(self->m_Symbol);

    // As we are changing the layout of memory.  We need to be prepared to 
    // move the internal object structure dict and weaklist.	
	Py_ssize_t priorDictOffset = prior->tp_dictoffset;
	Py_ssize_t requestedDictOffset = requested->tp_dictoffset;
	Py_ssize_t priorWeakOffset = prior->tp_weaklistoffset;
	Py_ssize_t requestedWeakOffset = requested->tp_weaklistoffset;
	PyObject* dict =  *(PyObject **) ((char *)self + priorDictOffset);
	PyObject* weak =  *(PyObject **) ((char *)self + priorWeakOffset);
		
	// Both of these cases are too large to fit in the footprint of the 
	// forward object so we need to convert the to resolved.
	if (PyObject_IsInstance(value, (PyObject*) PyJPPackage_Type) 
			|| PyObject_IsInstance(value, (PyObject*) PyJPClass_Type))
	{
		// Set the instance
		Py_INCREF(value);
		self->m_Instance = value;

		// Polymorph to a resolved type
		Py_DECREF(Py_TYPE(self));
		Py_TYPE(self) = PyJPResolved_Type;
		Py_INCREF(Py_TYPE(self));
		
		// Copy the Java slot over
		JPValue *jvalue = PyJPValue_getJavaSlot(value);
		PyJPValue_assignJavaSlot(frame, (PyObject*) self, *jvalue);
		
		// No need to move dict and weak as JResolved is same as JForward
		
		// Return the resolved object
		Py_INCREF(self);
		return (PyObject*) self;
	}

	if (PyObject_IsInstance(value, (PyObject*) PyJPArray_Type))
	{
		PyJPArray *aself = (PyJPArray*) self;		
		JPValue *jvalue = PyJPValue_getJavaSlot(value);
		
		// Polymorph the type
		Py_DECREF(Py_TYPE(self));
		Py_TYPE(self) = Py_TYPE(value);
		Py_INCREF(Py_TYPE(self));
		
		// Copy the value 
		aself->m_Array = new JPArray(*jvalue);
		aself->m_View = NULL;
		PyJPValue_assignJavaSlot(frame, (PyObject*) self, *jvalue);
		
		// Move the dict and weak to their proper locations
		*(PyObject **) ((char *)self + requestedDictOffset) = dict;
		*(PyObject **) ((char *)self + requestedWeakOffset) = weak; 
		
		// Return the resolved object
		Py_INCREF(self);
		return (PyObject*) self;
	}

	if (PyObject_IsInstance(value, (PyObject*) PyJPNumberBool_Type))
	{
		PyErr_SetString(PyExc_TypeError, "Forward declarations of boolean not supported");
		return 0;
	}

	if (PyObject_IsInstance(value, (PyObject*) PyJPNumberLong_Type))
	{
		PyErr_SetString(PyExc_TypeError, "Forward declarations of long not supported");
		return 0;
	}

	if (PyObject_IsInstance(value, (PyObject*) PyJPNumberFloat_Type))
	{
		PyErr_SetString(PyExc_TypeError, "Forward declarations of float not supported");
		return 0;
	}
	
	if (PyObject_IsInstance(value, (PyObject*) PyJPObject_Type))
	{
		// Currently we only work on simple objects
		if (prior->tp_basicsize<requested->tp_basicsize)
		{
			PyErr_SetString(PyExc_TypeError, "Forward size is mismatched");
			return 0;
		}
		
		// Polymorph the type
		Py_DECREF(Py_TYPE(self));
		Py_TYPE(self) = Py_TYPE(value);
		Py_INCREF(Py_TYPE(self));
		
		JPValue *jvalue = PyJPValue_getJavaSlot(value);
		PyJPValue_assignJavaSlot(frame, (PyObject*) self, *jvalue);

		// Move the dict and weak to their proper locations
		*(PyObject **) ((char *)self + requestedDictOffset) = dict;
		*(PyObject **) ((char *)self + requestedWeakOffset) = weak; 
		
		// Return the resolved object
		Py_INCREF(self);
		return (PyObject*) self;
	}

	JP_PY_CATCH(NULL);
}

static int PyJPForward_traverse(PyJPForward *self, visitproc visit, void *arg)
{
	Py_VISIT(self->m_Instance);
	Py_VISIT(self->m_Symbol);
	Py_VISIT(self->m_Dict);
	return 0;
}

static int PyJPForward_clear(PyJPForward *self)
{
	Py_CLEAR(self->m_Instance);
	Py_CLEAR(self->m_Symbol);
	Py_CLEAR(self->m_Dict);
	return 0;
}

static PyMemberDef forwardMembers[] = {
    {"__weakrefoffset__", T_PYSSIZET, offsetof(PyJPForward, m_WeakRef), READONLY},
    {"__dictoffset__", T_PYSSIZET, offsetof(PyJPForward, m_Dict), READONLY},
	{"_symbol", T_OBJECT, offsetof(PyJPForward, m_Symbol), 0},
	{"_instance", T_OBJECT, offsetof(PyJPForward, m_Symbol), READONLY},
	{0} 
};

static PyMethodDef forwardMethods[] = {
	{"_resolve", (PyCFunction) PyJPForward_resolve, METH_O, ""},
	{NULL},
};

static PyType_Slot forwardSlots[] = {
	{ Py_tp_new, (void*) PyJPForward_new},
	{ Py_tp_methods, (void*) &forwardMethods},
	{ Py_tp_traverse, (void*) PyJPForward_traverse},
	{ Py_tp_clear, (void*) PyJPForward_clear},
	{ Py_tp_members, (void*) forwardMembers},
	{0}
};

PyType_Spec PyJPForwardSpec = {
	"_jpype._JForward",
	sizeof (PyJPForward),
	0,
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	forwardSlots
};

PyTypeObject* PyJPForward_Type = NULL;

//******************

static PyObject *PyJPResolved_cast(PyJPForward *self, PyObject *other)
{
	return PyNumber_MatrixMultiply(self->m_Instance, other);
}

static PyObject *PyJPResolved_castEq(PyJPForward *self, PyObject *other)
{
	return PyNumber_InPlaceMatrixMultiply(self->m_Instance, other);
}

static PyObject *PyJPResolved_call(PyJPForward *self, PyObject *args, PyObject *kwargs)
{
	return PyObject_Call(self->m_Instance, args, kwargs);
}

static PyObject *PyJPResolved_array(PyJPForward *self, PyObject *item)
{
	return PyObject_GetItem(self->m_Instance, item);
}

static PyObject *PyJPResolved_str(PyJPForward *self)
{
	return PyObject_Str(self->m_Instance);
}

static PyObject *PyJPResolved_repr(PyJPForward *self)
{
	return PyObject_Repr(self->m_Instance);
}

static PyObject *PyJPResolved_getattro(PyJPForward *obj, PyObject *name)
{
	return PyObject_GetAttr(obj->m_Instance, name);
}

static int PyJPResolved_setattro(PyJPForward *obj, PyObject *name, PyObject *value)
{
	return PyObject_SetAttr(obj->m_Instance, name, value);
}

static int PyJPResolved_instancecheck(PyJPForward *self, PyObject *test)
{
	return PyObject_IsInstance(test, self->m_Instance);
}

static int  PyJPResolved_subclasscheck(PyJPForward *self, PyObject *test)
{
	return PyObject_IsSubclass(test, self->m_Instance);
}

static PyMethodDef resolvedMethods[] = {
	{"__instancecheck__", (PyCFunction) PyJPResolved_instancecheck, METH_O, ""},
	{"__subclasscheck__", (PyCFunction) PyJPResolved_subclasscheck, METH_O, ""},
	{NULL},
};

static PyType_Slot resolvedSlots[] = {
	{ Py_tp_methods, (void*) &resolvedMethods},
	{ Py_tp_getattro, (void*) &PyJPResolved_getattro},
	{ Py_tp_setattro, (void*) &PyJPResolved_setattro},
	{ Py_tp_str, (void*) &PyJPResolved_str},
	{ Py_tp_repr, (void*) &PyJPResolved_repr},
	{ Py_tp_call, (void*) &PyJPResolved_call},
	{ Py_mp_subscript, (void*) PyJPResolved_array},
	{ Py_nb_matrix_multiply, (void*) PyJPResolved_cast},
	{ Py_nb_inplace_matrix_multiply, (void*) PyJPResolved_castEq},
	{0}
};

PyType_Spec PyJPResolvedSpec = {
	"_jpype._JResolved",
	sizeof (PyJPForward),
	0,
	Py_TPFLAGS_DEFAULT,
	resolvedSlots
};

PyTypeObject* PyJPResolved_Type = NULL;

#ifdef __cplusplus
}
#endif

void PyJPForward_initType(PyObject* module)
{
	PyObject* bases = PyTuple_Pack(1, PyJPObject_Type);
	PyJPForward_Type = (PyTypeObject*) PyJPClass_FromSpecWithBases(&PyJPForwardSpec, bases);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	PyModule_AddObject(module, "_JForward", (PyObject*) PyJPForward_Type);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	
	bases = PyTuple_Pack(1, PyJPForward_Type);
	PyJPResolved_Type = (PyTypeObject*) PyJPClass_FromSpecWithBases(&PyJPResolvedSpec, bases);
	Py_DECREF(bases);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
	PyModule_AddObject(module, "_JResolved", (PyObject*) PyJPResolved_Type);
	JP_PY_CHECK(); // GCOVR_EXCL_LINE
}
