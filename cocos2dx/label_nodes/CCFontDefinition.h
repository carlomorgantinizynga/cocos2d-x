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

#ifndef __TestNewStringStuff__CCFontDefinition__
#define __TestNewStringStuff__CCFontDefinition__



#include <CoreText/CTFont.h>
#include <CoreText/CTTypesetter.h>
#include <CoreFoundation/CoreFoundation.h>

#include "cocos2d.h"
#include "CCTextImage.h"

struct CCLetterDefinition
{
    
    char letterChar;
    float U;
    float V;
    float width;
    float height;
    float offset;
    int   textureID;
    
};

class CCFontDefinition
{
public:
    
    CCFontDefinition();
    ~CCFontDefinition();
    bool CreateFontDefinition(char *fontName, int fontSize, char *letters, int textureSize = 512);
    CCLetterDefinition & getLetterDefinition(char theLetter);
    CTFontRef & getFontRef();
    cocos2d::CCTexture2D * getTexture(int index);
    
private:
    
    
    char * spaceLetters(char *pSourceString);
    void addLetterDef(CCLetterDefinition &defToAdd);
    
    std::map<char, CCLetterDefinition> mFontLettersDefinition;
    CTFontRef mFontRef;
    CCTextImage *mpTextImages;
};


#endif /* defined(__TestNewStringStuff__CCFontDefinition__) */
