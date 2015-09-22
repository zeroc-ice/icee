// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "stdafx.h"
#include "HelloClient.h"
#include "HelloClientDlg.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#ifdef _WIN32_WCE
#  include <initguid.h>
#  include <connmgr.h>
#endif

#define WM_AMI_EXCEPTION                      (WM_USER + 1)
#define WM_AMI_SAY_HELLO_RESPONSE             (WM_USER + 2)
#define WM_AMI_SAY_HELLO_SENT                 (WM_USER + 3)
#define WM_AMI_FLUSH_BATCH_REQUESTS_SENT      (WM_USER + 4)
#define WM_AMI_SHUTDOWN_SENT                  (WM_USER + 5)

using namespace std;
using namespace Demo;

class SayHelloCB : public AMI_Hello_sayHello, public Ice::AMISentCallback
{
public:

    SayHelloCB(CHelloClientDlg* dialog) : _dialog(dialog)
    {
    }

    virtual void
    ice_sent()
    {
        _dialog->PostMessage(WM_AMI_SAY_HELLO_SENT, 0, 0);
    }

    virtual void
    ice_response()
    {
        _dialog->PostMessage(WM_AMI_SAY_HELLO_RESPONSE, 0, 0);
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        _dialog->PostMessage(WM_AMI_EXCEPTION, 0, reinterpret_cast<LONG>(ex.ice_clone()));
    }

private:

    CHelloClientDlg* _dialog;
};

#ifdef ICEE_HAS_BATCH
class FlushBatchRequestsCB : public Ice::AMI_Object_ice_flushBatchRequests, public Ice::AMISentCallback
{
public:
    
    FlushBatchRequestsCB(CHelloClientDlg* dialog) : _dialog(dialog)
    {
    }

    virtual void
    ice_sent()
    {
        _dialog->PostMessage(WM_AMI_FLUSH_BATCH_REQUESTS_SENT, 0, 0);
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        _dialog->PostMessage(WM_AMI_EXCEPTION, 0, reinterpret_cast<LONG>(ex.ice_clone()));
    }

private:

    CHelloClientDlg* _dialog;
};
#endif

class ShutdownCB : public AMI_Hello_shutdown, public Ice::AMISentCallback
{
public:
    
    ShutdownCB(CHelloClientDlg* dialog) : _dialog(dialog)
    {
    }

    virtual void
    ice_sent()
    {
        _dialog->PostMessage(WM_AMI_SHUTDOWN_SENT, 0, 0);
    }

    virtual void
    ice_response()
    {
    }

    virtual void
    ice_exception(const Ice::Exception& ex)
    {
        _dialog->PostMessage(WM_AMI_EXCEPTION, 0, reinterpret_cast<LONG>(ex.ice_clone()));
    }

private:

    CHelloClientDlg* _dialog;
};

CHelloClientDlg::CHelloClientDlg(const Ice::CommunicatorPtr& communicator, CWnd* pParent /*=NULL*/) :
    CDialog(CHelloClientDlg::IDD, pParent), 
    _communicator(communicator), 
    _currentMode(0),
    _useTimeout(false),
    _connection(INVALID_HANDLE_VALUE)
{
    _hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void
CHelloClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHelloClientDlg, CDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_INVOKE, OnSayHello)
    ON_BN_CLICKED(IDC_FLUSH, OnFlush)
    ON_BN_CLICKED(IDC_SHUTDOWN, OnShutdown)
    ON_MESSAGE(WM_AMI_EXCEPTION, OnAMIException)
    ON_MESSAGE(WM_AMI_SAY_HELLO_RESPONSE, OnAMISayHelloResponse)
    ON_MESSAGE(WM_AMI_SAY_HELLO_SENT, OnAMISayHelloSent)
    ON_MESSAGE(WM_AMI_FLUSH_BATCH_REQUESTS_SENT, OnAMIFlushBatchRequestsSent)
    ON_MESSAGE(WM_AMI_SHUTDOWN_SENT, OnAMIShutdownSent)
END_MESSAGE_MAP()

BOOL
CHelloClientDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(_hIcon, TRUE);            // Set big icon
    SetIcon(_hIcon, FALSE);        // Set small icon

    //
    // Retrieve the controls.
    //
    _host = (CEdit*)GetDlgItem(IDC_HOST);
    _mode = (CComboBox*)GetDlgItem(IDC_MODE);
    _timeout = (CButton*)GetDlgItem(IDC_TIMEOUT);
    _delay = (CButton*)GetDlgItem(IDC_DELAY);
    _status = (CStatic*)GetDlgItem(IDC_STATUSBAR);

    //
    // Use twoway mode as the initial default.
    //
    _mode->SetCurSel(_currentMode);

    //
    // Disable flush button if built without batch support.
    //
#ifndef ICEE_HAS_BATCH
    (CButton*)GetDlgItem(IDC_FLUSH)->EnableWindow(FALSE);
#endif

    //
    // Create the proxy.
    //
    updateProxy();
    _host->SetWindowText(CString(_hostname.c_str()));

#ifdef _WIN32_WCE
    CONNMGR_CONNECTIONINFO info = { 0 };
    info.cbSize = sizeof(info);
    info.dwParams = CONNMGR_PARAM_GUIDDESTNET;
    info.dwPriority = CONNMGR_PRIORITY_USERINTERACTIVE;
    info.guidDestNet = IID_DestNetInternet;
    if(ConnMgrEstablishConnection(&info, &_connection) != S_OK)
    {
        AfxMessageBox(CString("Couldn't establish internet connection"), MB_OK|MB_ICONEXCLAMATION);
    }
#endif

    _status->SetWindowText(CString(" Ready"));
    return TRUE;  // return TRUE unless you set the focus to a control
}

void
CHelloClientDlg::OnClose()
{
#ifdef _WIN32_WCE
    if(_connection != INVALID_HANDLE_VALUE)
    {
        ConnMgrReleaseConnection(_connection, true);
    }
#endif

    //
    // Destroy the communicator. If AMI calls are still in progress they will be
    // interrupted with an Ice::CommunicatorDestroyedException.
    //
    try
    {
        _communicator->destroy();
    }
    catch(const IceUtil::Exception&)
    {
    }
}

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void
CHelloClientDlg::OnPaint() 
{
#ifdef _WIN32_WCE
    CDialog::OnPaint();
#else
    if(IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, _hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
#endif
}

// The system calls this function to obtain the cursor to display while the user drags
// the minimized window.
HCURSOR
CHelloClientDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(_hIcon);
}

void
CHelloClientDlg::OnSayHello()
{
    try
    {
        updateProxy();
#ifdef ICEE_HAS_BATCH
        if(!_currentProxy->ice_isBatchOneway() && !_currentProxy->ice_isBatchDatagram())
        {
#endif
            if(_currentProxy->sayHello_async(new SayHelloCB(this), _delay->GetCheck() == BST_CHECKED ? 2500 : 0))
            {
                if(_currentProxy->ice_isTwoway())
                {
                    _status->SetWindowText(CString(" Wait for response"));
                }
            }
            else
            {
                _status->SetWindowText(CString(" Sending request"));
            }
#ifdef ICEE_HAS_BATCH
        }
        else
        {
            _currentProxy->sayHello(_delay->GetCheck() == BST_CHECKED ? 2500 : 0);
            _status->SetWindowText(CString(" Queued batch request"));
        }
#endif
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
}

void
CHelloClientDlg::OnFlush()
{
#ifdef ICEE_HAS_BATCH
    try
    {
        updateProxy();
        if(_currentProxy->ice_flushBatchRequests_async(new FlushBatchRequestsCB(this)))
        {
            _status->SetWindowText(CString(" Flushed batch requests"));
        }
        else
        {
            _status->SetWindowText(CString(" Flushing batch requests"));
        }
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
#endif
}

void
CHelloClientDlg::OnShutdown()
{
    try
    {
        updateProxy();
#ifdef ICEE_HAS_BATCH
        if(!_currentProxy->ice_isBatchOneway() && !_currentProxy->ice_isBatchDatagram())
        {
#endif
            if(_currentProxy->shutdown_async(new ShutdownCB(this)))
            {
                _status->SetWindowText(CString(" Sent shutdown request"));
            }
            else
            {
                _status->SetWindowText(CString(" Sending shutdown request"));
            }
#ifdef ICEE_HAS_BATCH
        }
        else
        {
            _currentProxy->shutdown();
            _status->SetWindowText(CString(" Queued shutdown request"));
        }
#endif
    }
    catch(const IceUtil::Exception& ex)
    {
        handleException(ex);
    }
}

LRESULT
CHelloClientDlg::OnAMIException(WPARAM, LPARAM lParam)
{
    Ice::Exception* ex = reinterpret_cast<Ice::Exception*>(lParam);
    if(!dynamic_cast<Ice::CommunicatorDestroyedException*>(ex))
    {
        handleException(*ex);
    }
    delete ex;
    return 0;
}

LRESULT
CHelloClientDlg::OnAMISayHelloSent(WPARAM, LPARAM)
{
    if(_currentProxy->ice_isTwoway())
    {
        _status->SetWindowText(CString(" Waiting for response"));
    }
    else
    {
        _status->SetWindowText(CString(" Ready"));
    }
    return 0;
}

LRESULT
CHelloClientDlg::OnAMISayHelloResponse(WPARAM, LPARAM)
{
    _status->SetWindowText(CString(" Ready"));
    return 0;
}

LRESULT
CHelloClientDlg::OnAMIFlushBatchRequestsSent(WPARAM, LPARAM)
{
    _status->SetWindowText(CString(" Flushed batch requests"));
    return 0;
}

LRESULT
CHelloClientDlg::OnAMIShutdownSent(WPARAM, LPARAM)
{
    _status->SetWindowText(CString(" Sent shutdown request"));
    return 0;
}

void
CHelloClientDlg::updateProxy()
{
    int mode = _mode->GetCurSel();
    bool timeout = _timeout->GetCheck() == BST_CHECKED;

    CString h;
    _host->GetWindowText(h);
#ifdef _UNICODE
    char buf[1024];
    wcstombs(buf, (LPCTSTR)h, sizeof(buf));
    string hostname = buf;
#else
    string hostname = (LPCTSTR)h;
#endif

    if(_currentProxy && 
       hostname == _hostname &&
       mode == _currentMode &&
       timeout == _useTimeout)
    {
        return;
    }

    if(!_proxy)
    {
        _proxy = HelloPrx::uncheckedCast(_communicator->stringToProxy("hello:tcp -p 10000"));
        _hostname = "localhost";
    }
    else if(hostname != _hostname)
    {
        try
        {
            _proxy = HelloPrx::uncheckedCast(_communicator->stringToProxy("hello:tcp -p 10000 -h " + hostname));
            _hostname = hostname;
        }
        catch(const Ice::EndpointParseException&)
        {
            AfxMessageBox(CString("The provided hostname is invalid."), MB_OK|MB_ICONEXCLAMATION);
        }
    }

    Ice::ObjectPrx proxy;
    switch(mode)
    {
    case 0:
        proxy = _proxy->ice_twoway();
        break;
    case 1:
        proxy = _proxy->ice_oneway();
        break;
    case 2:
#ifdef ICEE_HAS_BATCH
        proxy = _proxy->ice_batchOneway();
        break;
#else
        AfxMessageBox(CString("Batch mode is currently not enabled."), MB_OK|MB_ICONEXCLAMATION);
        return;
#endif
    default:
        assert(false);
    }
    if(timeout)
    {
        proxy = proxy->ice_timeout(2000);
    }
    else
    {
        proxy = proxy->ice_timeout(-1);
    }

    _currentProxy = HelloPrx::uncheckedCast(proxy);
    _currentMode = mode;
    _useTimeout = timeout;
}

void
CHelloClientDlg::handleException(const IceUtil::Exception& e)
{
    try
    {
        e.ice_throw();
    }
    catch(const Ice::NoEndpointException&)
    {
        AfxMessageBox(CString("The proxy does not support the current configuration"), MB_OK|MB_ICONEXCLAMATION);
    }
    catch(const IceUtil::Exception& ex)
    {
        AfxMessageBox(CString(ex.toString().c_str()), MB_OK|MB_ICONEXCLAMATION);
    }
    _status->SetWindowText(CString(" Ready"));
}
