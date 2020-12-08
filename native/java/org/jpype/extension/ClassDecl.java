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

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author nelson85
 */
public class ClassDecl
{

  final String name;
  ArrayList<MethodDecl> ctors = new ArrayList<>();
  ArrayList<MethodDecl> methods = new ArrayList<>();
  ArrayList<FieldDecl> fields = new ArrayList<>();
  Class[] bases;
  Class base;
  List<Class> interfaces;
  String internalName;

  public ClassDecl(String name, Class[] bases)
  {
    this.name = name;
    this.bases = bases;
  }

  public FieldDecl addField(Class cls, String name, Object value, int modifiers)
  {
    FieldDecl field = new FieldDecl(cls, name, value, modifiers);
    this.fields.add(field);
    return field;
  }
  
  public MethodDecl addCtor(Class[] arguments, Class[] exceptions, int modifiers)
  {
    MethodDecl method = new MethodDecl("<init>", null, arguments, exceptions, modifiers);
    this.ctors.add(method);
    return method;
  }
  
    public MethodDecl addMethod(String name, Class ret, Class[] arguments, Class[] exceptions, int modifiers)
  {
    MethodDecl method = new MethodDecl(name, ret, arguments, exceptions, modifiers);
    this.methods.add(method);
    return method;
  }

  void setBase(Class base)
  {
    this.base = base;
  }

  void setInterfaces(List<Class> interfaces)
  {
    this.interfaces = interfaces;
  }
}
