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

/**
 *
 * @author nelson85
 */
public class Proto extends Base
{

  static final long[] _c1 = new long[]
  {
    45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55
  };

  public Proto(String p0)
  {
    super(p0);
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = p0;
    Factory._call(context, 123512, this, 423512, _c1, o);
  }

  @Override
  public String call(String s)
  {
    Object[] o = new Object[1];
    o[0] = s;
    Object ret = Factory._call(99999, 123512, this, 423512, _c1, o);
    return (String) ret;
  }

  public void foo(boolean z, byte b, char c, short s, int i, long l, float f, double d)
  {
    Object[] o = new Object[9];
    o[0] = z;
    o[1] = b;
    o[2] = c;
    o[3] = s;
    o[4] = i;
    o[5] = l;
    o[6] = f;
    o[7] = d;
    Factory._call(99999, 123512, this, 423512, _c1, o);

  }

  public void retVoid(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Factory._call(context, 123512, this, 423512, _c1, o);
  }

  public boolean retBoolean(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Boolean) ret);
  }

  public byte retByte(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).byteValue();
  }

  public char retChar(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Character) ret);
  }

  public short retShort(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).shortValue();
  }

  public int retInt(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).intValue();
  }

  public long retLong(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).longValue();
  }

  public float retFloat(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).floatValue();
  }

  public double retDouble(String s)
  {
    Object[] o = new Object[1];
    long context = JPypeContext.getInstance().getContext();
    o[0] = s;
    Object ret = Factory._call(context, 123512, this, 423512, _c1, o);
    return ((Number) ret).doubleValue();
  }

}
