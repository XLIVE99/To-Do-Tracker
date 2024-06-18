//#include <wx/wxprec.h>
#include <wx/app.h>
#include <wx/taskbar.h>
#include <wx/notifmsg.h>
#include <ProjectSelection.h>
#include <MongoDatabase.h>
#include <IDs.h>

//#ifndef WX_PRECOMP
//#include <wx/wx.h>
//#endif

#define APPICON "#101"
#define APPNAME "To-Do Tracker"
#define APPVERSION "0.1"

//#define _CRTDBG_MAP_ALLOC
//#ifdef _MSC_VER
////#include <wx/msw/msvcrt.h>
//#define _CRTDBG_MAP_ALLOC
////#include <cstdlib>
////#include <crtdbg.h>
//#endif

//#if !defined(_INC_CRTDBG) || !defined(_CRTDBG_MAP_ALLOC)
//#error Debug CRT functions have not been included
//#endif

// Application main frame window

class Frame : public wxFrame
{
public:
	Frame(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
	bool preventNormalClose;
	std::unique_ptr<wxNotificationMessage> notification;
	bool notificationShown;
private:
	//void OnExit(wxCommandEvent& event);
	void OnExitWnd(wxCloseEvent& event);
	void OnAbout(wxCommandEvent& event);
};

// Application task bar icon

class AppTaskBarIcon : public wxTaskBarIcon
{
public:
#if defined(__WXOSX__) && wxOSX_USE_COCOA
	AppTaskBarIcon(Frame* frame, wxTaskBarIconType iconType = wxTBI_DEFAULT_TYPE)
		: wxTaskBarIcon(iconType)
#else
	AppTaskBarIcon(Frame* frame)
#endif
	;

	void OnLeftButtonDClick(wxTaskBarIconEvent&);
	void OnMenuShow(wxCommandEvent&);
	void OnMenuExit(wxCommandEvent&);

	virtual wxMenu* CreatePopupMenu() override;

private:
	Frame* m_frame;
	//std::unique_ptr<wxMenu> taskBarMenu;
};

// Application class

class HelloWorld : public wxApp
{
public:
	// Initialization method
	virtual bool OnInit();
private:
#if defined(__WXOSX__) && wxOSX_USE_COCOA
	AppTaskBarIcon* m_dockIcon;
#endif
};

//static std::unique_ptr<DockDialog> dockDialog = nullptr;
//static std::unique_ptr<Frame> mainFrame = nullptr;

// Implement OnInit()

bool HelloWorld::OnInit()
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(189086);

#if _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	wxInitAllImageHandlers();

	// Create main window
	//mainFrame = std::unique_ptr<Frame>(new Frame("To Dos", wxPoint(350, 150), wxSize(800, 500)));
	Frame* frame = new Frame(APPNAME, wxPoint(350, 150), wxSize(800, 500));

	// Display the frame window
	frame->Show(true);

	// Connect automatically at start (Connects to local database)
	//MongoDatabase* db = MongoDatabase::GetInstance();
	//db->Connect();

	// Show project selection panel
	ProjectSelection* pSelection = new ProjectSelection(frame, wxID_ANY, wxDefaultPosition, wxSize(800, 500));
	pSelection->Show(true);

	wxBoxSizer* frameSizer = new wxBoxSizer(wxHORIZONTAL);
	frameSizer->Add(
		pSelection,
		wxSizerFlags(1).Expand().FixedMinSize()
	);
	
	frame->SetSizerAndFit(frameSizer);

	if (wxTaskBarIcon::IsAvailable())
	{
		// The user can exit the app from taskbar
		frame->preventNormalClose = true;

		// Automatically deleted by wxWidget, no need to destroy manually
		AppTaskBarIcon* taskBarIcon = new AppTaskBarIcon(frame);

		wxNotificationMessage* notification = new wxNotificationMessage(APPNAME, "The app still open in background");
		notification->UseTaskBarIcon(taskBarIcon);
		frame->notification = std::unique_ptr<wxNotificationMessage>(notification);

		if (!taskBarIcon->SetIcon(wxIcon(APPICON),
			APPNAME))
		{
			wxLogError("Could not set dock icon.");
		}

#if defined(__WXOSX__) && wxOSX_USE_COCOA
		m_dockIcon = new AppTaskBarIcon(wxTBI_DOCK);
		if (!m_dockIcon->SetIcon(wxIcon(APPICON, wxBITMAP_TYPE_ICO)))
		{
			wxLogError("Could not set icon.");
		}
#endif

		//wxApp::SetExitOnFrameDelete(false);
	}

	// Start the event loop
	return true;
}

// Start running the application

wxIMPLEMENT_APP(HelloWorld);

// TaskBarIcon implementation

AppTaskBarIcon::AppTaskBarIcon(Frame* frame)
{
	m_frame = frame;

	// Register events
	Bind(wxEVT_TASKBAR_LEFT_DCLICK, &AppTaskBarIcon::OnLeftButtonDClick, this);
	Bind(wxEVT_MENU, &AppTaskBarIcon::OnMenuShow, this, ID_TASKBAR_SHOW);
	Bind(wxEVT_MENU, &AppTaskBarIcon::OnMenuExit, this, wxID_EXIT);
}

wxMenu* AppTaskBarIcon::CreatePopupMenu()
{
	wxMenu* menu = new wxMenu;
	//taskBarMenu = std::unique_ptr<wxMenu>(menu);
	menu->Append(ID_TASKBAR_SHOW, wxString("&Show ") + wxString(APPNAME));

#ifdef __WXOSX__
	if (OSXIsStatusItem())
#endif
	{
		menu->AppendSeparator();
		menu->Append(wxID_EXIT, wxString("&Exit ") + wxString(APPNAME));
	}

	return menu;
}

void AppTaskBarIcon::OnLeftButtonDClick(wxTaskBarIconEvent&)
{
	m_frame->Show(true);
}

void AppTaskBarIcon::OnMenuShow(wxCommandEvent&)
{
	// Bring up if minimized the frame
	m_frame->Iconize(false);

	// Grab focus and raise to the top
	m_frame->SetFocus();
	m_frame->Raise();

	// Make frame visible
	m_frame->Show(true);
}

void AppTaskBarIcon::OnMenuExit(wxCommandEvent&)
{
	// We have to manually destroy task bar icon
	// It will be visible in task bar if we don't destroy it and cause memory leak!
	Destroy();
	//m_frame->Close(true);
	// Kill the application
	wxApp::GetInstance()->Exit();
}

// Frame constructor

Frame::Frame(const wxString& title, const wxPoint& pos, const wxSize& size, long style)
	:wxFrame(NULL, wxID_ANY, title, pos, size, style)
{
	//wxMenu* menuFile = new wxMenu;
	//menuFile->Append(wxID_EXIT);

	SetIcon(wxIcon(APPICON));

	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar* menuBar = new wxMenuBar;
	//menuBar->Append(menuFile, "&File");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);
	//CreateStatusBar();
	//SetStatusText("Welcome to wxWidgets!");

	preventNormalClose = false;
	notificationShown = false;

	// Register events for this frame
	Bind(wxEVT_MENU, &Frame::OnAbout, this, wxID_ABOUT);
	Bind(wxEVT_CLOSE_WINDOW, &Frame::OnExitWnd, this);
	//Bind(wxEVT_MENU, &Frame::OnExit, this, wxID_EXIT);
}

// Define event handlers

// Event handler for clicking about button
// Help > About
void Frame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(wxString("Simple app to track your to-do items\nVersion ") + wxString(APPVERSION)
		, APPNAME, wxOK | wxICON_INFORMATION);
}

// File > Exit (REMOVED)
//void Frame::OnExit(wxCommandEvent& event)
//{
//	Close(true);
//	//_CrtDumpMemoryLeaks();
//}

void Frame::OnExitWnd(wxCloseEvent& event)
{
	// Prevent closing the window (taskbar used)
	if (event.CanVeto() && preventNormalClose)
	{
		// Notify the user
		if (notification != nullptr && !notificationShown)
		{
			notification.get()->Show(wxNotificationMessage::Timeout_Auto);
			// Only show it once (can be annoying showing it everytime)
			notificationShown = true;
		}
		Hide();
		event.Veto();
	}
	else
		event.Skip(true);
}