#ifndef SCRIPTEDITOR_H
#define SCRIPTEDITOR_H

#include "qgluaeditor.h"
#include <QDialog>

class ScriptEditor : public QDialog
{
	Q_OBJECT
public:
	ScriptEditor(QWidget *parent = 0, Qt::WindowFlags f = 0);
	~ScriptEditor();

	void setScript(const QString &script);
	QString getScript();

private:
	QGluaEditor *editor;
};

#endif