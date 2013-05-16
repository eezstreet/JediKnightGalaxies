// SWF file parser.
//
// I uploaded this file *as is* while I was in the middle
// of debugging something (so its a bit of a mess) but still,
// it fixes most (all?) of the bugs in the original parser
// and includes quite a bit more functionality.
//
// It also uses the Zlib stuff in ParseDefineBitsLossless()
// so you can either comment that out or grab the Zlib source
// and link it in.
//
// Cheers, David.
//
// Change History:
//
// 99.07.12:    Uploaded first version (as is)
// 99.07.22:    Added code to parse Flash 4 ActionScripts,
//              and switches to dump image/sound/tag data
//              Thanks to Jason Schuchert!
// 99.07.28:    More flash 4 actionscript.  
//              Added GetUrl2 (thanks Olivier)
//              Fixed callFrame (there's no arguments to print)
//              Added WaitForFrameExpression
//              - Jason Schuchert
// 99.10.21:    Now parses streaming sound and MP3 headers
//              - David Michie
// 99.10.28:    Now parses DefineMorphShape and correctly
//              interprets FLOAT values with ActionPush
//              - David Michie
// 99.11.02:    Now parses DefineFont2 and DefineEditText
//              - David Michie
//
//////////////////////////////////////////////////////////////////////


//!!@ dump sound data

#include <stdio.h>
#include <string.h>


void usage( void )
{
    fprintf(stderr, "usage: swfdump [options] inputFile\n");
    fprintf(stderr, "       -a dumps all data (tag, image, sound)\n" );
    fprintf(stderr, "       -i dumps image data\n" );
    fprintf(stderr, "       -s dumps sound data\n" );
    fprintf(stderr, "       -t dumps tag data\n" );
}

extern "C"
{
    #include <zlib/zlib.h>
}


#if defined(_DEBUG) && defined(_WIN32)
    #include <windows.h>
    #define DEBUG_ASSERT    DebugBreak()
#else
    #define DEBUG_ASSERT
    typedef unsigned long BOOL;
#endif

// Global Types
typedef unsigned long U32, *P_U32, **PP_U32;
typedef signed long S32, *P_S32, **PP_S32;
typedef unsigned short U16, *P_U16, **PP_U16;
typedef signed short S16, *P_S16, **PP_S16;
typedef unsigned char U8, *P_U8, **PP_U8;
typedef signed char S8, *P_S8, **PP_S8;
typedef signed long SFIXED, *P_SFIXED;
typedef signed long SCOORD, *P_SCOORD;

typedef struct SPOINT
{
    SCOORD x;
    SCOORD y;
} SPOINT, *P_SPOINT;

typedef struct SRECT 
{
    SCOORD xmin;
    SCOORD xmax;
    SCOORD ymin;
    SCOORD ymax;
} SRECT, *P_SRECT;

// Start Sound Flags
enum {
    soundHasInPoint     = 0x01,
    soundHasOutPoint    = 0x02,
    soundHasLoops       = 0x04,
    soundHasEnvelope    = 0x08

    // the upper 4 bits are reserved for synchronization flags
};

enum {
    fillGradient        =   0x10,
    fillLinearGradient  =   0x10,
    fillRadialGradient  =   0x12,
    fillMaxGradientColors   =   8,
    // Texture/bitmap fills
    fillBits            =   0x40    // if this bit is set, must be a bitmap pattern
};

// Flags for defining a shape character
enum {
        // These flag codes are used for state changes - and as return values from ShapeParser::GetEdge()
        eflagsMoveTo       = 0x01,
        eflagsFill0        = 0x02,
        eflagsFill1        = 0x04,
        eflagsLine         = 0x08,
        eflagsNewStyles    = 0x10,

        eflagsEnd          = 0x80  // a state change with no change marks the end
};

enum FontFlags
{
    fontUnicode   = 0x20,
    fontShiftJIS  = 0x10,
    fontANSI      = 0x08,
    fontItalic    = 0x04,
    fontBold      = 0x02,
    fontWideCodes = 0x01
};

// Edit text field flags
enum
{
    sfontFlagsBold          = 0x01,
    sfontFlagsItalic        = 0x02,
    sfontFlagsWideCodes     = 0x04,
    sfontFlagsWideOffsets   = 0x08,
    sfontFlagsANSI          = 0x10,
    sfontFlagsUnicode       = 0x20,
    sfontFlagsShiftJIS      = 0x40,
    sfontFlagsHasLayout     = 0x80
};

// Edit Text Flags
enum {
    seditTextFlagsHasFont       = 0x0001,
    seditTextFlagsHasMaxLength  = 0x0002,
    seditTextFlagsHasTextColor  = 0x0004,
    seditTextFlagsReadOnly      = 0x0008,
    seditTextFlagsPassword      = 0x0010,
    seditTextFlagsMultiline     = 0x0020,
    seditTextFlagsWordWrap      = 0x0040,
    seditTextFlagsHasText       = 0x0080,
    seditTextFlagsUseOutlines   = 0x0100,
    seditTextFlagsBorder        = 0x0800,
    seditTextFlagsNoSelect      = 0x1000,
    seditTextFlagsHasLayout     = 0x2000
};


enum TextFlags
{
    isTextControl = 0x80,

    textHasFont   = 0x08,
    textHasColor  = 0x04,
    textHasYOffset= 0x02,
    textHasXOffset= 0x01
};


typedef struct MATRIX
{
    SFIXED a;
    SFIXED b;
    SFIXED c;
    SFIXED d;
    SCOORD tx;
    SCOORD ty;
} MATRIX, *P_MATRIX;

typedef struct CXFORM
{
    /*
    S32 flags;
    enum
    { 
        needA=0x1,  // Set if we need the multiply terms.
        needB=0x2   // Set if we need the constant terms.
    };
    */

    S16 aa, ab;     // a is multiply factor, b is addition factor
    S16 ra, rb;
    S16 ga, gb;
    S16 ba, bb;
}
CXFORM, *P_CXFORM;


#ifndef NULL
#define NULL 0
#endif

// Tag values that represent actions or data in a Flash script.
enum
{ 
    stagEnd                 = 0,
    stagShowFrame           = 1,
    stagDefineShape         = 2,
    stagFreeCharacter       = 3,
    stagPlaceObject         = 4,
    stagRemoveObject        = 5,
    stagDefineBits          = 6,
    stagDefineButton        = 7,
    stagJPEGTables          = 8,
    stagSetBackgroundColor  = 9,
    stagDefineFont          = 10,
    stagDefineText          = 11,
    stagDoAction            = 12,
    stagDefineFontInfo      = 13,
    stagDefineSound         = 14,   // Event sound tags.
    stagStartSound          = 15,
    stagDefineButtonSound   = 17,
    stagSoundStreamHead     = 18,
    stagSoundStreamBlock    = 19,
    stagDefineBitsLossless  = 20,   // A bitmap using lossless zlib compression.
    stagDefineBitsJPEG2     = 21,   // A bitmap using an internal JPEG compression table.
    stagDefineShape2        = 22,
    stagDefineButtonCxform  = 23,
    stagProtect             = 24,   // This file should not be importable for editing.

    // These are the new tags for Flash 3.
    stagPlaceObject2        = 26,   // The new style place w/ alpha color transform and name.
    stagRemoveObject2       = 28,   // A more compact remove object that omits the character tag (just depth).
    stagDefineShape3        = 32,   // A shape V3 includes alpha values.
    stagDefineText2         = 33,   // A text V2 includes alpha values.
    stagDefineButton2       = 34,   // A button V2 includes color transform, alpha and multiple actions
    stagDefineBitsJPEG3     = 35,   // A JPEG bitmap with alpha info.
    stagDefineBitsLossless2 = 36,   // A lossless bitmap with alpha info.
    stagDefineEditText      = 37,   // An editable Text Field
    stagDefineSprite        = 39,   // Define a sequence of tags that describe the behavior of a sprite.
    stagNameCharacter       = 40,   // Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds).
    stagFrameLabel          = 43,   // A string label for the current frame.
    stagSoundStreamHead2    = 45,   // For lossless streaming sound, should not have needed this...
    stagDefineMorphShape    = 46,   // A morph shape definition
    stagDefineFont2         = 48,   // 
};

// PlaceObject2 Flags
enum
{
    splaceMove          = 0x01, // this place moves an exisiting object
    splaceCharacter     = 0x02, // there is a character tag (if no tag, must be a move)
    splaceMatrix        = 0x04, // there is a matrix (matrix)
    splaceColorTransform= 0x08, // there is a color transform (cxform with alpha)
    splaceRatio         = 0x10, // there is a blend ratio (word)
    splaceName          = 0x20, // there is an object name (string)
    splaceDefineClip    = 0x40  // this shape should open or close a clipping bracket (character != 0 to open, character == 0 to close)
    // one bit left for expansion
};

// Action codes
enum
{
    sactionNone                     = 0x00,
    sactionNextFrame                = 0x04,
    sactionPrevFrame                = 0x05,
    sactionPlay                     = 0x06,
    sactionStop                     = 0x07,
    sactionToggleQuality            = 0x08,
    sactionStopSounds               = 0x09,
    sactionAdd                      = 0x0A,
    sactionSubtract                 = 0x0B,
    sactionMultiply                 = 0x0C,
    sactionDivide                   = 0x0D,
    sactionEqual                    = 0x0E,
    sactionLessThan                 = 0x0F,
    sactionLogicalAnd               = 0x10,
    sactionLogicalOr                = 0x11,
    sactionLogicalNot               = 0x12,
    sactionStringEqual              = 0x13,
    sactionStringLength             = 0x14,
    sactionSubString                = 0x15,
    sactionInt                      = 0x18,
    sactionEval                     = 0x1C,
    sactionSetVariable              = 0x1D,
    sactionSetTargetExpression      = 0x20,
    sactionStringConcat             = 0x21,
    sactionGetProperty              = 0x22,
    sactionSetProperty              = 0x23,
    sactionDuplicateClip            = 0x24,
    sactionRemoveClip               = 0x25,
    sactionTrace                    = 0x26,
    sactionStartDragMovie           = 0x27,
    sactionStopDragMovie            = 0x28,
    sactionStringLessThan           = 0x29,
    sactionRandom                   = 0x30,
    sactionMBLength                 = 0x31,
    sactionOrd                      = 0x32,
    sactionChr                      = 0x33,
    sactionGetTimer                 = 0x34,
    sactionMBSubString              = 0x35,
    sactionMBOrd                    = 0x36,
    sactionMBChr                    = 0x37,
    sactionHasLength                = 0x80,
    sactionGotoFrame                = 0x81, // frame num (WORD)
    sactionGetURL                   = 0x83, // url (STR), window (STR)
    sactionWaitForFrame             = 0x8A, // frame needed (WORD), 
                                            // actions to skip (BYTE)
    sactionSetTarget                = 0x8B, // name (STR)
    sactionGotoLabel                = 0x8C, // name (STR)
    sactionWaitForFrameExpression   = 0x8D, // frame needed on stack,
                                            // actions to skip (BYTE)
    sactionPushData                 = 0x96,
    sactionBranchAlways             = 0x99,
    sactionGetURL2                  = 0x9A,
    sactionBranchIfTrue             = 0x9D,
    sactionCallFrame                = 0x9E,
    sactionGotoExpression           = 0x9F
};

//////////////////////////////////////////////////////////////////////
// Input script object definition.
//////////////////////////////////////////////////////////////////////

// An input script object.  This object represents a script created from 
// an external file that is meant to be inserted into an output script.
struct CInputScript  
{
    // Pointer to file contents buffer.
    U8 *m_fileBuf;

    // File state information.
    U32 m_filePos;
    U32 m_fileSize;
    U32 m_fileStart;
    U16 m_fileVersion;

    // Bit Handling
    S32 m_bitPos;
    U32 m_bitBuf;

    // Tag parsing information.
    U32 m_tagStart;
    U32 m_tagZero;
    U32 m_tagEnd;
    U32 m_tagLen;
    
    // Parsing information.
    S32 m_nFillBits;
    S32 m_nLineBits;

   // Set to true if we wish to dump all contents long form
    U32 m_dumpAll;

    // if set to true will dump image guts (i.e. jpeg, zlib, etc. data)
    U32 m_dumpGuts;

    // if set to true will dump sound guts 
    U32 m_dumpSoundGuts;
    
    
    // Handle to output file.
    FILE *m_outputFile;

    // Font glyph counts (gotta save it somewhere!)
    int m_iGlyphCounts[256];

    U8* m_srcAdpcm;
    U32 m_bitBufAdpcm;      // this should always contain at least 24 bits of data
    S32 m_bitPosAdpcm;
    U32 m_nSamplesAdpcm;    // number of samples decompressed so far

    // Streaming sound info from SoundStreamHead tag
    int m_iStreamCompression;
    int m_iStreamSampleRate;
    int m_iStreamSampleSize;
    int m_iStreamStereoMono;
    int m_nStreamSamples;

    // Constructor/destructor.
    CInputScript();
    ~CInputScript();

    // Tag scanning methods.
    U16 GetTag(void);
    void SkipBytes(int n);
    U8 GetByte(void);
    U16 GetWord(void);
    U32 GetDWord(void);
    void GetRect(SRECT *r);
    void GetMatrix(MATRIX *matrix);
    void GetCxform(CXFORM *cxform, BOOL hasAlpha);
    void PrintCxform(char* str, CXFORM *cxform);
    char *GetString(void);
    U32 GetColor(BOOL fWithAlpha=false);

    void PrintMatrix(MATRIX matrix, char* str);
    void PrintRect(SRECT rect, char* str);

    // Routines for reading arbitrary sized bit fields from the stream.
    // Always call start bits before gettings bits and do not intermix 
    // these calls with GetByte, etc... 
    void InitBits();
    S32 GetSBits(S32 n);
    U32 GetBits(S32 n);
    void where();

    // Tag subcomponent parsing methods
    // For shapes
    void ParseShapeStyle(char *str, BOOL fWithAlpha=false);
    BOOL ParseShapeRecord(char *str, int& xLast, int& yLast, BOOL fWithAlpha=false);
    void ParseButtonRecord(char *str, U32 byte, BOOL fGetColorMatrix=true);
    BOOL ParseTextRecord(char* str, int nGlyphBits, int nAdvanceBits);

    // Parsing methods.
    void ParseEnd(char *str);                               // 00: stagEnd
    void ParseShowFrame(char *str, U32 frame, U32 offset);  // 01: stagShowFrame
    void ParseDefineShape(char *str, BOOL fWithAlpha=false);// 02: stagDefineShape
    void ParseFreeCharacter(char *str);                     // 03: stagFreeCharacter
    void ParsePlaceObject(char *str);                       // 04: stagPlaceObject
    void ParseRemoveObject(char *str);                      // 05: stagRemoveObject
    void ParseDefineBits(char *str);                        // 06: stagDefineBits
    void ParseDefineButton(char *str);       //x 07: stagDefineButton
    void ParseJPEGTables(char *str);                        // 08: stagJPEGTables
    void ParseSetBackgroundColor(char *str);                // 09: stagSetBackgroundColor
    void ParseDefineFont(char *str);         //x 10: stagDefineFont
    void ParseDefineText(char *str);         //x 11: stagDefineText
    void ParseDoAction(char *str, BOOL fPrintTag=true);                          // 12: stagDoAction    
    void ParseDefineFontInfo(char *str);     //x 13: stagDefineFontInfo
    void ParseDefineSound(char *str);                       // 14: stagDefineSound
    void ParseStartSound(char *str);                        // 15: stagStartSound
    void ParseStopSound(char *str);                         // 16: stagStopSound
    void ParseDefineButtonSound(char *str);                 // 17: stagDefineButtonSound
    void ParseSoundStreamHead(char *str);                   // 18: stagSoundStreamHead
    void ParseSoundStreamBlock(char *str);                  // 19: stagSoundStreamBlock
    void ParseDefineBitsLossless(char *str);                // 20: stagDefineBitsLossless
    void ParseDefineBitsJPEG2(char *str);                   // 21: stagDefineBitsJPEG2
    void ParseDefineShape2(char *str);       //x 22: stagDefineShape2
    void ParseDefineButtonCxform(char *str);                // 23: stagDefineButtonCxform
    void ParseProtect(char *str);                           // 24: stagProtect
    void ParsePlaceObject2(char *str);                      // 26: stagPlaceObject2
    void ParseRemoveObject2(char *str);                     // 28: stagRemoveObject2
    void ParseDefineShape3(char *str);       //x 32: stagDefineShape3
    void ParseDefineText2(char *str);        //x 33: stagDefineText2
    void ParseDefineButton2(char *str);      //x 34: stagDefineButton2
    void ParseDefineBitsJPEG3(char *str);                   // 35: stagDefineBitsJPEG3
    void ParseDefineBitsLossless2(char *str);               // 36: stagDefineBitsLossless2
    void ParseDefineEditText(char *str);                    // 37: stagDefineEditText
    void ParseDefineMouseTarget(char *str);                 // 38: stagDefineMouseTarget
    void ParseDefineSprite(char *str);       //x 39: stagDefineSprite
    void ParseNameCharacter(char *str);                     // 40: stagNameCharacter
    void ParseFrameLabel(char *str);                        // 43: stagFrameLabel
    void ParseSoundStreamHead2(char *str, BOOL fIsHead2=true);                  // 45: stagSoundStreamHead2
    void ParseDefineMorphShape(char *str);   //x 46: stagDefineMorphShape
    void ParseDefineFont2(char *str);        //x 48: stagDefineFont2
    void ParseUnknown(char *str, U16 code);
    void ParseTags(BOOL sprite, U32 tabs);
    BOOL ParseFile(char * pInput);
    void S_DumpImageGuts(char *str);

    // ADPCM stuff
    void AdpcmFillBuffer();
    long AdpcmGetBits(int n);
    long AdpcmGetSBits(int n);
    void AdpcmDecompress(long n, long stereo, int if16bit, U8 *dst=NULL);

    // MP3 stuff
    void DecodeMp3Headers(char* str, int iSamplesPerFrame);
    void DecodeMp3Frame(U8* pbFrame, int iEncodedSize, int iDecodedSize);
};

#define INDENT  printf("      ");

//////////////////////////////////////////////////////////////////////
// Inline input script object methods.
//////////////////////////////////////////////////////////////////////

//
// Inlines to parse a Flash file.
//

inline void CInputScript::SkipBytes(int n)
{
    m_filePos += n;
}

inline U8 CInputScript::GetByte(void) 
{
    //printf("GetByte: filePos: %02x [%02x]\n", m_filePos, m_fileBuf[m_filePos]);
    InitBits();
    return m_fileBuf[m_filePos++];
}

inline U16 CInputScript::GetWord(void)
{
    //printf("GetWord: filePos: %02x\n", m_filePos);
    U8* s = m_fileBuf + m_filePos;
    m_filePos += 2;
    InitBits();
    return (U16) s[0] | ((U16) s[1] << 8);
}

inline U32 CInputScript::GetDWord(void)
{
    //printf("GetDWord: filePos: %02x\n", m_filePos);
    U8 * s = m_fileBuf + m_filePos;
    m_filePos += 4;
    InitBits();
    return (U32) s[0] | ((U32) s[1] << 8) | ((U32) s[2] << 16) | ((U32) s [3] << 24);
}

void CInputScript::PrintMatrix(MATRIX matrix, char* str)
{
    printf("%s\t[%5.3f   %5.3f]\n", str, (double)matrix.a/65536.0, (double)matrix.b/65536.0);
    printf("%s\t[%5.3f   %5.3f]\n", str, (double)matrix.c/65536.0, (double)matrix.d/65536.0);
    printf("%s\t[%5.3f   %5.3f]\n", str, (double)matrix.tx/20.0, (double)matrix.ty/20.0);
    /*
    printf("%s\t[%08x   %08x]\n", str, matrix.a, matrix.b);
    printf("%s\t[%08x   %08x]\n", str, matrix.c, matrix.d);
    printf("%s\t[%d   %d]\n", str, matrix.tx, matrix.ty);
    */
}

void CInputScript::PrintRect(SRECT rect, char* str)
{
    printf("\t%s(%g, %g)[%g x %g]\n", str,
            (double)rect.xmin / 20.0, (double)rect.ymin / 20.0,
            (double)(rect.xmax - rect.xmin) / 20.0,
            (double)(rect.ymax - rect.ymin) / 20.0);
}


void CInputScript::where()
{
    printf("where: %04x [%02x]\n", m_filePos, m_fileBuf[m_filePos]);
}




//////////////////////////////////////////////////////////////////////
// Input script object methods.
//////////////////////////////////////////////////////////////////////

CInputScript::CInputScript(void)
// Class constructor.
{
    // Initialize the input pointer.
    m_fileBuf = NULL;

    // Initialize the file information.
    m_filePos = 0;
    m_fileSize = 0;
    m_fileStart = 0;
    m_fileVersion = 0;

    // Initialize the bit position and buffer.
    m_bitPos = 0;
    m_bitBuf = 0;

    // Initialize the output file.
    m_outputFile = NULL;

    // Set to true if we wish to dump all contents long form
    m_dumpAll = false;

    // if set to true will dump image guts (i.e. jpeg, zlib, etc. data)
    m_dumpGuts = false;

    // if set to true will dump sound guts
    m_dumpSoundGuts = false;

    return;
}


CInputScript::~CInputScript(void)
// Class destructor.
{
    // Free the buffer if it is there.
    if (m_fileBuf)
    {
        delete m_fileBuf;
        m_fileBuf = NULL;
        m_fileSize = 0;
    }
}


U16 CInputScript::GetTag(void)
{
    // Save the start of the tag.
    m_tagStart = m_filePos;
    m_tagZero  = m_tagStart;

    // Get the combined code and length of the tag.
    U16 wRawCode = GetWord();
    U16 code = wRawCode;

    // The length is encoded in the tag.
    U32 len = code & 0x3f;

    // Remove the length from the code.
    code = code >> 6;

    // Determine if another long word must be read to get the length.
    if (len == 0x3f)
    {
        len = (U32) GetDWord();
        //printf("\nGetTag: long tag: raw-code: %04x len: 0x%08x\n", wRawCode, len);
        m_tagZero += 4;
    }

    //printf("->> GetTag: code:%04x len:%08x\n", code, len);

    // Determine the end position of the tag.
    m_tagEnd = m_filePos + (U32) len;
    m_tagLen = (U32) len;

    return code;
}


void CInputScript::GetRect (SRECT * r)
{
    InitBits();
    int nBits = (int) GetBits(5);
    r->xmin = GetSBits(nBits);
    r->xmax = GetSBits(nBits);
    r->ymin = GetSBits(nBits);
    r->ymax = GetSBits(nBits);
}


void CInputScript::GetMatrix(MATRIX* mat)
{
    InitBits();

    // Scale terms
    if (GetBits(1))
    {
        int nBits = (int) GetBits(5);
        mat->a = GetSBits(nBits);
        mat->d = GetSBits(nBits);
    }
    else
    {
        mat->a = mat->d = 0x00010000L;
    }

    // Rotate/skew terms
    if (GetBits(1))
    {
        int nBits = (int)GetBits(5);
        mat->b = GetSBits(nBits);
        mat->c = GetSBits(nBits);
    }
    else
    {
        mat->b = mat->c = 0;
    }

    // Translate terms
    int nBits = (int) GetBits(5);
    mat->tx = GetSBits(nBits);
    mat->ty = GetSBits(nBits);
}


void CInputScript::GetCxform(CXFORM* cx, BOOL hasAlpha)
{
    InitBits();

    // !!! The spec has these bits reversed !!!
    BOOL fNeedAdd = (GetBits(1) != 0);
    BOOL fNeedMul = (GetBits(1) != 0);
    // !!! The spec has these bits reversed !!!

    //printf("fNeedMul:%d fNeedAdd:%d\n", fNeedMul, fNeedAdd);

    int nBits = (int) GetBits(4);

    cx->aa = 256; cx->ab = 0;
    if (fNeedMul)
    {
        cx->ra = (S16) GetSBits(nBits);
        cx->ga = (S16) GetSBits(nBits);
        cx->ba = (S16) GetSBits(nBits);
        if (hasAlpha) cx->aa = (S16) GetSBits(nBits);
    }
    else
    {
        cx->ra = cx->ga = cx->ba = 256;
    }

    if (fNeedAdd)
    {
        cx->rb = (S16) GetSBits(nBits);
        cx->gb = (S16) GetSBits(nBits);
        cx->bb = (S16) GetSBits(nBits);
        if (hasAlpha) cx->ab = (S16) GetSBits(nBits);
    }
    else
    {
        cx->rb = cx->gb = cx->bb = 0;
    }
}

void CInputScript::PrintCxform(char* str, CXFORM* pCxform)
{
    printf("%sCXFORM:\n", str);
    printf("%sAlpha:  mul:%04u  add:%04u\n",str, pCxform->aa, pCxform->ab);
    printf("%sRed:    mul:%04u  add:%04u\n",str, pCxform->ra, pCxform->rb);
    printf("%sGreen:  mul:%04u  add:%04u\n",str, pCxform->ga, pCxform->gb);
    printf("%sBlue:   mul:%04u  add:%04u\n",str, pCxform->ba, pCxform->bb);
}

char *CInputScript::GetString(void)
{
    // Point to the string.
    char *str = (char *) &m_fileBuf[m_filePos];

    // Skip over the string.
    while (GetByte());

    return str;
}

U32 CInputScript::GetColor(BOOL fWithAlpha)
{
    U32 r = GetByte();
    U32 g = GetByte();
    U32 b = GetByte();
    U32 a = 0xff;

    if (fWithAlpha)
        a = GetByte();

    return (a << 24) | (r << 16) | (g << 8) | b;
}

void CInputScript::InitBits(void)
{
    // Reset the bit position and buffer.
    m_bitPos = 0;
    m_bitBuf = 0;
}


S32 CInputScript::GetSBits(S32 n)
// Get n bits from the string with sign extension.
{
    // Get the number as an unsigned value.
    S32 v = (S32) GetBits(n);

    // Is the number negative?
    if (v & (1L << (n - 1)))
    {
        // Yes. Extend the sign.
        v |= -1L << n;
    }

    return v;
}


U32 CInputScript::GetBits (S32 n)
// Get n bits from the stream.
{
    U32 v = 0;

    while (true)
    {
        //if (m_bitPos == 0)
        //  printf("bitPos is ZERO: m_bitBuf: %02x\n", m_bitBuf);

        S32 s = n - m_bitPos;
        if (s > 0)
        {
            // Consume the entire buffer
            v |= m_bitBuf << s;
            n -= m_bitPos;

            // Get the next buffer
            m_bitBuf = GetByte();
            m_bitPos = 8;
        }
        else
        {
            // Consume a portion of the buffer
            v |= m_bitBuf >> -s;
            m_bitPos -= n;
            m_bitBuf &= 0xff >> (8 - m_bitPos); // mask off the consumed bits

            //printf("GetBits: nBitsToRead:%d m_bitPos:%d m_bitBuf:%02x v:%d\n", nBitsToRead, m_bitPos, m_bitBuf, v);
            return v;
        }
    }
}


void CInputScript::ParseEnd(char *str)
{
    printf("%stagEnd\n", str);
}


void CInputScript::ParseShowFrame(char *str, U32 frame, U32 offset)
{
    printf("%stagShowFrame\n", str);
    printf("\n%s<----- dumping frame %d file offset 0x%04x ----->\n", str, frame + 1, offset);
}


void CInputScript::ParseFreeCharacter(char *str)
{
    U32 tagid = (U32) GetWord();
    printf("%stagFreeCharacter \ttagid %-5u\n", str, tagid);
}


void CInputScript::ParsePlaceObject(char *str)
{
    U32 tagid = (U32) GetWord();
    U32 depth = (U32) GetWord();
    
    printf("%stagPlaceObject \ttagid %-5u depth %-5u\n", str, tagid, depth);
    
    if (!m_dumpAll)
        return;
        
    MATRIX matrix;
    GetMatrix(&matrix);
    PrintMatrix(matrix, str);

    if (m_filePos < m_tagEnd) 
    {
        CXFORM cxform;
        GetCxform(&cxform, false);
        PrintCxform(str, &cxform);
    }
}


void CInputScript::ParsePlaceObject2(char *str)
{
    U8 flags = GetByte();
    U32 depth = GetWord();

    printf("%stagPlaceObject2 \tflags %-5u depth %-5u ", str, (int) flags, (int) depth);

    if ( flags & splaceMove )
        printf("move ");

    // Get the tag if specified.
    if (flags & splaceCharacter)
    {
        U32 tag = GetWord();
        printf("tag %-5u\n", tag);
    }
    else
    {
        printf("\n");
    }

    // Get the matrix if specified.
    if (flags & splaceMatrix)
    {
        // this one gets called

        MATRIX matrix;
        GetMatrix(&matrix);
        PrintMatrix(matrix, str);
    }

    // Get the color transform if specified.
    if (flags & splaceColorTransform) 
    {
        CXFORM cxform;
        GetCxform(&cxform, true);
        PrintCxform(str, &cxform);
    }        

    // Get the ratio if specified.
    if (flags & splaceRatio)
    {
        U32 ratio = GetWord();
        
        INDENT;
        printf("%ratio %u\n", ratio);
    }        

    // Get the clipdepth if specified.
    if (flags & splaceDefineClip) 
    {
        U32 clipDepth = GetWord();
        INDENT;
        printf("clipDepth %i\n", clipDepth);
    }

    // Get the instance name
    if (flags & splaceName) 
    {
        char* pszName = GetString();
        INDENT;
        printf("instance name %s\n", pszName);
    }
        
    if (!m_dumpAll)
        return;
}


void CInputScript::ParseRemoveObject(char *str)
{
    U32 tagid = (U32) GetWord();
    U32 depth = (U32) GetWord();
    
    printf("%stagRemoveObject \ttagid %-5u depth %-5u\n", str, tagid, depth);
}


void CInputScript::ParseRemoveObject2(char *str)
{
    U32 depth = (U32) GetWord();
    
    printf("%stagRemoveObject2 depth %-5u\n", str, depth);
}


void CInputScript::ParseSetBackgroundColor(char *str)
{
    U32 r = GetByte();
    U32 g = GetByte();
    U32 b = GetByte();
    U32 color = (r << 16) | (g << 8) | b;
    
    printf("%stagSetBackgroundColor \tRGB_HEX %06x\n", str, color);
}




void CInputScript::ParseStartSound(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagStartSound \ttagid %-5u\n", str, tagid);
    
    if (!m_dumpAll)
        return;

    U32 code = GetByte();
    INDENT;   
    printf("%scode %-3u", str, code);

    if ( code & soundHasInPoint )
        printf(" inpoint %u ", GetDWord());
    if ( code & soundHasOutPoint )
        printf(" oupoint %u", GetDWord());
    if ( code & soundHasLoops )
        printf(" loops %u", GetDWord());

    printf("\n");
    if ( code & soundHasEnvelope ) 
    {
        int points = GetByte();

        for ( int i = 0; i < points; i++ ) 
        {
            printf("\n");
            INDENT;   
            printf("%smark44 %u", str, GetDWord());
            printf(" left chanel %u", GetWord());
            printf(" right chanel %u", GetWord());
            printf("\n");
        }
    }
}


void CInputScript::ParseStopSound(char *str)
{
    printf("%stagStopSound\n", str);
}


void CInputScript::ParseProtect(char *str)
{
    printf("%stagProtect\n", str);
}

BOOL CInputScript::ParseShapeRecord(char *str, int& xLast, int& yLast, BOOL fWithAlpha)
{
    // Determine if this is an edge.
    BOOL isEdge = (BOOL) GetBits(1);

    if (!isEdge)
    {
        // Handle a state change
        U16 flags = (U16) GetBits(5);

        // Are we at the end?
        if (flags == 0)
        {
            printf("\tEnd of shape.\n\n");
            return true;
        }

        // Process a move to.
        if (flags & eflagsMoveTo)
        {
            U16 nBits = (U16) GetBits(5);
            S32 x = GetSBits(nBits);
            S32 y = GetSBits(nBits);
            //printf("\tmoveto: nBits:%d x:%d y:%d\n", nBits, x, y);
            xLast = x;
            yLast = y;
            printf("\n\tmoveto: (%g,%g)\n", double(xLast)/20.0, double(yLast)/20.0);
        }
        // Get new fill info.
        if (flags & eflagsFill0)
        {
            int i = GetBits(m_nFillBits);
            printf("\tFillStyle0: %d (%d bits)\n", i, m_nFillBits);
        }
        if (flags & eflagsFill1)
        {
            int i = GetBits(m_nFillBits);
            printf("\tFillStyle1: %d (%d bits)\n", i, m_nFillBits);
        }
        // Get new line info
        if (flags & eflagsLine)
        {
            int i = GetBits(m_nLineBits);
            printf("\tLineStyle: %d\n", i);
        }
        // Check to get a new set of styles for a new shape layer.
        if (flags & eflagsNewStyles)
        {
            printf("\tFound more Style info\n");

            // Parse the style.
            ParseShapeStyle(str, fWithAlpha);

            // Reset.
            m_nFillBits = (U16) GetBits(4);
            m_nLineBits = (U16) GetBits(4);

            //printf("\tm_nFillBits:%d  m_nLineBits:%d\n", m_nFillBits, m_nLineBits);
        }
        if (flags & eflagsEnd)
        {
            printf("\tEnd of shape.\n\n");
        }
  
        return flags & eflagsEnd ? true : false;
    }
    else
    {
        if (GetBits(1))
        {
            // Handle a line
            U16 nBits = (U16) GetBits(4) + 2;   // nBits is biased by 2

            // Save the deltas
            if (GetBits(1))
            {
                // Handle a general line.
                S32 x = GetSBits(nBits);
                S32 y = GetSBits(nBits);
                xLast += x;
                yLast += y;

                printf("\tlineto: (%g,%g).\n", double(xLast)/20.0, double(yLast)/20.0);
            }
            else
            {
                // Handle a vert or horiz line.
                if (GetBits(1))
                {
                    // Vertical line
                    S32 y = GetSBits(nBits);
                    yLast += y;

                    printf("\tvlineto: (%g,%g).\n", double(xLast)/20.0, double(yLast)/20.0);
                }
                else
                {
                    // Horizontal line
                    S32 x = GetSBits(nBits);
                    xLast += x;

                    printf("\thlineto: (%g,%g).\n", double(xLast)/20.0, double(yLast)/20.0);
                }
            }
        }
        else
        {
            // Handle a curve
            U16 nBits = (U16) GetBits(4) + 2;   // nBits is biased by 2

            // Get the control
            S32 cx = GetSBits(nBits);
            S32 cy = GetSBits(nBits);
            xLast += cx;
            yLast += cy;

            printf("\tcurveto: (%g,%g)", double(xLast)/20.0, double(yLast)/20.0);

            // Get the anchor
            S32 ax = GetSBits(nBits);
            S32 ay = GetSBits(nBits);
            xLast += ax;
            yLast += ay;

            printf("(%g,%g)\n", double(xLast)/20.0, double(yLast)/20.0);
        }

        return false;
    }
}

void CInputScript::ParseShapeStyle(char *str, BOOL fWithAlpha)
{
    U16 i = 0;

    // Get the number of fills.
    U16 nFills = GetByte();

    // Do we have a larger number?
    if (nFills == 255)
    {
        // Get the larger number.
        nFills = GetWord();
    }

    printf("\tNumber of fill styles \t%u\n", nFills);

    // Get each of the fill style.
    for (i = 1; i <= nFills; i++)
    {
        U16 fillStyle = GetByte();

        if (fillStyle & fillGradient)
        {
            // Get the gradient matrix.
            MATRIX mat;
            GetMatrix(&mat);

            // Get the number of colors.
            U16 nColors = (U16) GetByte();
            printf("%s\tGradient Fill with %u colors\n", str, nColors);

            // Get each of the colors.
            for (U16 j = 0; j < nColors; j++)
            {
                U8 pos = GetByte();
                U32 rgba = GetColor(fWithAlpha);
                printf("%s\tcolor:%d: at:%d  RGBA:%08x\n", str, j, pos, rgba);
            }
            printf("%s\tGradient Matrix:\n", str);
            PrintMatrix(mat, str);
        }
        else if (fillStyle & fillBits)
        {
            // Get the bitmap matrix.
            U16 uBitmapID = GetWord();
            printf("%s\tBitmap Fill: %04x\n", str, uBitmapID);
            MATRIX mat;
            GetMatrix(&mat);
            PrintMatrix(mat, str);
        }
        else
        {
            // A solid color
            U32 color = GetColor(fWithAlpha);
            printf("%s\tSolid Color Fill RGB_HEX %06x\n", str, color);
        }
    }

    // Get the number of lines.
    U16 nLines = GetByte();

    // Do we have a larger number?
    if (nLines == 255)
    {
        // Get the larger number.
        nLines = GetWord();
    }

    printf("\tNumber of line styles \t%u\n", nLines);

    // Get each of the line styles.
    for (i = 1; i <= nLines; i++)
    {
        U16 width = GetWord();
        U32 color = GetColor(fWithAlpha);
    
        printf("\tLine style %-5u width %g color RGB_HEX %06x\n", i, (double)width/20.0, color);
    }
}

void CInputScript::ParseDefineShape(char *str, BOOL fWithAlpha)
{
    U32 tagid = (U32) GetWord();
    printf("%stagDefineShape \ttagid %-5u\n", str, tagid);

    // Get the bounding rectangle
    SRECT rect;
    GetRect(&rect);

    ParseShapeStyle(str, fWithAlpha);

    InitBits();     // Bug!  this was not in the original swfparse.cpp
                    // Required to reset bit counters and read byte aligned.

    m_nFillBits = (U16) GetBits(4);
    m_nLineBits = (U16) GetBits(4);

    //printf("%sm_nFillBits:%d  m_nLineBits:%d\n", str, m_nFillBits, m_nLineBits);

    int xLast = 0;
    int yLast = 0;

    BOOL atEnd = false;
    while (!atEnd)
        atEnd = ParseShapeRecord(str, xLast, yLast, fWithAlpha);
}


void CInputScript::ParseDefineShape2(char *str)
{
    printf("%stagDefineShape2 -> ", str);
    ParseDefineShape(str);
    //U32 tagid = (U32) GetWord();
    //printf("%stagDefineShape2 \ttagid %-5u\n", str, tagid);
}


void CInputScript::ParseDefineShape3(char *str)
{
    printf("%stagDefineShape3: -> ", str);
    ParseDefineShape(str, true);
    //U32 tagid = (U32) GetWord();
    //printf("%stagDefineShape3 \ttagid %-5u\n", str, tagid);
}


void CInputScript::S_DumpImageGuts(char *str)
{
    U32 lfCount = 0;                
    INDENT;        
    printf("%s----- dumping image details -----", str);
    while (m_filePos < m_tagEnd)
    {
        if ((lfCount % 16) == 0)
        {
            printf("\n");
            INDENT;        
            printf("%s", str);
        }
        lfCount += 1;
        printf("%02x ", GetByte());
    }
    printf("\n");
}

void CInputScript::ParseDefineBits(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineBits \ttagid %-5u\n", str, tagid);

    if (!m_dumpGuts)
        return;

    S_DumpImageGuts(str);
}


void CInputScript::ParseDefineBitsJPEG2(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineBitsJPEG2 \ttagid %-5u\n", str, tagid);

    if (!m_dumpGuts)
        return;

    //S_DumpImageGuts(str);
}


void CInputScript::ParseDefineBitsJPEG3(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineBitsJPEG3 \ttagid %-5u\n", str, tagid);

    if (!m_dumpGuts)
        return;
        
    S_DumpImageGuts(str);
}

void CInputScript::ParseDefineBitsLossless(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineBitsLossless tagid %-5u\n", str, tagid);
    
    if (!m_dumpAll)
        return;
     
    int format = GetByte();
    int width  =  GetWord();
    int height = GetWord();
    int tableSize = 0;

    if (format == 3)
        tableSize = GetByte();

    //INDENT;        
    printf("%sformat %-3u width %-5u height %-5u tableSize %d\n", str, format, width, height, tableSize);

    tableSize += 1;

    z_stream    stream;
    unsigned char*  buffer = &m_fileBuf[m_filePos];
    unsigned char*  colorTable = new unsigned char[tableSize*3];

    stream.next_in = buffer;
    stream.avail_in = 1;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;


    stream.next_out = colorTable;
    stream.avail_out = tableSize*3;

    inflateInit(&stream);

    int status;
    while (1)
    {
        status = inflate(&stream, Z_SYNC_FLUSH);
        if (status == Z_STREAM_END)
            break;
        if (status != Z_OK)
        {
            printf("Zlib cmap error : %s\n", stream.msg);
            return;
        }
        stream.avail_in = 1;
        // Colormap if full
        if (stream.avail_out == 0)
            break;
    }

    printf("\n");
    for (int i=0; i<tableSize*3; i+=3)
    {
        printf("%02x%02x%02x ", colorTable[i], colorTable[i+1], colorTable[i+2]);
        if ((i % 24) == 21)
            printf("\n");
    }

    unsigned char* data = new unsigned char[width*height];

    stream.next_out = data;
    stream.avail_out = width*height;


    while (1)
    {
        status = inflate(&stream, Z_SYNC_FLUSH) ;
        if (status == Z_STREAM_END)
            break;

        if (status != Z_OK)
        {
            printf("Zlib data error : %s\n", stream.msg);
            return;
        }
        stream.avail_in = 1;
    }

    inflateEnd(&stream);

    printf("\n");
    int i = 0;
    for (int y=0; y<height; y++)
    {
        for (int x=0; x<width; x++, i++)
        {
            printf("%02x", data[i]);
        }
        printf("\n");
    }
    printf("\n");



    if (!m_dumpGuts)
        return;
        
    S_DumpImageGuts(str);
}

void CInputScript::ParseDefineBitsLossless2(char *str)
{
    ParseDefineBitsLossless(str);
}

void CInputScript::ParseJPEGTables(char *str)
{
    printf("%stagJPEGTables\n", str);

    if (!m_dumpGuts)
        return;

    S_DumpImageGuts(str);
}


void CInputScript::ParseButtonRecord(char *str, U32 iByte, BOOL fGetColorMatrix)
{
    U32 iPad = iByte >> 4;
    U32 iButtonStateHitTest = (iByte & 0x8);
    U32 iButtonStateDown = (iByte & 0x4);
    U32 iButtonStateOver = (iByte & 0x2);
    U32 iButtonStateUp = (iByte & 0x1);

    U32 iButtonCharacter = (U32)GetWord();
    U32 iButtonLayer = (U32)GetWord();

    printf("%s\tParseButtonRecord: char:%d layer:%d ", str, iButtonCharacter, iButtonLayer);

    if (iButtonStateHitTest != 0)   printf("HIT ");
    if (iButtonStateDown != 0)      printf("DOWN ");
    if (iButtonStateOver != 0)      printf("OVER ");
    if (iButtonStateUp != 0)        printf("UP ");
    printf("\n");

    MATRIX matrix;
    GetMatrix(&matrix);
    PrintMatrix(matrix, str);

    if (fGetColorMatrix)
    {
        // nCharactersInButton always seems to be one
        int nCharactersInButton = 1;

        for (int i=0; i<nCharactersInButton; i++)
        {
            CXFORM cxform;
            GetCxform(&cxform, true);   // ??could be false here??
        }
    }
}

void CInputScript::ParseDefineButton(char *str)
{
    U32 tagid = (U32) GetWord();

    U32 iButtonEnd = (U32)GetByte();
    do
        ParseButtonRecord(str, iButtonEnd, false);
    while ((iButtonEnd = (U32)GetByte()) != 0);

    // parse ACTIONRECORDs until ActionEndFlag
    ParseDoAction(str, false);
}

void CInputScript::ParseDefineButton2(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineButton2 \ttagid %-5u\n", str, tagid);

    U32 iTrackAsMenu = (U32) GetByte();

    // Get offset to first "Button2ActionCondition"
    // This offset is not in the spec!
    U32 iOffset = (U32) GetWord();
    U32 iNextAction = m_filePos + iOffset - 2;

    //
    // Parse Button Records
    //

    U32 iButtonEnd = (U32)GetByte();
    do
        ParseButtonRecord(str, iButtonEnd, true);
    while ((iButtonEnd = (U32)GetByte()) != 0);

    //
    // Parse Button2ActionConditions
    //

    m_filePos = iNextAction;

    U32 iActionOffset = 0;
    while (true)
    {
        iActionOffset = (U32) GetWord();
        //printf("iActionOffset: %04x\n", iActionOffset);
        iNextAction  = m_filePos + iActionOffset - 2;

        U32 iCondition = (U32) GetWord();

        printf("%sCondition: %04x\n", str, iCondition);

        // parse ACTIONRECORDs until ActionEndFlag
        ParseDoAction(str, false);

        // Action Offset of zero means there's no more
        if (iActionOffset == 0)
            break;

        m_filePos = iNextAction;
    }

    printf("\n");
}

void CInputScript::ParseDoAction(char *str, BOOL fPrintTag)
{
    if (fPrintTag)
    {
        printf("%stagDoAction\n",  str);
    }

    for (;;) 
    {
        // Handle the action
        int actionCode = GetByte();
        INDENT;
        printf("%saction code 0x%02x ", str, actionCode);
        if (actionCode == 0)
        {
            // Action code of zero indicates end of actions
            printf("\n");
            return;
        }

        int len = 0;
        if (actionCode & sactionHasLength) 
        {
            len = GetWord();
            printf("has length %-5u ", len);
        }        

        S32 pos = m_filePos + len;

        switch ( actionCode ) 
        {
            case sactionNextFrame:
            {
                printf( "gotoNextFrame\n" );
                break;
            }

            case sactionPrevFrame:
            {
                printf( "gotoPrevFrame\n" );
                break;
            }

            case sactionPlay:
            {
                printf( "play\n" );
                break;
            }

            case sactionStop:
            {
                printf( "stop\n" );
                break;
            }

            case sactionToggleQuality:
            {
                printf( "toggleQuality\n" );
                break;
            }

            case sactionStopSounds:
            {
                printf( "stopSounds\n" );
                break;
            }

            case sactionAdd:
            {
                printf( "add\n" );
                break;
            }

            case sactionSubtract:
            {
                printf( "subtract\n" );
                break;
            }

            case sactionMultiply:
            {
                printf( "multiply\n" );
                break;
            }

            case sactionDivide:
            {
                printf( "divide\n" );
                break;
            }

            case sactionEqual:
            {
                printf( "equal\n" );
                break;
            }

            case sactionLessThan:
            {
                printf( "lessThan\n" );
                break;
            }

            case sactionLogicalAnd:
            {
                printf( "logicalAnd\n" );
                break;
            }

            case sactionLogicalOr:
            {
                printf( "logicalOr\n" );
                break;
            }

            case sactionLogicalNot:
            {
                printf( "logicalNot\n" );
                break;
            }

            case sactionStringEqual:
            {
                printf( "stringEqual\n" );
                break;
            }

            case sactionStringLength:
            {
                printf( "stringLength\n" );
                break;
            }

            case sactionSubString:
            {
                printf( "subString\n" );
                break;
            }

            case sactionInt:
            {
                printf( "int\n" );
                break;
            }

            case sactionEval:
            {
                printf( "eval\n" );
                break;
            }

            case sactionSetVariable:
            {
                printf( "setVariable\n" );
                break;
            }

            case sactionSetTargetExpression:
            {
                printf( "setTargetExpression\n" );
                break;
            }

            case sactionStringConcat:
            {
                printf( "stringConcat\n" );
                break;
            }

            case sactionGetProperty:
            {
                printf( "getProperty\n" );
                break;
            }

            case sactionSetProperty:
            {
                printf( "setProperty\n" );
                break;
            }

            case sactionDuplicateClip:
            {
                printf( "duplicateClip\n" );
                break;
            }

            case sactionRemoveClip:
            {
                printf( "removeClip\n" );
                break;
            }

            case sactionTrace:
            {
                printf( "trace\n" );
                break;
            }

            case sactionStartDragMovie:
            {
                printf( "startDragMovie\n" );
                break;
            }

            case sactionStopDragMovie:
            {
                printf( "stopDragMovie\n" );
                break;
            }

            case sactionStringLessThan:
            {
                printf( "stringLessThan\n" );
                break;
            }

            case sactionRandom:
            {
                printf( "random\n" );
                break;
            }

            case sactionMBLength:
            {
                printf( "mbLength\n" );
                break;
            }

            case sactionOrd:
            {
                printf( "ord\n" );
                break;
            }

            case sactionChr:
            {
                printf( "chr\n" );
                break;
            }

            case sactionGetTimer:
            {
                printf( "getTimer\n" );
                break;
            }

            case sactionMBSubString:
            {
                printf( "mbSubString\n" );
                break;
            }

            case sactionMBOrd:
            {
                printf( "mbOrd\n" );
                break;
            }

            case sactionMBChr:
            {
                printf( "mbChr\n" );
                break;
            }

            case sactionGotoFrame:
            {
                printf("gotoFrame %5u\n", GetWord());
                break;
            }

            case sactionGetURL:
            {
                char *url = GetString();
                char *target = GetString();
                printf("getUrl %s target %s\n", url, target);
                break;
            }

            case sactionWaitForFrame:
            {
                int frame = GetWord();
                int skipCount = GetByte();
                printf("waitForFrame %-5u skipCount %-5u\n", frame, skipCount);
                break;
            }

            case sactionSetTarget:
            {
                // swfparse used to crash here!
                printf("setTarget %s\n", &m_fileBuf[m_filePos]);
                break;
            }

            case sactionGotoLabel: 
            {
                // swfparse used to crash here!
                printf("gotoLabel %s\n", &m_fileBuf[m_filePos]);
                break;
            }

            case sactionWaitForFrameExpression:
            {
                int skipCount = GetByte();
                printf( "waitForFrameExpression skipCount %-5u\n", skipCount );
                break;
            }
                
            case sactionPushData:
            {
                U8 dataType = GetByte();

                // property ids are pushed as floats for some reason
                if ( dataType == 1 )
                {
                    union
                    {
                        U32 dw;
                        float f;
                    } u;

                    u.dw = GetDWord();
                    printf("pushData (float): %08x %.1f\n", u.dw, u.f);
                }
                else
                if ( dataType == 0 )
                {
                    printf( 
                        "pushData (string): %s\n",
                        &m_fileBuf[ m_filePos ]
                    );
                }
                else
                {
                    printf( 
                        "pushData invalid dataType: %02x\n",
                        dataType
                    );
                }


                break;
            }

            case sactionBranchAlways:
            {
                U16 offset = GetWord();
                printf(
                    "branchAlways offset: %-5u\n",
                    offset
                );
                break;
            }

            case sactionGetURL2:
            {
                U8 flag = GetByte();

                if ( flag == 1 )
                {
                    printf( "getUrl2 sendvars=GET\n" );
                }
                else
                if ( flag == 2 )
                {
                    printf( "getUrl2 sendvars=POST\n" );
                }
                else
                {
                    printf( "getUrl2 sendvars=Don't send\n" );
                }
                break;
            }

            case sactionBranchIfTrue:
            {
                U16 offset = GetWord();
                printf(
                    "branchIfTrue offset: %-5u\n",
                    offset
                );
                break;
            }

            case sactionCallFrame:
            {
                printf( "callFrame\n" );
                break;
            }

            case sactionGotoExpression: 
            {
                U8 stopFlag = GetByte();
                if ( stopFlag == 0 )
                {
                    printf("gotoExpression and Stop\n" );
                }
                else
                if ( stopFlag == 1 )
                {
                    printf("gotoExpression and Play\n" );
                }
                else
                {
                    printf("gotoExpression invalid stopFlag: %d\n", stopFlag );
                }
                break;
            }

            default:
            {
                printf("UNKNOWN?\n");
                break;
            }
        }

        m_filePos = pos;
    }
}



void CInputScript::ParseDefineFont(char *str)
{
    U32 iFontID = (U32)GetWord();
    printf("%stagDefineFont \t\tFont ID %-5u\n", str, iFontID);

    int iStart = m_filePos;

    int iOffset = GetWord();
    //printf("%s\tiOffset: 0x%04x\n", str, iOffset);

    int iGlyphCount = iOffset/2;
    m_iGlyphCounts[iFontID] = iGlyphCount;
    printf("%s\tnumber of glyphs: %d\n", str, iGlyphCount);

    int* piOffsetTable = new int[iGlyphCount];
    piOffsetTable[0] = iOffset;

    for(int n=1; n<iGlyphCount; n++)
        piOffsetTable[n] = GetWord();

    for(int n=0; n<iGlyphCount; n++)
    {
        m_filePos = piOffsetTable[n] + iStart;

        InitBits(); // reset bit counter

        m_nFillBits = (U16) GetBits(4);
        m_nLineBits = (U16) GetBits(4);

        //printf("%s\tm_nFillBits:%d m_nLineBits:%d\n", str, m_nFillBits, m_nLineBits);

        int xLast = 0;
        int yLast = 0;

        BOOL fAtEnd = false;

        while (!fAtEnd)
            fAtEnd = ParseShapeRecord(str, xLast, yLast);
    }

    delete piOffsetTable;
}


void CInputScript::ParseDefineFontInfo(char *str)
{
    U32 iFontID = (U32) GetWord();
    printf("%stagDefineFontInfo \tFont ID %-5u\n", str, iFontID);

    int iNameLen = GetByte();
    char* pszName = new char[iNameLen+1];
    int n;
    for(n=0; n < iNameLen; n++)
        pszName[n] = (char)GetByte();
    pszName[n] = '\0';

    printf("%s\tFontName: '%s'\n",str, pszName);

    delete pszName;

    U8 flags = (FontFlags)GetByte();

    int iGlyphCount = m_iGlyphCounts[iFontID];

    int *piCodeTable = new int[iGlyphCount];

    printf("%s\t", str);
    for(n=0; n < iGlyphCount; n++)
    {
        if (flags & fontWideCodes)
            piCodeTable[n] = (int)GetWord();
        else
            piCodeTable[n] = (int)GetByte();
        printf("[%d,'%c'] ", piCodeTable[n], (char)piCodeTable[n]);
    }

    printf("\n\n");

    delete piCodeTable;
}

BOOL CInputScript::ParseTextRecord(char* str, int nGlyphBits, int nAdvanceBits)
{
    U8 flags = (TextFlags)GetByte();
    if (flags == 0) return 0;
    printf("\n%s\tflags: 0x%02x\n", str, flags);

    if (flags & isTextControl)
    {
        if (flags & textHasFont)
        {
            long fontId = GetWord();
            printf("%s\tfontId: %d\n", str, fontId);
        }
        if (flags & textHasColor)
        {
            int r = GetByte();
            int g = GetByte();
            int b = GetByte();
            printf("%s\tfontColour: (%d,%d,%d)\n", str, r, g, b);
        }
        if (flags & textHasXOffset)
        {
            int iXOffset = GetWord();
            printf("%s\tX-offset: %d\n", str, iXOffset);
        }
        if (flags & textHasYOffset)
        {
            int iYOffset = GetWord();
            printf("%s\tY-offset: %d\n", str, iYOffset);
        }
        if (flags & textHasFont)
        {
            int iFontHeight = GetWord();
            printf("%s\tFont Height: %d\n", str, iFontHeight);
        }
    }
    else
    {
        int iGlyphCount = flags;
        printf("%s\tnumber of glyphs: %d\n", str, iGlyphCount);

        InitBits();     // reset bit counter

        printf("%s\t", str);
        for (int g = 0; g < iGlyphCount; g++)
        {
            int iIndex = GetBits(nGlyphBits);
            int iAdvance = GetBits(nAdvanceBits);
            printf("[%d,%d] ", iIndex, iAdvance);
        }
        printf("\n");
    }

    return true;
}


void CInputScript::ParseDefineText(char *str)
{
    U32 tagid = (U32) GetWord();
    printf("%stagDefineText \t\ttagid %-5u\n", str, tagid);

    SRECT   rect;
    GetRect(&rect);
    PrintRect(rect, str);

    MATRIX  m;
    GetMatrix(&m);
    PrintMatrix(m, str);


    int nGlyphBits = (int)GetByte();
    int nAdvanceBits = (int)GetByte();

    printf("%s\tnGlyphBits:%d nAdvanceBits:%d\n", str, nGlyphBits, nAdvanceBits);

    BOOL fContinue = true;

    do
        fContinue = ParseTextRecord(str, nGlyphBits, nAdvanceBits);
    while (fContinue);

    printf("\n");
}

void CInputScript::ParseDefineEditText(char* str)
{
    U32 tagid = (U32) GetWord();

    SRECT rBounds;
    GetRect(&rBounds);

    U16 flags = GetWord();

    printf("%stagDefineEditText: tagid %d flags:%04x ", str, tagid, flags);

    if (flags & seditTextFlagsHasFont)
    {
        U16 uFontId = GetWord();
        U16 uFontHeight = GetWord();
        printf("FontId:%d FontHeight:%d ", uFontId, uFontHeight);
    }

    if (flags & seditTextFlagsHasTextColor)
    {
        GetColor(true);
    }

    if (flags & seditTextFlagsHasMaxLength)
    {
        int iMaxLength = GetWord();
        printf("length:%d ", iMaxLength);
    }

    if (flags & seditTextFlagsHasLayout)
    {
        int iAlign = GetByte();
        U16 uLeftMargin = GetWord();
        U16 uRightMargin = GetWord();
        U16 uIndent = GetWord();
        U16 uLeading = GetWord();
    }

    char* pszVariable = GetString();
    printf("variable:%s ", pszVariable);

    if (flags & seditTextFlagsHasText )
    {
        char* pszInitialText = GetString();
        printf("text:%s ", pszInitialText);
    }

    printf("\n");
}

void CInputScript::ParseDefineFont2(char *str)
{
    U32 tagid = (U32) GetWord();

    U16 flags = GetWord();

    // Skip the font name
    int iNameLen = GetByte();
    char szFontName[256];
    int	i;
    for (i=0; i<iNameLen; i++)
        szFontName[i] = GetByte();
    szFontName[i] = NULL;
    
    // Get the number of glyphs.
    U16 nGlyphs = GetWord();

    int iDataPos = m_filePos;

    printf("%stagDefineFont2 \ttagid %-5u flags:%04x nGlyphs:%d\n", str, tagid, flags, nGlyphs);

    if (nGlyphs > 0)
    {
        //
        // Get the FontOffsetTable
        //

        U32* puOffsetTable = new U32[nGlyphs];
        for (int n=0; n<nGlyphs; n++)
            if (flags & sfontFlagsWideOffsets)
                puOffsetTable[n] = GetDWord();
            else
                puOffsetTable[n] = GetWord();

        //
        // Get the CodeOffset
        //

        U32 iCodeOffset = 0;
        if (flags & sfontFlagsWideOffsets)
            iCodeOffset = GetDWord();
        else
            iCodeOffset = GetWord();

        //
        // Get the Glyphs
        //

        for(int n=0; n<nGlyphs; n++)
        {
            printf("\n\t%s>>> Glyph:%d", str, n);
            m_filePos = iDataPos + puOffsetTable[n];

            InitBits(); // reset bit counter

            m_nFillBits = (U16) GetBits(4);
            m_nLineBits = (U16) GetBits(4);

            int xLast = 0;
            int yLast = 0;

            BOOL fAtEnd = false;

            while (!fAtEnd)
                fAtEnd = ParseShapeRecord(str, xLast, yLast);
        }

        delete puOffsetTable;


        if (m_filePos != iDataPos + iCodeOffset)
        {
            printf("Bad CodeOffset\n");
            return;
        }

        //
        // Get the CodeTable
        //
            
        m_filePos = iDataPos + iCodeOffset;
        printf("\n%sCodeTable:\n%s", str, str);

        for (int i=0; i<nGlyphs; i++)
        {
            if (flags & sfontFlagsWideOffsets)
                printf("%02x:[%04x] ", i, GetWord());
            else
                printf("%02x:[%c] ", i, GetByte());

            if ((i & 7) == 7)
                printf("\n%s", str);
        }
        printf("\n");
    }

    if (flags & sfontFlagsHasLayout)
    {
        //
        // Get "layout" fields
        //

        S16 iAscent = GetWord();
        S16 iDescent = GetWord();
        S16 iLeading = GetWord();

        printf("\n%sHasLayout: iAscent:%d iDescent:%d iLeading:%d\n", str, iAscent, iDescent, iLeading);

        // Skip Advance table
        SkipBytes(nGlyphs * 2);


        // Get BoundsTable
        int i;
        for (i=0; i<nGlyphs; i++)
        {
            SRECT rBounds;
            GetRect(&rBounds);
            //printf("rBounds: (%d,%d)(%d,%d)\n", rBounds.xmin, rBounds.ymin, rBounds.xmax, rBounds.ymax);
        }

        //
        // Get Kerning Pairs
        //

        S16 iKerningCount = GetWord();
        printf("\n%sKerning Pair Count:%d\n", str, iKerningCount);
        for (i=0; i<iKerningCount; i++)
        {
            U16 iCode1, iCode2;
            if (flags & sfontFlagsWideOffsets)
            {
                iCode1 = GetWord();
                iCode2 = GetWord();
            }
            else
            {
                iCode1 = GetByte();
                iCode2 = GetByte();
            }
            S16 iAdjust = GetWord();

            printf("%sKerningPair:%-4d %c <--> %c : %d\n", str, i, iCode1, iCode2, iAdjust);
        }

        printf("m_tagEnd:%08x m_filePos:%08x\n", m_tagEnd, m_filePos);
    }
}



void CInputScript::ParseDefineText2(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineText2 \ttagid %-5u\n", str, tagid);
}

void CInputScript::ParseDefineMorphShape(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineMorphShape: tagid:%d\n", str, tagid);

    SRECT r1, r2;
    GetRect(&r1);
    GetRect(&r2);

    // Calculate the position of the end shape edges
    U32 iOffset = GetDWord();
    U32 iEndShapePos = m_filePos + iOffset;

    // Always get RGBA not RGB for DefineMorphShape
    BOOL fGetAlpha = true;

    // Get the fills
    int nFills = GetByte();
    if ( nFills >= 255 )
        nFills = GetWord();

    for (int i = 1; i <= nFills; i++ )
    {
        int fillStyle = GetByte();
        if (fillStyle & fillGradient)
        {
            // Gradient fill
            MATRIX mat1, mat2;
            GetMatrix(&mat1);
            GetMatrix(&mat2);

            // Get the gradient color points
            int nColors = GetByte();
            for (int j = 0; j < nColors; j++)
            {
                U8 r1, r2;
                U32 c1, c2;

                r1 = GetByte();
                c1 = GetColor(fGetAlpha);
                r2 = GetByte();
                c2 = GetColor(fGetAlpha);
            }
        }
        else if (fillStyle & fillBits)
        {
            // A bitmap fill
            U16 tag = GetWord();        // the bitmap tag

            MATRIX mat1, mat2;
            GetMatrix(&mat1);
            GetMatrix(&mat2);
        }
        else
        {
            // A solid color
            U32 rgb1 = GetColor(fGetAlpha);
            U32 rgb2 = GetColor(fGetAlpha);
        }
    }
    
    // Get the lines
    int nLines = GetByte();
    if ( nLines >= 255 )
        nLines = GetWord();

    for (int i = 1; i <= nLines; i++ )
    {
        U16 thick1, thick2;
        U32 rgb1, rgb2;
            
        // get the thickness
        thick1 = GetWord();
        thick2 = GetWord();

        // get the color
        rgb1 = GetColor(fGetAlpha);
        rgb2 = GetColor(fGetAlpha);
    }

    //
    // Get the bits per style index for the start shape
    //

    InitBits();
    m_nFillBits = (U16) GetBits(4);
    m_nLineBits = (U16) GetBits(4);

    //
    // Parse the start shape
    //

    printf("\n\t--- StartShape ---");
    BOOL atEnd = false;
    int  xLast = 0;
    int  yLast = 0;
    while (!atEnd)
        atEnd = ParseShapeRecord(str, xLast, yLast, true);

    if (m_filePos != iEndShapePos)
    {
        printf("Bad offset to end shape\n");
        return;
    }

    //
    // Get the bits per style index for the end shape
    // THIS IS POINTLESS -- THERE ARE NO STYLES ?!
    //

    InitBits();
    m_nFillBits = (U16) GetBits(4);     // not sure if we should save these to n_FillBits & nLineBits
    m_nLineBits = (U16) GetBits(4);     // there are no styles so none of this make sense.

    //
    // Parse the end shape
    //

    printf("\t--- EndShape ---");
    atEnd = false;
    xLast = 0;
    yLast = 0;
    while (!atEnd)
        atEnd = ParseShapeRecord(str, xLast, yLast, true);
}

//
// MP3 tables
//

int vertab[4]={2,3,1,0};
int freqtab[4]={44100,48000,32000};
int ratetab[2][3][16]=
{
  {
    {  0, 32, 64, 96,128,160,192,224,256,288,320,352,384,416,448,  0},
    {  0, 32, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,384,  0},
    {  0, 32, 40, 48, 56, 64, 80, 96,112,128,160,192,224,256,320,  0},
  },
  {
    {  0, 32, 48, 56, 64, 80, 96,112,128,144,160,176,192,224,256,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
    {  0,  8, 16, 24, 32, 40, 48, 56, 64, 80, 96,112,128,144,160,  0},
  },
};

void CInputScript::DecodeMp3Frame(U8* pbFrame, int iEncodedSize, int iDecodedSize)
{
    // This is left as an exercise for the reader (he he)
}

void CInputScript::DecodeMp3Headers(char* str, int iSamplesPerFrame)
{
    int iFrameCount = 0;

    if (iSamplesPerFrame > 0)
    {
        while (true)
        {
            // Get the MP3 frame header
            U8  hdr[4];
            for (int i=0; i<4; i++)
                hdr[i] = GetByte();

            // Decode the MP3 frame header
            int ver     = vertab[((hdr[1] >> 3) & 3)];
            int layer   = 3 - ((hdr[1] >> 1) & 3);
            int pad     = (hdr[2] >>1 ) & 1;
            int stereo  = ((hdr[3] >> 6) & 3) != 3;
            int freq    = 0;
            int rate    = 0;

            if (hdr[0] != 0xFF || hdr[1] < 0xE0 || ver==3 || layer != 2)
            {
                // bad MP3 header
                printf("\t\tBAD MP3 FRAME HEADER\n");
                break;
            }
            else
            {
                freq = freqtab[(hdr[2] >>2 ) & 3] >> ver;
                rate = ratetab[ver ? 1 : 0][layer][(hdr[2] >> 4) & 15] * 1000;

                if (!freq || !rate)
                {
                    // bad MP3 header
                    printf("\t\tBAD MP3 FRAME HEADER\n");
                    break;
                }
            }

            // Get the size of a decoded MP3 frame
            int iDecodedFrameSize = (576 * (stereo + 1));
            if (!ver)
                iDecodedFrameSize *= 2;

            // Get the size of this encoded MP3 frame
            int iEncodedFrameSize = ((ver ? 72 : 144) * rate) / freq + pad - 4;

            char* ppszMpegVer[4] = {"1","2","2.5","3?"};

            printf("%s  Frame%d: MPEG%s Layer%d %dHz %s %dbps size:Encoded:%d Decoded:%d\n",
                str, iFrameCount, ppszMpegVer[ver], layer+1, freq, stereo ? "stereo" : "mono", rate, iEncodedFrameSize, iDecodedFrameSize);

            // Decode the MP3 frame
            DecodeMp3Frame(&m_fileBuf[m_filePos], iEncodedFrameSize, iDecodedFrameSize);

            // Move to the next frame
            iFrameCount++;
            if (m_filePos + iEncodedFrameSize >= m_tagEnd)
                break;
            m_filePos += iEncodedFrameSize;
        }
    }
    printf("\n");
}

//
// ADPCM tables
//

static const int indexTable2[2] = {
    -1, 2,
};

// Is this ok?
static const int indexTable3[4] = {
    -1, -1, 2, 4,
};

static const int indexTable4[8] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static const int indexTable5[16] = {
 -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16,
};

static const int* indexTables[] = {
 indexTable2,
 indexTable3,
 indexTable4,
 indexTable5
};

static const int stepsizeTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};



void CInputScript::AdpcmFillBuffer()
{
    while (m_bitPosAdpcm <= 24)
    {
        m_bitBufAdpcm = (m_bitBufAdpcm<<8) | *m_srcAdpcm++;
        m_bitPosAdpcm += 8;
    }
}

long CInputScript::AdpcmGetBits(int n)
{
    if (m_bitPosAdpcm < n)
        AdpcmFillBuffer();

    //assert(bitPos >= n);

    long v = ((U32)m_bitBufAdpcm << (32-m_bitPosAdpcm)) >> (32-n);
    m_bitPosAdpcm -= n;

    return v;
}

long CInputScript::AdpcmGetSBits(int n)
{
    if (m_bitPosAdpcm < n)
        AdpcmFillBuffer();

    //assert(bitPos >= n);

    long v = ((S32)m_bitBufAdpcm << (32-m_bitPosAdpcm)) >> (32-n);
    m_bitPosAdpcm -= n;

    return v;
}


void CInputScript::AdpcmDecompress(long n, long stereo, int f16Bit, U8* data)
{
    long    valpred[2]; // Current state
    int     index[2];

    for (int i=0; i<2; i++)
    {
        valpred[i] = 0;
        index[i] = 0;
    }

    int iSampleCount = n;

    // Get the compression header
    int nBits = (int)GetBits(2)+2;

    printf("%d-bit ADPCM %d-bit %s ", nBits, f16Bit ? 16 : 8, stereo ? "stereo" : "mono");
    if (!m_dumpSoundGuts)
        printf("\n");

    const int* indexTable = indexTables[nBits-2];
    int k0 = 1 << (nBits-2);
    int signmask = 1 << (nBits-1);


    if (f16Bit)
    {
        short*  dst = (short*)data;

        if (!stereo)
        {
            // Optimize for mono
            long    vp = valpred[0]; // maybe these can get into registers...
            int     ind = index[0];
            long    ns = m_nSamplesAdpcm;

            while ( n-- > 0 )
            {
                ns++;

                if ((ns & 0xFFF) == 1)
                {
                    // Get a new block header
                    vp = GetSBits(16);
                    if (dst != NULL)
                        *dst++ = (short)vp;

                    ind = (int)GetBits(6); // The first sample in a block does not have a delta

                    if (m_dumpSoundGuts)
                        printf("initial sample:%d index:%d\n", vp, ind);
                }
                else
                {
                    // Process a delta value
                    int delta = (int)GetBits(nBits);

                    // Compute difference and new predicted value
                    // Computes 'vpdiff = (delta+0.5)*step/4'
                    int step = stepsizeTable[ind];
                    long vpdiff = 0;
                    int k = k0;

                    do
                    {
                        if (delta & k)
                            vpdiff += step;
                        step >>= 1;
                        k >>= 1;
                    }
                    while (k);

                    vpdiff += step; // add 0.5

                    if (delta & signmask) // the sign bit
                        vp -= vpdiff;
                    else
                        vp += vpdiff;

                    // Find new index value
                    ind += indexTable[delta & (~signmask)];

                    if (ind < 0)
                        ind = 0;
                    else if (ind > 88)
                        ind = 88;

                    // clamp output value
                    if (vp != (short)vp)
                        vp = vp < 0 ? -32768 : 32767;

                    if (m_dumpSoundGuts)
                    {
                        printf("%-2d=>%-6d ", delta, (short)vp);
                        if ((ns & 3) == 0)
                            printf("\n");
                    }

                    /* Step 7 - Output value */
                    if (dst != NULL)
                        *dst++ = (short)vp;

                    /*
                    if (m_dumpSoundGuts)
                    {
                        if ((ns & 0xFFF) == 0)
                        {
                            printf("\nind:%d vp:%d delta:%d\n", ind, vp, delta);
                        }
                    }
                    */
                }
            }

            valpred[0] = vp;
            index[0] = ind;
            m_nSamplesAdpcm = ns;
        }
        else
        {
            int sn = stereo ? 2 : 1;

            // Stereo
            while ( n-- > 0 ) {

                m_nSamplesAdpcm++;

                if ((m_nSamplesAdpcm & 0xFFF) == 1 )
                {
                    // Get a new block header
                    for ( int i = 0; i < sn; i++ )
                    {
                        valpred[i] = GetSBits(16);
                        if (dst != NULL)
                            *dst++ = (short)valpred[i];

                        // The first sample in a block does not have a delta
                        index[i] = (int)GetBits(6);
                    }
                }
                else
                {
                    // Process a delta value
                    for ( int i = 0; i < sn; i++ ) {
                        int delta = (int)GetBits(nBits);

                        // Compute difference and new predicted value
                        // Computes 'vpdiff = (delta+0.5)*step/4'

                        int step = stepsizeTable[index[i]];
                        long vpdiff = 0;
                        int k = k0;

                        do {
                            if ( delta & k ) vpdiff += step;
                            step >>= 1;
                            k >>= 1;
                        } while ( k );
                        vpdiff += step; // add 0.5


                        if ( delta & signmask ) // the sign bit
                            valpred[i] -= vpdiff;
                        else
                            valpred[i] += vpdiff;

                        // Find new index value
                        index[i] += indexTable[delta&(~signmask)];

                        if ( index[i] < 0 )
                            index[i] = 0;
                        else if ( index[i] > 88 )
                            index[i] = 88;

                        // clamp output value
                        if ( valpred[i] != (short)valpred[i] )
                            valpred[i] = valpred[i] < 0 ? -32768 : 32767;


                        /* Step 7 - Output value */
                        if (dst != NULL)
                            *dst++ = (short)valpred[i];
                    }
                }
            }
        }
    }
    else
    {
        U8* dst = data;
        U8* pbSamples = dst;

        if (!stereo)
        {
            // Optimize for mono
            long    vp = valpred[0]; // maybe these can get into registers...
            int     ind = index[0];
            long    ns = m_nSamplesAdpcm;

            while ( n-- > 0 )
            {
                ns++;

                if ((ns & 0xFFF) == 1)
                {
                    // Get a new block header
                    vp = GetBits(8);
                    if (dst != NULL)
                        *dst++ = (U8)vp;

                    ind = (int)GetBits(6); // The first sample in a block does not have a delta

                    if (m_dumpSoundGuts)
                        printf("initial sample:%d index:%d\n", vp, ind);
                }
                else
                {
                    // Process a delta value
                    int delta = (int)GetBits(nBits);

                    // Compute difference and new predicted value
                    // Computes 'vpdiff = (delta+0.5)*step/4'
                    int step = stepsizeTable[ind];
                    long vpdiff = 0;
                    int k = k0;

                    do
                    {
                        if (delta & k)
                            vpdiff += step;
                        step >>= 1;
                        k >>= 1;
                    }
                    while (k);

                    vpdiff += step; // add 0.5

                    if (delta & signmask) // the sign bit
                        vp -= vpdiff;
                    else
                        vp += vpdiff;

                    // Find new index value
                    ind += indexTable[delta & (~signmask)];

                    if (ind < 0)
                        ind = 0;
                    else if (ind > 88)
                        ind = 88;

                    // clamp output value
                    if (vp < 0)
                        vp = 0;
                    else if (vp > 255)
                        vp = 255;

                    if (m_dumpSoundGuts)
                    {
                        printf("%-2d=>%-6d ", delta, vp);
                        if ((ns & 3) == 0)
                            printf("\n");
                    }

                    /* Step 7 - Output value */
                    if (dst != NULL)
                        *dst++ = (U8)vp;

                    if ( m_dumpSoundGuts )
                    {
                        if ((ns & 0xFFF) == 0)
                        {
                            printf("\nind:%d vp:%d delta:%d\n", ind, vp, delta);
                        }
                    }
                }
            }

            valpred[0] = vp;
            index[0] = ind;
            m_nSamplesAdpcm = ns;

        }
        else
        {
            int sn = stereo ? 2 : 1;

            // Stereo
            while ( n-- > 0 ) {

                m_nSamplesAdpcm++;

                if ((m_nSamplesAdpcm & 0xFFF) == 1 )
                {
                    // Get a new block header
                    for ( int i = 0; i < sn; i++ )
                    {
                        valpred[i] = GetBits(8);
                        if (dst != NULL)
                            *dst++ = (U8)valpred[i];

                        // The first sample in a block does not have a delta
                        index[i] = (int)GetBits(6);
                    }
                }
                else
                {
                    // Process a delta value
                    for ( int i = 0; i < sn; i++ )
                    {
                        int delta = (int)GetBits(nBits);

                        // Compute difference and new predicted value
                        // Computes 'vpdiff = (delta+0.5)*step/4'

                        int step = stepsizeTable[index[i]];
                        long vpdiff = 0;
                        int k = k0;

                        do {
                            if ( delta & k ) vpdiff += step;
                            step >>= 1;
                            k >>= 1;
                        } while ( k );
                        vpdiff += step; // add 0.5


                        if ( delta & signmask ) // the sign bit
                            valpred[i] -= vpdiff;
                        else
                            valpred[i] += vpdiff;

                        // Find new index value
                        index[i] += indexTable[delta&(~signmask)];

                        if ( index[i] < 0 )
                            index[i] = 0;
                        else if ( index[i] > 88 )
                            index[i] = 88;

                        // clamp output value
                        if ( valpred[i] != (short)valpred[i] )
                            valpred[i] = valpred[i] < 0 ? -32768 : 32767;


                        /* Step 7 - Output value */
                        if (dst != NULL)
                            *dst++ = (U8)valpred[i];
                    }
                }
            }
        }
    }

    /*
    printf("\n");
    for (i=0; i<iSampleCount; i++)
    {
        printf("%-6d ", psSamples[i]);
        if ((i & 7) == 7)
            printf("\n");
    }
    printf("\n");
    */
}



void CInputScript::ParseDefineSound(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineSound: ", str);
        
    int iCompression = GetBits(4);      // uncompressed, ADPCM or MP3
    int iSampleRate  = GetBits(2);
    int iSampleSize  = GetBits(1);
    int iStereoMono  = GetBits(1);
    int iSampleCount = GetDWord();


    const char* ppszCompression[3] = {"uncompressed", "ADPCM", "MP3"};
    const char* ppszSampleRate[4]  = {"5.5", "11", "22", "44"};
    const char* pszSampleSize      = (iSampleSize == 0 ? "8" : "16");
    const char* pszStereoMono      = (iStereoMono == 0 ? "mono" : "stereo");

    printf("%s %skHz %s-bit %s NumberOfSamples:%d (%08x)\n",
        ppszCompression[iCompression], ppszSampleRate[iSampleRate],
        pszSampleSize, pszStereoMono, iSampleCount, iSampleCount);

    if (!m_dumpSoundGuts)
        return;

    switch (iCompression)
    {
        case 0:
        {
            printf("%s  uncompressed samples\n", str);
            break;
        }
        case 1:
        {
            m_nSamplesAdpcm = 0;
            m_srcAdpcm = &m_fileBuf[m_filePos];
            AdpcmDecompress(iSampleCount, iStereoMono, iSampleSize);
            break;
        }
        case 2:
        {
            int iDelay = GetWord();
            printf("%s  MP3: delay:%d\n", str, iDelay);
            DecodeMp3Headers(str, iSampleCount);
            break;
        }
    }

    //U8* pbSamples = new U8[iSampleCount * (iSoundSize ? 2 : 1)];
    //delete pbSamples;
}


void CInputScript::ParseDefineButtonSound(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineButtonSound \ttagid %-5u\n", str, tagid);
    
    if (!m_dumpAll)
        return;
        
    // step through for button states
    for (int i = 0; i < 3; i++)
    {
        U32 soundTag = GetWord();
        switch (i)
        {
            case 0:         
                INDENT;   
                printf("%supState \ttagid %-5u\n", str, soundTag);
                break;
            case 1:            
                INDENT;   
                printf("%soverState \ttagid %-5u\n", str, soundTag);
                break;
            case 2:            
                INDENT;   
                printf("%sdownState \ttagid %-5u\n", str, soundTag);
                break;
        }
         
        if (soundTag)
        {
            U32 code = GetByte();
            INDENT;   
            printf("%ssound code %u", str, code);

            if ( code & soundHasInPoint )
                printf(" inpoint %u", GetDWord());
            if ( code & soundHasOutPoint )
                printf(" outpoint %u", GetDWord());
            if ( code & soundHasLoops )
                printf(" loops %u", GetDWord());

            printf("\n");
            if ( code & soundHasEnvelope ) 
            {
                int points = GetByte();

                for ( int i = 0; i < points; i++ ) 
                {
                    printf("\n");
                    INDENT;   
                    printf("%smark44 %u", str, GetDWord());
                    printf(" left chanel %u", GetWord());
                    printf(" right chanel %u", GetWord());
                    printf("\n");
                }
            }
        }
    }
        
}


void CInputScript::ParseSoundStreamHead(char *str)
{
    ParseSoundStreamHead2(str, false);
}

void CInputScript::ParseSoundStreamHead2(char *str, BOOL fIsHead2)
{
    int iMixFormat = GetByte();

    // The stream settings
    m_iStreamCompression = GetBits(4);
    m_iStreamSampleRate  = GetBits(2);
    m_iStreamSampleSize  = GetBits(1);
    m_iStreamStereoMono  = GetBits(1);
    m_nStreamSamples     = GetWord();

    const char* ppszCompression[3] = {"uncompressed", "ADPCM", "MP3"};
    const char* ppszSampleRate[4]  = {"5.5", "11", "22", "44"};
    const char* pszSampleSize      = (m_iStreamSampleSize == 0 ? "8" : "16");
    const char* pszStereoMono      = (m_iStreamStereoMono == 0 ? "mono" : "stereo");

    printf("%stagSoundStreamHead%s: %s %skHz %s-bit %s AverageSamplesPerFrame:%d\n",
        str, fIsHead2 ? "2" : "", ppszCompression[m_iStreamCompression], ppszSampleRate[m_iStreamSampleRate],
        pszSampleSize, pszStereoMono, m_nStreamSamples);
}

void CInputScript::ParseSoundStreamBlock(char *str)
{
    printf("%stagSoundStreamBlock: ", str);

    switch (m_iStreamCompression)
    {
        case 0:
        {
            printf("%s  uncompressed samples\n", str);
            break;
        }
        case 1:
        {
            m_nSamplesAdpcm = 0;
            m_srcAdpcm = &m_fileBuf[m_filePos];
            AdpcmDecompress(m_nStreamSamples, m_iStreamStereoMono, m_iStreamSampleSize);
            break;
        }
        case 2:
        {
            U16 iSamplesPerFrame  = GetWord();
            U16 iDelay  = GetWord();

            printf("%s  MP3: SamplesPerFrame:%d Delay:%d\n", str, iSamplesPerFrame, iDelay);
            DecodeMp3Headers(str, iSamplesPerFrame);
        }
    }
}


void CInputScript::ParseDefineButtonCxform(char *str)
{
    U32 tagid = (U32) GetWord();

    printf("%stagDefineButtonCxform \ttagid %-5u\n", str, tagid);
    
    if (!m_dumpAll)
        return;
        
    while (m_filePos < m_tagEnd)
    {
        CXFORM cxform;
        GetCxform(&cxform, false);
        PrintCxform(str, &cxform);
    }
}

void CInputScript::ParseNameCharacter(char *str)
{
    U32 tagid = (U32) GetWord();
    char *label = GetString();

    printf("%stagNameCharacter \ttagid %-5u label '%s'\n", str, tagid, label);
}


void CInputScript::ParseFrameLabel(char *str)
{
    char *label = GetString();

    printf("%stagFrameLabel lable \"%s\"\n", str, label);
}


void CInputScript::ParseDefineMouseTarget(char *str)
{
    printf("%stagDefineMouseTarget\n", str);
}


void CInputScript::ParseUnknown(char *str, U16 code)
{
    printf("%sUnknown Tag:0x%02x len:0x%02x\n", str, code, m_tagLen);
}


void CInputScript::ParseTags(BOOL sprite, U32 tabs)
// Parses the tags within the file.
{

    char str[33];   // indent level
    
    {
        U32 i = 0;
    
        for (i = 0; i < tabs && i < 32; i++)
        {
            str[i] = '\t';
        }
        str[i] = 0;
    }        

    if (sprite)
    {
        U32 tagid = (U32) GetWord();
        U32 frameCount = (U32) GetWord();

        printf("%stagDefineSprite \ttagid %-5u \tframe count %-5u\n", str, tagid, frameCount);
    }
    else
    {        
        printf("\n%s<----- dumping frame %d file offset 0x%04x ----->\n", str, 0, m_filePos);
        
        // Set the position to the start position.
        m_filePos = m_fileStart;
    }        
    
    // Initialize the end of frame flag.
    BOOL atEnd = false;

    // Reset the frame position.
    U32 frame = 0;

    // Loop through each tag.
    while (!atEnd)
    {
        // Get the current tag.
        U16 code = GetTag();

        if (false)
        {
            printf("Tag dump: %04x: ", m_tagStart);
            for (U32 i=m_tagStart; i<m_tagStart+8; i++)
                printf("%02x ", m_fileBuf[i]);
        }


        // Get the tag ending position.
        U32 tagEnd = m_tagEnd;

        switch (code)
        {
            case stagEnd:

                // Parse the end tag.
                ParseEnd(str);

                // We reached the end of the file.
                atEnd = true;

                break;
        
            case stagShowFrame:
                ParseShowFrame(str, frame, tagEnd);

                // Increment to the next frame.
                ++frame;
                break;

            case stagFreeCharacter:
                ParseFreeCharacter(str);
                break;

            case stagPlaceObject:
                ParsePlaceObject(str);
                break;

            case stagPlaceObject2:
                ParsePlaceObject2(str);
                break;

            case stagRemoveObject:
                ParseRemoveObject(str);
                break;

            case stagRemoveObject2:
                ParseRemoveObject2(str);
                break;

            case stagSetBackgroundColor:
                ParseSetBackgroundColor(str);
                break;

            case stagDoAction:
                ParseDoAction(str);
                break;

            case stagStartSound:
                ParseStartSound(str);
                break;

            case stagProtect:
                ParseProtect(str);
                break;

            case stagDefineShape: 
                ParseDefineShape(str);
                break;

            case stagDefineShape2:
                ParseDefineShape2(str);
                break;

            case stagDefineShape3:
                ParseDefineShape3(str);
                break;

            case stagDefineBits:
                ParseDefineBits(str);
                break;

            case stagDefineBitsJPEG2:
                ParseDefineBitsJPEG2(str);
                break;

            case stagDefineBitsJPEG3:
                ParseDefineBitsJPEG3(str);
                break;

            case stagDefineBitsLossless:
                ParseDefineBitsLossless(str);
                break;

            case stagDefineBitsLossless2:
                ParseDefineBitsLossless2(str);
                break;

            case stagJPEGTables:
                ParseJPEGTables(str);
                break;

            case stagDefineButton:
                ParseDefineButton(str);
                break;

            case stagDefineButton2:
                ParseDefineButton2(str);
                break;

            case stagDefineFont:
                ParseDefineFont(str);
                break;

            case stagDefineMorphShape:
                ParseDefineMorphShape(str);
                break;

            case stagDefineFontInfo:
                ParseDefineFontInfo(str);
                break;

            case stagDefineText:
                ParseDefineText(str);
                break;

            case stagDefineText2:
                ParseDefineText2(str);
                break;

            case stagDefineSound:
                ParseDefineSound(str);
                break;

            case stagDefineEditText:
                ParseDefineEditText(str);
                break;

            case stagDefineButtonSound:
                ParseDefineButtonSound(str);
                break;

            case stagSoundStreamHead:
                ParseSoundStreamHead(str);
                break;

            case stagSoundStreamHead2:
                ParseSoundStreamHead2(str);
                break;

            case stagSoundStreamBlock:
                ParseSoundStreamBlock(str);
                break;

            case stagDefineButtonCxform:
                ParseDefineButtonCxform(str);
                break;

            case stagDefineSprite:
                ParseTags(true, tabs + 1);
                break;

            case stagNameCharacter:
                ParseNameCharacter(str);
                break;

            case stagFrameLabel:
                ParseFrameLabel(str);
                break;

            case stagDefineFont2:
                ParseDefineFont2(str);
                break;

            default:
                ParseUnknown(str, code);
                break;
        }

        // Increment the past the tag.
        m_filePos = tagEnd;
    }
}


BOOL CInputScript::ParseFile(char * pInput)
{
    U8 fileHdr[8];
    BOOL sts = true;
    FILE *inputFile = NULL;

    // Free the buffer if it is there.
    if (m_fileBuf != NULL)
    {
        delete m_fileBuf;
        m_fileBuf = NULL;
        m_fileSize = 0;
    }

    //printf("***** Dumping SWF File Information *****\n");
    
    // Open the file for reading.
    inputFile = fopen(pInput, "rb");

    // Did we open the file?
    if (inputFile == NULL) 
    {
        sts = false;
        printf("ERROR: Can't open file %s\n", pInput);
    }

    // Are we OK?
    if (sts)
    {
        // Read the file header.
        if (fread(fileHdr, 1, 8, inputFile) != 8)
        {
            sts = false;
            printf("ERROR: Can't read the header of file %s\n", pInput);
        }
    }

    // Are we OK?
    if (sts)
    {
        printf("----- Reading the file header -----\n");

        // Verify the header and get the file size.
        if (fileHdr[0] != 'F' || fileHdr[1] != 'W' || fileHdr[2] != 'S' )
        {
            printf("ERROR: Illegal Header - not a Shockwave Flash File\n");

            // Bad header.
            sts = false;
        }
        else
        {
            // Get the file version.
            m_fileVersion = (U16) fileHdr[3];

            printf("FWS\n");
            printf("File version \t%u\n", m_fileVersion);
        }
    }
        
    // Are we OK?
    if (sts)
    {
        // Get the file size.
        m_fileSize = (U32) fileHdr[4] | ((U32) fileHdr[5] << 8) | ((U32) fileHdr[6] << 16) | ((U32) fileHdr[7] << 24);

        printf("File size \t%u\n", m_fileSize);

        // Verify the minimum length of a Flash file.
        if (m_fileSize < 21)
        {
            printf("ERROR: file size is too short\n");
            // The file is too short.
            sts = false;
        }
    }

    // Are we OK?
    if (sts)
    {
        // Allocate the buffer.
        m_fileBuf = new U8[m_fileSize];

        // Is there data in the file?
        if (m_fileBuf == NULL)
        {
            sts = false;
        }
    }
        
    // Are we OK?
    if (sts)
    {
        // Copy the data already read from the file.
        memcpy(m_fileBuf, fileHdr, 8);

        // Read the file into the buffer.
        if (fread(&m_fileBuf[8], 1, m_fileSize - 8, inputFile) != (m_fileSize - 8))
        {
            sts = false;
        }
    }

    // Do we have a file handle?
    if (inputFile != NULL)
    {
        // Close the file.
        fclose(inputFile);
        inputFile = NULL;
    }

    // Are we OK?
    if (sts)
    {
        SRECT rect;
        
        // Set the file position past the header and size information.
        m_filePos = 8;

        // Get the frame information.
        GetRect(&rect);
        printf("Movie width \t%u\n", (rect.xmax - rect.xmin) / 20);
        printf("Movie height \t%u\n", (rect.ymax - rect.ymin) / 20);

        U32 frameRate = GetWord() >> 8;
        printf("Frame rate \t%u\n", frameRate);

        U32 frameCount = GetWord();
        printf("Frame count \t%u\n", frameCount);

        // Set the start position.
        m_fileStart = m_filePos;    

        printf("\n----- Reading movie details -----\n");
        fflush(stdout);

        // Parse the tags within the file.
        ParseTags(false, 0);
        printf("\n***** Finished Dumping SWF File Information *****\n");
    }

    // Free the buffer if it is there.
    if (m_fileBuf != NULL)
    {
        delete m_fileBuf;
        m_fileBuf = NULL;
    }

    // Reset the file information.
    m_filePos = 0;
    m_fileSize = 0;
    m_fileStart = 0;
    m_fileVersion = 0;

    // Reset the bit position and buffer.
    m_bitPos = 0;
    m_bitBuf = 0;

    return sts;
}

#ifdef __COMMAND_LINE_PARSE__

int main (int argc, char *argv[])
// Main program.
{
    CInputScript * pInputScript = NULL;
    char *fileName = 0;
    int i = 0;

    // Check the argument count.
    if (argc < 2)
    {
        // Bad arguments.
        usage();
        return -1;
    }

    // Create a flash script object.
    if ((pInputScript = new CInputScript()) == NULL)
    {
        fprintf( stderr, "Couldn't allocate CInputScript\n" );
        return -1;
    }

    for (i = 1; i < argc; i++)
    {
        char *str = argv[i];

        if (str[0] == '-')
        {
            switch (strlen(str))
            {
                case 2:
                {
                    switch( str[ 1 ] )
                    {
                        case 't':
                        {
                            pInputScript->m_dumpAll = true;
                            break;
                        }
                        case 'i':
                        {
                            pInputScript->m_dumpGuts = true;
                            break;
                        }
                        case 's':
                        {
                            pInputScript->m_dumpSoundGuts = true;
                            break;
                        }
                        case 'a':
                        {
                            pInputScript->m_dumpAll = true;
                            pInputScript->m_dumpGuts = true;
                            pInputScript->m_dumpSoundGuts = true;
                            break;
                        }
                        default:
                        {
                            fprintf( stderr, 
                                        "Ignoring invalid option: %c\n",
                                        &str[ 1 ] );
                        }
                    }
                    break;
                }
            }
        }
        else
        {
            fileName = argv[i];
        }
    }

            
    // Parse the file passed in.
    pInputScript->ParseFile(fileName);

    delete pInputScript;
    pInputScript = NULL;

    return 0;
}

#endif //__COMMAND_LINE_PARSE__
