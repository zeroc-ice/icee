// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Outgoing.h>
#include <IceE/RequestHandler.h>
#include <IceE/ConnectionI.h>
#include <IceE/Reference.h>
#include <IceE/Endpoint.h>
#include <IceE/LocalException.h>
#include <IceE/Protocol.h>
#include <IceE/Instance.h>
#include <IceE/ReplyStatus.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::LocalExceptionWrapper::LocalExceptionWrapper(const LocalException& ex, bool r) :
    _retry(r)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
}

IceInternal::LocalExceptionWrapper::LocalExceptionWrapper(const LocalExceptionWrapper& ex) :
    _retry(ex._retry)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.get()->ice_clone()));
}

void
IceInternal::LocalExceptionWrapper::throwWrapper(const std::exception& ex)
{
    const UserException* ue = dynamic_cast<const UserException*>(&ex);
    if(ue)
    {
        throw LocalExceptionWrapper(UnknownUserException(__FILE__, __LINE__, ue->toString()), false);
    }

    const LocalException* le = dynamic_cast<const LocalException*>(&ex);
    if(le)
    {
        if(dynamic_cast<const UnknownException*>(le) ||
           dynamic_cast<const ObjectNotExistException*>(le) ||
           dynamic_cast<const OperationNotExistException*>(le) ||
           dynamic_cast<const FacetNotExistException*>(le))
        {
            throw LocalExceptionWrapper(*le, false);
        }
        throw LocalExceptionWrapper(UnknownLocalException(__FILE__, __LINE__, le->toString()), false);
    }
    string msg = "std::exception: ";
    throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, msg + ex.what()), false);
}

const LocalException*
IceInternal::LocalExceptionWrapper::get() const
{
    assert(_ex.get());
    return _ex.get();
}

bool
IceInternal::LocalExceptionWrapper::retry() const
{
    return _retry;
}

IceInternal::Outgoing::Outgoing(RequestHandler* handler, Reference* reference, const string& operation,
                                OperationMode mode, const Context* context) :
    _handler(handler),
    _reference(reference),
    _state(StateUnsent),
    _is(reference->getInstance().get(), reference->getInstance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
        , reference->getInstance()->initializationData().stringConverter,
        reference->getInstance()->initializationData().wstringConverter
#endif
       ),
    _os(reference->getInstance().get(), reference->getInstance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
        , reference->getInstance()->initializationData().stringConverter,
        reference->getInstance()->initializationData().wstringConverter
#endif
       ),
    _sent(false)
{
    switch(_reference->getMode())
    {
        case ReferenceModeTwoway:
        case ReferenceModeOneway:
        {
            _os.writeBlob(requestHdr, sizeof(requestHdr));
            break;
        }

        case ReferenceModeBatchOneway:
        {
#ifdef ICEE_HAS_BATCH
            _handler->prepareBatchRequest(&_os);
            break;
#else
            throw FeatureNotSupportedException(__FILE__, __LINE__, "batch proxy mode");
#endif
        }

	case ReferenceModeDatagram:
	case ReferenceModeBatchDatagram:
	{
            throw FeatureNotSupportedException(__FILE__, __LINE__, "datagram proxy");
	    break;
	}
    }

    try
    {
        _reference->getIdentity().__write(&_os);

        //
        // For compatibility with the old FacetPath.
        //
        if(_reference->getFacet().empty())
        {
            _os.write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = _reference->getFacet();
            _os.write(&facet, &facet + 1);
        }

        _os.write(operation, false);

        _os.write(static_cast<Byte>(mode));

        if(context == 0)
        {
            context = _reference->getContext();
        }

        _os.writeSize(Int(context->size()));
        Context::const_iterator p;
        for(p = context->begin(); p != context->end(); ++p)
        {
            _os.write(p->first);
            _os.write(p->second);
        }

        //
        // Input and output parameters are always sent in an
        // encapsulation, which makes it possible to forward requests as
        // blobs.
        //
        _os.startWriteEncaps();
    }
    catch(const LocalException& ex)
    {
        abort(ex);
    }
}

bool
IceInternal::Outgoing::invoke()
{
    assert(_state == StateUnsent);

    _os.endWriteEncaps();

    switch(_reference->getMode())
    {
        case ReferenceModeTwoway:
        {
            _state = StateInProgress;

            Ice::ConnectionI* connection = _handler->sendRequest(this, true);
            assert(connection);

            bool timedOut = false;

            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

                //
                // If the request is being sent in the background we first wait for the
                // sent notification.
                //
                while(_state != StateFailed && !_sent)
                {
                    _monitor.wait();
                }

                //
                // Wait until the request has completed, or until the request times out.
                //

                Int timeout = connection->timeout();
                while(_state == StateInProgress && !timedOut)
                {
                    if(timeout >= 0)
                    {
                        _monitor.timedWait(IceUtil::Time::milliSeconds(timeout));

                        if(_state == StateInProgress)
                        {
                            timedOut = true;
                        }
                    }
                    else
                    {
                        _monitor.wait();
                    }
                }
            }

            if(timedOut)
            {
                //
                // Must be called outside the synchronization of this
                // object.
                //
                connection->exception(TimeoutException(__FILE__, __LINE__));

                //
                // We must wait until the exception set above has
                // propagated to this Outgoing object.
                //
                {
                    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                    while(_state == StateInProgress)
                    {
                        _monitor.wait();
                    }
                }
            }

            if(_exception.get())
            {
                //
                // A CloseConnectionException indicates graceful
                // server shutdown, and is therefore always repeatable
                // without violating "at-most-once". That's because by
                // sending a close connection message, the server
                // guarantees that all outstanding requests can safely
                // be repeated.
                //
                // An ObjectNotExistException can always be retried as
                // well without violating "at-most-once" (see the
                // implementation of the checkRetryAfterException
                // method of the ProxyFactory class for the reasons
                // why it can be useful).
                //
                if(!_sent ||
                   dynamic_cast<CloseConnectionException*>(_exception.get()) ||
                   dynamic_cast<ObjectNotExistException*>(_exception.get()))
                {
                    _exception->ice_throw();
                }

                //
                // Throw the exception wrapped in a LocalExceptionWrapper,
                // to indicate that the request cannot be resent without
                // potentially violating the "at-most-once" principle.
                //
                throw LocalExceptionWrapper(*_exception.get(), false);
            }

            if(_state == StateUserException)
            {
                return false;
            }
            else
            {
                assert(_state == StateOK);
                return true;
            }
        }

        case ReferenceModeOneway:
        {
            _state = StateInProgress;
            if(_handler->sendRequest(this, false))
            {
                //
                // If the handler returns the connection, we must wait for the sent callback.
                //
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                while(_state != StateFailed && !_sent)
                {
                    _monitor.wait();
                }

                if(_exception.get())
                {
                    assert(!_sent);
                    _exception->ice_throw();
                }
            }
            return true;
        }

        case ReferenceModeBatchOneway:
        {
#ifdef ICEE_HAS_BATCH
            //
            // For batch oneways and datagrams, the same rules as for
            // regular oneways and datagrams (see comment above)
            // apply.
            //
            _state = StateInProgress;
            _handler->finishBatchRequest(&_os);
            return true;
#endif
        }

        case ReferenceModeDatagram:
        case ReferenceModeBatchDatagram:
        {
            assert(false);
            break;
        }
    }

    assert(false);
    return false;
}

void
IceInternal::Outgoing::abort(const LocalException& ex)
{
    assert(_state == StateUnsent);

#ifdef ICEE_HAS_BATCH
    //
    // If we didn't finish a batch oneway or datagram request, we must
    // notify the connection about that we give up ownership of the
    // batch stream.
    //
    if(_reference->getMode() == ReferenceModeBatchOneway ||
       _reference->getMode() == ReferenceModeBatchDatagram)
    {
        _handler->abortBatchRequest();
    }
#endif

    ex.ice_throw();
}

void
IceInternal::Outgoing::sent(bool notify)
{
    if(notify)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _sent = true;
        _monitor.notify();
    }
    else
    {
        //
        // No synchronization is necessary if called from sendRequest() because the connection
        // send mutex is locked and no other threads can call on Outgoing until it's released.
        //
        _sent = true;
    }
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    assert(_reference->getMode() == ReferenceModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);

    _is.swap(is);
    Byte replyStatus;
    _is.read(replyStatus);

    switch(replyStatus)
    {
        case replyOK:
        {
            _is.startReadEncaps();
            _state = StateOK; // The state must be set last, in case there is an exception.
            break;
        }

        case replyUserException:
        {
            _is.startReadEncaps();
            _state = StateUserException; // The state must be set last, in case there is an exception.
            break;
        }

        case replyObjectNotExist:
        case replyFacetNotExist:
        case replyOperationNotExist:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            Identity ident;
            ident.__read(&_is);

            //
            // For compatibility with the old FacetPath.
            //
            vector<string> facetPath;
            _is.read(facetPath);
            string facet;
            if(!facetPath.empty())
            {
                if(facetPath.size() > 1)
                {
                    throw MarshalException(__FILE__, __LINE__);
                }
                facet.swap(facetPath[0]);
            }

            string operation;
            _is.read(operation, false);

            RequestFailedException* ex;
            switch(replyStatus)
            {
                case replyObjectNotExist:
                {
                    ex = new ObjectNotExistException(__FILE__, __LINE__);
                    break;
                }

                case replyFacetNotExist:
                {
                    ex = new FacetNotExistException(__FILE__, __LINE__);
                    break;
                }

                case replyOperationNotExist:
                {
                    ex = new OperationNotExistException(__FILE__, __LINE__);
                    break;
                }

                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }

            ex->id = ident;
            ex->facet = facet;
            ex->operation = operation;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }

        case replyUnknownException:
        case replyUnknownLocalException:
        case replyUnknownUserException:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            string unknown;
            _is.read(unknown, false);

            UnknownException* ex;
            switch(replyStatus)
            {
                case replyUnknownException:
                {
                    ex = new UnknownException(__FILE__, __LINE__);
                    break;
                }

                case replyUnknownLocalException:
                {
                    ex = new UnknownLocalException(__FILE__, __LINE__);
                    break;
                }

                case replyUnknownUserException:
                {
                    ex = new UnknownUserException(__FILE__, __LINE__);
                    break;
                }

                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }

            ex->unknown = unknown;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }

        default:
        {
            //_exception.reset(new UnknownReplyStatusException(__FILE__, __LINE__));
            _exception.reset(new ProtocolException(__FILE__, __LINE__, "unknown reply status"));
            _state = StateLocalException;
            break;
        }
    }

    _monitor.notify();
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    assert(_state <= StateInProgress);

    _state = StateFailed;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
    _monitor.notify();
}

#ifdef ICEE_HAS_BATCH

IceInternal::BatchOutgoing::BatchOutgoing(RequestHandler* handler, Instance* instance) :
    _handler(handler),
    _sent(false),
    _os(instance, instance->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
        , instance->initializationData().stringConverter,
        instance->initializationData().wstringConverter
#endif
       )
{
}

void
IceInternal::BatchOutgoing::invoke()
{
    assert(_handler);
    if(_handler && !_handler->flushBatchRequests(this))
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        while(!_exception.get() && !_sent)
        {
            _monitor.wait();
        }

        if(_exception.get())
        {
            assert(!_sent);
            _exception->ice_throw();
        }
    }
}

void
IceInternal::BatchOutgoing::sent(bool notify)
{
    if(notify)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _sent = true;
        _monitor.notify();
    }
    else
    {
        _sent = true;
    }
}

void
IceInternal::BatchOutgoing::finished(const Ice::LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
    _monitor.notify();
}

#endif
