#include <ProjectSelection.h>

using namespace std;

ProjectSelection::ProjectSelection(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxPanel(parent, id, pos, size)
{
	MongoDatabase* currentDatabase = MongoDatabase::GetInstance();

	// Database connection window
	ProjectSelection::dbPanel = ProjectSelection::DatabaseConnWindow(this, currentDatabase->GetClient() == nullptr, pos, size);
	ProjectSelection::projects = new ProjectsWindow(this, wxID_ANY, pos, size);

	// Show projects if already connected to database
	projects->Show(!dbPanel->IsShown());

	ProjectSelection::panelSizer = new wxBoxSizer(wxVERTICAL);
	panelSizer->Add(
		dbPanel,
		wxSizerFlags(1).Expand().Border(wxTOP | wxBOTTOM, 10).FixedMinSize()
	);

	panelSizer->Add(
		projects,
		wxSizerFlags(1).Expand().FixedMinSize()
	);

	SetSizer(panelSizer);

	// Connect button binding
	Bind(wxEVT_BUTTON, &ProjectSelection::OnConnectButton, this, ID_DATABASE_CONNECT);
	//Bind(wxEVT_BUTTON, &ProjectSelection::OnProjectButton, this, ID_PROJECTS_BUTTON);
}

wxPanel* ProjectSelection::DatabaseConnWindow(wxWindow* parent, bool show, const wxPoint& pos, const wxSize& size)
{
	// Connection to database window
	wxPanel* dbCon = new wxPanel(parent, wxID_ANY, pos, size);

	wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);
	verticalSizer->Add(
		new wxStaticText(dbCon, wxID_ANY, "Please connect to a Mongo database (Leaving it empty will connect to the local database)", wxDefaultPosition, wxSize(250, 100), wxALIGN_CENTRE_HORIZONTAL),
		wxSizerFlags(0).Expand().Border(wxALL, 10).FixedMinSize()
	);

	// Add stretchable space so controls will located at bottom
	verticalSizer->AddStretchSpacer();

	ProjectSelection::databaseUriCtrl = new wxTextCtrl(dbCon, ID_DATABASE_URI, wxEmptyString, wxDefaultPosition, wxSize(400, 30), wxTE_PROCESS_ENTER);
	verticalSizer->Add(
		databaseUriCtrl,
		wxSizerFlags(0).Align(wxALIGN_CENTER).Border(wxALL, 10).FixedMinSize()
	);

	verticalSizer->Add(
		new wxButton(dbCon, ID_DATABASE_CONNECT, "Connect to the database", wxDefaultPosition, wxSize(200, 60)),
		wxSizerFlags(0).Align(wxALIGN_CENTER).Border(wxBOTTOM, 40).FixedMinSize()
	);

	dbCon->Show(show);

	dbCon->SetSizer(verticalSizer);

	// Pressing enter in textCtrl will act like pressed as connect button
	databaseUriCtrl->Bind(wxEVT_TEXT_ENTER, &ProjectSelection::OnConnectButton, this);

	return dbCon;
}

ProjectsWindow::ProjectsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxPanel(parent, id, pos, size)
{
	ProjectsWindow::mainHorizontalSizer = new wxBoxSizer(wxHORIZONTAL);
	ProjectsWindow::projectsScrollable = new ProjectScrollable(this, wxID_ANY, pos, wxSize(size.x * 0.7, size.y));
	wxBoxSizer* buttonsSizer = new wxBoxSizer(wxVERTICAL);

	// Set colors of the windows
	
	// Set background color of projects
	projectsScrollable->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
	// Set background color of side buttons
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR));

	// Create "create project" action button
	wxButton* createProjectBtn = new wxButton(this, ID_CREATE_PROJECT, "Create a Project", wxDefaultPosition, wxSize(100, 60));
	createProjectBtn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
	//createProjectBtn->SetupColours();

	buttonsSizer->Add(
		createProjectBtn,
		wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 5).FixedMinSize()
	);

	// All available projects
	mainHorizontalSizer->Add(
		projectsScrollable,
		wxSizerFlags(1).Expand()
	);

	// Line separating action buttons and projects
	mainHorizontalSizer->Add(
		new wxStaticLine(this, wxID_ANY, pos, wxSize(2, size.y), wxLI_VERTICAL),
		wxSizerFlags(0).Expand()
	);

	// Action buttons (such as create new project)
	mainHorizontalSizer->Add(
		buttonsSizer,
		wxSizerFlags(0).Border(wxRIGHT | wxLEFT, 10)
	);

	//mainHorizontalSizer->Add(
	//	new wxStaticText(this, wxID_ANY, "test text"),
	//	wxSizerFlags(1).Border(wxALL, 10)
	//);

	// Bind button events
	Bind(wxEVT_BUTTON, &ProjectsWindow::OnCreateProject, this, ID_CREATE_PROJECT);
	//Bind(wxEVT_BUTTON, &ProjectsWindow::OnProjectDeleteButton, this, ID_PROJECTS_DELETE_BUTTON);

	projectsScrollable->Show(true);
	SetSizer(mainHorizontalSizer);
	Fit();
}

ProjectScrollable::ProjectScrollable(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
	: wxVScrolledWindow(parent, id, pos, size)
{
	// Set trash bin svg
	//ProjectScrollable::trashSVG = wxBitmapBundle::FromSVGFile("TrashBin.svg", wxSize(32, 32));
	ProjectScrollable::trashSVG = wxBitmapBundle::FromSVGResource("#102", wxSize(32, 32));
		
	// Create project sizer
	ProjectScrollable::projectsSizer = new wxBoxSizer(wxVERTICAL);

	// Update projects if collections is empty and we are connected to a database
	MongoDatabase* currentDatabase = MongoDatabase::GetInstance();
	if (currentDatabase->GetClient() != nullptr)
		UpdateProjects();

	SetSizer(projectsSizer);
	FitInside();
}

wxCoord ProjectScrollable::OnGetRowHeight(size_t height) const
{
	return wxCoord(10);
}

void ProjectScrollable::UpdateProjects()
{
	// Clear project sizer
	projectsSizer->Clear(true);

	MongoDatabase* database = MongoDatabase::GetInstance();

	auto dbs = database->GetDatabases();
	// Check if database exist
	if (!(std::find(dbs.begin(), dbs.end(), DATABASE_NAME) != dbs.end()))
	{
		// Create a new database
		(*database->GetClient())[DATABASE_NAME];
#if _DEBUG
		cout << "created database" << endl;
#endif
	}
	
	ProjectScrollable::currentDB = (*database->GetClient()).database(DATABASE_NAME);
	// Update collections
	ProjectScrollable::collections = currentDB.list_collection_names();

	ProjectsWindow* projectW = (ProjectsWindow*)GetParent();
	ProjectSelection* projectSelW = (ProjectSelection*)projectW->GetParent();

	//Bind(wxEVT_BUTTON, &ProjectsWindow::OnCreateProject, this, ID_CREATE_PROJECT);
	//Bind(wxEVT_BUTTON, &ProjectsWindow::OnProjectDeleteButton, this, ID_PROJECTS_DELETE_BUTTON);

	// Clear old datas from memory
	collectionDatas.clear();

	// Resize before adding datas (to gain performance beforehand)
	collectionDatas.resize(collections.size());

	for (auto col : collections)
	{
		wxBoxSizer* projectItemSizer = new wxBoxSizer(wxHORIZONTAL);
		wxStringClientData* clientData = new wxStringClientData(col);
		collectionDatas.push_back(std::unique_ptr<wxStringClientData>(clientData));

		// Project button
		// ID_PROJECTS_BUTTON (Assigning same ID and changing background color causes button text to be same)
		wxButton* btn = new wxButton(this, wxID_ANY, wxString(col), wxDefaultPosition, wxSize(100, 50));
		btn->SetClientData(clientData);
		btn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
		//btn->SetWindowStyle(wxBORDER_NONE);

		//btn->Bind(wxEVT_BUTTON, &ProjectsWindow::OnCreateProject, projectW);
		btn->Bind(wxEVT_BUTTON, &ProjectSelection::OnProjectButton, projectSelW);

		// In this instance, background color updates the button somehow???
		// Setting background color doesn't immediately updates the button, refresh is required
		//btn->ClearBackground();
		//btn->Refresh();

		// Project delete button
		// ID_PROJECTS_DELETE_BUTTON
		wxBitmapButton* deleteBtn = new wxBitmapButton(this, wxID_ANY, trashSVG, wxDefaultPosition, wxSize(40, 50));
		deleteBtn->SetClientData(clientData);
		//deleteBtn->SetBackgroundColour(wxColour(150, 55, 65));
		deleteBtn->SetForegroundColour(wxColour(150, 55, 65));

		deleteBtn->Bind(wxEVT_BUTTON, &ProjectsWindow::OnProjectDeleteButton, projectW);

		projectItemSizer->Add(
			btn,
			wxSizerFlags(1).Expand()
		);

		projectItemSizer->Add(
			deleteBtn,
			wxSizerFlags(0).Align(wxALIGN_CENTER).FixedMinSize()
		);

		projectsSizer->Add(
			projectItemSizer,
			wxSizerFlags(0).Expand().Border(wxALL, 10).FixedMinSize()
		);
	}

	if (collections.empty())
	{
		// There is no project

		// Show text
		projectsSizer->Add(
			new wxStaticText(this, wxID_ANY, "Create your first project"),
			wxSizerFlags(0).Align(wxALIGN_CENTER).Border(wxTOP | wxBOTTOM, 10)
		);

		// Show create project button
		projectsSizer->Add(
			new wxButton(this, ID_CREATE_PROJECT, "Create"),
			wxSizerFlags(0).Expand().Border(wxTOP | wxBOTTOM, 10).FixedMinSize()
		);
	}

	SetRowCount((collections.size() * 7) - 1);

	projectsSizer->Layout();
	Refresh();
}

ProjectScrollable* ProjectsWindow::GetProjectsPanel()
{
	return projectsScrollable;
}

mongocxx::database ProjectScrollable::GetDatabase()
{
	return ProjectScrollable::currentDB;
}

std::vector<std::string> ProjectScrollable::GetCollections()
{
	return ProjectScrollable::collections;
}

void ProjectsWindow::OnCreateProject(wxCommandEvent& e)
{
	wxTextEntryDialog projectDialog(this, "Enter the project name", "Project name");
	std::vector<std::string> collections = projectsScrollable->GetCollections();
	if (projectDialog.ShowModal() == wxID_OK)
	{
		std::string projectN = projectDialog.GetValue().ToStdString();

		// Trim so we won't be able to create empty space projects
		rtrim(projectN);
		ltrim(projectN);

		if (projectN.empty())
		{
			wxLogMessage("Project name can't be empty!");
			return;
		}

		if (!(std::find(collections.begin(), collections.end(), projectN) != collections.end()))
		{
			// Collection odes not exist, safe to create
			projectsScrollable->GetDatabase().create_collection(projectN);

			projectsScrollable->UpdateProjects();
		}
		else
		{
			// Collection exist, show error
			wxLogMessage("Project \"" + wxString(projectN) + "\" is already exist!");
		}
	}
}

void ProjectsWindow::OnProjectDeleteButton(wxCommandEvent& e)
{
	wxButton* clickedBtn = static_cast<wxButton*>(e.GetEventObject());
	wxStringClientData* data = static_cast<wxStringClientData*>(clickedBtn->GetClientData());

	wxString projectName = data->GetData();
	// Show a warning dialog to make sure user didn't click on it by accident
	wxMessageDialog warning(this, "You are trying to delete " + projectName + "\nAre you sure?", "Delete", wxYES_NO | wxCANCEL | wxNO_DEFAULT | wxICON_WARNING);
	if (warning.ShowModal() == wxID_YES)
	{
		ProjectSelection* selection = (ProjectSelection*)GetParent();

		// Check if this project is open
		selection->OnDeleteProject(projectName);

		// Drop the collection
#if _DEBUG
		cout << "dropping " << projectName << endl;
#endif
		projectsScrollable->GetDatabase().collection(projectName.ToStdString()).drop();

		// Update projects again
		projectsScrollable->UpdateProjects();
		//Fit();
	}
}

void ProjectSelection::OnProjectButton(wxCommandEvent& e)
{
	wxButton* clickedBtn = static_cast<wxButton*>(e.GetEventObject());
	wxStringClientData* data = static_cast<wxStringClientData*>(clickedBtn->GetClientData());

	wxString projectName = data->GetData();
	mongocxx::collection selectedProject = projects->GetProjectsPanel()->GetDatabase().collection(projectName.ToStdString());

	// Check if the user already opened this project
	for (auto projects : openedProjects)
	{
		if (projects->OpenedProject().IsSameAs(projectName))
		{
			// Project already opened (Show with focusing to project frame)
			//wxLogInfo(projectName + " is already open");

			// If minimzed bring it up
			projects->Iconize(false);

			// Focus on the opened project
			projects->SetFocus();
			projects->Raise();

			return;
		}
	}

	// Open and add project main frame to the opened projects vector
	ProjectMain* projectFrame = new ProjectMain(nullptr, selectedProject, projectName, GetParent()->GetPosition(), GetParent()->GetSize());
	openedProjects.push_back(projectFrame);

	projectFrame->Bind(wxEVT_CLOSE_WINDOW, &ProjectSelection::OnProjectClosed, this);

	// New frame will be created, no need to replace
	//wxWindow* parent = GetParent();
	//wxSizer* parentSizer = parent->GetSizer();
	//parentSizer->Replace(
	//	this,
	//	new ProjectMain(parent, selectedProject)
	//);

	// New frame will be created, no need to hide
	//Show(false);

	//parentSizer->Layout();
}

void ProjectSelection::OnDeleteProject(wxString project)
{
	// Check if the user already opened this project
	for (auto projects : openedProjects)
	{
		if (projects->OpenedProject().IsSameAs(project))
		{
			// Close this frame
			projects->Close(true);
			return;
		}
	}
}

void ProjectSelection::OnConnectButton(wxCommandEvent& e)
{
	MongoDatabase* currentDatabase = MongoDatabase::GetInstance();
	int connectionStatus = 0;
	if (currentDatabase->GetClient() == nullptr)
	{
#if _DEBUG
		cout << "Client not detected on database, creating connection to " << databaseUriCtrl->GetValue().ToStdString() << endl;
#endif
		connectionStatus = currentDatabase->Connect(databaseUriCtrl->GetValue().ToStdString());
	}
	else
	{
#if _DEBUG
		cout << "Connection already established, show collections" << endl;
#endif
	}

	if (connectionStatus != 0)
	{
		wxLogError("Error occured while trying to connect to the mongo database. Make sure Database URI is correct and you are connected to the internet.");
		return;
	}

	ProjectSelection::projects->GetProjectsPanel()->UpdateProjects();

	// Switch panels after update projects completed
	// It will show empty projects panel if switched before UpdateProjects()
	panelSizer->Show(dbPanel, !dbPanel->IsShown());
	panelSizer->Show(projects, !projects->IsShown());

	// Update layout to fit projects panel
	projects->Layout();

	Fit();
}

void ProjectSelection::OnProjectClosed(wxCloseEvent& e)
{
	ProjectMain* project = (ProjectMain*)e.GetEventObject();

	wxASSERT(project != nullptr);

	// Remove closing project from the vector (so we can open it again)
	wxString closedProject = project->OpenedProject();
	for (auto it = openedProjects.begin(); it != openedProjects.end(); ++it)
	{
		if ((*it)->OpenedProject().IsSameAs(closedProject))
		{
			openedProjects.erase(it);
			break;
		}
	}

	// Continue to close the frame
	e.Skip(true);
}

void ProjectSelection::OnAppExiting()
{
	// Close every opened project if we are closing the app
	for (auto& project : openedProjects)
	{
		project->Close(true);
	}
}