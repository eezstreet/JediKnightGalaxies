#include "scripteditor.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

ScriptEditor::ScriptEditor(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f | Qt::WindowMinMaxButtonsHint)
{

	setWindowTitle("Script Editor");
	setWindowIcon(QIcon(":/icons/script.png"));

	resize(600,400);
	

	QVBoxLayout *vbox =  new QVBoxLayout;
	editor = new QGluaEditor(this);

	vbox->addWidget(editor, 1);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Expanding, QSizePolicy::Ignored));
	
	QPushButton *okbutton = new QPushButton("Apply", this);
	connect(okbutton, SIGNAL(clicked()), this, SLOT(accept()));

	QPushButton *cancelbutton = new QPushButton("Cancel", this);
	connect(cancelbutton, SIGNAL(clicked()), this, SLOT(reject()));

	hbox->addWidget(okbutton);
	hbox->addWidget(cancelbutton);

	vbox->addLayout(hbox);
	setLayout(vbox);

}

ScriptEditor::~ScriptEditor()
{

}

void ScriptEditor::setScript(const QString &script)
{
	editor->setText(script);
}

QString ScriptEditor::getScript()
{
	return editor->text();
}