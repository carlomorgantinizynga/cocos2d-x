//
//  CCFontDefinition.cpp
//  TestNewStringStuff
//
//  Created by Carlo Morgantini on 4/11/13.
//
//

#include "CCFontDefinition.h"

using namespace cocos2d;

CCFontDefinition::CCFontDefinition():mpTextImages(0)
{
}

CCFontDefinition::~CCFontDefinition()
{
    if (mpTextImages)
    {
        delete mpTextImages;
    }
}

char * CCFontDefinition::spaceLetters(char *pSourceString)
{
    int numChar         = strlen(pSourceString);
    char *spacedLetters = new char[(numChar*2)];
    int c1              = 0;
    
    for(int c = 0; c < ((numChar*2)-1); ++c)
    {
       if (c&1)
       {
           spacedLetters[c] = ' ';
       }
       else
       {
           spacedLetters[c] = pSourceString[c1];
           ++c1;
       }
    }
    
    spacedLetters[((numChar*2)-1)] = 0;
    return spacedLetters;
}

bool CCFontDefinition::CreateFontDefinition(char *fontName, int fontSize, char *letters, int textureSize)
{
    // constants
    float LINE_PADDING  = 1.9;
    float PIXEL_PADDING = 1.0;
    
    
    // preare texture/image stuff
    mpTextImages = new CCTextImage();
    assert(mpTextImages);
    if (!mpTextImages)
        return false;
    
    mpTextImages->initWithString(letters, textureSize, textureSize, fontName, fontSize);
    // debug mpTextImages->debugSaveToFile("grande.png", false);
    
    // get all the pages
    CCTextFontPages *pPages = mpTextImages->getPages();
    if (!pPages)
        return (false);
    
    // get reference to font
    mFontRef = mpTextImages->getFont();
    if (NULL == mFontRef)
        return false;

    
    // loops all the pages
    for (int cPages = 0; cPages<pPages->getNumPages(); ++cPages)
    {
        // loops all the lines in this page
        for (int cLines = 0; cLines<pPages->getPageAt(cPages)->getNumLines(); ++cLines)
        {
            float posXUV       = 0.0;
            float posYUV       = pPages->getPageAt(cPages)->getLineAt(cLines)->getY();
            
            int   charsCounter = 0;
            
            for (int c = 0; c < pPages->getPageAt(cPages)->getLineAt(cLines)->getNumGlyph(); ++c)
            {
                
                // the current glyph
                CCGlyphDef currentGlyph = pPages->getPageAt(cPages)->getLineAt(cLines)->getGlyphAt(c);
                
                // letter width
                float letterWidth  = currentGlyph.getRect().size.width; 
                
                // letter height
                float letterHeight = pPages->getPageAt(cPages)->getLineAt(cLines)->getHeight();
                
                // add this letter definition
                CCLetterDefinition tempDef;
                
                tempDef.letterChar  =   currentGlyph.getLetter();
                tempDef.width       =   (letterWidth + PIXEL_PADDING);
                tempDef.height      =   letterHeight;
                tempDef.U           =   (posXUV + LINE_PADDING - PIXEL_PADDING);
                tempDef.V           =   posYUV;
                tempDef.offset      =   currentGlyph.getRect().origin.x;
                tempDef.textureID   =   cPages;
                
                
                // add this definition
                addLetterDef(tempDef);
                
                // move bounding box to the next letter (use the advance here)
                posXUV += letterWidth + currentGlyph.getPadding();
                
                // next char in the string
                ++charsCounter;
                
            }
        }
    }
    
    return true;
}

void CCFontDefinition::addLetterDef(CCLetterDefinition &defToAdd)
{
    if (mFontLettersDefinition.find(defToAdd.letterChar) == mFontLettersDefinition.end())
    {
        mFontLettersDefinition[defToAdd.letterChar] = defToAdd;
    }
}

CCLetterDefinition & CCFontDefinition::getLetterDefinition(char theLetter)
{
    return mFontLettersDefinition[theLetter];
}

CTFontRef & CCFontDefinition::getFontRef()
{
    return mFontRef;
}

cocos2d::CCTexture2D * CCFontDefinition::getTexture(int index)
{
    CCTextFontPages *pPages = mpTextImages->getPages();
    if (!pPages)
        return (false);
    
    return pPages->getPageAt(index)->getPageTexture();
}
