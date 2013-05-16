#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include "mdibase.h"
#include "qgluaeditor.h"
#include <QEvent>
#include <QtBrowserItem>
#include <QList>

class LuaEditor : public MdiSubBase
{
	Q_OBJECT
public:
	LuaEditor(MdiInterface *interface, QWidget *parent = 0, Qt::WFlags flags = 0);
	~LuaEditor();

	virtual eMdiWindowType mdiWindowType() { return MdiSubBase::LuaEditor; };
	virtual void newFile();
	virtual void open(const QString &filename);
	virtual void save();
	virtual void saveas();
	virtual void compile();
	virtual void cut();
	virtual void copy();
	virtual void paste();
	virtual void undo();
	virtual void redo();
	virtual void updateMenus();
	virtual void updateProperties();
	virtual void updateUndo();
	virtual void propertyChanged(QtProperty *prop, const QVariant &value) {};

protected:
	void closeEvent(QCloseEvent *ev);

private slots:
	void onCopyAvailable(bool available);
	void onTextChanged();
	void onModificationChanged(bool changed);

private:
	QGluaEditor *m_sci;

	QtProperty *m_propTitle;
	QtProperty *m_propPath;

	QList<QtAbstractPropertyManager *> m_mngrlist;

	QString m_filename;
	QString m_filetitle;
	bool m_untitled;
};

#endif