/*
 * Copyright 2018 nelson85.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PYJP_H
#define PYJP_H

#define ASSERT_JVM_RUNNING(context) (JPContext*)context->assertJVMRunning(JP_STACKINFO())
#define PY_STANDARD_CATCH(...) catch(...) { JPPythonEnv::rethrow(JP_STACKINFO()); } return __VA_ARGS__

/** This is the module specifically for the cpython modules to include.
 */
#include <Python.h>
#include <jpype.h>
#include <pyjp_module.h>

// Forward declare the types
extern PyObject* PyJPContext_Type;
extern PyObject *PyJPClass_Type;
extern PyObject *PyJPClassMeta_Type;
extern PyObject *PyJPField_Type;
extern PyObject *PyJPMethod_Type;
extern PyObject *PyJPMonitor_Type;
extern PyObject *PyJPProxy_Type;
extern PyObject *PyJPValueBase_Type;
extern PyObject *PyJPValue_Type;
extern PyObject *PyJPValueInt_Type;
extern PyObject *PyJPValueExc_Type;

struct PyJPContext
{
	PyObject_HEAD
	JPContext *m_Context;
	PyObject *m_Dict;
	PyObject *m_Classes;

	static void initType(PyObject *module);
	static bool check(PyObject *o);
} ;

struct PyJPValue
{
	PyObject_HEAD
	JPValue m_Value;
	PyJPContext *m_Context;

	static JPPyObject create(PyTypeObject* wrapper, JPContext* context, JPClass* cls, jvalue value);
	static void initType(PyObject *module);
	static PyJPValue* getValue(PyObject *self);
} ;

extern PyObject* PyJPArray_Type;

/** This is a wrapper for accessing the array method.  It is structured to
 * be like a bound method.  It should not be the primary handle to the object.
 * That will be a PyJPValue.
 */
struct PyJPArray
{
	PyJPValue m_Value;
	JPArray *m_Array;

	static void initType(PyObject *module);
} ;

struct PyJPClass
{
	PyJPValue m_Value;
	JPClass *m_Class;
	//	PyJPContext *m_Context;
	static PyTypeObject Type;

	// Python-visible methods
	static void initType(PyObject *module);

	/** Create a PyJPClass instance from within the module.
	 */
	static JPPyObject alloc(PyTypeObject* obj, JPContext* context, JPClass *cls);

} ;

struct PyJPMethod
{
	PyFunctionObject func;
	PyJPContext *m_Context;
	JPMethodDispatch* m_Method;
	PyObject* m_Instance;
	PyObject* m_Doc;
	PyObject* m_Annotations;
	PyObject* m_CodeRep;

	// Python-visible methods
	static void initType(PyObject *module);
	static JPPyObject alloc(JPMethodDispatch *mth, PyObject *obj);
} ;

struct PyJPField
{
	PyJPValue m_Value;
	JPField *m_Field;

	// Python-visible methods
	static void initType(PyObject *module);
	static JPPyObject alloc(JPField *mth);
} ;

/** Python object to support jpype.synchronized(object) command.
 */
struct PyJPMonitor
{
	PyObject_HEAD
	JPMonitor *m_Monitor;
	PyJPContext *m_Context;

	// Python-visible methods
	static void initType(PyObject *module);
} ;

struct PyJPProxy
{
	PyObject_HEAD
	JPProxy *m_Proxy;
	PyObject *m_Target;
	PyJPContext *m_Context;

	static void initType(PyObject *module);
} ;

inline JPContext* PyJPValue_getContext(PyJPValue* value)
{
	JPContext *context = value->m_Context->m_Context;
	if (context == NULL)
	{
		JP_RAISE_RUNTIME_ERROR("Context is null");
	}
	ASSERT_JVM_RUNNING(context);
	return;
}
#define PyJPValue_GET_CONTEXT(X) PyJPValue_getContext((PyJPValue*)X)

#include <pyjp_classhints.h>
#include <pyjp_module.h>

#endif /* PYJP_H */
