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

/**
 *
 * @author nelson85
 */
public class FieldDecl
{

  final Class type;
  final String name;
  final Object value;
  final int modifiers;
  
  public FieldDecl(Class type, String name, Object value, int modifiers)
  {
    this.type = type;
    this.name = name;
    this.value = value;
    this.modifiers = modifiers;
  }
}
