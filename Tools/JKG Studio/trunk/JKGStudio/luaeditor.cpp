#include "luaeditor.h"

#include <QVBoxLayout>
#include <QVariant>
#include <QFont>

//#include <Qsci\qsciapis.h>
//#include "glualexer.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QShortcut>
#include <QPixmap>
#include <QIcon>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QtStringPropertyManager>

LuaEditor::LuaEditor(MdiInterface *interface, QWidget *parent, Qt::WFlags flags) : MdiSubBase(interface, parent, flags)
{
	setWindowIcon(QIcon(":/icons/script.png"));

	m_sci = new QGluaEditor(this);

	setWidget(m_sci);

	m_filename = "";
	m_untitled = true;

	setWindowTitle(m_filename);
	
	m_propTitle = m_mdiInterface->propertyManager()->addLabelProperty(this, "Name", m_filetitle);
	m_propPath = m_mdiInterface->propertyManager()->addLabelProperty(this, "Path", m_filename);

	connect(m_sci, SIGNAL(copyAvailable(bool)), this, SLOT(onCopyAvailable(bool)));
	connect(m_sci, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
	connect(m_sci, SIGNAL(modificationChanged(bool)), this, SLOT(onModificationChanged(bool)));
}

LuaEditor::~LuaEditor()
{
	m_mdiInterface->propertyManager()->clearProperties(this);
}

void LuaEditor::newFile()
{
	static int sequence = 1;

	m_sci->clear();

	
	m_filetitle = QString("untitled%1.lua").arg(sequence++);
	m_filename = "";
	m_untitled = true;

	m_mdiInterface->propertyManager()->setLabelValue(m_propTitle, m_filetitle);
	m_mdiInterface->propertyManager()->setLabelValue(m_propPath, m_filename);

	setWindowTitle(m_filetitle);

}

void LuaEditor::open(const QString &filename)
{
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(this, "Open failed", "Could not open file");
		return;
	}
	QString text = f.readAll();
	f.close();

	QFileInfo fi(filename);
	m_filename = filename;
	m_filetitle = fi.fileName();

	m_mdiInterface->propertyManager()->setLabelValue(m_propTitle, m_filetitle);
	m_mdiInterface->propertyManager()->setLabelValue(m_propPath, m_filename);

	m_sci->setText(text);
	m_sci->setModified(false);
}

void LuaEditor::save()
{
	if (m_filename.isEmpty())
	{
		saveas();
		return;
	}
	
	QFile f(m_filename);
	if (!f.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(this, "Save failed", "Could not open file");
		return;
	}

	f.write(m_sci->text().toLatin1());
	f.close();

	m_sci->setModified(false);
}

void LuaEditor::saveas()
{
	QString path = QFileDialog::getSaveFileName(this, "Save Lua Script", m_filename, "Lua Scripts (*.lua)");
	if (path.isEmpty())
		return;
	

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(this, "Save failed", "Could not open file");
		return;
	}

	f.write(m_sci->text().toLatin1());
	f.close();

	QFileInfo fi(path);
	m_filename = path;
	m_filetitle = fi.fileName();
	
	m_mdiInterface->propertyManager()->setLabelValue(m_propTitle, m_filetitle);
	m_mdiInterface->propertyManager()->setLabelValue(m_propPath, m_filename);

	m_sci->setModified(false);
}

void LuaEditor::compile()
{

}

void LuaEditor::closeEvent(QCloseEvent *ev)
{
	bool block = false;

	if (m_sci->isModified())
	{
		QMessageBox::StandardButton ret = QMessageBox::information(this, "Save changes", QString("The script %1 has been modified.\nWould you like to save changes?").arg(m_filename), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
		if (ret == QMessageBox::Yes) {
			save();
		} else if (ret == QMessageBox::Cancel) {
			block = true;
		}
	}

	if (block) {
		ev->ignore();
	} else {
		acceptClose();
		ev->accept();
	}
}

void LuaEditor::cut()
{
	m_sci->cut();
}

void LuaEditor::copy()
{
	m_sci->copy();
}

void LuaEditor::paste()
{
	m_sci->paste();
}

void LuaEditor::undo()
{
	m_sci->undo();
}

void LuaEditor::redo()
{
	m_sci->redo();
}


void LuaEditor::updateMenus()
{
	m_mdiInterface->menuList()["save"]->setEnabled(m_sci->isModified());
	m_mdiInterface->menuList()["saveas"]->setEnabled(true);
	m_mdiInterface->menuList()["undo"]->setEnabled(m_sci->isUndoAvailable());
	m_mdiInterface->menuList()["redo"]->setEnabled(m_sci->isRedoAvailable());
	m_mdiInterface->menuList()["cut"]->setEnabled(!m_sci->selectedText().isEmpty());
	m_mdiInterface->menuList()["copy"]->setEnabled(!m_sci->selectedText().isEmpty());
	m_mdiInterface->menuList()["paste"]->setEnabled(true);
	m_mdiInterface->menuList()["compile"]->setEnabled(false);
};

void LuaEditor::updateProperties()
{

	if (!isActive()) 
		return;

	m_mdiInterface->propertyManager()->setActiveOwner(this);
}

void LuaEditor::updateUndo()
{
	m_mdiInterface->undoView()->setStack(NULL);	// Undo view not available
	m_mdiInterface->undoView()->setEnabled(false);
}

void LuaEditor::onCopyAvailable(bool available)
{
	m_mdiInterface->menuList()["cut"]->setEnabled(available);
	m_mdiInterface->menuList()["copy"]->setEnabled(available);
}

void LuaEditor::onTextChanged()
{
	m_mdiInterface->menuList()["undo"]->setEnabled(m_sci->isUndoAvailable());
	m_mdiInterface->menuList()["redo"]->setEnabled(m_sci->isRedoAvailable());
}

void LuaEditor::onModificationChanged(bool changed)
{
	m_mdiInterface->menuList()["save"]->setEnabled(changed);

	setWindowTitle(QString("%1%2").arg(m_filetitle).arg(changed ? "*" : ""));
}