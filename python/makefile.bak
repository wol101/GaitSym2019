# Note use "make -j 8" to use 8 threads
# get the system and architecture
ifeq ($(OS),Windows_NT)
	SYSTEM := WIN32
	ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
		ARCH := AMD64
	endif
	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		ARCH := AMD64
	endif
	ifeq ($(PROCESSOR_ARCHITECTURE),x86)
		ARCH := IA32
	endif
	UNAME_S := $(shell uname -s)
	ifneq (,$(findstring CYGWIN,$(UNAME_S)))
		SYSTEM := CYGWIN
	endif
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		SYSTEM := LINUX
	endif
	ifeq ($(UNAME_S),Darwin)
		SYSTEM := OSX
	endif
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		ARCH := AMD64
	endif
	ifneq ($(filter %86,$(UNAME_P)),)
		ARCH := IA32
	endif
	ifneq ($(filter arm%,$(UNAME_P)),)
		ARCH := ARM
	endif
	ifneq ($(filter ppc%,$(UNAME_P)),)
		ARCH := PPC
	endif
endif
HOST := $(shell hostname)
cc_AVAIL := $(shell command -v cc 2> /dev/null)
gcc_AVAIL := $(shell command -v gcc 2> /dev/null)
CC_AVAIL := $(shell command -v CC 2> /dev/null)
gpp_AVAIL := $(shell command -v g++ 2> /dev/null)

ifeq ($(SYSTEM),OSX)
	CXXFLAGS = -Wall -ffast-math -DEXPERIMENTAL -std=c++14 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DUSE_UNIX_ERRORS -DHAVE_ALLOCA_H -DMAKEFILE_OSX
	LDFLAGS =
	LIBS = -lpthread -lm -framework CoreServices
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

ifeq ($(SYSTEM),LINUX)
	CXXFLAGS = -fPIC -O3 -ffast-math -DEXPERIMENTAL -std=c++17 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DHAVE_MALLOC_H -DUSE_UNIX_ERRORS -DMAKEFILE_LINUX
	LDFLAGS = -shared 
ifdef gcc_AVAIL
	CC = gcc
endif
ifdef gpp_AVAIL
	CXX = g++
endif
ifdef cc_AVAIL
	CC = cc
endif
ifdef CC_AVAIL
	CXX = CC
endif
	LIBS = 
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

ifeq ($(SYSTEM),CYGWIN)
	# cygwin64 gcc version
	CXXFLAGS = -O3 -g -ffast-math -Wa,-mbig-obj -Dexprtk_disable_enhanced_features -std=c++14 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DHAVE_MALLOC_H -DUSE_UNIX_ERRORS -DHAVE_ALLOCA_H \
	-DALLOCA_H_NEEDED -DNEED_BCOPY -DSTRINGS_H_NEEDED -DMAKEFILE_CYGWIN
	LDFLAGS =
	CXX = g++
	CC = gcc
	LIBS = -lpthread -lm
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

ifeq ($(HOST),submitter.itservices.manchester.ac.uk)
	#CXXFLAGS = -static -ffast-math -O3 -DEXPERIMENTAL \
	#CXXFLAGS = -static -O2 -DEXPERIMENTAL -std=c++14 \
	CXXFLAGS = -static -O3 -ffast-math -DEXPERIMENTAL -std=c++14 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DHAVE_MALLOC_H -DUSE_UNIX_ERRORS -DMAKEFILE_SUBMITTER
	LDFLAGS = -static
	# condor_compile is tricky with pthread so best avoided and just use vanilla universe
	# CXX = condor_compile g++
	# CC = condor_compile gcc
	CXX = g++
	CC = gcc
	LIBS = -lpthread -lm
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

ifeq ($(HOST),l-u-roboticssuite.it.manchester.ac.uk)
	#CXXFLAGS = -static -ffast-math -O3 -DEXPERIMENTAL
	CXXFLAGS = -static -O3 -ffast-math -DEXPERIMENTAL -std=c++14 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DHAVE_MALLOC_H -DUSE_UNIX_ERRORS -DMAKEFILE_ROBOTICS
	LDFLAGS = -static
	# condor_compile is tricky with pthread so best avoided and just use vanilla universe
	# CXX = condor_compile g++
	# CC = condor_compile gcc
	CXX = g++
	CC = gcc
	LIBS = -lpthread -lm
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

# this looks for the substring csf
ifneq (,$(findstring csf,$(HOST)))
	# -static libraries are not available on csf
	CXXFLAGS = -O3 -ffast-math -DEXPERIMENTAL -std=c++14 \
	-DdIDEDOUBLE -DTRIMESH_ENABLED -DTRIMESH_OPCODE -DCCD_IDEDOUBLE -DdLIBCCD_ENABLED -DdTHREADING_INTF_DISABLED \
	-DBYTE_ORDER_LITTLE_ENDIAN -DHAVE_MALLOC_H -DUSE_UNIX_ERRORS -DMAKEFILE_CSF
	LDFLAGS =
	CXX = g++
	CC = gcc
	LIBS = -lpthread -lm
	INC_DIRS = -I../src -I../rapidxml-1.13 -I../exprtk -I../ode-0.15/ode/src -I../ode-0.15/libccd/src -I../ode-0.15/OPCODE \
	-I../ode-0.15/include -I../ann_1.1.2/include -I../fast_double_parser -I../pystring -Iinclude -I/usr/include/python3.8
endif

# vpath %.cpp src
# vpath %.c src

GAITSYMSRC = \
ArgParse.cpp\
AMotorJoint.cpp\
BallJoint.cpp\
Body.cpp\
BoxGeom.cpp\
ButterworthFilter.cpp\
CappedCylinderGeom.cpp\
Colour.cpp\
Contact.cpp\
Controller.cpp\
ConvexGeom.cpp\
CyclicDriver.cpp\
CylinderWrapStrap.cpp\
DampedSpringMuscle.cpp\
DataFile.cpp\
DataTarget.cpp\
DataTargetMarkerCompare.cpp\
DataTargetQuaternion.cpp\
DataTargetScalar.cpp\
DataTargetVector.cpp\
Drivable.cpp\
Driver.cpp\
ErrorHandler.cpp\
Filter.cpp\
FixedDriver.cpp\
FixedJoint.cpp\
FloatingHingeJoint.cpp\
FluidSac.cpp\
FluidSacIdealGas.cpp\
FluidSacIncompressible.cpp\
Geom.cpp\
Global.cpp\
HingeJoint.cpp\
Joint.cpp\
LMotorJoint.cpp\
MAMuscleComplete.cpp\
MAMuscle.cpp\
Marker.cpp\
MarkerPositionDriver.cpp\
MD5.cpp\
MovingAverage.cpp\
Muscle.cpp\
NamedObject.cpp\
NPointStrap.cpp\
ParseXML.cpp\
PCA.cpp\
PIDErrorInController.cpp\
PIDMuscleLengthController.cpp\
PlaneGeom.cpp\
RayGeom.cpp\
Reporter.cpp\
Simulation.cpp\
SliderJoint.cpp\
SphereGeom.cpp\
StackedBoxCarDriver.cpp\
StepDriver.cpp\
Strap.cpp\
SwingClearanceAbortReporter.cpp\
TegotaeDriver.cpp\
ThreeHingeJointDriver.cpp\
TwoHingeJointDriver.cpp\
TorqueReporter.cpp\
TrimeshGeom.cpp\
TwoCylinderWrapStrap.cpp\
TwoPointStrap.cpp\
UniversalJoint.cpp\
GSUtil.cpp\
Warehouse.cpp\
XMLConverter.cpp

LIBCCDSRC = \
alloc.c \
ccd.c \
mpr.c \
polytope.c \
support.c \
vec3.c

ODESRC = \
array.cpp \
box.cpp \
capsule.cpp \
collision_convex_trimesh.cpp \
collision_cylinder_box.cpp \
collision_cylinder_plane.cpp \
collision_cylinder_sphere.cpp \
collision_cylinder_trimesh.cpp \
collision_kernel.cpp \
collision_libccd.cpp \
collision_quadtreespace.cpp \
collision_sapspace.cpp \
collision_space.cpp \
collision_transform.cpp \
collision_trimesh_box.cpp \
collision_trimesh_ccylinder.cpp \
collision_trimesh_disabled.cpp \
collision_trimesh_distance.cpp \
collision_trimesh_gimpact.cpp \
collision_trimesh_opcode.cpp \
collision_trimesh_plane.cpp \
collision_trimesh_ray.cpp \
collision_trimesh_sphere.cpp \
collision_trimesh_trimesh_new.cpp \
collision_trimesh_trimesh.cpp \
collision_util.cpp \
convex.cpp \
cylinder.cpp \
error.cpp \
export-dif.cpp \
fastdot.cpp \
fastldlt.cpp \
fastlsolve.cpp \
fastltsolve.cpp \
heightfield.cpp \
lcp.cpp \
mass.cpp \
mat.cpp \
matrix.cpp \
memory.cpp \
misc.cpp \
nextafterf.c \
objects.cpp \
obstack.cpp \
ode.cpp \
odeinit.cpp \
odemath.cpp \
odeou.cpp \
odetls.cpp \
plane.cpp \
quickstep.cpp \
ray.cpp \
rotation.cpp \
sphere.cpp \
step.cpp \
threading_base.cpp \
threading_impl.cpp \
threading_pool_posix.cpp \
threading_pool_win.cpp \
timer.cpp \
util.cpp

ODEJOINTSSRC = \
amotor.cpp \
ball.cpp \
contact.cpp \
dball.cpp \
dhinge.cpp \
fixed.cpp \
floatinghinge.cpp \
hinge.cpp \
hinge2.cpp \
joint.cpp \
lmotor.cpp \
null.cpp \
piston.cpp \
plane2d.cpp \
pr.cpp \
pu.cpp \
slider.cpp \
transmission.cpp \
universal.cpp

OPCODEICESRC = \
IceAABB.cpp \
IceContainer.cpp \
IceHPoint.cpp \
IceIndexedTriangle.cpp \
IceMatrix3x3.cpp \
IceMatrix4x4.cpp \
IceOBB.cpp \
IcePlane.cpp \
IcePoint.cpp \
IceRandom.cpp \
IceRay.cpp \
IceRevisitedRadix.cpp \
IceSegment.cpp \
IceTriangle.cpp \
IceUtils.cpp

OPCODESRC = \
OPC_AABBCollider.cpp \
OPC_AABBTree.cpp \
OPC_BaseModel.cpp \
OPC_Collider.cpp \
OPC_Common.cpp \
OPC_HybridModel.cpp \
OPC_LSSCollider.cpp \
OPC_MeshInterface.cpp \
OPC_Model.cpp \
OPC_OBBCollider.cpp \
OPC_OptimizedTree.cpp \
OPC_Picking.cpp \
OPC_PlanesCollider.cpp \
OPC_RayCollider.cpp \
OPC_SphereCollider.cpp \
OPC_TreeBuilders.cpp \
OPC_TreeCollider.cpp \
OPC_VolumeCollider.cpp \
Opcode.cpp \
StdAfx.cpp

ANNSRC = \
ANN.cpp \
bd_fix_rad_search.cpp \
bd_pr_search.cpp \
bd_search.cpp \
bd_tree.cpp \
brute.cpp \
kd_dump.cpp \
kd_fix_rad_search.cpp \
kd_pr_search.cpp \
kd_search.cpp \
kd_split.cpp \
kd_tree.cpp \
kd_util.cpp \
perf.cpp

PYSTRINGSRC = \
pystring.cpp

PYBINDSRC = \
GaitSym2019PythonLibrary.cpp

GAITSYMOBJ = $(addsuffix .o, $(basename $(GAITSYMSRC) ) )
GAITSYMHEADER = $(addsuffix .h, $(basename $(GAITSYMSRC) ) ) PGDMath.h SimpleStrap.h SmartEnum.h MPIStuff.h TCPIPMessage.h

LIBCCDOBJ = $(addsuffix .o, $(basename $(LIBCCDSRC) ) )
ODEOBJ = $(addsuffix .o, $(basename $(ODESRC) ) )
ODEJOINTSOBJ = $(addsuffix .o, $(basename $(ODEJOINTSSRC) ) )
OPCODEICEOBJ = $(addsuffix .o, $(basename $(OPCODEICESRC) ) )
OPCODEOBJ = $(addsuffix .o, $(basename $(OPCODESRC) ) )
ANNOBJ = $(addsuffix .o, $(basename $(ANNSRC) ) )
PYSTRINGOBJ = $(addsuffix .o, $(basename $(PYSTRINGSRC) ) )
PYBINDOBJ = $(addsuffix .o, $(basename $(PYBINDSRC) ) )

LIBRARY = GaitSym2019.cpython-38-x86_64-linux-gnu.so

all: directories library

directories: obj

library: $(LIBRARY)

obj:
	-mkdir obj
	-mkdir obj/cl
	-mkdir obj/libccd
	-mkdir obj/ode
	-mkdir obj/odejoints
	-mkdir obj/opcodeice
	-mkdir obj/opcode
	-mkdir obj/ann
	-mkdir obj/pystring
	-mkdir obj/pybind


obj/cl/%.o : ../src/%.cpp
	$(CXX) -DUSE_CL $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/libccd/%.o : ../ode-0.15/libccd/src/%.c
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/ode/%.o : ../ode-0.15/ode/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/ode/%.o : ../ode-0.15/ode/src/%.c
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/odejoints/%.o : ../ode-0.15/ode/src/joints/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/opcodeice/%.o : ../ode-0.15/OPCODE/Ice/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/opcode/%.o : ../ode-0.15/OPCODE/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/ann/%.o : ../ann_1.1.2/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/pystring/%.o : ../pystring/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@

obj/pybind/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS) -c $< -o $@


$(LIBRARY): $(addprefix obj/cl/, $(GAITSYMOBJ) ) $(addprefix obj/libccd/, $(LIBCCDOBJ) ) $(addprefix obj/ode/, $(ODEOBJ) ) \
$(addprefix obj/odejoints/, $(ODEJOINTSOBJ) ) $(addprefix obj/opcodeice/, $(OPCODEICEOBJ) ) $(addprefix obj/opcode/, $(OPCODEOBJ) ) \
$(addprefix obj/ann/, $(ANNOBJ) ) \
$(addprefix obj/pystring/, $(PYSTRINGOBJ) )\
$(addprefix obj/pybind/, $(PYBINDOBJ) )
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
        # strip $@

clean:
	rm -rf obj
	rm -rf distribution*
	rm -rf build*


