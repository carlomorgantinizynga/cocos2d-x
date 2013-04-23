//
//  CCTextImage.mm
//  TestNewStringStuff
//
//  Created by Carlo Morgantini on 4/9/13.
//
//
#include <vector>
#include <string>
#include <CoreText/CTFont.h>
#include <CoreText/CTTypesetter.h>
#include <CoreFoundation/CoreFoundation.h>

#include "CCTextImage.h"


using namespace cocos2d;

CCTextLineDef::CCTextLineDef(float x, float y, float width, float height) :mX(x), mY(y), mWidth(width), mHeight(height)
{
    //if ( text && strlen(text) )
    //{
    //    mLineString = new char[strlen(text)+1];
    //    strcpy(mLineString, text);
    //}
}

//CCTextLineDef::~CCTextLineDef()
//{
    //if(mLineString)
    //{
    //    delete [] mLineString;
    //}
//}




CCTextPageDef::CCTextPageDef(int pageNum, int width, int height):   mPageNum(pageNum),
                                                                    mWidth(width),
                                                                    mHeight(height),
                                                                    mPageData(0),
                                                                    mPageTexture(0)
{
}

CCTextPageDef::~CCTextPageDef()
{
    int numLines = mLines.size();
    for( int c = 0; c<numLines; ++c)
    {
        delete mLines[c];
    }
    
    if (mPageData)
    {
        delete [] mPageData;
    }
    
    if (mPageTexture)
    {
        delete mPageTexture;
    }
}

bool CCTextPageDef::generatePageTexture()
{
    if (mPageTexture)
    {
        delete mPageTexture;
        mPageTexture = 0;
    }
    CCSize imageSize = CCSizeMake((float)(mWidth), (float)(mHeight));
    
    mPageTexture = new CCTexture2D();
    if (!mPageTexture)
        return false;
    
    bool textureCreated = mPageTexture->initWithData(mPageData, kCCTexture2DPixelFormat_Default, mWidth, mHeight, imageSize);
    mPageTexture->setPremultipliedAlpha(true);
    
    return textureCreated;
}

cocos2d::CCTexture2D *CCTextPageDef::getPageTexture()
{
    if (!mPageTexture)
    {
        generatePageTexture();
    }
    
    return mPageTexture;
}

CCTextFontPages::CCTextFontPages(char *fontName, int fontSize): mFontSize(fontSize)
{
    mFontName = new char[strlen(fontName)+1];
    strcpy(mFontName, fontName);
}

CCTextFontPages::~CCTextFontPages()
{
    int numPages = mPages.size();
    for( int c = 0; c<numPages; ++c)
    {
        delete mPages[c];
    }
    
    if (mFontName)
    {
        delete [] mFontName;
    }
}

NSString * _getStringInBoundsUsingGlyphSize(std::map<char, CCGlyphDef> &glyphDefs, NSString *str, id font, CGSize *constrainSize)
{
    // regular size
    CGSize tmp = [str sizeWithFont:font];
    
    // size using bounding boxes
    float widthWithBBX      =  0.0f;

    const char *pCharString =  [str UTF8String];
    
    for (int c = 0; c<strlen(pCharString); ++c)
    {
        widthWithBBX+= (glyphDefs[pCharString[c]].getRect().size.width + glyphDefs[pCharString[c]].getPadding());
    }
    
    float maxWidth = std::max(tmp.width, widthWithBBX);
    if ( maxWidth < constrainSize->width )
    {
        return str;
    }
    else
    {
        // remove the last character
        int stringLenght = [str length];
        NSString *newStr = [str substringToIndex: stringLenght-1];
        return _getStringInBoundsUsingGlyphSize(glyphDefs, newStr, font, constrainSize);
    }
}

float _getLineHeight(NSString *str, id font)
{
    CGSize tmp = [str sizeWithFont:font];
    return tmp.height;
}

float _getLineWidth(NSString *str, id font)
{
    CGSize tmp = [str sizeWithFont:font];
    return tmp.width;
}

CCTextImage::~CCTextImage()
{
    if (mFontPages)
    {
        delete mFontPages;
    }
}

bool CCTextImage::initWithString(const char * pText, int nWidth, int nHeight, const char * pFontName, int nSize)
{
    // create the reference to the system font
    if ( !createFontRef(pFontName, nSize) )
        return false;
    
    // generate the glyphs
    if ( !generateTextGlyphs(pText) )
        return false;
    
    
    NSString * str      = [NSString stringWithUTF8String:pText];
    NSString * fntName  = [NSString stringWithUTF8String:pFontName];
    
    CGSize constrainSize;
    
    constrainSize.width  = nWidth;
    constrainSize.height = nHeight;
    
    // On iOS custom fonts must be listed beforehand in the App info.plist (in order to be usable) and referenced only the by the font family name itself when
    // calling [UIFont fontWithName]. Therefore even if the developer adds 'SomeFont.ttf' or 'fonts/SomeFont.ttf' to the App .plist, the font must
    // be referenced as 'SomeFont' when calling [UIFont fontWithName]. Hence we strip out the folder path components and the extension here in order to get just
    // the font family name itself. This stripping step is required especially for references to user fonts stored in CCB files; CCB files appear to store
    // the '.ttf' extensions when referring to custom fonts.
    
    fntName = [[fntName lastPathComponent] stringByDeletingPathExtension];
    
    // create the font
    id font = [UIFont fontWithName:fntName size:nSize];
    NSString *inputString = str;

    int delta           = 0;
    int currentPage     = 0;
    float currentY      = 0.0;
    float lineHeight    = _getLineHeight(inputString, font);

    // check if at least one line will fit in the texture
    if ( lineHeight > constrainSize.height )
    {
        // we can't even fit one line in this texture
        return false;
    }

    // create pages for the font
    mFontPages = new CCTextFontPages((char *)pFontName, nSize);
    if (!mFontPages)
        return false;
    
    // create the first page (ther is going to be at least one page)
    CCTextPageDef *currentPageDef = new CCTextPageDef(currentPage, nWidth, nHeight);
    if ( !currentPageDef )
        return false;
    
    // add the current page
    mFontPages->addPage(currentPageDef);


    do {
        
        // choose texture page
        if ( ( currentY + lineHeight ) > constrainSize.height )
        {
            currentY     = 0;
            currentPage += 1;
            
            // create a new page and add
            currentPageDef = new CCTextPageDef(currentPage, nWidth, nHeight);
            if ( !currentPageDef )
                return false;
            
            mFontPages->addPage(currentPageDef);
        }
        
        // get the new fitting string
        NSString *boundedString = _getStringInBoundsUsingGlyphSize(mTextGlyphs, inputString, font, &constrainSize);
        
        // get the string width
        float newStringWidth = _getLineWidth(boundedString, font);
        
        // create the new line and add to the current page
        const char *textPointer = [boundedString UTF8String];
        CCTextLineDef *newLine  = new CCTextLineDef(0.0, currentY, newStringWidth, lineHeight);
        if ( !newLine )
            return false;
        
        // add all the glyphs to this line
        addGlyphsToLine(newLine, textPointer);
        
        // add the line the to current page
        currentPageDef->addLine(newLine);
        
        // get the remaining string 
        delta = inputString.length - boundedString.length;
        
        if(delta)
        {
            NSString *newString = [inputString substringFromIndex: boundedString.length];
            inputString = newString;
        }
        
        // go to next line
        currentY += lineHeight;
        
        
    } while( delta );
    
    
    // actually create the needed images
    return createImageDataFromPages(mFontPages);
}

bool CCTextImage::createFontRef(const char *fontName, int fontSize)
{
    CFStringRef theRefString = NULL;
    theRefString = CFStringCreateWithCString(kCFAllocatorDefault, fontName, CFStringGetSystemEncoding());
    
    // actually create iOS font
    mFontRef = CTFontCreateWithName(theRefString, fontSize, NULL);
    return (mFontRef != NULL);
}

bool CCTextImage::addGlyphsToLine(CCTextLineDef *line, const char *lineText)
{
    int count = strlen(lineText);
    
    for (int c=0; c<count; ++c)
    {
        line->addGlyph(mTextGlyphs[lineText[c]]);
    }
    
    return true;
}

bool CCTextImage::generateTextGlyphs(const char * pText)
{
    float CHAR_PADDING = 10.0f;
    
    const char *lineAsChar      = pText;
    CFStringRef lettersString   = CFStringCreateWithCString(kCFAllocatorDefault, lineAsChar, CFStringGetSystemEncoding());
    
    if (NULL == lettersString)
        return false;
    
    UniChar *characters;
    CGGlyph *glyphs;
    CFIndex  count;
    
    count = CFStringGetLength(lettersString);
    
    // Allocate our buffers for characters and glyphs.
    characters = new UniChar[count];
    assert(characters != NULL);
    
    glyphs = new CGGlyph[count];
    assert(glyphs != NULL);
    
    // Get the characters from the string.
    CFStringGetCharacters(lettersString, CFRangeMake(0, count), characters);
    
    // Get the glyphs for the characters.
    CTFontGetGlyphsForCharacters(mFontRef, characters, glyphs, count);
    CGGlyph *theFirstGlyph = &glyphs[0];
    
    // get letters bounding boxes
    CGRect *BBOx  = new CGRect[count];
    CTFontGetBoundingRectsForGlyphs(mFontRef, kCTFontHorizontalOrientation, theFirstGlyph, BBOx, count);
    
    for (int c=0; c<count; ++c)
    {
        cocos2d::CCRect tempRect;
        tempRect.origin.x       = BBOx[c].origin.x;
        tempRect.origin.y       = BBOx[c].origin.y;
        tempRect.size.width     = BBOx[c].size.width;
        tempRect.size.height    = BBOx[c].size.height;
        CCGlyphDef tempGlyph(pText[c], tempRect);
        tempGlyph.setPadding(CHAR_PADDING);
        mTextGlyphs[pText[c]] = tempGlyph;
    }
    
    // free stuff
    delete [] characters;
    delete [] glyphs;
    delete [] BBOx;

    return true;
}

bool CCTextImage::createImageDataFromPages(CCTextFontPages *thePages)
{
    int numPages = thePages->getNumPages();
    if (!numPages)
        return false;
 
    for (int c = 0; c < numPages; ++c)
    {
        unsigned char *pPageData = 0;
        pPageData = preparePageGlyphData(thePages->getPageAt(c), thePages->getFontName(), thePages->getFontSize());
        
        if (pPageData)
        {
            thePages->getPageAt(c)->setPageData(pPageData);
        }
        else
        {
            return false;
        }
    }
    
    return true;
}

unsigned char * CCTextImage::preparePageGlyphData(CCTextPageDef *thePage, char *fontName, int fontSize)
{
    // constants
    float   LINE_PADDING    = 1.9;
    
    if (!thePage)
        return NULL;
    
    if (thePage->getNumLines() == 0)
        return NULL;
    
    int pageWidth  = thePage->getWidth();
    int pageHeight = thePage->getHeight();
    
    // prepare memory ans set to 0
    int sizeInBytes = (pageWidth * pageHeight * 4);
    unsigned char* data = new unsigned char[sizeInBytes];
    memset(data, 0, sizeInBytes);
    
    // prepare the context
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(data, pageWidth, pageHeight, 8, pageWidth * 4, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    
    if (!context)
    {
        delete[] data;
        return 0;
    }
    
    // prepare the context
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    CGContextTranslateCTM(context, 0.0f, pageHeight);
    CGContextScaleCTM(context, 1.0f, -1.0f); //NOTE: NSString draws in UIKit referential i.e. renders upside-down compared to CGBitmapContext referential
    UIGraphicsPushContext(context);
    UITextAlignment align = NSTextAlignmentLeft;
    
    // create the font
    NSString *nsFontName = [NSString stringWithUTF8String:fontName];
    id font = [UIFont fontWithName:nsFontName size:fontSize];
    
    int     numLines        = thePage->getNumLines();
    for (int c = 0; c<numLines; ++c)
    {
        CCTextLineDef *pCurrentLine = thePage->getLineAt(c);
        float lineHeight            = pCurrentLine->getHeight();
        
        float origX         = LINE_PADDING;
        float origY         = pCurrentLine->getY();

        int numGlyphToRender = pCurrentLine->getNumGlyph();
        char oneCharString[2];
        
        for (int cglyph = 0; cglyph < numGlyphToRender; ++cglyph)
        {
            CCGlyphDef currGlyph      = pCurrentLine->getGlyphAt(cglyph);
            oneCharString[0]          = currGlyph.getLetter();
            oneCharString[1]          = 0;
            
            NSString *lineString = [NSString stringWithUTF8String:oneCharString];
            
            CGRect tempRect;
            tempRect.origin.x       = (origX - currGlyph.getRect().origin.x);
            tempRect.origin.y       = origY;
            tempRect.size.width     = currGlyph.getRect().size.width;
            tempRect.size.height    = lineHeight;
            
            // actually draw one character
            [lineString drawInRect: tempRect withFont:font lineBreakMode:NSLineBreakByWordWrapping alignment:align];
            
            // move to next character
            origX += (tempRect.size.width + currGlyph.getPadding());
        }
    }
    
    
    // clean everything
    UIGraphicsPopContext();
    CGContextRelease(context);
    
    // everything looks good
    return data;
}

bool CCTextImage::debugSaveToFile(const char *pszFilePath, bool bIsToRGB)
{
    if ( (!mFontPages) || (mFontPages->getNumPages() == 0))
        return false;
    
    bool saveToPNG = false;
    bool needToCopyPixels = false;
    std::string filePath(pszFilePath);
    if (std::string::npos != filePath.find(".png"))
    {
        saveToPNG = true;
    }
    
    int bitsPerComponent    = 8;
    int bitsPerPixel        = 32;
    
    if ((! saveToPNG) || bIsToRGB)
    {
        bitsPerPixel = 24;
    }
    
    int pageWidth  = mFontPages->getPageAt(0)->getWidth();
    int pageHeight = mFontPages->getPageAt(0)->getHeight();
    
    
    int bytesPerRow    = (bitsPerPixel/8) * pageWidth;
    int myDataLength = bytesPerRow * pageHeight;
    
    for(int cPages = 0; cPages<mFontPages->getNumPages(); ++cPages)
    {
        CCTextPageDef *pCurrentPage = mFontPages->getPageAt(cPages);
        if(!pCurrentPage)
            return false;
        
        
        unsigned char *pixels    = pCurrentPage->getPageData();
        
        // The data has alpha channel, and want to save it with an RGB png file,
        // or want to save as jpg,  remove the alpha channel.
        /*
        if ((saveToPNG && m_bHasAlpha && bIsToRGB)
            || (! saveToPNG))
        {
            pixels = new unsigned char[myDataLength];
            
            for (int i = 0; i < m_nHeight; ++i)
            {
                for (int j = 0; j < m_nWidth; ++j)
                {
                    pixels[(i * m_nWidth + j) * 3] = m_pData[(i * m_nWidth + j) * 4];
                    pixels[(i * m_nWidth + j) * 3 + 1] = m_pData[(i * m_nWidth + j) * 4 + 1];
                    pixels[(i * m_nWidth + j) * 3 + 2] = m_pData[(i * m_nWidth + j) * 4 + 2];
                }
            }
            
            needToCopyPixels = true;
        }
         */
        
        // make data provider with data.
        CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
        if (saveToPNG)
        {
            bitmapInfo |= kCGImageAlphaPremultipliedLast;
        }
        
        CGDataProviderRef provider        = CGDataProviderCreateWithData(NULL, pixels, myDataLength, NULL);
        CGColorSpaceRef colorSpaceRef    = CGColorSpaceCreateDeviceRGB();
        CGImageRef iref                    = CGImageCreate(pageWidth, pageHeight,
                                                           bitsPerComponent, bitsPerPixel, bytesPerRow,
                                                           colorSpaceRef, bitmapInfo, provider,
                                                           NULL, false,
                                                           kCGRenderingIntentDefault);
        
        UIImage* image                    = [[UIImage alloc] initWithCGImage:iref];
        
        CGImageRelease(iref);
        CGColorSpaceRelease(colorSpaceRef);
        CGDataProviderRelease(provider);
        
        NSData *data;
        
        if (saveToPNG)
        {
            data = UIImagePNGRepresentation(image);
        }
        else
        {
            data = UIImageJPEGRepresentation(image, 1.0f);
        }
        
        char tempString[1000];
        sprintf(tempString, "%d", cPages);
        std::string couterString(tempString);
        std::string finalFileName = couterString + pszFilePath;
        
        [data writeToFile:[NSString stringWithUTF8String:finalFileName.c_str()] atomically:YES];
        
        [image release];
        
        if (needToCopyPixels)
        {
            delete [] pixels;
        }
        
    }
    
    
    return true;
}













