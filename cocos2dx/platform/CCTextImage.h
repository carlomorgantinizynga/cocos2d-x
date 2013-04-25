/****************************************************************************
 Copyright (c) 2010-2012 cocos2d-x.org
 Copyright (c) 2013      Zynga Inc.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#ifndef TestNewStringStuff_CCTextImage_h
#define TestNewStringStuff_CCTextImage_h

#include <map>
#include <vector>

#include "cocoa/CCGeometry.h"
#include "textures/CCTexture2D.h"



/** @brief CCGlyphDef defines one single glyph (character) in a text image 
 *
 * it defines the bounding box for the glyph in the texture page, the character the padding (spacing) between characters
 *
 */

class CCGlyphDef
{
public:
    
    CCGlyphDef()                                    {}
    CCGlyphDef(char letter, cocos2d::CCRect &rect)  { mGliphRect = rect; mLetter = letter; }
    
    char getLetter()                                { return mLetter; }
    cocos2d::CCRect & getRect()                     { return mGliphRect; }
    void setPadding(float padding)                  { mPadding = padding; }
    float getPadding()                              { return mPadding; }
    
private:
    
    cocos2d::CCRect mGliphRect;
    char            mLetter;
    float           mPadding;
    
};


/** @brief CCTextLineDef define a line of text in a text image texture page
 *
 * conllects all the CCGlyphDef for a text line plus line size and line position in text image space
 *
 */
class CCTextLineDef
{
public:
    
    CCTextLineDef(float x, float y, float width, float height);
    
    float getX()        { return mX; }
    float getY()        { return mY; }
    float getWidth()    { return mWidth; }
    float getHeight()   { return mHeight; }
    void  addGlyph(CCGlyphDef theGlyph) { mGlyphs.push_back(theGlyph); }
    int   getNumGlyph()   { return mGlyphs.size(); }
    CCGlyphDef & getGlyphAt(int index) { return mGlyphs[index]; }
    
private:
    
    float mX;
    float mY;
    float mWidth;
    float mHeight;
    std::vector<CCGlyphDef> mGlyphs;
    
};

/** @brief CCTextPageDef defines one text image page (a CCTextImage can have/use more than one page)
 *
 * collects all the CCTextLineDef for one page, the witdh and height of the page and the  graphics (texture) for the page
 *
 */
class CCTextPageDef
{
public:
    
    CCTextPageDef(int pageNum, int width, int height);
    ~CCTextPageDef();
    
    void addLine(CCTextLineDef *theLine)    { mLines.push_back(theLine); }
    int  getNumLines()                      { return mLines.size(); }
    CCTextLineDef * getLineAt(int index)    { return mLines[index]; }
    int getWidth()                          { return mWidth;  }
    int getHeight()                         { return mHeight; }
    int getPageNumber()                     { return mPageNum; }
    void setPageData(unsigned char *pData)  { mPageData = pData; }
    unsigned char * getPageData()           { return mPageData; }
    cocos2d::CCTexture2D *getPageTexture();
    
    
private:
    
    bool generatePageTexture();
    
    int mPageNum;
    int mWidth;
    int mHeight;
    std::vector<CCTextLineDef *> mLines;
    unsigned char *mPageData;
    cocos2d::CCTexture2D *mPageTexture;
    
};

/** @brief CCTextFontPages collection of pages (CCTextPageDef)
 *
 * a CCTextImage is composed by one or more text pages. This calss collects all of those pages
 */
class CCTextFontPages
{
public:
    
    CCTextFontPages(char *fontName, int fontSize);
    ~CCTextFontPages();
    void addPage(CCTextPageDef *newPage)    { mPages.push_back(newPage); }
    int getNumPages()                       { return mPages.size(); }
    CCTextPageDef * getPageAt(int index)    { return mPages[index]; }
    char * getFontName()                    { return mFontName; }
    int getFontSize()                       { return mFontSize; }
    
private:
    
    std::vector<CCTextPageDef *> mPages;
    int  mFontSize;
    char *mFontName;
};

/** @brief CCTextImage 
 *
 */
class CCTextImage
{
public:
    
    ~CCTextImage();
    bool initWithString(const char * pText, int nWidth, int nHeight, const char * pFontName, int nSize);
    bool debugSaveToFile(const char *pszFilePath, bool bIsToRGB);
    CCTextFontPages * getPages()    { return mFontPages; }
    CTFontRef getFont()             { return mFontRef; }
    
private:
    
    bool createImageDataFromPages(CCTextFontPages *thePages);
    unsigned char * preparePageGlyphData(CCTextPageDef *thePage, char *fontName, int fontSize);
    bool createFontRef(const char *fontName, int size);
    bool generateTextGlyphs(const char * pText);
    bool addGlyphsToLine(CCTextLineDef *line, const char *lineText);
    std::map<char, CCGlyphDef> mTextGlyphs;
    CCTextFontPages * mFontPages;
    CTFontRef         mFontRef;
    
};


#endif
