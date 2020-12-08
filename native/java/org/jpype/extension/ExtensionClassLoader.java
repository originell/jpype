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

import org.jpype.JPypeContext;
import org.jpype.classloader.DynamicClassLoader;

/**
 * Internal class for loading extension classes.
 * 
 * @author nelson85
 */
class ExtensionClassLoader extends ClassLoader
{
  ClassLoader instance = new ExtensionClassLoader(JPypeContext.getInstance().getClassLoader());

  private ExtensionClassLoader(ClassLoader classLoader)
  {
    super(classLoader);
  }
  
  Class loadClass(String name, byte[] b)
  {
    return this.defineClass(name, b, 0, b.length);
  }
}
