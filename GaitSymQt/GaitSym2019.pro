#/*
# *  GaitSym2019.pro
# *  GaitSymODE2019
# *
# *  Created by Bill Sellers on 08/10/2018.
# *  Copyright 2018 Bill Sellers. All rights reserved.
# *
# */

VERSION = 2019
AUTHOR = "Bill Sellers 2019"
OBJECTS_DIR = obj
QT += opengl xml gui svg widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets
TARGET = GaitSym2019
TEMPLATE = app
CONFIG += no_batch # this gets around a bug in Visual Studio with the object_parallel_to_source option
CONFIG += object_parallel_to_source # this is important to stop obj files overwriting each other
CONFIG += c++17

macx {
    DEFINES += \
        dIDEDOUBLE dTRIMESH_ENABLED dTRIMESH_OPCODE CCD_IDEDOUBLE dLIBCCD_ENABLED dTHREADING_INTF_DISABLED \
        HAVE_ALLOCA_H BYTE_ORDER_LITTLE_ENDIAN \
        DEBUG_OUTPUT \
        EXPERIMENTAL
    INCLUDEPATH += \
        ../ann_1.1.2/include \
        ../enet-1.3.14/include \
        ../exprtk \
        ../fast_double_parser \
        ../libgwavi \
        ../ode-0.15/OPCODE \
        ../ode-0.15/include \
        ../ode-0.15/libccd/src \
        ../ode-0.15/ode/src \
        ../pystring \
        ../rapidxml-1.13 \
        ../src \
        ../tinyply
    LIBS += \
        -framework QTKit \
        -framework Cocoa \
        -framework Accelerate
    HEADERS +=
    OBJECTIVE_SOURCES +=
    ICON = GaitSymQt.icns
    QMAKE_APPLE_DEVICE_ARCHS = x86_64 arm64
}

win32 {
    RC_FILE = app.rc
    DEFINES += \
        dIDEDOUBLE dTRIMESH_ENABLED=1 dTRIMESH_OPCODE=1 dTRIMESH_GIMPACT=0 dTRIMESH_16BIT_INDICES=0 \
        CCD_IDEDOUBLE dLIBCCD_ENABLED=1 dTHREADING_INTF_DISABLED=1 dTRIMESH_OPCODE_USE_OLD_TRIMESH_TRIMESH_COLLIDER=0 \
        ICE_NO_DLL dTLS_ENABLED=0 \
        IGNORE_DLL_API \
        BYTE_ORDER_LITTLE_ENDIAN \
        HAVE_MALLOC_H USE_UNIX_ERRORS NEED_BCOPY \
        _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS _WINSOCK_DEPRECATED_NO_WARNINGS NOMINMAX \
        DEBUG_OUTPUT \
        EXPERIMENTAL
    INCLUDEPATH += \
        ../ann_1.1.2/include \
        ../enet-1.3.14/include \
        ../exprtk \
        ../fast_double_parser \
        ../libgwavi \
        ../ode-0.15/OPCODE \
        ../ode-0.15/include \
        ../ode-0.15/libccd/src \
        ../ode-0.15/ode/src \
        ../pystring \
        ../rapidxml-1.13 \
        ../src \
        ../tinyply
    LIBS += -lGdi32 -lUser32 -lAdvapi32 -lWs2_32 -lWinmm
    HEADERS +=
        win32-msvc {
        QMAKE_CXXFLAGS += -bigobj
        # QMAKE_CXXFLAGS += -openmp
        # -Od set the debugging options on, -RTCsu enables a lot of run time checks
        QMAKE_CXXFLAGS_DEBUG += -Od -RTCsu
        # -O2 is maximise speed, -fp:fast is allow non IEEE math to increase speed (like gcc -fast-math), -GL is global optimisations
        # -GL likes to have -LTCG as linker flags
        # QMAKE_CXXFLAGS_RELEASE += -O2 -fp:fast -GL
        QMAKE_CXXFLAGS_RELEASE += -O2 -GL
        QMAKE_LFLAGS_RELEASE += -LTCG
        }
        win32-g++ | win32-clang-g++ {
        DEFINES += exprtk_disable_enhanced_features
        QMAKE_CXXFLAGS += -Wa,-mbig-obj
        }

    # this line deletes the AboutDialog object file after linking which will force compile every time
    # this is necessary because I use __TIME__ and __DATE__ in the About Dialog to get a build time
    QMAKE_POST_LINK = C:\Windows\System32\cmd.exe /C "del obj\AboutDialog.obj"
}

unix:!macx {
    DEFINES += \
        dIDEDOUBLE dTRIMESH_ENABLED dTRIMESH_OPCODE CCD_IDEDOUBLE dLIBCCD_ENABLED dTHREADING_INTF_DISABLED \
        HAVE_ALLOCA_H BYTE_ORDER_LITTLE_ENDIAN \
        DEBUG_OUTPUT \
        EXPERIMENTAL
    INCLUDEPATH += \
        ../ann_1.1.2/include \
        ../enet-1.3.14/include \
        ../exprtk \
        ../fast_double_parser \
        ../libgwavi \
        ../ode-0.15/OPCODE \
        ../ode-0.15/include \
        ../ode-0.15/libccd/src \
        ../ode-0.15/ode/src \
        ../pystring \
        ../rapidxml-1.13 \
        ../src \
        ../tinyply
    LIBS += -lX11 # -lXxf86vm
    HEADERS +=
    QMAKE_CXXFLAGS_RELEASE += -O3 -ffast-math

    # this line deletes the AboutDialog object file after linking which will force compile every time
    # this is necessary because I use __TIME__ and __DATE__ in the About Dialog to get a build time
    QMAKE_POST_LINK = rm -f obj\AboutDialog.obj
}

CONFIG(debug, debug|release) {
    message(Debug build)
    DEFINES += GAITSYM_DEBUG_BUILD
}
CONFIG(release, debug|release) {
    message(Release build)
    DEFINES += dNODEBUG NDEBUG GAITSYM_RELEASE_BUILD
}

SOURCES += \
    ../ann_1.1.2/src/ANN.cpp \
    ../ann_1.1.2/src/bd_fix_rad_search.cpp \
    ../ann_1.1.2/src/bd_pr_search.cpp \
    ../ann_1.1.2/src/bd_search.cpp \
    ../ann_1.1.2/src/bd_tree.cpp \
    ../ann_1.1.2/src/brute.cpp \
    ../ann_1.1.2/src/kd_dump.cpp \
    ../ann_1.1.2/src/kd_fix_rad_search.cpp \
    ../ann_1.1.2/src/kd_pr_search.cpp \
    ../ann_1.1.2/src/kd_search.cpp \
    ../ann_1.1.2/src/kd_split.cpp \
    ../ann_1.1.2/src/kd_tree.cpp \
    ../ann_1.1.2/src/kd_util.cpp \
    ../ann_1.1.2/src/perf.cpp \
    ../enet-1.3.14/callbacks.c \
    ../enet-1.3.14/compress.c \
    ../enet-1.3.14/host.c \
    ../enet-1.3.14/list.c \
    ../enet-1.3.14/packet.c \
    ../enet-1.3.14/peer.c \
    ../enet-1.3.14/protocol.c \
    ../enet-1.3.14/unix.c \
    ../enet-1.3.14/win32.c \
    ../glextrusion/ex_angle.c \
    ../glextrusion/ex_cut_round.c \
    ../glextrusion/ex_raw.c \
    ../glextrusion/extrude.c \
    ../glextrusion/intersect.c \
    ../glextrusion/qmesh.c \
    ../glextrusion/rot_prince.c \
    ../glextrusion/rotate.c \
    ../glextrusion/round_cap.c \
    ../glextrusion/segment.c \
    ../glextrusion/texgen.c \
    ../glextrusion/urotate.c \
    ../glextrusion/view.c \
    ../libgwavi/avi-utils.c \
    ../libgwavi/fileio.c \
    ../libgwavi/gwavi.c \
    ../ode-0.15/OPCODE/Ice/IceAABB.cpp \
    ../ode-0.15/OPCODE/Ice/IceContainer.cpp \
    ../ode-0.15/OPCODE/Ice/IceHPoint.cpp \
    ../ode-0.15/OPCODE/Ice/IceIndexedTriangle.cpp \
    ../ode-0.15/OPCODE/Ice/IceMatrix3x3.cpp \
    ../ode-0.15/OPCODE/Ice/IceMatrix4x4.cpp \
    ../ode-0.15/OPCODE/Ice/IceOBB.cpp \
    ../ode-0.15/OPCODE/Ice/IcePlane.cpp \
    ../ode-0.15/OPCODE/Ice/IcePoint.cpp \
    ../ode-0.15/OPCODE/Ice/IceRandom.cpp \
    ../ode-0.15/OPCODE/Ice/IceRay.cpp \
    ../ode-0.15/OPCODE/Ice/IceRevisitedRadix.cpp \
    ../ode-0.15/OPCODE/Ice/IceSegment.cpp \
    ../ode-0.15/OPCODE/Ice/IceTriangle.cpp \
    ../ode-0.15/OPCODE/Ice/IceUtils.cpp \
    ../ode-0.15/OPCODE/OPC_AABBCollider.cpp \
    ../ode-0.15/OPCODE/OPC_AABBTree.cpp \
    ../ode-0.15/OPCODE/OPC_BaseModel.cpp \
    ../ode-0.15/OPCODE/OPC_Collider.cpp \
    ../ode-0.15/OPCODE/OPC_Common.cpp \
    ../ode-0.15/OPCODE/OPC_HybridModel.cpp \
    ../ode-0.15/OPCODE/OPC_LSSCollider.cpp \
    ../ode-0.15/OPCODE/OPC_MeshInterface.cpp \
    ../ode-0.15/OPCODE/OPC_Model.cpp \
    ../ode-0.15/OPCODE/OPC_OBBCollider.cpp \
    ../ode-0.15/OPCODE/OPC_OptimizedTree.cpp \
    ../ode-0.15/OPCODE/OPC_Picking.cpp \
    ../ode-0.15/OPCODE/OPC_PlanesCollider.cpp \
    ../ode-0.15/OPCODE/OPC_RayCollider.cpp \
    ../ode-0.15/OPCODE/OPC_SphereCollider.cpp \
    ../ode-0.15/OPCODE/OPC_TreeBuilders.cpp \
    ../ode-0.15/OPCODE/OPC_TreeCollider.cpp \
    ../ode-0.15/OPCODE/OPC_VolumeCollider.cpp \
    ../ode-0.15/OPCODE/Opcode.cpp \
    ../ode-0.15/OPCODE/StdAfx.cpp \
    ../ode-0.15/libccd/src/alloc.c \
    ../ode-0.15/libccd/src/ccd.c \
    ../ode-0.15/libccd/src/mpr.c \
    ../ode-0.15/libccd/src/polytope.c \
    ../ode-0.15/libccd/src/support.c \
    ../ode-0.15/libccd/src/vec3.c \
    ../ode-0.15/ode/src/array.cpp \
    ../ode-0.15/ode/src/box.cpp \
    ../ode-0.15/ode/src/capsule.cpp \
    ../ode-0.15/ode/src/collision_convex_trimesh.cpp \
    ../ode-0.15/ode/src/collision_cylinder_box.cpp \
    ../ode-0.15/ode/src/collision_cylinder_plane.cpp \
    ../ode-0.15/ode/src/collision_cylinder_sphere.cpp \
    ../ode-0.15/ode/src/collision_cylinder_trimesh.cpp \
    ../ode-0.15/ode/src/collision_kernel.cpp \
    ../ode-0.15/ode/src/collision_libccd.cpp \
    ../ode-0.15/ode/src/collision_quadtreespace.cpp \
    ../ode-0.15/ode/src/collision_sapspace.cpp \
    ../ode-0.15/ode/src/collision_space.cpp \
    ../ode-0.15/ode/src/collision_transform.cpp \
    ../ode-0.15/ode/src/collision_trimesh_box.cpp \
    ../ode-0.15/ode/src/collision_trimesh_ccylinder.cpp \
    ../ode-0.15/ode/src/collision_trimesh_disabled.cpp \
    ../ode-0.15/ode/src/collision_trimesh_distance.cpp \
    ../ode-0.15/ode/src/collision_trimesh_gimpact.cpp \
    ../ode-0.15/ode/src/collision_trimesh_opcode.cpp \
    ../ode-0.15/ode/src/collision_trimesh_plane.cpp \
    ../ode-0.15/ode/src/collision_trimesh_ray.cpp \
    ../ode-0.15/ode/src/collision_trimesh_sphere.cpp \
    ../ode-0.15/ode/src/collision_trimesh_trimesh.cpp \
    ../ode-0.15/ode/src/collision_trimesh_trimesh_new.cpp \
    ../ode-0.15/ode/src/collision_util.cpp \
    ../ode-0.15/ode/src/convex.cpp \
    ../ode-0.15/ode/src/cylinder.cpp \
    ../ode-0.15/ode/src/error.cpp \
    ../ode-0.15/ode/src/export-dif.cpp \
    ../ode-0.15/ode/src/fastdot.cpp \
    ../ode-0.15/ode/src/fastldlt.cpp \
    ../ode-0.15/ode/src/fastlsolve.cpp \
    ../ode-0.15/ode/src/fastltsolve.cpp \
    ../ode-0.15/ode/src/heightfield.cpp \
    ../ode-0.15/ode/src/joints/amotor.cpp \
    ../ode-0.15/ode/src/joints/ball.cpp \
    ../ode-0.15/ode/src/joints/contact.cpp \
    ../ode-0.15/ode/src/joints/dball.cpp \
    ../ode-0.15/ode/src/joints/dhinge.cpp \
    ../ode-0.15/ode/src/joints/fixed.cpp \
    ../ode-0.15/ode/src/joints/floatinghinge.cpp \
    ../ode-0.15/ode/src/joints/hinge.cpp \
    ../ode-0.15/ode/src/joints/hinge2.cpp \
    ../ode-0.15/ode/src/joints/joint.cpp \
    ../ode-0.15/ode/src/joints/lmotor.cpp \
    ../ode-0.15/ode/src/joints/null.cpp \
    ../ode-0.15/ode/src/joints/piston.cpp \
    ../ode-0.15/ode/src/joints/plane2d.cpp \
    ../ode-0.15/ode/src/joints/pr.cpp \
    ../ode-0.15/ode/src/joints/pu.cpp \
    ../ode-0.15/ode/src/joints/slider.cpp \
    ../ode-0.15/ode/src/joints/transmission.cpp \
    ../ode-0.15/ode/src/joints/universal.cpp \
    ../ode-0.15/ode/src/lcp.cpp \
    ../ode-0.15/ode/src/mass.cpp \
    ../ode-0.15/ode/src/mat.cpp \
    ../ode-0.15/ode/src/matrix.cpp \
    ../ode-0.15/ode/src/memory.cpp \
    ../ode-0.15/ode/src/misc.cpp \
    ../ode-0.15/ode/src/nextafterf.c \
    ../ode-0.15/ode/src/objects.cpp \
    ../ode-0.15/ode/src/obstack.cpp \
    ../ode-0.15/ode/src/ode.cpp \
    ../ode-0.15/ode/src/odeinit.cpp \
    ../ode-0.15/ode/src/odemath.cpp \
    ../ode-0.15/ode/src/odeou.cpp \
    ../ode-0.15/ode/src/odetls.cpp \
    ../ode-0.15/ode/src/plane.cpp \
    ../ode-0.15/ode/src/quickstep.cpp \
    ../ode-0.15/ode/src/ray.cpp \
    ../ode-0.15/ode/src/rotation.cpp \
    ../ode-0.15/ode/src/sphere.cpp \
    ../ode-0.15/ode/src/step.cpp \
    ../ode-0.15/ode/src/threading_base.cpp \
    ../ode-0.15/ode/src/threading_impl.cpp \
    ../ode-0.15/ode/src/threading_pool_posix.cpp \
    ../ode-0.15/ode/src/threading_pool_win.cpp \
    ../ode-0.15/ode/src/timer.cpp \
    ../ode-0.15/ode/src/util.cpp \
    ../pystring/pystring.cpp \
    ../src/AMotorJoint.cpp \
    ../src/ArgParse.cpp \
    ../src/BallJoint.cpp \
    ../src/Body.cpp \
    ../src/BoxGeom.cpp \
    ../src/ButterworthFilter.cpp \
    ../src/CappedCylinderGeom.cpp \
    ../src/Colour.cpp \
    ../src/Contact.cpp \
    ../src/Controller.cpp \
    ../src/ConvexGeom.cpp \
    ../src/CyclicDriver.cpp \
    ../src/CylinderWrapStrap.cpp \
    ../src/DampedSpringMuscle.cpp \
    ../src/DataFile.cpp \
    ../src/DataTarget.cpp \
    ../src/DataTargetMarkerCompare.cpp \
    ../src/DataTargetQuaternion.cpp \
    ../src/DataTargetScalar.cpp \
    ../src/DataTargetVector.cpp \
    ../src/Drivable.cpp \
    ../src/Driver.cpp \
    ../src/ErrorHandler.cpp \
    ../src/Filter.cpp \
    ../src/FixedDriver.cpp \
    ../src/FixedJoint.cpp \
    ../src/FloatingHingeJoint.cpp \
    ../src/FluidSac.cpp \
    ../src/FluidSacIdealGas.cpp \
    ../src/FluidSacIncompressible.cpp \
    ../src/GSUtil.cpp \
    ../src/Geom.cpp \
    ../src/Global.cpp \
    ../src/HingeJoint.cpp \
    ../src/Joint.cpp \
    ../src/LMotorJoint.cpp \
    ../src/MAMuscle.cpp \
    ../src/MAMuscleComplete.cpp \
    ../src/MD5.cpp \
    ../src/Marker.cpp \
    ../src/MarkerEllipseDriver.cpp \
    ../src/MarkerPositionDriver.cpp \
    ../src/MovingAverage.cpp \
    ../src/Muscle.cpp \
    ../src/NPointStrap.cpp \
    ../src/NamedObject.cpp \
    ../src/ObjectiveMain.cpp \
    ../src/PCA.cpp \
    ../src/PIDErrorInController.cpp \
    ../src/PIDMuscleLengthController.cpp \
    ../src/ParseXML.cpp \
    ../src/PlaneGeom.cpp \
    ../src/RayGeom.cpp \
    ../src/Reporter.cpp \
    ../src/Simulation.cpp \
    ../src/SliderJoint.cpp \
    ../src/SphereGeom.cpp \
    ../src/StackedBoxCarDriver.cpp \
    ../src/StepDriver.cpp \
    ../src/Strap.cpp \
    ../src/SwingClearanceAbortReporter.cpp \
    ../src/TegotaeDriver.cpp \
    ../src/ThreeHingeJointDriver.cpp \
    ../src/TwoHingeJointDriver.cpp \
    ../src/TorqueReporter.cpp \
    ../src/TrimeshGeom.cpp \
    ../src/TwoCylinderWrapStrap.cpp \
    ../src/TwoPointStrap.cpp \
    ../src/UniversalJoint.cpp \
    ../src/Warehouse.cpp \
    ../src/XMLConverter.cpp \
    AVIWriter.cpp \
    AboutDialog.cpp \
    BasicXMLSyntaxHighlighter.cpp \
    DialogAssembly.cpp \
    DialogBodyBuilder.cpp \
    DialogCreateMirrorElements.cpp \
    DialogCreateTestingDrivers.cpp \
    DialogDrivers.cpp \
    DialogGeoms.cpp \
    DialogGlobal.cpp \
    DialogInfo.cpp \
    DialogJoints.cpp \
    DialogMarkerImportExport.cpp \
    DialogMarkers.cpp \
    DialogMuscles.cpp \
    DialogOutputSelect.cpp \
    DialogPreferences.cpp \
    DialogProperties.cpp \
    DialogRename.cpp \
    DoubleValidator.cpp \
    DrawBody.cpp \
    DrawCustom.cpp \
    DrawFluidSac.cpp \
    DrawGeom.cpp \
    DrawJoint.cpp \
    DrawMarker.cpp \
    DrawMuscle.cpp \
    Drawable.cpp \
    ElementTreeWidget.cpp \
    FacetedAxes.cpp \
    FacetedBox.cpp \
    FacetedCappedCylinder.cpp \
    FacetedCheckerboard.cpp \
    FacetedConicSegment.cpp \
    FacetedObject.cpp \
    FacetedPolyline.cpp \
    FacetedRect.cpp \
    FacetedSphere.cpp \
    GLUtils.cpp \
    IntersectionHits.cpp \
    LineEditDouble.cpp \
    LineEditPath.cpp \
    LineEditUniqueName.cpp \
    MainWindow.cpp \
    MainWindowActions.cpp \
    MeshStore.cpp \
    Preferences.cpp \
    SimulationWidget.cpp \
    StrokeFont.cpp \
    TextEditDialog.cpp \
    TrackBall.cpp \
    UniqueNameValidator.cpp \
    ViewControlWidget.cpp \
    main.cpp

HEADERS += \
    ../ann_1.1.2/include/ANN/ANN.h \
    ../ann_1.1.2/include/ANN/ANNperf.h \
    ../ann_1.1.2/include/ANN/ANNx.h \
    ../ann_1.1.2/src/bd_tree.h \
    ../ann_1.1.2/src/kd_fix_rad_search.h \
    ../ann_1.1.2/src/kd_pr_search.h \
    ../ann_1.1.2/src/kd_search.h \
    ../ann_1.1.2/src/kd_split.h \
    ../ann_1.1.2/src/kd_tree.h \
    ../ann_1.1.2/src/kd_util.h \
    ../ann_1.1.2/src/pr_queue.h \
    ../ann_1.1.2/src/pr_queue_k.h \
    ../enet-1.3.14/include/enet/callbacks.h \
    ../enet-1.3.14/include/enet/enet.h \
    ../enet-1.3.14/include/enet/list.h \
    ../enet-1.3.14/include/enet/protocol.h \
    ../enet-1.3.14/include/enet/time.h \
    ../enet-1.3.14/include/enet/types.h \
    ../enet-1.3.14/include/enet/unix.h \
    ../enet-1.3.14/include/enet/utility.h \
    ../enet-1.3.14/include/enet/win32.h \
    ../exprtk/exprtk.hpp \
    ../fast_double_parser/fast_double_parser.h \
    ../glextrusion/copy.h \
    ../glextrusion/extrude.h \
    ../glextrusion/gle.h \
    ../glextrusion/intersect.h \
    ../glextrusion/port.h \
    ../glextrusion/rot.h \
    ../glextrusion/segment.h \
    ../glextrusion/tube_gc.h \
    ../glextrusion/vvector.h \
    ../libgwavi/avi-utils.h \
    ../libgwavi/fileio.h \
    ../libgwavi/gwavi.h \
    ../libgwavi/gwavi_private.h \
    ../ode-0.15/OPCODE/Ice/IceAABB.h \
    ../ode-0.15/OPCODE/Ice/IceAxes.h \
    ../ode-0.15/OPCODE/Ice/IceBoundingSphere.h \
    ../ode-0.15/OPCODE/Ice/IceContainer.h \
    ../ode-0.15/OPCODE/Ice/IceFPU.h \
    ../ode-0.15/OPCODE/Ice/IceHPoint.h \
    ../ode-0.15/OPCODE/Ice/IceIndexedTriangle.h \
    ../ode-0.15/OPCODE/Ice/IceLSS.h \
    ../ode-0.15/OPCODE/Ice/IceMatrix3x3.h \
    ../ode-0.15/OPCODE/Ice/IceMatrix4x4.h \
    ../ode-0.15/OPCODE/Ice/IceMemoryMacros.h \
    ../ode-0.15/OPCODE/Ice/IceOBB.h \
    ../ode-0.15/OPCODE/Ice/IcePairs.h \
    ../ode-0.15/OPCODE/Ice/IcePlane.h \
    ../ode-0.15/OPCODE/Ice/IcePoint.h \
    ../ode-0.15/OPCODE/Ice/IcePreprocessor.h \
    ../ode-0.15/OPCODE/Ice/IceRandom.h \
    ../ode-0.15/OPCODE/Ice/IceRay.h \
    ../ode-0.15/OPCODE/Ice/IceRevisitedRadix.h \
    ../ode-0.15/OPCODE/Ice/IceSegment.h \
    ../ode-0.15/OPCODE/Ice/IceTriList.h \
    ../ode-0.15/OPCODE/Ice/IceTriangle.h \
    ../ode-0.15/OPCODE/Ice/IceTypes.h \
    ../ode-0.15/OPCODE/Ice/IceUtils.h \
    ../ode-0.15/OPCODE/OPC_AABBCollider.h \
    ../ode-0.15/OPCODE/OPC_AABBTree.h \
    ../ode-0.15/OPCODE/OPC_BaseModel.h \
    ../ode-0.15/OPCODE/OPC_BoxBoxOverlap.h \
    ../ode-0.15/OPCODE/OPC_Collider.h \
    ../ode-0.15/OPCODE/OPC_Common.h \
    ../ode-0.15/OPCODE/OPC_HybridModel.h \
    ../ode-0.15/OPCODE/OPC_IceHook.h \
    ../ode-0.15/OPCODE/OPC_LSSAABBOverlap.h \
    ../ode-0.15/OPCODE/OPC_LSSCollider.h \
    ../ode-0.15/OPCODE/OPC_LSSTriOverlap.h \
    ../ode-0.15/OPCODE/OPC_MeshInterface.h \
    ../ode-0.15/OPCODE/OPC_Model.h \
    ../ode-0.15/OPCODE/OPC_OBBCollider.h \
    ../ode-0.15/OPCODE/OPC_OptimizedTree.h \
    ../ode-0.15/OPCODE/OPC_Picking.h \
    ../ode-0.15/OPCODE/OPC_PlanesAABBOverlap.h \
    ../ode-0.15/OPCODE/OPC_PlanesCollider.h \
    ../ode-0.15/OPCODE/OPC_PlanesTriOverlap.h \
    ../ode-0.15/OPCODE/OPC_RayAABBOverlap.h \
    ../ode-0.15/OPCODE/OPC_RayCollider.h \
    ../ode-0.15/OPCODE/OPC_RayTriOverlap.h \
    ../ode-0.15/OPCODE/OPC_Settings.h \
    ../ode-0.15/OPCODE/OPC_SphereAABBOverlap.h \
    ../ode-0.15/OPCODE/OPC_SphereCollider.h \
    ../ode-0.15/OPCODE/OPC_SphereTriOverlap.h \
    ../ode-0.15/OPCODE/OPC_TreeBuilders.h \
    ../ode-0.15/OPCODE/OPC_TreeCollider.h \
    ../ode-0.15/OPCODE/OPC_TriBoxOverlap.h \
    ../ode-0.15/OPCODE/OPC_TriTriOverlap.h \
    ../ode-0.15/OPCODE/OPC_VolumeCollider.h \
    ../ode-0.15/OPCODE/Opcode.h \
    ../ode-0.15/OPCODE/Stdafx.h \
    ../ode-0.15/include/ode/collision.h \
    ../ode-0.15/include/ode/collision_space.h \
    ../ode-0.15/include/ode/collision_trimesh.h \
    ../ode-0.15/include/ode/common.h \
    ../ode-0.15/include/ode/compatibility.h \
    ../ode-0.15/include/ode/contact.h \
    ../ode-0.15/include/ode/error.h \
    ../ode-0.15/include/ode/export-dif.h \
    ../ode-0.15/include/ode/mass.h \
    ../ode-0.15/include/ode/matrix.h \
    ../ode-0.15/include/ode/memory.h \
    ../ode-0.15/include/ode/misc.h \
    ../ode-0.15/include/ode/objects.h \
    ../ode-0.15/include/ode/ode.h \
    ../ode-0.15/include/ode/odeconfig.h \
    ../ode-0.15/include/ode/odecpp.h \
    ../ode-0.15/include/ode/odecpp_collision.h \
    ../ode-0.15/include/ode/odeinit.h \
    ../ode-0.15/include/ode/odemath.h \
    ../ode-0.15/include/ode/odemath_legacy.h \
    ../ode-0.15/include/ode/precision.h \
    ../ode-0.15/include/ode/rotation.h \
    ../ode-0.15/include/ode/threading.h \
    ../ode-0.15/include/ode/threading_impl.h \
    ../ode-0.15/include/ode/timer.h \
    ../ode-0.15/include/ode/version.h \
    ../ode-0.15/libccd/src/ccd/alloc.h \
    ../ode-0.15/libccd/src/ccd/ccd.h \
    ../ode-0.15/libccd/src/ccd/compiler.h \
    ../ode-0.15/libccd/src/ccd/dbg.h \
    ../ode-0.15/libccd/src/ccd/list.h \
    ../ode-0.15/libccd/src/ccd/polytope.h \
    ../ode-0.15/libccd/src/ccd/precision.h \
    ../ode-0.15/libccd/src/ccd/quat.h \
    ../ode-0.15/libccd/src/ccd/simplex.h \
    ../ode-0.15/libccd/src/ccd/support.h \
    ../ode-0.15/libccd/src/ccd/vec3.h \
    ../ode-0.15/ode/src/array.h \
    ../ode-0.15/ode/src/collision_kernel.h \
    ../ode-0.15/ode/src/collision_libccd.h \
    ../ode-0.15/ode/src/collision_space_internal.h \
    ../ode-0.15/ode/src/collision_std.h \
    ../ode-0.15/ode/src/collision_transform.h \
    ../ode-0.15/ode/src/collision_trimesh_colliders.h \
    ../ode-0.15/ode/src/collision_trimesh_internal.h \
    ../ode-0.15/ode/src/collision_util.h \
    ../ode-0.15/ode/src/config.h \
    ../ode-0.15/ode/src/error.h \
    ../ode-0.15/ode/src/fastdot_impl.h \
    ../ode-0.15/ode/src/fastldlt_impl.h \
    ../ode-0.15/ode/src/fastltsolve_impl.h \
    ../ode-0.15/ode/src/fastsolve_impl.h \
    ../ode-0.15/ode/src/heightfield.h \
    ../ode-0.15/ode/src/joints/amotor.h \
    ../ode-0.15/ode/src/joints/ball.h \
    ../ode-0.15/ode/src/joints/contact.h \
    ../ode-0.15/ode/src/joints/dball.h \
    ../ode-0.15/ode/src/joints/dhinge.h \
    ../ode-0.15/ode/src/joints/fixed.h \
    ../ode-0.15/ode/src/joints/floatinghinge.h \
    ../ode-0.15/ode/src/joints/hinge.h \
    ../ode-0.15/ode/src/joints/hinge2.h \
    ../ode-0.15/ode/src/joints/joint.h \
    ../ode-0.15/ode/src/joints/joint_internal.h \
    ../ode-0.15/ode/src/joints/joints.h \
    ../ode-0.15/ode/src/joints/lmotor.h \
    ../ode-0.15/ode/src/joints/null.h \
    ../ode-0.15/ode/src/joints/piston.h \
    ../ode-0.15/ode/src/joints/plane2d.h \
    ../ode-0.15/ode/src/joints/pr.h \
    ../ode-0.15/ode/src/joints/pu.h \
    ../ode-0.15/ode/src/joints/slider.h \
    ../ode-0.15/ode/src/joints/transmission.h \
    ../ode-0.15/ode/src/joints/universal.h \
    ../ode-0.15/ode/src/lcp.h \
    ../ode-0.15/ode/src/mat.h \
    ../ode-0.15/ode/src/matrix.h \
    ../ode-0.15/ode/src/matrix_impl.h \
    ../ode-0.15/ode/src/objects.h \
    ../ode-0.15/ode/src/obstack.h \
    ../ode-0.15/ode/src/odemath.h \
    ../ode-0.15/ode/src/odeou.h \
    ../ode-0.15/ode/src/odetls.h \
    ../ode-0.15/ode/src/quickstep.h \
    ../ode-0.15/ode/src/step.h \
    ../ode-0.15/ode/src/threading_atomics_provs.h \
    ../ode-0.15/ode/src/threading_base.h \
    ../ode-0.15/ode/src/threading_fake_sync.h \
    ../ode-0.15/ode/src/threading_impl.h \
    ../ode-0.15/ode/src/threading_impl_posix.h \
    ../ode-0.15/ode/src/threading_impl_templates.h \
    ../ode-0.15/ode/src/threading_impl_win.h \
    ../ode-0.15/ode/src/threadingutils.h \
    ../ode-0.15/ode/src/typedefs.h \
    ../ode-0.15/ode/src/util.h \
    ../pystring/pystring.h \
    ../rapidxml-1.13/rapidxml.hpp \
    ../rapidxml-1.13/rapidxml_iterators.hpp \
    ../rapidxml-1.13/rapidxml_print.hpp \
    ../rapidxml-1.13/rapidxml_utils.hpp \
    ../src/AMotorJoint.h \
    ../src/ArgParse.h \
    ../src/BallJoint.h \
    ../src/Body.h \
    ../src/BoxGeom.h \
    ../src/ButterworthFilter.h \
    ../src/CappedCylinderGeom.h \
    ../src/Colour.h \
    ../src/Contact.h \
    ../src/Controller.h \
    ../src/ConvexGeom.h \
    ../src/CyclicDriver.h \
    ../src/CylinderWrapStrap.h \
    ../src/DampedSpringMuscle.h \
    ../src/DataFile.h \
    ../src/DataTarget.h \
    ../src/DataTargetMarkerCompare.h \
    ../src/DataTargetQuaternion.h \
    ../src/DataTargetScalar.h \
    ../src/DataTargetVector.h \
    ../src/Drivable.h \
    ../src/Driver.h \
    ../src/ErrorHandler.h \
    ../src/Filter.h \
    ../src/FixedDriver.h \
    ../src/FixedJoint.h \
    ../src/FloatingHingeJoint.h \
    ../src/FluidSac.h \
    ../src/FluidSacIdealGas.h \
    ../src/FluidSacIncompressible.h \
    ../src/GSUtil.h \
    ../src/Geom.h \
    ../src/Global.h \
    ../src/HingeJoint.h \
    ../src/Joint.h \
    ../src/LMotorJoint.h \
    ../src/MAMuscle.h \
    ../src/MAMuscleComplete.h \
    ../src/MD5.h \
    ../src/MPIStuff.h \
    ../src/Marker.h \
    ../src/MarkerEllipseDriver.h \
    ../src/MarkerPositionDriver.h \
    ../src/MovingAverage.h \
    ../src/Muscle.h \
    ../src/NPointStrap.h \
    ../src/NamedObject.h \
    ../src/ObjectiveMain.h \
    ../src/PCA.h \
    ../src/PGDMath.h \
    ../src/PIDErrorInController.h \
    ../src/PIDMuscleLengthController.h \
    ../src/ParseXML.h \
    ../src/PlaneGeom.h \
    ../src/RayGeom.h \
    ../src/Reporter.h \
    ../src/SimpleStrap.h \
    ../src/Simulation.h \
    ../src/SliderJoint.h \
    ../src/SmartEnum.h \
    ../src/SphereGeom.h \
    ../src/StackedBoxCarDriver.h \
    ../src/StepDriver.h \
    ../src/Strap.h \
    ../src/SwingClearanceAbortReporter.h \
    ../src/TegotaeDriver.h \
    ../src/ThreeHingeJointDriver.h \
    ../src/TwoHingeJointDriver.h \
    ../src/TorqueReporter.h \
    ../src/TrimeshGeom.h \
    ../src/TwoCylinderWrapStrap.h \
    ../src/TwoPointStrap.h \
    ../src/UniversalJoint.h \
    ../src/Warehouse.h \
    ../src/XMLConverter.h \
    ../tinyply/tinyply.h \
    AVIWriter.h \
    AboutDialog.h \
    BasicXMLSyntaxHighlighter.h \
    DialogAssembly.h \
    DialogBodyBuilder.h \
    DialogCreateMirrorElements.h \
    DialogCreateTestingDrivers.h \
    DialogDrivers.h \
    DialogGeoms.h \
    DialogGlobal.h \
    DialogInfo.h \
    DialogJoints.h \
    DialogMarkerImportExport.h \
    DialogMarkers.h \
    DialogMuscles.h \
    DialogOutputSelect.h \
    DialogPreferences.h \
    DialogProperties.h \
    DialogRename.h \
    DoubleValidator.h \
    DrawBody.h \
    DrawCustom.h \
    DrawFluidSac.h \
    DrawGeom.h \
    DrawJoint.h \
    DrawMarker.h \
    DrawMuscle.h \
    Drawable.h \
    ElementTreeWidget.h \
    FacetedAxes.h \
    FacetedBox.h \
    FacetedCappedCylinder.h \
    FacetedCheckerboard.h \
    FacetedConicSegment.h \
    FacetedObject.h \
    FacetedPolyline.h \
    FacetedRect.h \
    FacetedSphere.h \
    GLUtils.h \
    IntersectionHits.h \
    LineEditDouble.h \
    LineEditPath.h \
    LineEditUniqueName.h \
    MainWindow.h \
    MainWindowActions.h \
    MeshStore.h \
    Preferences.h \
    SimulationWidget.h \
    StrokeFont.h \
    TextEditDialog.h \
    TrackBall.h \
    UniqueNameValidator.h \
    ViewControlWidget.h

FORMS += \
    AboutDialog.ui \
    DialogAssembly.ui \
    DialogBodyBuilder.ui \
    DialogCreateMirrorElements.ui \
    DialogCreateTestingDrivers.ui \
    DialogDrivers.ui \
    DialogGeoms.ui \
    DialogGlobal.ui \
    DialogInfo.ui \
    DialogJoints.ui \
    DialogMarkerImportExport.ui \
    DialogMarkers.ui \
    DialogMuscles.ui \
    DialogOutputSelect.ui \
    DialogPreferences.ui \
    DialogProperties.ui \
    DialogRename.ui \
    MainWindow.ui \
    TextEditDialog.ui

RESOURCES += resources.qrc

OTHER_FILES += \
    app.rc \
    Icon.ico

DISTFILES += \
    ../Ideas.txt \
    ../makefile \
    ../ode-0.15/ode/src/Makefile.am \
    ../scripts/apply_genome.py \
    ../scripts/conditional_attrib_edit.py \
    ../scripts/convert_obj_to_sac.py \
    ../scripts/convert_obj_to_tri.py \
    ../scripts/convert_old_model.py \
    ../scripts/extract_muscles_by_joint.py \
    ../scripts/movies_to_ppt_compatible_mp4.py \
    ../scripts/strip_ifdef.py \
    DeferredRenderer/final_gl2.frag \
    DeferredRenderer/final_gl2.vert \
    DeferredRenderer/final_gl3.frag \
    DeferredRenderer/final_gl3.vert \
    DeferredRenderer/geometry_gl2.frag \
    DeferredRenderer/geometry_gl2.vert \
    DeferredRenderer/geometry_gl3.frag \
    DeferredRenderer/geometry_gl3.vert \
    astyle_project.txt \
    opengl/fragment_shader.glsl \
    opengl/fragment_shader_2.glsl \
    opengl/vertex_shader.glsl \
    opengl/vertex_shader_2.glsl



