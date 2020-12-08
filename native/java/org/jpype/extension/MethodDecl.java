/** ***************************************************************************
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * See NOTICE file for details.
 **************************************************************************** */
package org.jpype.extension;

import java.lang.reflect.Method;
import org.jpype.JPypeContext;
import org.jpype.asm.Type;
import org.jpype.manager.TypeManager;

/**
 *
 * @author nelson85
 */
public class MethodDecl
{

  public final String name;
  public final Class ret;
  public final Class[] parameters;
  public final Class[] exceptions;
  public final int modifiers;
  public Method method;
  public long retId;
  public long[] parametersId;
  public String parametersName;
  public long functionId;

  public MethodDecl(String name, Class ret, Class[] parameters, Class[] exceptions, int modifiers)
  {
    this.name = name;
    if (ret == null)
      ret = Void.TYPE;
    this.ret = ret;
    this.parameters = parameters;
    this.exceptions = exceptions;
    this.modifiers = modifiers;
  }

  boolean matches(Method m)
  {
    if (!m.getName().equals(name))
      return false;
    Class<?>[] param2 = m.getParameterTypes();
    if (param2.length != parameters.length)
      return false;
    for (int i = 0; i < param2.length; ++i)
    {
      if (param2[i] != parameters[i])
        return false;
    }

    // FIXME match Exceptions (this can throw less than the full list or
    // be covariant.   So long as everything specified is a child of something
    // in the original specification we are okay.
    // FIXME need to use 
    if (ret.isPrimitive())
      return ret == m.getReturnType();
    return m.getReturnType().isAssignableFrom(ret);
  }

  void bind(Method m)
  {
    this.method = m;
  }

  void resolve()
  {
    TypeManager typemanager = JPypeContext.getInstance().getTypeManager();
    retId = typemanager.findClass(ret);
    this.parametersId = new long[this.parameters.length];
    for (int i = 0; i < this.parameters.length; ++i)
      this.parametersId[i] = typemanager.findClass(parameters[i]);
  }

  String descriptor()
  {
    StringBuilder sb = new StringBuilder();
    sb.append('(');
    for (Class i : this.parameters)
      sb.append(Type.getDescriptor(i));
    sb.append(')');
    sb.append(Type.getDescriptor(ret));
    return sb.toString();
  }

}
