//
//  CCStringTTF.cpp
//  TestNewStringStuff
//
//  Created by Carlo Morgantini on 4/11/13.
//
//

#include "CCStringTTF.h"

using namespace cocos2d;


CCStringTTF::CCStringTTF(CCFontDefinition *theDef)
{
    mFontDef = theDef;
}

bool CCStringTTF::setText(char *pStringToRender)
{
    // release all the sprites
    moveAllSpritesToCache();
    
    // create the reference to the string
    CFStringRef lettersString = CFStringCreateWithCString(kCFAllocatorDefault, pStringToRender, CFStringGetSystemEncoding());
    if (NULL == lettersString)
    {
        return false;
    }
    
    UniChar *characters;
    CGGlyph *glyphs;
    CFIndex  count;
    
    // num char
    count = CFStringGetLength(lettersString);
    
    // Allocate our buffers for characters and glyphs.
    characters = new UniChar[count];
    assert(characters != NULL);
    
    glyphs = new CGGlyph[count];
    assert(glyphs != NULL);
    
    // Get the characters from the string.
    CFStringGetCharacters(lettersString, CFRangeMake(0, count), characters);
    
    // Get the glyphs for the characters.
    CTFontGetGlyphsForCharacters(mFontDef->getFontRef(), characters, glyphs, count);
    
    CGGlyph *theFirstGlyph = &glyphs[0];
    
    // get the letter advances
    CGSize *pSize = new CGSize[count];
    assert(pSize);
    
    if(!pSize)
        return false;
    
    CTFontGetAdvancesForGlyphs(mFontDef->getFontRef(), kCTFontHorizontalOrientation,
                               theFirstGlyph,
                               pSize,
                               count );
    
    
    int stringLenght = strlen(pStringToRender);
    
    float posX = 0.0;
    float posY = 0.0;
    
    for (int c = 0; c<stringLenght; ++c)
    {
        // create the sprite
        CCSprite *pNewSprite = createNewSpriteFromLetterDefinition(mFontDef->getLetterDefinition(pStringToRender[c]),
                                                                   mFontDef->getTexture(mFontDef->getLetterDefinition(pStringToRender[c]).textureID) );
        
        if (!pNewSprite)
            return false;
        
        // set the position
        pNewSprite->setPosition(CCPoint(posX, posY));
        
        // store the sprite
        mSpriteVector.push_back(pNewSprite);
        
        // next letter
        posX += (pSize[c].width - mFontDef->getLetterDefinition(pStringToRender[c]).offset);
    }
    
    // add to self
    addAllLetters();
    
    // free stuff
    delete [] characters;
    delete [] glyphs;
    delete [] pSize;
    
    return true;
}

cocos2d::CCSprite * CCStringTTF::createNewSpriteFromLetterDefinition(CCLetterDefinition &theDefinition, cocos2d::CCTexture2D *theTexture)
{
    
    CCRect uvRect;
    uvRect.size.height = theDefinition.height;
    uvRect.size.width  = theDefinition.width;
    uvRect.origin.x    = theDefinition.U;
    uvRect.origin.y    = theDefinition.V;
    
    CCSpriteFrame *pFrame = CCSpriteFrame::createWithTexture(theTexture, uvRect);
    cocos2d::CCSprite *tempSprite = getSprite();
    
    if (!tempSprite)
        return 0;
    
    tempSprite->initWithSpriteFrame(pFrame);
    tempSprite->setAnchorPoint(CCPoint(0.0, 1.0));
    
    return tempSprite;
}

void CCStringTTF::addAllLetters()
{
    int numLetters = mSpriteVector.size();
    for (int c = 0; c<numLetters; ++c)
    {
        this->addChild(mSpriteVector[c]);
    }
}

void CCStringTTF::moveAllSpritesToCache()
{
    int numSprites = mSpriteVector.size();
    
    if (numSprites)
    {
        for (int c = 0; c<numSprites; ++c)
        {
            mSpriteVector[c]->removeFromParent();
            mSpriteChache.push_back(mSpriteVector[c]);
        }
        // clear
        mSpriteVector.clear();
    }
}

cocos2d::CCSprite * CCStringTTF::getSprite()
{
    if (!mSpriteChache.empty())
    {
        cocos2d::CCSprite *retSprite = mSpriteChache.back();
        mSpriteChache.pop_back();
        return retSprite;
    }
    else
    {
        cocos2d::CCSprite *retSprite = new CCSprite;
        return retSprite;
    }
}
