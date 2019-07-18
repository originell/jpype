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
#include <jpype.h>

JPStringType::JPStringType(JPContext* context,
			   jclass clss,
			   const string& name,
			   JPClass* super,
			   JPClassList& interfaces,
			   jint modifiers)
: JPClass(context, clss, name, super, interfaces, modifiers)
{
	JPJavaFrame frame(context);
	m_String_ToCharArrayID = frame.GetMethodID(clss, "toCharArray", "()[C");
}

JPStringType::~JPStringType()
{
}

jobject JPStringType::stringToCharArray(jstring str)
{
	JPJavaFrame frame(m_Context);
	jobject res = frame.CallObjectMethodA(str, m_String_ToCharArrayID, 0);
	return frame.keep(res);
}

JPPyObject JPStringType::convertToPythonObject(jvalue val)
{
	JP_TRACE_IN("JPStringType::asHostObject");

	if (val.l == NULL)
	{
		return JPPyObject::getNone();
	}

	if (m_Context->getConvertStrings())
	{
		bool unicode = false;
		string str = m_Context->toStringUTF8((jstring) (val.l));
#if PY_MAJOR_VERSION < 3
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i]&0x80)
			{
				unicode = true;
				break;
			}
		}
#endif
		return JPPyString::fromStringUTF8(str, unicode);
	}

	return JPPythonEnv::newJavaObject(JPValue(this, val));
	JP_TRACE_OUT;
}

JPMatch::Type JPStringType::getJavaConversion(JPMatch& match, JPJavaFrame& frame, PyObject* pyobj)
{
	JP_TRACE_IN("JPStringType::getJavaConversion");
	if (nullConversion->matches(match, frame, this, pyobj) != JPMatch::_none)
		return match.type;
	if (objectConversion->matches(match, frame, this, pyobj) != JPMatch::_none)
		return match.type;
	if (JPPyString::check(pyobj))
	{
		match.conversion = stringConversion;
		return match.type = JPMatch::_exact;
	}
	return match.type;
	JP_TRACE_OUT;
}

JPValue JPStringType::newInstance(JPPyObjectVector& args)
{
	JP_TRACE_IN("JPStringClass::newInstance");
	if (args.size() == 1 && JPPyString::check(args[0]))
	{
		// JNI has a short cut for constructing java.lang.String
		JP_TRACE("Direct");
		string str = JPPyString::asStringUTF8(args[0]);
		jvalue res;
		res.l = m_Context->fromStringUTF8(str);
		return JPValue(this, res);
	}
	return JPClass::newInstance(args);
	JP_TRACE_OUT;
}
