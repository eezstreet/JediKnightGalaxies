#include "qgluaeditor.h"

#include <Qsci\qsciapis.h>
#include "glualexer.h"
#include <QShortcut>
#include <QVariant>

QGluaEditor::QGluaEditor(QWidget *parent)
	: QsciScintilla(parent)
{
	setupScintilla();
}

QGluaEditor::~QGluaEditor()
{

}


void QGluaEditor::setupScintilla()
{
	QsciLexerGLua *lexer = new QsciLexerGLua(this);
	setLexer(lexer);

	setMarginLineNumbers(1, true);
	
	setEolMode(QsciScintilla::EolUnix);
	setMarginWidth(1, "99999");
	
	setFolding(QsciScintilla::BoxedTreeFoldStyle);

	setBackspaceUnindents(true);
	setIndentationsUseTabs(false);
	setTabIndents(true);
	setIndentationWidth(4);

	QsciAPIs *api = new QsciAPIs(lexer);
	api->load(":/api/glua.api");
	api->prepare();

	lexer->setProperty("fold", "1");
	lexer->setProperty("fold.comment", "0");
	lexer->setFoldCompact(false);

	setBraceMatching(QsciScintilla::SloppyBraceMatch);
	setAutoCompletionSource(QsciScintilla::AcsAPIs);
	setAutoCompletionReplaceWord(false);
	setAutoCompletionThreshold(2);
	setAutoCompletionFillupsEnabled(true);
	setAutoCompletionFillups("(.:");
	setAutoIndent(true);
	setCaretLineVisible(true);

	setCaretLineBackgroundColor(QColor("#E8E8FF"));


	QShortcut *autoComplete = new QShortcut(QKeySequence("Ctrl+Space"), this);
	connect(autoComplete, SIGNAL(activated()), this, SLOT(autoCompleteFromAll()));

}
