# **********************************************************************
#
# Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
#
# This copy of Ice-E is licensed to you under the terms described in the
# ICEE_LICENSE file included in this distribution.
#
# **********************************************************************

top_srcdir	= ..\..

LIBNAME		= $(top_srcdir)\lib\icee$(LIBSUFFIX).lib
DLLNAME		= $(top_srcdir)\bin\icee$(SOVERSION)$(LIBSUFFIX).dll

TARGETS		= $(LIBNAME) $(DLLNAME)

TRANSPORT_DIR   = $(top_srcdir)\src\TcpTransport

TRANSPORT_OBJS  = Acceptor.obj \
                  Connector.obj \
                  EndpointFactory.obj \
                  TcpEndpoint.obj \
                  Transceiver.obj

LOCAL_OBJS      = Base64.obj \
		  BasicStream.obj \
                  Buffer.obj \
                  BuiltinSequences.obj \
                  Communicator.obj \
                  Cond.obj \
                  ConnectionI.obj \
		  ConnectRequestHandler.obj \
		  RequestHandler.obj \
                  ConvertUTF.obj \
                  Current.obj \
                  DefaultsAndOverrides.obj \
		  DispatchInterceptor.obj \
                  Endpoint.obj \
		  EventHandler.obj \
                  ExceptionBase.obj \
                  FactoryTable.obj \
                  FactoryTableDef.obj \
                  Identity.obj \
                  Incoming.obj \
                  IncomingConnectionFactory.obj \
                  Initialize.obj \
                  Instance.obj \
                  LocalException.obj \
                  Locator.obj \
                  LocatorInfo.obj \
                  Logger.obj \
                  LoggerI.obj \
                  LoggerUtil.obj \
		  MutexProtocol.obj \
                  Network.obj \
                  Object.obj \
                  ObjectAdapter.obj \
                  ObjectAdapterFactory.obj \
		  ObjectFactoryManager.obj \
		  ObjectFactoryManagerI.obj \
                  OperationMode.obj \
                  Outgoing.obj \
		  OutgoingAsync.obj \
                  OutgoingConnectionFactory.obj \
                  Properties.obj \
                  Protocol.obj \
                  Proxy.obj \
                  ProxyFactory.obj \
		  Random.obj \
                  RecMutex.obj \
                  Reference.obj \
                  ReferenceFactory.obj \
		  RetryQueue.obj \
                  Router.obj \
                  RouterInfo.obj \
                  SafeStdio.obj \
		  SelectorThread.obj \
                  ServantManager.obj \
                  Shared.obj \
                  StringConverter.obj \
                  StringUtil.obj \
                  Thread.obj \
                  ThreadException.obj \
		  ThreadPool.obj \
                  Time.obj \
		  Timer.obj \
                  TraceLevels.obj \
                  TraceUtil.obj \
                  UnknownEndpoint.obj \
                  Unicode.obj \
                  UUID.obj

SRCS		= $(LOCAL_OBJS:.obj=.cpp) \
		  $(TRANSPORT_DIR)\Acceptor.cpp \
                  $(TRANSPORT_DIR)\Connector.cpp \
                  $(TRANSPORT_DIR)\EndpointFactory.cpp \
                  $(TRANSPORT_DIR)\TcpEndpoint.cpp \
                  $(TRANSPORT_DIR)\Transceiver.cpp

HDIR		= $(headerdir)\IceE
SDIR		= $(slicedir)\IceE

!include $(top_srcdir)\config\Make.rules.mak

CPPFLAGS	= -I.. $(CPPFLAGS) -DICE_API_EXPORTS -DFD_SETSIZE=1024 -WX -DWIN32_LEAN_AND_MEAN
SLICE2CPPEFLAGS	= --ice --include-dir IceE --dll-export ICE_API $(SLICE2CPPEFLAGS)

!if "$(STATICLIBS)" != "yes" && "$(OPTIMIZE_SPEED)" != "yes" && "$(OPTIMIZE_SIZE)" != "yes" 
PDBFLAGS        = /pdb:$(DLLNAME:.dll=.pdb)
!endif

{$(TRANSPORT_DIR)\}.cpp.obj::
    $(CXX) /c $(CPPFLAGS) $(CXXFLAGS) $<

!if "$(STATICLIBS)" == "yes"

$(DLLNAME): 

$(LIBNAME): $(LOCAL_OBJS) $(TRANSPORT_OBJS)
	$(AR) $(ARFLAGS) $(PDBFLAGS) $(LOCAL_OBJS) $(TRANSPORT_OBJS) /out:$(LIBNAME)

!else

$(LIBNAME): $(DLLNAME)

$(DLLNAME): $(LOCAL_OBJS) $(TRANSPORT_OBJS)
	$(LINK) $(LDFLAGS) /dll $(PDBFLAGS) $(LOCAL_OBJS) $(TRANSPORT_OBJS) /out:$(DLLNAME) $(BASELIBS)
	move $(DLLNAME:.dll=.lib) $(LIBNAME)
	@if exist $@.manifest echo ^ ^ ^ Embedding manifest using $(MT) && \
	    $(MT) -nologo -manifest $@.manifest -outputresource:$@;#2 && del /q $@.manifest
	@if exist $(DLLNAME:.dll=.exp) del /q $(DLLNAME:.dll=.exp)

!endif

clean::
	del /q BuiltinSequences.cpp $(HDIR)\BuiltinSequences.h
	del /q Identity.cpp $(HDIR)\Identity.h
	del /q LocatorF.cpp $(HDIR)\LocatorF.h
	del /q Locator.cpp $(HDIR)\Locator.h
	del /q RouterF.cpp $(HDIR)\RouterF.h
	del /q Router.cpp $(HDIR)\Router.h
	del /q $(LIBNAME:.lib=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)

!if "$(STATICLIBS)" != "yes"

clean::
	del /q $(DLLNAME:.dll=.*)

install:: all
	copy $(LIBNAME) $(install_libdir)
	copy $(DLLNAME) $(install_bindir)

!endif

!include .depend
