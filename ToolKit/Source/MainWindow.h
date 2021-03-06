#pragma once

#include <QMainWindow>
#include <QTimer>
#include <QShortcut>

#include <Actor.h>
#include <EventManager.h>
#include <Utilities\XMFloatUtil.h>
#ifndef Q_MOC_RUN
#include <ResourceManager.h>
#endif

#include <map>
#include "CameraInterpolation.h"
#include <QProgressBar> 


class QFileSystemModelDialog;
class QGroupBox;
class QTableWidgetItem;
class QTreeWidgetItem;


class ActorFactory;
class AnimationLoader;
class IGraphics;
class IPhysics;
class ObjectManager;
class TreeItem;
class RotationTool;



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	Ui::MainWindow *ui;
	QTimer m_Timer;
	QIcon m_DefaultObjectIcon;

	EventManager m_EventManager;
	ResourceManager m_ResourceManager;
	std::unique_ptr<ObjectManager> m_ObjectManager;
	std::unique_ptr<AnimationLoader> m_AnimationLoader;
	std::shared_ptr<ActorFactory> m_ActorFactory;
	std::unique_ptr<RotationTool> m_RotationTool;
	IGraphics* m_Graphics;
	IPhysics* m_Physics;
    std::vector<QGroupBox*> m_Boxes;

	std::map<std::string, QShortcut*> m_Hotkey;
	bool m_LevelLoaded;
    QFileSystemModelDialog* m_FileSystemDialog;
	CameraInterpolation m_CamInt;
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void signalAndSlotsDefinitions();
    void pushBoxes();
    void sortPropertiesBoxes();
	void fillPowerPieOptions();
private slots:
	// Engine changes
	void onActorAdded(std::string p_ObjectType, Actor::ptr p_Actor);
	void idle();

	// Property changes
	void splitCameraPosition(Vector3 p_cameraPosition);
    void setCameraPosition();

    void setObjectScale();
    void setObjectRotation();
    void setObjectPosition();

	void setObjectRotation(Vector3 p_Rotation);
	void setObjectTranslation(Vector3 p_Translation);

	void setLightPosition();
	void setLightColor();
	void setLightDirection();
	void setLightAngles();
	void setLightRange();
	void setLightIntensity();
	void deselectAllTreeItems();
	void levelLoaded();
	void shortcutDeselect();

	// QT object changes
    void on_m_ObjectTree_itemSelectionChanged();

	// QT Triggers
	void on_actionProperties_triggered();
	void on_actionObject_Tree_triggered();
	void on_actionExit_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionGo_To_Selected_triggered();
    void on_actionHelp_window_triggered();

    void on_actionPower_Pie_triggered();

    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_saveButton_clicked();

    void on_m_FileSystemTreeView_clicked(const QModelIndex &index);

    void on_m_FileSystemListView_doubleClicked(const QModelIndex &index);

    void on_actionAdd_Object_triggered();

    void on_actionSet_to_Default_Scale_triggered();

    void on_moveUpButton_clicked();

    void on_moveDownButton_clicked();

signals:
    void setCameraPositionSignal(Vector3 p_CameraPosition);
	void deselectAll();

private:
	void onFrame(float p_DeltaTime);
	void loadLevel(const std::string& p_Filename);
	void saveLevel(const std::string& p_Filename);

	void initializeSystems();
	void uninitializeSystems();
	void initializeHotkeys();
	void pick(IEventData::Ptr p_Data);

	void itemPropertiesChanged(void);
	void hideItemProperties(void);

	Vector3 findMiddlePoint(QList<TreeItem*> p_Items);
};
