Fun with using LLVM IR to implement [Kaleidoscope](https://www.llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

-------------------------------------------------------------------------------
# Getting the source code and building kaleidoscope

1. Get kaleidoscope:
   * ``git clone https://github.com/atsmtat/kaleidoscope.git``

2. Get llvm:
   * on macOs, `brew install llvm` will install the includes and libs in `/usr/local/opt/llvm`
   * update path: `export PATH="/usr/local/opt/llvm/bin:$PATH"` to use llvm-config util to get build flags.
	
3. Configure, build, and install kaleidoscope:
   * ``cd kaleidoscope``
   * ``mkdir build``
   * ``cd build``
   * ``cmake ..``
   * ``cmake --build .``
   * ``make install``




