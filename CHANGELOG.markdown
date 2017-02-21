Change Log
==============

### 1.1.0 ###

- Support for more object/variations being returned by integrity polls and read requests
- Can add pre-constructed physical layers to PhysicalLayerManager
- Decoupling of PhysLayerInstance and PhysicalLayerMap
- Java binding tests to verify swig interface
- Added support for DNS resolution of TCPClient/TCPServer addresses
- Added Python swig bindings
- Added DNP3 master demo/example program
- Example master/slave programs configurable via command line
- Upgraded Windows build to Visual Studio 2010
- Added C++/CLI binding layer for .NET support
- Can produce 64-bit Windows binaries
- All tests running in autotools build
- Refactored source paths to fit Linux/autotools style
- Moved VS2010 project/solutions to own directory
- Removed Rake build system
- Java binding shared library/DLL renamed to “opendnp3-java”
- Renamed Java packages to simpler “org.totalgrid.dnp3”
- Java artifact now being published by Maven as org.totalgrid.dnp3:opendnp3
- Renamed test set to “dnp3testset” to distinguish it in unix ecosystem when installed
- Re-enabled “start online” ability in slave test set
- Slave unsolicited response configuration was not being read from XML
- Missing files added to Swig binding configuration
- Missing headers added to installed headers list
- Fix for incorrect behavior when unsolicited is enabled but no classes are specified


### 1.0.0 ###
- Migrated to Boost 1.47.0
- Added Virtual Terminal Object (VTO) support with standard implementation and experimental stream support
- Migrated all external numeric types to use boost type definitions for interoperability
- Added autotools support thanks to Chris Verges - Rake may be replaced entirely with autotools in a future release
- Added Astyle code formatter to project build systems
- Master/Slave can now both notify of their current state using an enumeration
- TestSet includes address scanning feature to detect devices that don't support broadcast addressing
- sub-Masters on same channel can now use same address if talking to different slaves
- Log messages can now contain well known key-value pairs
- Management class 'AsyncStackManager' now automatically runs when stacks are added, no Start() call
- AsyncStackManger blocks when RemovePort() or RemoveStack() are called until the operation has completed
- Fixed bugs: 18, 15, 9, 7

### 0.9.4 ###
Rebased java namespace to org.totalgrid.reef.protocol.dnp3
Java SO static loading can be overriden using system property "org.totalgrid.reef.protocol.dnp3.nostaticload"
Fixed a major bug in slave unsolcited mode on event buffer overflow.


### 0.9.3 ###
- Fixed parsing bug in testset that caused erroneous pulse repititions
- Fixed a bug in DataPoll.cpp that caused stack to crash if it eas configured with an empty exception scan
- Added an address scanning feature to the TestSet
- Removed ASIOIncludes.h and replaced with preprocessor defines for all platforms.
- Fix for issue #9
- Jar now require shared library to be install on java.library.path



