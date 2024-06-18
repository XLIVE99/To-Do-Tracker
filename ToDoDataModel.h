//#include <wx/wxprec.h>
#include <wx/log.h>
#include <wx/dataview.h>
//#include <wx/artprov.h>
#pragma once

class ToDoModelNode;
using ToDoModelNodePtr = std::unique_ptr<ToDoModelNode>;
using ToDoModelNodePtrArray = std::vector<ToDoModelNodePtr>;

class ToDoModelNode
{
public:
	ToDoModelNode(ToDoModelNode* parent, const wxString& task, const bool& status, const wxString& docID)
	{
		m_parent = parent;

		if (m_parent != nullptr)
			m_parent->Append(this);

		m_task = task;
		m_status = status;

		m_docID = docID;
	}

	~ToDoModelNode() = default;

	bool IsContainer() const
	{
		return GetChildCount() > 0;
	}

	ToDoModelNode* GetParent()
	{
		return m_parent;
	}

	ToDoModelNodePtrArray& GetChildren()
	{
		return m_children;
	}

	ToDoModelNode* GetNthChild(unsigned int n)
	{
		return m_children.at(n).get();
	}

	// This will only change parent value, you have to append this item to the new parent
	void ReParent(ToDoModelNode* newParent)
	{
		// Change parent value
		m_parent = newParent;
	}

	void ReParent(ToDoModelNode* newParent, ToDoModelNodePtr& itemPtr, int index)
	{
		// Change to new parent and append this to parent childs
		m_parent = newParent;
		if (m_parent != nullptr) // Top nodes has null parent
		{
			if (index >= 0)
			{
				m_parent->Insert(itemPtr, index);
			}
			else
			{
				m_parent->Append(itemPtr);
			}
		}
	}

	void Insert(ToDoModelNode* child, unsigned int n)
	{
		m_children.insert(m_children.begin() + n, ToDoModelNodePtr(child));
	}

	void Insert(ToDoModelNodePtr& childPtr, unsigned int n)
	{
		m_children.insert(m_children.begin() + n, std::move(childPtr));
	}

	void Append(ToDoModelNode* child)
	{
		if (child->m_parent != nullptr && child->m_parent != this)
		{
			wxLogWarning("child node parent changed!");
			child->m_parent = this;
		}

		m_children.push_back(ToDoModelNodePtr(child));
	}

	void Append(ToDoModelNodePtr& childPtr)
	{
		m_children.push_back(std::move(childPtr));
	}

	unsigned int GetChildCount() const
	{
		return m_children.size();
	}

	std::string GetDocID() const
	{
		return m_docID.ToStdString();
	}

	// Calculates status with children
	bool GetStatus()
	{
		if (IsContainer())
		{
			bool isCompleted = true;
			for (auto& child : GetChildren())
			{
				if (!child->GetStatus())
				{
					// Item has an incomplete task
					isCompleted = false;
					break;
				}
			}

			return isCompleted;
		}
		else
		{
			return m_status;
		}
	}

public: // To avoid getter setters
	wxString	m_task;
	bool		m_status;
private:
	ToDoModelNode* m_parent;
	ToDoModelNodePtrArray m_children;

	// Only assign at initialization
	wxString	m_docID;
};

/*
Implement this data model
		Task				Status
----------------------------------------------
1: Complete this project	-
	2: Project selection	false
	3: To do tree list		true
*/
class ToDoTreeModel : public wxDataViewModel
{
public:
	ToDoTreeModel() = default;
	~ToDoTreeModel()
	{
		m_top.clear();
	}

	ToDoModelNodePtrArray& GetTopNodes();

	// Helper methods for wxLog

	wxString GetTask(const wxDataViewItem& item) const;
	bool GetStatus(const wxDataViewItem& item) const;

	// Helper methods to change the model

	ToDoModelNode* Add(ToDoModelNode* parent, const wxString& task, const bool& status, const wxString& docID);
	ToDoModelNode* Add(const wxString& task, const bool& status, const wxString& docID);
	void ReParent(ToDoModelNode* item, ToDoModelNode* newParent, int index);
	void Delete(const wxDataViewItem& item);
	void Clear();

	/// <summary>
	/// Updates all top node parents (Useful for reparenting after all the nodes loaded from database)
	/// </summary>
	/// <param name="schema">Schema of the tree</param>
	//void UpdateTopNodeParents(const ToDoModelNodePtrArray& schema);

	// Implementation of base class virtuals to define model

	virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override;

	virtual bool IsEnabled(const wxDataViewItem& item, unsigned int col) const override;

	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const override;

	virtual bool GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const override;

	ToDoModelNode* m_draggingNode;

private:
	ToDoModelNodePtrArray m_top; // All the top level nodes
};