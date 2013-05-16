
#ifndef QSCILEXERGLUA_H
#define QSCILEXERGLUA_H

#ifdef __APPLE__
extern "C++" {
#endif

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qscilexerlua.h>


//! \brief The QsciLexerGLua class encapsulates the Scintilla Lua
//! lexer and implements GLua styles
class QsciLexerGLua : public QsciLexerLua
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! Lua lexer.
    enum {
        //! The default.
        Default = 0,

        //! A block comment.
        Comment = 1,

        //! A line comment.
        LineComment = 2,

        //! A number.
        Number = 4,

        //! A keyword.
        Keyword = 5,

        //! A string.
        String = 6,

        //! A character.
        Character = 7,

        //! A literal string.
        LiteralString = 8,

        //! Preprocessor
        Preprocessor = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! The end of a line where a string is not closed.
        UnclosedString = 12,

        //! Base functions and (GLua) libraries
        KeywordSet2 = 13,

        //! Main libraries
        KeywordSet3 = 14,

        //! GLua libraries
        KeywordSet4 = 15,

        //! GLua Constants
        KeywordSet5 = 16,

        //! GLua methods
        KeywordSet6 = 17,

        //! A keyword defined in keyword set number 7.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet7 = 18,

        //! A keyword defined in keyword set number 8.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet8 = 19
    };

    //! Construct a QsciLexerGLua with parent \a parent.  \a parent is typically
    //! the QsciScintilla instance.
    QsciLexerGLua(QObject *parent = 0);

    //! Destroys the QsciLexerGLua instance.
    virtual ~QsciLexerGLua();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! \internal Returns the character sequences that can separate
    //! auto-completion words.
    QStringList autoCompletionWordSeparators() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    const char *blockStart(int *style = 0) const;

    //! \internal Returns the style used for braces for brace matching.
    int braceStyle() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultPaper()
    QColor defaultColor(int style) const;

    //! Returns the end-of-line fill for style number \a style.
    bool defaultEolFill(int style) const;

    //! Returns the font for style number \a style.
    QFont defaultFont(int style) const;

    //! Returns the background colour of the text for style number \a style.
    //!
    //! \sa defaultColor()
    QColor defaultPaper(int style) const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.
    const char *keywords(int set) const;

    //! Returns the descriptive name for style number \a style.  If the
    //! style is invalid for this language then an empty QString is returned.
    //! This is intended to be used in user preference dialogs.
    QString description(int style) const;

private:

    QsciLexerGLua(const QsciLexerGLua &);
    QsciLexerGLua &operator=(const QsciLexerGLua &);
};

#ifdef __APPLE__
}
#endif

#endif
