#include "Tree.h"

#include "TreeFilter.h"
#include "TreeItem.h"

Tree::Tree(QWidget* parent) 
	: QTreeWidget(parent)
{
}

void Tree::clearTree()
{
	clear();
	m_ObjectCount.clear();
}

void Tree::removeItem()
{
	QTreeWidgetItem *currItem = currentItem();

    removeChild(currItem);
}

void Tree::objectCreated(std::string p_MeshName, int p_ActorId)
{
	if(m_ObjectCount.count(p_MeshName) > 0)
		m_ObjectCount.at(p_MeshName)++;
	else
	{
		m_ObjectCount.insert(std::pair<std::string, int>(p_MeshName, 0));

		emit addTableObject(p_MeshName);
	}

	addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName)), p_ActorId));
}

void Tree::addFilter()
{
    TreeFilter *newFilter = new TreeFilter("NewFilter");

    QTreeWidgetItem *currItem = currentItem();
    if(currItem)
    {
        QTreeWidgetItem *currItemParent = currItem->parent();
        TreeFilter *cFilter = dynamic_cast<TreeFilter*>(currItem);

        if(cFilter)
        {
            currItem->addChild(newFilter);
        }
        else
        {
            if(currItemParent)
                currItemParent->addChild(newFilter);
            else
                addTopLevelItem(newFilter);
        }
    }
    else
    {
        addTopLevelItem(newFilter);
    }
}

void Tree::removeChild(QTreeWidgetItem* currItem)
{
    if(currItem && currItem->isSelected())
    {
        for (int i = 0; i < currItem->childCount(); i++)
        {
            QTreeWidgetItem *currChild = currItem->takeChild(i);
            if (currChild->childCount() != 0)
            {
                removeChild(currChild);
            }
            else
                delete currChild;
        }

        delete currItem;
    }
}
