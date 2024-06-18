#include <ToDoDataModel.h>

ToDoModelNodePtrArray& ToDoTreeModel::GetTopNodes()
{
	return m_top;
}

wxString ToDoTreeModel::GetTask(const wxDataViewItem& item) const
{
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	if (!node) // Happens if item.isOK() returns false
		return wxEmptyString;

	return node->m_task;
}

bool ToDoTreeModel::GetStatus(const wxDataViewItem& item) const
{
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();

	wxASSERT(!node);

	if (!node) // Happens if item.isOK() returns false
		return wxEmptyString;

	return node->m_status;
}

ToDoModelNode* ToDoTreeModel::Add(ToDoModelNode* parent, const wxString& task, const bool& status, const wxString& docID)
{
	ToDoModelNode* child = new ToDoModelNode(parent, task, status, docID);

	if (parent == nullptr)
	{
		m_top.push_back(ToDoModelNodePtr(child));
	}
	//else
	//{
	//	// I believe this can be moved to constructor
	//	parent->Append(child);

	//	std::cout << "Added to child" << std::endl;
	//}

	// Notify control
	wxDataViewItem parentItem(parent);
	wxDataViewItem childItem(child);
	ItemAdded(parentItem, childItem);

	return child;
}

ToDoModelNode* ToDoTreeModel::Add(const wxString& task, const bool& status, const wxString& docID)
{
	return Add(NULL, task, status, docID);
}

void ToDoTreeModel::ReParent(ToDoModelNode* item, ToDoModelNode* newParent, int index)
{
	wxDataViewItem parent(item->GetParent());
	ToDoModelNodePtrArray* siblings;
	if (!parent.IsOk())
	{
		// We are removing from m_top node
		siblings = &m_top;
	}
	else
	{
		siblings = &(item->GetParent()->GetChildren());
	}

	// first remove the node from the parent's array of children
	//auto& siblings = node->GetParent()->GetChildren();
	for (auto it = (*siblings).begin(); it != (*siblings).end(); ++it)
	{
		if (it->get() == item)
		{
			bool shouldErase = true;
			// Item found
			// Already in this parent, only change index
			if (item->GetParent() == newParent)
			{
				size_t newIndex = index;
				newIndex = std::clamp(newIndex, (size_t)0, (*siblings).size() - 1);
				int currentIndex = std::distance((*siblings).begin(), it);
#if _DEBUG
				std::cout << "current index: " << currentIndex << "\nNew index: " << newIndex << std::endl;
#endif
				if (currentIndex > newIndex)
				{
					std::rotate((*siblings).rend() - currentIndex - 1, (*siblings).rend() - currentIndex, (*siblings).rend() - newIndex);
				}
				else
				{
					std::rotate((*siblings).begin() + currentIndex, (*siblings).begin() + currentIndex + 1, (*siblings).begin() + newIndex + 1);
				}
				shouldErase = false;
			}
			// Move to the top nodes
			else if (newParent == nullptr)
			{
				item->ReParent(nullptr);

				if (index >= 0)
				{
					m_top.insert(m_top.begin() + index, std::move(*it));
				}
				else
				{
					// Add to the top nodes
					m_top.push_back(std::move(*it));
				}
			}
			// Change parent
			else
			{
				item->ReParent(newParent, *it, index);

				//newParent->Append(*it);
			}

			if (shouldErase)
			{
				// Remove found node from the old parent
				(*siblings).erase(it);
			}
			break;
		}
	}

	// Notify control
	wxDataViewItem newParentItem(item->GetParent());
	wxDataViewItem childItem(item);
	if (newParentItem != parent)
	{
		// Item added to the new parent
		ItemAdded(newParentItem, childItem);

		// Update the database values (if not null)
		if (newParentItem.IsOk())
		{
			ItemChanged(newParentItem);
		}

		// Item removed from old parent
		ItemDeleted(parent, childItem);

		// Update the database values (if not null)
		if (parent.IsOk())
		{
			ItemChanged(parent);
		}
	}
	else
	{
		// First delete the item
		ItemDeleted(parent, childItem);

		// Then add item
		ItemAdded(newParentItem, childItem);

		// Update parent's database value (if not null)
		if (parent.IsOk())
		{
			// Update parent database values (to save order correctly)
			ItemChanged(parent);
		}
	}

	// Item's parent changed (No need for this, ItemAdded and ItemDeleted handles everything)
	//ItemChanged(childItem);
}

void ToDoTreeModel::Delete(const wxDataViewItem& item)
{
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	if (!node)
		return;

	// m_root implementation (Prevent deletion of the m_root)
	// We don't need this since we switched to m_top implementation
	// We wouldn't be able to see m_top node
	wxDataViewItem parent(node->GetParent());
	ToDoModelNodePtrArray* siblings;
	if (!parent.IsOk())
	{
		// We are removing from m_top node
		siblings = &m_top;
	}
	else
	{
		siblings = &(node->GetParent()->GetChildren());
	}

	// first remove the node from the parent's array of children
	//auto& siblings = node->GetParent()->GetChildren();
	for (auto it = (*siblings).begin(); it != (*siblings).end(); ++it)
	{
		if (it->get() == node)
		{
			(*siblings).erase(it);
			break;
		}
	}

	// Notify control
	ItemDeleted(parent, item);
}

void ToDoTreeModel::Clear()
{
	m_top.clear();

	Cleared();
}

void ToDoTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	wxASSERT(item.IsOk());

	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	switch (col)
	{
	case 0: // Task
		variant = node->m_task;
		break;
	case 1: // Status
		variant = node->m_status;
		break;
	default:
		wxLogError("ToDoTreeModel::GetValue: wrong column %d", col);
	}
}

bool ToDoTreeModel::SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col)
{
	wxASSERT(item.IsOk());

	bool isValueChanged = false;
	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	switch (col)
	{
	case 0: // task
		node->m_task = variant.GetString();
		isValueChanged = true;
		break;
	case 1: // status
		node->m_status = variant.GetBool();
		isValueChanged = true;
		break;
	default:
		wxLogError("ToDoTreeModel::SetValue: wrong column %d", col);
		break;
	}

	// No need to call it again (wxWidget already calls the event)
	//if (isValueChanged)
	//{
	//	// Update current item
	//	ValueChanged(item, col);
	//}

	return isValueChanged;
}

bool ToDoTreeModel::IsEnabled(const wxDataViewItem& item, unsigned int col) const
{
	wxASSERT(item.IsOk());

	ToDoModelNode* node = (ToDoModelNode*)item.GetID();

	// We can set what we can edit
	// Disable editing of task (Example)
	//if (col == 0) // Task
	//	return false;

	// Allow editing
	return true;
}

wxDataViewItem ToDoTreeModel::GetParent(const wxDataViewItem& item) const
{
	// the invisible root node has no parent
	if (!item.IsOk())
		return wxDataViewItem(0);

	ToDoModelNode* node = (ToDoModelNode*)item.GetID();

	// Check if item is in m_top
	size_t count = m_top.size();
	for (size_t i = 0; i < count; i++)
	{
		if (node == m_top[i].get())
			return wxDataViewItem(0);
	}

	return wxDataViewItem((void*)node->GetParent());
}

bool ToDoTreeModel::IsContainer(const wxDataViewItem& item) const
{
	// the invisible root node can have children
	if (!item.IsOk())
		return true;

	ToDoModelNode* node = (ToDoModelNode*)item.GetID();

	return node->IsContainer();
}

unsigned int ToDoTreeModel::GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const
{
	ToDoModelNode* node = (ToDoModelNode*)parent.GetID();
	if (!node)
	{
		// Hides root node (Causes crash on editing the item)
		size_t count = m_top.size();
		for (size_t i = 0; i < count; i++)
		{
			array.Add(wxDataViewItem(m_top[i].get()));
		}
		return count;
	}

	if (node->GetChildCount() == 0)
	{
		return 0;
	}

	for (const auto& child : node->GetChildren())
	{
		array.Add(wxDataViewItem(child.get()));
	}

	return array.size();
}

bool ToDoTreeModel::GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const
{
	if (!item.IsOk())
		return false;

	ToDoModelNode* node = (ToDoModelNode*)item.GetID();
	if (node->GetStatus())
		attr.SetBackgroundColour(wxColour(70, 190, 70));
	else
		attr.SetBackgroundColour(wxColour(220, 225, 230));

	// Tell wxwidget we overrided attribute
	return true;
}