package org.jpype.ref;

import java.lang.ref.PhantomReference;
import java.lang.ref.ReferenceQueue;

/**
 * (internal) Reference to a PyObject*.
 */
class JPypeReference extends PhantomReference
{
  long hostReference;

  public JPypeReference(ReferenceQueue arg1, Object javaObject, long hostObject)
  {
    super(javaObject, arg1);
    hostReference = hostObject;
  }

  @Override
  public int hashCode()
  {
    return (int) hostReference;
  }

  @Override
  public boolean equals(Object arg0)
  {
    if (!(arg0 instanceof JPypeReference))
    {
      return false;
    }

    return ((JPypeReference) arg0).hostReference == hostReference;
  }
}
