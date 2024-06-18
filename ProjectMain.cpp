#include <ProjectMain.h>

using namespace std;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::kvp;

ProjectMain::ProjectMain(wxWindow* parent, mongocxx::collection selectedCollection, wxString projectN, const wxPoint& position, const wxSize& size)
	: wxFrame(parent, wxID_ANY, projectN, position, size)
{
	m_projectName = projectN;

	// Menu bar
	wxMenu* menuHelp = new wxMenu;
	menuHelp->Append(ID_PROJECT_MENU_CREATE_ITEM, "Create Task");

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(menuHelp, "&Project");

	SetMenuBar(menuBar);

	Bind(wxEVT_MENU, &ProjectMain::OnCreateItemMenu, this, ID_PROJECT_MENU_CREATE_ITEM);

	// Project sizer
	wxBoxSizer* verticalSizer = new wxBoxSizer(wxVERTICAL);

	// Project name (Moved to the frame name)
	//wxStaticText* projectText = new wxStaticText(this, wxID_ANY, std::string(selectedCollection.name()), wxDefaultPosition, wxSize(100, 40));
	//verticalSizer->Add(
	//	new wxStaticText(this, wxID_ANY, std::string(selectedCollection.name()), wxDefaultPosition, wxSize(100, 40)),
	//	wxSizerFlags(0).FixedMinSize().Align(wxALIGN_LEFT)
	//);

	// Load items
	ProjectMain::items = new ProjectItems(this, selectedCollection);
	verticalSizer->Add(
		items,
		wxSizerFlags(1).Expand()
	);

	SetSizer(verticalSizer);

	Show(true);
}

wxString ProjectMain::OpenedProject()
{
	return m_projectName;
}

void ProjectMain::OnCreateItemMenu(wxCommandEvent& e)
{
	items->CreateItem(wxDataViewItem(NULL), "", false);
}

#pragma region ProjectItems

ProjectItems::ProjectItems(wxWindow* parent, mongocxx::collection collection)
	: wxPanel(parent, wxID_ANY)
{
	ProjectItems::currentCollection = collection;

	ProjectItems::treeSizer = new wxBoxSizer(wxHORIZONTAL);
	ProjectItems::tree = new wxDataViewCtrl(this, ID_PROJECT_LIST_TREE, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VARIABLE_LINE_HEIGHT | wxDV_NO_HEADER);

	ProjectItems::treeData = new ToDoTreeModel();
	tree->AssociateModel(treeData);
	treeData->DecRef();

	// Append task string renderer
	MultiLineCustomRenderer* tr = new MultiLineCustomRenderer(wxDATAVIEW_CELL_EDITABLE);
	wxDataViewColumn* column0 = new wxDataViewColumn("Task", tr, 0, FromDIP(740), wxALIGN_LEFT, 0);
	column0->SetMinWidth(FromDIP(80));
	tree->AppendColumn(column0);

	tree->SetExpanderColumn(column0);

	tree->AppendToggleColumn("Status", 1, wxDATAVIEW_CELL_ACTIVATABLE, FromDIP(40), wxALIGN_CENTER, 0)->SetMinWidth(FromDIP(40));

	int documentCount = collection.count_documents({});

	if (documentCount > 0)
	{
		auto cursor = collection.find({});
		
		auto i = *cursor.begin();
		
		// Created nodes (parented or top node)
		vector<ToDoModelNode*> createds;
		createds.reserve(documentCount);

		for (auto&& doc : cursor)
		{
			//std::cout << bsoncxx::to_json(doc) << std::endl;
			//std::cout << doc["task"].get_string().value << std::endl;

			std::string docID = doc["_id"].get_oid().value.to_string();

			ToDoModelNode* parent = nullptr;
			ToDoModelNode* added = nullptr;
			// First check if created nodes have this id as a child
			for (const auto& created : createds)
			{
				ToDoModelNodePtrArray& nodeChilds = created->GetChildren();
				for (const auto& child : nodeChilds)
				{
					assert(!child.get()->GetDocID().empty());

					if (child.get()->GetDocID() == docID)
					{
						// This node has a child with this document ID
						parent = created;
						added = child.get();
						break;
					}
				}

				// Suitable parent found, break the loop
				if (parent != nullptr)
					break;
			}

			if(added == nullptr)
				added = treeData->Add(parent, (std::string)doc["task"].get_string().value, doc["status"].get_bool().value, docID);
			else
			{
				// This node already spawned, change the values
				added->m_task = (std::string)doc["task"].get_string().value;
				added->m_status = doc["status"].get_bool().value;
			}

			// Spawn the childs (if any) of the added node
			// This will create hollow nodes to be filled later
			if (doc["childs"])
			{
				auto docArray = doc["childs"].get_array().value;
				if (!docArray.empty())
				{
					for (auto i = docArray.begin(); i != docArray.end(); ++i)
					{
						treeData->Add(added, "", false, (std::string)i->get_string().value);
						//added->Append(new ToDoModelNode(added, "", false, (std::string)i->get_string().value));
					}
				}
			}

			// Second, check if newly created (added node) has any created as a child
			// This can happen if child created first then parent created
			ToDoModelNodePtrArray& addedChilds = added->GetChildren();
			for (const auto& aChild : addedChilds)
			{
				std::string checkDocID = aChild.get()->GetDocID();
				for (auto& created : createds)
				{
					if (created->GetDocID() == checkDocID)
					{
						assert((created->GetParent()) == nullptr && "Somehow this node must have 2 parent at the same time!");

						// This node has a child with this document ID
						created->ReParent(added);
					}
				}
			}

			// Add newly created node to created vector
			createds.push_back(added);
		}

		// All the nodes are outside right now, update parents to show in tree structure
		//treeData->UpdateTopNodeParents(hollows);
	}

	//treeData->Add("Test1", false);
	//treeData->Add("Test2", false);

	tree->EnableDragSource(wxDF_UNICODETEXT);
	tree->EnableDropTarget(wxDF_UNICODETEXT);

	treeSizer->Add(
		tree,
		wxSizerFlags(1).Expand()
	);

	SetSizer(treeSizer);

	// Create context menu (wxWidget don't automatically destroys pop up menus, reuse same menu)
	// And don't forget to delete this in destruct
	ProjectItems::contextMenu = new wxMenu();

	ProjectItems::contextMenu->Append(ID_PROJECT_CONTEXT_CREATE_ITEM, "Create Task");
	ProjectItems::contextMenu->Append(ID_PROJECT_CONTEXT_DELETE_ITEM, "Delete Selected Task (and it's children)");

	// Bind events

	tree->Bind(wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, &ProjectItems::OnBeginDrag, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_DROP, &ProjectItems::OnDrop, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, &ProjectItems::OnDropPossible, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ProjectItems::OnItemContextMenu, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, &ProjectItems::ItemValueChanged, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_EXPANDED, &ProjectItems::OnItemExpanded, this);
	tree->Bind(wxEVT_DATAVIEW_ITEM_COLLAPSING, &ProjectItems::OnItemCollapsing, this); // We can't collapse if the parent is collapsed, collapse childs just before the parent
	//Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &ProjectItems::OnItemContextMenu, this, ID_PROJECT_LIST_TREE);
	Bind(wxEVT_SIZE, &ProjectItems::OnSizeChanged, this);

	Bind(wxEVT_MENU, &ProjectItems::OnCreateItemContextMenu, this, ID_PROJECT_CONTEXT_CREATE_ITEM);
	Bind(wxEVT_MENU, &ProjectItems::OnDeleteItemContextMenu, this, ID_PROJECT_CONTEXT_DELETE_ITEM);
}

ProjectItems::~ProjectItems()
{
	delete contextMenu;
}

void ProjectItems::OnBeginDrag(wxDataViewEvent& e)
{
	wxDataViewItem item(e.GetItem());

	ToDoModelNode* dragNode = (ToDoModelNode*)item.GetID();
	treeData->m_draggingNode = dragNode;

	wxTextDataObject* data = new wxTextDataObject;
	data->SetText(dragNode->m_task);
	e.SetDataObject(data);
	e.SetDragFlags(wxDrag_DefaultMove);

#if _DEBUG
	std::cout << "Drag started" << std::endl;
#endif
}

void ProjectItems::OnDropPossible(wxDataViewEvent& e)
{
	if (treeData->m_draggingNode == nullptr)
		e.Veto();
	else
	{
		wxDataViewItem item(e.GetItem());
		ToDoModelNode* dropNode = (ToDoModelNode*)item.GetID();

		if (dropNode == treeData->m_draggingNode)
		{
			// Can't drop itself
			e.Veto();
			return;
		}

		e.SetDropEffect(wxDragMove);
	}
}

void ProjectItems::OnDrop(wxDataViewEvent& e)
{
	wxDataViewItem item(e.GetItem());

#if _DEBUG
	std::cout << "Dropped node to " << e.GetProposedDropIndex() << std::endl;
#endif

	if (item.IsOk())
	{
		ToDoModelNode* dropNode = (ToDoModelNode*)item.GetID();
		treeData->ReParent(treeData->m_draggingNode, dropNode, e.GetProposedDropIndex());
	}
	else
	{
		// Dragging node dropped on background
		treeData->ReParent(treeData->m_draggingNode, nullptr, e.GetProposedDropIndex());
	}

	// Reset the dragging node
	treeData->m_draggingNode = nullptr;
}

void ProjectItems::OnItemContextMenu(wxDataViewEvent& e)
{
	PopupMenu(contextMenu);
}

void ProjectItems::OnCreateItemContextMenu(wxCommandEvent& e)
{
	wxDataViewItem selected = tree->GetSelection();

	//std::cout << "create on menu" << std::endl;

	ToDoModelNode* added = CreateItem(selected, "", false);
}

void ProjectItems::OnDeleteItemContextMenu(wxCommandEvent& e)
{
	wxDataViewItem selected = tree->GetSelection();

	if (selected == NULL)
	{
		// We can't delete null item
		return;
	}

	DeleteItem(selected);
}

void ProjectItems::OnItemExpanded(wxDataViewEvent& e)
{
	if (wxGetKeyState(WXK_SHIFT))
	{
		// Unbind so we can avoid calling this multiple times
		tree->Unbind(wxEVT_DATAVIEW_ITEM_EXPANDED, &ProjectItems::OnItemExpanded, this);

		// Since this triggers expanded event
		tree->ExpandChildren(e.GetItem());

		// Bind again
		tree->Bind(wxEVT_DATAVIEW_ITEM_EXPANDED, &ProjectItems::OnItemExpanded, this);
	}
}

void ProjectItems::OnItemCollapsing(wxDataViewEvent& e)
{
	if (wxGetKeyState(WXK_SHIFT))
	{
		// No need to UnBind since there isn't any CollapseChildren method
		// Calling tree->Collapse will cause this method to called again
		// Therefore this method act like recursive
		// Collapse recursive if holding shift

		ToDoModelNode* currentItem = (ToDoModelNode*)e.GetItem().GetID();
		
		if (currentItem->IsContainer())
		{
			ToDoModelNodePtrArray& children = currentItem->GetChildren();
			for (auto& child : children)
			{
				// Collapse only if this item is a container
				// This will trigger the "wxEVT_DATAVIEW_ITEM_COLLAPSED" event
				if (child.get()->IsContainer())
					tree->Collapse(wxDataViewItem(child.get()));
			}
		}
	}
}

void ProjectItems::OnSizeChanged(wxSizeEvent& e)
{
	wxDataViewColumn* lastColumn = tree->GetColumn(tree->GetColumnCount() - 1);
	// Expand tree column (IDK why we need to reduce 4)
	wxDataViewColumn* expanderColumn = tree->GetExpanderColumn();
	expanderColumn->SetWidth(FromDIP(e.GetSize().x - lastColumn->GetMinWidth() - 4));

	// Set last column the min width (Last column resized when expander column resize)
	//lastColumn->SetWidth(lastColumn->GetWidth());

	// We only need refresh
	tree->Refresh();

	ToDoModelNodePtrArray& tops = treeData->GetTopNodes();

	if (tops.size() > 0)
	{
		// Update the height of the rows (if there are any items)
		// Updating only 1 item triggers the calculations of other row heights too
		treeData->ValueChanged(wxDataViewItem(tops[0].get()), 0);
	}

	// Call Skip() so sizer can function properly
	e.Skip(true);
}

// Creates and start editing the item
ToDoModelNode* ProjectItems::CreateItem(const wxDataViewItem& parent, const wxString& task, const bool& status)
{
	ToDoModelNode* added;

	mongocxx::options::insert opt;
	auto result = currentCollection.insert_one(make_document(kvp("task", task.ToStdString()), kvp("status", false)));

	std::string docID = result->inserted_id().get_oid().value.to_string();

	//std::cout << "Created: " << docID << std::endl;

	if (!parent.IsOk())
	{
		added = treeData->Add(task, status, docID);
	}
	else
	{
		ToDoModelNode* node = (ToDoModelNode*)parent.GetID();
		added = treeData->Add(node, task, status, docID);

		SaveItemValue(parent);
	}

	// Expand if it is not expanded, so we can edit newly added item
	if (parent != NULL && !tree->IsExpanded(parent))
	{
		tree->Expand(parent);
	}

	// If we don't refresh and update, edit item control appears on wrong position
	tree->Refresh();
	tree->Update();

	// Add bind for check editing (Need to delete if task setted empty)
	tree->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &ProjectItems::ItemCreationCheck, this);
	// Edit the task
	tree->EditItem(wxDataViewItem(added), tree->GetColumn(0));

	return added;
}

void ProjectItems::DeleteItem(const wxDataViewItem& item)
{
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	ToDoModelNode* itemParent = node->GetParent();

	// Delete created task's document
	currentCollection.delete_one(make_document(kvp("_id", bsoncxx::oid(node->GetDocID()))));

	treeData->Delete(item);

	// Update parent childs
	if (itemParent != nullptr)
		SaveItemValue(wxDataViewItem(itemParent));
}

void ProjectItems::ItemValueChanged(wxDataViewEvent& e)
{
	//std::cout << "value changed" << std::endl;

	SaveItemValue(e.GetItem());

	// Refresh the changed background colors
	tree->Refresh();
}

//void ProjectItems::ItemValueChanged(wxDataViewEvent& e)
//{
//	std::cout << "value changed" << std::endl;
//}

void ProjectItems::SaveItemValue(const wxDataViewItem& item)
{
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();

	// Tree not initialized yet! (Called when window first opened)
	//if (node->m_task.empty())
	//	return;

	auto searchDoc = make_document(kvp("_id", bsoncxx::oid(node->GetDocID())));
	auto updateDoc = make_document(kvp("$set", make_document(kvp("task", node->m_task.ToStdString()),
															kvp("status", node->m_status))));

#if _DEBUG
	std::cout << "searching to edit document: " << node->GetDocID() << " task: " << node->m_task << std::endl;
#endif
	
	mongocxx::model::update_one upOp{searchDoc.view(), updateDoc.view()};

	auto bulkWrite = currentCollection.create_bulk_write();

	bulkWrite.append(upOp);

	const ToDoModelNodePtrArray& childs = node->GetChildren();
	// Update current item childs (useful when item moved)
	if(!childs.empty())
	{
		int size = childs.size();

		bsoncxx::builder::basic::array idsArray;

#if _DEBUG
		std::cout << "Node children size: " << size << std::endl;
#endif

		for (int i = 0; i < size; ++i)
		{
#if _DEBUG
			std::cout << "Added to child array " << childs.at(i).get()->GetDocID() << " task: " << childs.at(i).get()->m_task << std::endl;
#endif
			idsArray.append(childs.at(i).get()->GetDocID());
		}

		auto parentSearchDoc = make_document(kvp("_id", bsoncxx::oid(node->GetDocID())));
		auto parentUpdateDoc = make_document(kvp("$set", make_document(kvp("childs", idsArray))));

		// Add to the write operation
		mongocxx::model::update_one upParentOp{parentSearchDoc.view(), parentUpdateDoc.view()};

		bulkWrite.append(upParentOp);
	}
	else
	{
		bsoncxx::builder::basic::array idsArray;

#if _DEBUG
		std::cout << "Removing childs array" << std::endl;
#endif

		auto parentSearchDoc = make_document(kvp("_id", bsoncxx::oid(node->GetDocID())));
		auto parentUpdateDoc = make_document(kvp("$unset", make_document(kvp("childs", true))));

		// Add to the write operation
		mongocxx::model::update_one upParentOp{parentSearchDoc.view(), parentUpdateDoc.view()};

		bulkWrite.append(upParentOp);
	}

	auto result = bulkWrite.execute();

	if (!result)
	{
		wxLogError("There is problem while writing to the database");
	}
}

void ProjectItems::ItemCreationCheck(wxDataViewEvent& e)
{
	// Unbind no matter what
	//tree->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &ProjectItems::ItemCreationCheck, this);
	tree->Unbind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, &ProjectItems::ItemCreationCheck, this);

	// Only check if we edited the task column
	if (e.GetColumn() != 0)
		return;

	ToDoModelNode* node = (ToDoModelNode*)e.GetItem().GetID();
	wxString editedValue = e.GetValue().GetString();

	// Created empty, editing done with empty
	// Delete the task
	if (node->m_task.IsEmpty() && editedValue.IsEmpty())
	{
		DeleteItem(e.GetItem());
	}
	// Item value changed triggered and save the item
	/*else
	{
		// Save task to the database
		SaveItemValue(e.GetItem());
	}*/
}

#pragma endregion

TestDataItemView::TestDataItemView(wxWindow* parent, void* pItem)
	: wxDataViewItem(pItem), wxPanel(parent, wxID_ANY)
{
	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(
		new wxButton(this, wxID_ANY, "Btn", wxDefaultPosition, wxSize(80, 50)),
		wxSizerFlags(0)
	);

	SetSizer(sizer);
}