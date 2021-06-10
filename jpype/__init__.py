# *****************************************************************************
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#   See NOTICE file for details.
#
# *****************************************************************************
import _jpype
from ._jinit import *
from ._jforward import JForward as _JForward
from ._jpackage import *
from ._jproxy import *
from ._core import *
from ._gui import *
from ._classpath import *
from ._jclass import *
from ._jobject import *
# There is a bug in lgtm with __init__ imports.  It will be fixed next month.
from . import _jarray       # lgtm [py/import-own-module]
from . import _jexception   # lgtm [py/import-own-module]
from .types import *
from ._jcustomizer import *
from . import nio           # lgtm [py/import-own-module]
from . import types         # lgtm [py/import-own-module]
from ._jcustomizer import *
# Import all the class customizers
# Customizers are applied in the order that they are defined currently.
from . import _jmethod      # lgtm [py/import-own-module]
from . import _jcollection  # lgtm [py/import-own-module]
from . import _jio          # lgtm [py/import-own-module]
from . import protocol      # lgtm [py/import-own-module]
from . import _jthread      # lgtm [py/import-own-module]

__all__ = ['java', 'javax']
__all__.extend(_jinit.__all__)
__all__.extend(_core.__all__)
__all__.extend(_classpath.__all__)
__all__.extend(types.__all__)
__all__.extend(_jproxy.__all__)
__all__.extend(_jpackage.__all__)
__all__.extend(_jclass.__all__)
__all__.extend(_jcustomizer.__all__)
__all__.extend(_gui.__all__)

__version__ = "1.3.1_dev0"
__version_info__ = __version__.split('.')


# FIXME these should be deprecated. The old JPackage system is only for
#  python2 series and generates lots of deceptive classes.  At some point
#  these two are going to have to go away.
java = JPackage("java", strict=True)
javax = JPackage("javax", strict=True)

JMethod = _jpype._JMethod
JField = _jpype._JField

if hasattr(_jpype, 'bootstrap'):
   _jpype.bootstrap()
   _core.initializeResources()

jroot = _JForward(None)
