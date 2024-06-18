//#include <wx/wxprec.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/button.h>
//#include <wx/vscroll.h>
//#include <wx/dataview.h>
//#include <wx/event.h>
#include <MongoDatabase.h>
//#include <bsoncxx/builder/basic/kvp.hpp>
#include <ToDoDataModel.h>
#include <IDs.h>
#include <MultiLineCustomRenderer.h>
#pragma once

class ProjectItems : public wxPanel
{
public:
	ProjectItems(wxWindow* parent, mongocxx::collection collection);
	~ProjectItems();

	// Creates and start editing the item
	ToDoModelNode* CreateItem(const wxDataViewItem& parent, const wxString& task, const bool& status);
private:
	mongocxx::collection currentCollection;
	ToDoTreeModel* treeData;

	wxBoxSizer* treeSizer;
	wxDataViewCtrl* tree;
	wxMenu* contextMenu;

	void OnBeginDrag(wxDataViewEvent& e);
	void OnDropPossible (wxDataViewEvent& e);
	void OnDrop(wxDataViewEvent& e);

	void OnItemContextMenu(wxDataViewEvent& e);
	void OnCreateItemContextMenu(wxCommandEvent& e);
	void OnDeleteItemContextMenu(wxCommandEvent& e);

	void OnItemExpanded(wxDataViewEvent& e);
	void OnItemCollapsing(wxDataViewEvent& e);

	void DeleteItem(const wxDataViewItem& item);
	void ItemValueChanged(wxDataViewEvent& e);
	//void ItemValueChanged(wxDataViewEvent& e);
	void SaveItemValue(const wxDataViewItem& item);

	// Check if item created with empty task
	void ItemCreationCheck(wxDataViewEvent& e);

	void OnSizeChanged(wxSizeEvent& e);
};

class ProjectMain : public wxFrame
{
public:
	ProjectMain(wxWindow* parent, mongocxx::collection selectedCollection, wxString projectN, const wxPoint& position, const wxSize& size);

	wxString OpenedProject();
private:
	ProjectItems* items;

	wxString m_projectName;

	void OnCreateItemMenu(wxCommandEvent& e);
};

class TestDataItemView : public wxDataViewItem, public wxPanel
{
public:
	TestDataItemView(wxWindow* parent, void* pItem);
};