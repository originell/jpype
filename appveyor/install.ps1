$env:Path += ";"+$env:PYTHON

# Check that we have the expected version and architecture for Python
ant.exe -version
python.exe --version
python.exe -c "import struct; print(struct.calcsize('P') * 8)"

# Install the build dependencies of the project. If some dependencies contain
# compiled extensions and are not provided as pre-built wheel packages,
# pip will build them from source using the MSVC compiler matching the
# target Python version and architecture

#seem like pip is broken in 3.4 and is not available via python -m
git clone --depth=1 https://github.com/pypa/setuptools.git
python setuptools\bootstrap.py
python setuptools\setup.py install

git clone --depth=1 git+https://github.com/pypa/wheel.git
python wheel\setup.py install

git clone --depth=1 https://github.com/pypa/pip.git
python pip\setup.py install
Remove-Item .\pip -Force -Recurse
Remove-Item .\setuptools -Force -Recurse

pip install --upgrade git+https://github.com/pypa/setuptools_scm.git
pip install --upgrade nose -r test-requirements.txt
#pip.exe install -r "test-requirements.txt" # -r dev-requirements.txt

ant.exe -f test\\build.xml

# Build the compiled extension and run the project tests
python.exe setup.py bdist_wheel
dir .\dist
Get-ChildItem -File -Path .\dist\*.whl | Foreach {pip install --upgrade $_.fullname}
