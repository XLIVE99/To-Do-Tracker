//#include <wx/wxprec.h>
//#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>
#include <wx/vscroll.h>
#include <wx/statline.h>
//#include <wx/colour.h>
//#include <mongocxx/database.hpp>
#include <MongoDatabase.h>
#include <ProjectMain.h>
#include <IDs.h>
#include <StringInline.h>
#pragma once

// Default database name
#define DATABASE_NAME	"ToDosDB"

class ProjectScrollable : public wxVScrolledWindow
{
public:
	ProjectScrollable(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	void UpdateProjects();
	mongocxx::database GetDatabase();
	std::vector<std::string> GetCollections();
protected:
	virtual wxCoord OnGetRowHeight(size_t n) const wxOVERRIDE;
private:
	std::vector<std::string> collections;
	std::vector<std::unique_ptr<wxStringClientData>> collectionDatas; // To delete in destruct
	mongocxx::database currentDB;

	wxBitmapBundle trashSVG;

	wxBoxSizer* projectsSizer;
};

class ProjectsWindow : public wxPanel
{
public:
	ProjectsWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);
	ProjectScrollable* GetProjectsPanel();

	void OnCreateProject(wxCommandEvent& e);
	void OnProjectDeleteButton(wxCommandEvent& e);
private:
	wxBoxSizer* mainHorizontalSizer;
	ProjectScrollable* projectsScrollable;
};

class ProjectSelection : public wxPanel
{
public:
	ProjectSelection(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size);

	void OnProjectButton(wxCommandEvent& e);
	void OnDeleteProject(wxString project);

	void OnAppExiting();
private:
	wxBoxSizer* panelSizer;
	ProjectsWindow* projects;
	wxPanel* dbPanel;

	wxTextCtrl* databaseUriCtrl;
	std::vector<ProjectMain*> openedProjects;

	wxPanel* DatabaseConnWindow(wxWindow* parent, bool show, const wxPoint& pos, const wxSize& size);
	void OnConnectButton(wxCommandEvent& e);

	void OnProjectClosed(wxCloseEvent& e);
};