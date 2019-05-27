/*
 *    Copyright 2019 Karl Einar Nelson
 *   
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *  
 *     http://www.apache.org/licenses/LICENSE-2.0
 *  
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
package org.jpype.manager;

import java.lang.reflect.Executable;
import java.lang.reflect.Field;

/**
 * Interface for creating new resources used by JPype.
 * 
 * This calls the C++ constructors with all of the required 
 * fields for each class. This pattern eliminates the need
 * for C++ layer probing Java for resources.
 * 
 * This is an interface for testing.
 * 
 * @author nelson85
 */
public interface TypeFactory
{
//<editor-fold desc="class" defaultstate="collapsed">
  /** Create a JPArray class.
   * 
   * @param cls is the class type.
   * @param name
   * @param componentPtr
   * @return the pointer to the JPArrayClass.
   */
  long defineArrayClass(
          Class cls, 
          String name, 
          long superClass, 
          long componentPtr, 
          int modifiers);
  
  /**
   * Create a class type.  
   * 
   * @param cls
   * @param superClass
   * @param interfaces
   * @param modifiers
   * @param name
   * @return the pointer to the JPClass.
   */
  long defineObjectClass(
          Class cls, 
          String name, 
          long superClass, 
          long[] interfaces, 
          int modifiers);
  
  /** 
   * Define a primitive types.
   * 
   * @param code is a code number used to determine which type of
   *      primitive this will attach to.
   * @param cls is the Java class for this primitive.
   * @param boxedPtr is the JPClass for the boxed class.
   * @param modifiers
   * @return 
   */
  long definePrimitive(
          int code, 
          Class cls, 
          long boxedPtr, 
          int modifiers);

//</editor-fold>
//<editor-fold desc="members" defaultstate="collapsed">
  /** 
   * Called after a class is constructed to populate the required
   * fields and methods.
   * 
   * @param cls is the JPClass to populate
   * @param ctorMethod is the JPMethod for the constructor.
   * @param methodList is a list of JPMethod for the method list.
   * @param fieldList is a list of JPField for the field list.
   */
  void assignMembers(long cls, 
          long ctorMethod,
          long[] methodList,
          long[] fieldList);
  
  /**
   * Create a Method.
   * 
   * @param cls is the class holding this.
   * @param name
   * @param field
   * @param fieldType
   * @param modifiers 
   * @return the pointer to the JPMethod.
   */
  long defineField(
          long cls, 
          String name,
          Field field, // This will convert to a field id
          long fieldType,
          int modifiers);
  
  /**
   * Create a Method.
   * 
   * @param cls is the class holding this.
   * @param name
   * @param method is the Java method that will be called, converts to a method id.
   * @param returnType 
   * @param argumentTypes
   * @param overloadList
   * @param modifiers 
   * @return the pointer to the JPMethod.
   */
  long defineMethod(
          long cls, 
          String name,
          Executable method, // This will convert to a method id
          long returnType,
          long[] argumentTypes,
          long[] overloadList,
          int modifiers);
  
  /** 
   * Create a Method dispatch for Python by name.
   * 
   * @param cls is the class that owns this dispatch.
   * @param name is the name of the dispatch.
   * @param overloadList is the list of all methods constructed for this class.
   * @param modifiers contains if the method is (CTOR, STATIC),
   * @return the pointer to the JPMethodDispatch.
   */
  long defineMethodDispatch(
          long cls,
          String name,
          long[] overloadList,
          int modifiers);
  
//</editor-fold>
//<editor-fold desc="destroy" defaultstate="collapsed">
  /**
   * Destroy the resources.
   * 
   * @param resources
   * @param sz 
   */
  void destroy(long[] resources, int sz);
//</editor-fold>
}
