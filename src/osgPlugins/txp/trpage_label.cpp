/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */

/* trpage_label.cpp
   Methods for the trpgLable object and its associated support structure
   including trpgTextStyle and trpgTextStyleTable.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <math.h>

#include <trpage_geom.h>
#include <trpage_read.h>

// *************** Text Style implementation

trpgTextStyle::trpgTextStyle(void)
{
    Reset();
}

trpgTextStyle::~trpgTextStyle(void)
{
}

void trpgTextStyle::Reset(void)
{
    font = "";
    bold = italic = underline = false;
    characterSize = float32(0.0042333333333); // 12 point in meter
    matId = -1;
}

void trpgTextStyle::SetMaterial(int inMatId)
{
    matId = inMatId;
}

int trpgTextStyle::GetMaterial(void) const
{
    return matId;
}

void trpgTextStyle::SetFont(std::string &inFont)
{
    font = inFont;
}

const std::string *trpgTextStyle::GetFont(void) const
{
    return &font;
}

void trpgTextStyle::SetBold(bool inBold)
{
    bold = inBold;
}

bool trpgTextStyle::GetBold(void) const
{
    return bold;
}

void trpgTextStyle::SetItalic(bool inItalic)
{
    italic = inItalic;
}

bool trpgTextStyle::GetItalic(void) const
{
    return italic;
}

void trpgTextStyle::SetUnderline(bool inUnder)
{
    underline = inUnder;
}

bool trpgTextStyle::GetUnderline(void) const
{
    return underline;
}

void trpgTextStyle::SetCharacterSize(float32 inSize)
{
    characterSize = inSize;
}

float32 trpgTextStyle::GetCharacterSize(void) const
{
    return characterSize;
}


// Write method

bool trpgTextStyle::Write(trpgWriteBuffer &buf)
{
    buf.Begin(TRPG_TEXT_STYLE);

    buf.Begin(TRPG_TEXT_STYLE_BASIC);
    buf.Add(font);
    buf.Add((int32)bold);
    buf.Add((int32)italic);
    buf.Add((int32)underline);
    buf.Add(characterSize);
    buf.Add(matId);
    buf.End();

    buf.End();

    return true;
}

// TextStyle CB
// Used to parse tokens from the text style structure.
// If we do it this way it's easier to expand later.
class textStyleCB : public trpgr_Callback
{
public:
    void * Parse(trpgToken,trpgReadBuffer &);
    trpgTextStyle *style;
};

void * textStyleCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    std::string sVal;
    int iVal;
    float32 fVal;

    try
    {
        switch (tok)
        {
        case TRPG_TEXT_STYLE_BASIC:
            buf.Get(sVal);
            style->SetFont(sVal);
            buf.Get(iVal);
            style->SetBold((iVal ? true : false));
            buf.Get(iVal);
            style->SetItalic((iVal ? true : false));
            buf.Get(iVal);
            style->SetUnderline((iVal ? true : false));
            buf.Get(fVal);
            style->SetCharacterSize(fVal);
            buf.Get(iVal);
            style->SetMaterial(iVal);
            break;
        default:
            break;
        }
    }
    catch (...)
    {
        return NULL;
    }

    return style;
}

// Read from a buffer

bool trpgTextStyle::Read(trpgReadBuffer &buf)
{
    Reset();

    trpgr_Parser parse;
    textStyleCB textStyleCb;

    textStyleCb.style = this;
    parse.AddCallback(TRPG_TEXT_STYLE_BASIC,&textStyleCb,false);
    parse.Parse(buf);

    return isValid();
}

bool trpgTextStyle::isValid(void) const
{
    // Need to have a font designation at least
    if (font.size() > 0)
        return true;

    return false;
}


bool trpgTextStyle::operator == (const trpgTextStyle& in) const
{
    if (font != in.font)
        return false;

    if (bold      != in.bold ||
        italic    != in.italic ||
        underline != in.underline)
        return false;

    if(fabs(double(characterSize - in.characterSize)) > 0.0001)
        return false;


    if (matId != in.matId)
        return false;

    return true;
}

// ******************* Text Style Table implementation

trpgTextStyleTable::trpgTextStyleTable()
{
    Reset();
}

trpgTextStyleTable::~trpgTextStyleTable()
{
}

void trpgTextStyleTable::Reset()
{
    styleMap.clear();
}

bool trpgTextStyleTable::isValid() const
{
    //for (int i=0;i<styles.size();i++)
    StyleMapType::const_iterator itr = styleMap.begin();
    for (  ; itr != styleMap.end( ); itr++)
        if (!itr->second.isValid())
            return false;

    return true;
}

int trpgTextStyleTable::AddStyle(const trpgTextStyle &style)
{
    int handle = style.GetHandle();
    if(handle==-1) {
        handle = styleMap.size();
    }
    styleMap[handle] = style;
    return handle;
}

int trpgTextStyleTable::FindAddStyle(const trpgTextStyle &style)
{
    StyleMapType::const_iterator itr = styleMap.begin();
    for (  ; itr != styleMap.end( ); itr++)
        if (itr->second == style)
            return itr->first;

    return AddStyle(style);
}

int trpgTextStyleTable::GetNumStyle() const
{
    return styleMap.size();
}

const trpgTextStyle *trpgTextStyleTable::GetStyleRef(int id) const
{
    if (id < 0)
        return NULL;

    StyleMapType::const_iterator itr = styleMap.find(id);
    if(itr == styleMap.end())
        return NULL;
    return &itr->second;
}

bool trpgTextStyleTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_TEXT_STYLE_TABLE);

    // Number of styles
    int numStyle = styleMap.size();
    buf.Add((int32)numStyle);

    // Write the styles
    StyleMapType::iterator itr = styleMap.begin();
    for (  ; itr != styleMap.end( ); itr++)
        itr->second.Write(buf);

    buf.End();

    return true;
}

bool trpgTextStyleTable::Read(trpgReadBuffer &buf)
{
    trpgTextStyle style;
    trpgToken styleTok;
    int32 len;
    bool status;
    int numStyle;
    int i;

    Reset();

    try
    {
        buf.Get(numStyle);
        if (numStyle < 0)
            throw 1;

        for (i=0;i<numStyle;i++) {
            buf.GetToken(styleTok,len);
            if (styleTok != TRPG_TEXT_STYLE) throw 1;
            buf.PushLimit(len);
            style.Reset();
            status = style.Read(buf);
            buf.PopLimit();
            if (!status) throw 1;
            AddStyle(style);
        }
    }
    catch (...)
    {
        return false;
    }

    return isValid();
}




// *************** Support Style implementation

trpgSupportStyle::trpgSupportStyle(void)
{
    Reset();
}

trpgSupportStyle::~trpgSupportStyle(void)
{
}

void trpgSupportStyle::Reset(void)
{
    type = Line;
    matId = -1;
    handle = -1;
    writeHandle = false;
}

void trpgSupportStyle::SetType(SupportType s)
{
    type = s;
}

trpgSupportStyle::SupportType trpgSupportStyle::GetType() const
{
    return type;
}

void trpgSupportStyle::SetMaterial(int inMatId)
{
    matId = inMatId;
}

int trpgSupportStyle::GetMaterial(void) const
{
    return matId;
}

// Write method
bool trpgSupportStyle::Write(trpgWriteBuffer &buf)
{
    buf.Begin(TRPG_SUPPORT_STYLE);

    buf.Begin(TRPG_SUPPORT_STYLE_BASIC);
    buf.Add(type);
    buf.Add(matId);
    buf.End();

    buf.End();

    return true;
}

// SupportStyle CB
// Used to parse tokens from the support style structure.
// If we do it this way it's easier to expand later.
class supportStyleCB : public trpgr_Callback
{
public:
    void * Parse(trpgToken,trpgReadBuffer &);
    trpgSupportStyle *style;
};

void * supportStyleCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    int iVal;

    try
    {
        switch (tok)
        {
        case TRPG_SUPPORT_STYLE_BASIC:
            buf.Get(iVal);
            style->SetType(trpgSupportStyle::SupportType(iVal));
            buf.Get(iVal);
            style->SetMaterial(iVal);
            break;
        default:
            break;
        }
    }
    catch (...)
    {
        return NULL;
    }

    return style;
}

// Read from a buffer

bool trpgSupportStyle::Read(trpgReadBuffer &buf)
{
    Reset();

    trpgr_Parser parse;
    supportStyleCB supportStyleCb;

    supportStyleCb.style = this;
    parse.AddCallback(TRPG_SUPPORT_STYLE_BASIC,&supportStyleCb,false);
    parse.Parse(buf);

    return isValid();
}

bool trpgSupportStyle::isValid(void) const
{
    return true;
}


bool trpgSupportStyle::operator == (const trpgSupportStyle& in) const
{
    if (type != in.type || matId != in.matId)
        return false;

    return true;
}


// ******************* Support Style Table implementation

trpgSupportStyleTable::trpgSupportStyleTable()
{
    Reset();
}
trpgSupportStyleTable::~trpgSupportStyleTable()
{
}

void trpgSupportStyleTable::Reset()
{
    //styles.resize(0);
    supportStyleMap.clear();
}

bool trpgSupportStyleTable::isValid() const
{
    //for (int i=0;i<styles.size();i++)
    SupportStyleMapType::const_iterator itr = supportStyleMap.begin();
    for (  ; itr != supportStyleMap.end( ); itr++)
    {
        if (!itr->second.isValid())
            return false;
    }
    return true;
}

int trpgSupportStyleTable::AddStyle(const trpgSupportStyle &style)
{
    int handle = style.GetHandle();
    if(handle==-1)
    {
        handle = supportStyleMap.size();
    }
    supportStyleMap[handle] = style;
    return handle;
}

int trpgSupportStyleTable::FindAddStyle(const trpgSupportStyle &style)
{
    SupportStyleMapType::const_iterator itr = supportStyleMap.begin();
    for (  ; itr != supportStyleMap.end( ); itr++)
        if (itr->second == style)
            return itr->first;

    return AddStyle(style);
}

int trpgSupportStyleTable::GetNumStyle() const
{
    return supportStyleMap.size();
}

const trpgSupportStyle *trpgSupportStyleTable::GetStyleRef(int id) const
{
    if (id < 0)
        return NULL;
    SupportStyleMapType::const_iterator itr = supportStyleMap.find(id);
    if(itr == supportStyleMap.end())
        return NULL;
    return &itr->second;
}

bool trpgSupportStyleTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_SUPPORT_STYLE_TABLE);

    // Number of styles
    int numStyle = supportStyleMap.size();
    buf.Add((int32)numStyle);

    // Write the styles
    SupportStyleMapType::iterator itr = supportStyleMap.begin();
    for (  ; itr != supportStyleMap.end( ); itr++)
        itr->second.Write(buf);

    buf.End();

    return true;
}

bool trpgSupportStyleTable::Read(trpgReadBuffer &buf)
{
    trpgSupportStyle style;
    trpgToken styleTok;
    int32 len;
    bool status;
    int numStyle;
    int i;

    Reset();

    try
    {
        buf.Get(numStyle);
        if (numStyle < 0)
            throw 1;
        //styles.resize(numStyle);
        for (i=0;i<numStyle;i++) {
            buf.GetToken(styleTok,len);
            if (styleTok != TRPG_SUPPORT_STYLE) throw 1;
            buf.PushLimit(len);
            style.Reset();
            status = style.Read(buf);
            buf.PopLimit();
            if (!status) throw 1;
            AddStyle(style);
        }
    }
    catch (...)
    {
        return false;
    }

    return isValid();
}



// *************** Label ppoperty implementation

trpgLabelProperty::trpgLabelProperty(void)
{
    Reset();
}

trpgLabelProperty::~trpgLabelProperty(void)
{
}

void trpgLabelProperty::Reset(void)
{
    fontId = -1;
    supportId = -1;
    type = VertBillboard;
    handle = -1;
    writeHandle = false;
}

void trpgLabelProperty::SetType(LabelType inType)
{
    type = inType;
}

trpgLabelProperty::LabelType trpgLabelProperty::GetType() const
{
    return type;
}

void trpgLabelProperty::SetFontStyle(int id)
{
    fontId = id;
}

int trpgLabelProperty::GetFontStyle() const
{
    return fontId;
}

void trpgLabelProperty::SetSupport(int id)
{
    supportId = id;
}

int trpgLabelProperty::GetSupport(void) const
{
    return supportId;
}

// Write method
bool trpgLabelProperty::Write(trpgWriteBuffer &buf)
{
    buf.Begin(TRPG_LABEL_PROPERTY);

    buf.Begin(TRPG_LABEL_PROPERTY_BASIC);
    buf.Add(fontId);
    buf.Add(supportId);
    buf.Add(type);
    buf.End();

    buf.End();

    return true;
}

// LabelProperty CB
// Used to parse tokens from the label property structure.
// If we do it this way it's easier to expand later.
class labelPropertyCB : public trpgr_Callback
{
public:
    void * Parse(trpgToken,trpgReadBuffer &);
    trpgLabelProperty *property;
};

void * labelPropertyCB::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    int iVal;

    try
    {
        int ival;
        switch (tok)
        {
        case TRPG_LABEL_PROPERTY_BASIC:
            buf.Get(iVal);
            property->SetFontStyle(iVal);
            buf.Get(iVal);
            property->SetSupport(iVal);
            buf.Get(ival);
            property->SetType(trpgLabelProperty::LabelType(ival));
            break;
        default:
            break;
        }
    }
    catch (...)
    {
        return NULL;
    }

    return property;
}

// Read from a buffer

bool trpgLabelProperty::Read(trpgReadBuffer &buf)
{
    Reset();

    trpgr_Parser parse;
    labelPropertyCB labelPropertyCb;

    labelPropertyCb.property = this;
    parse.AddCallback(TRPG_LABEL_PROPERTY_BASIC,&labelPropertyCb,false);
    parse.Parse(buf);

    return isValid();
}

bool trpgLabelProperty::isValid(void) const
{
    return supportId != -1 && fontId != -1 && type >=0  && type < MaxLabelType;
}


bool trpgLabelProperty::operator == (const trpgLabelProperty& in)const
{
    if (fontId != in.fontId || supportId != in.supportId || type != in.type)
        return false;

    return true;
}

// ******************* Label Property Table implementation

trpgLabelPropertyTable::trpgLabelPropertyTable()
{
    Reset();
}
trpgLabelPropertyTable::~trpgLabelPropertyTable()
{
}

void trpgLabelPropertyTable::Reset()
{
    labelPropertyMap.clear();
}

bool trpgLabelPropertyTable::isValid() const
{
    LabelPropertyMapType::const_iterator itr = labelPropertyMap.begin();
    for (  ; itr != labelPropertyMap.end( ); itr++)
    {
        if (!itr->second.isValid())
            return false;
    }
    return true;
}

int trpgLabelPropertyTable::AddProperty(const trpgLabelProperty &property)
{
    int handle = property.GetHandle();
    if(handle==-1)
    {
        handle = labelPropertyMap.size();
    }
    labelPropertyMap[handle] = property;
    return handle;
}

int trpgLabelPropertyTable::FindAddProperty(const trpgLabelProperty& property)
{
    LabelPropertyMapType::const_iterator itr = labelPropertyMap.begin();
    for (  ; itr != labelPropertyMap.end( ); itr++)
    {
        if (itr->second == property)
            return itr->first;
    }
    return AddProperty(property);
}

int trpgLabelPropertyTable::GetNumProperty() const
{
    return labelPropertyMap.size();
}

const trpgLabelProperty *trpgLabelPropertyTable::GetPropertyRef(int id) const
{
    if (id < 0)
        return NULL;

    LabelPropertyMapType::const_iterator itr = labelPropertyMap.find(id);
    if(itr == labelPropertyMap.end())
        return NULL;
    return &itr->second;
}

bool trpgLabelPropertyTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_LABEL_PROPERTY_TABLE);

    // Number of styles
    int numProperty = labelPropertyMap.size();
    buf.Add((int32)numProperty);

    // Write the properties
    LabelPropertyMapType::iterator itr = labelPropertyMap.begin();
    for (  ; itr != labelPropertyMap.end( ); itr++)
    {
        itr->second.Write(buf);
    }

    buf.End();

    return true;
}

bool trpgLabelPropertyTable::Read(trpgReadBuffer &buf)
{
    trpgLabelProperty property;
    trpgToken propertyTok;
    int32 len;
    bool status;
    int numProperty;
    int i;

    Reset();

    try
    {
        buf.Get(numProperty);
        if (numProperty < 0)
            throw 1;
        //properties.resize(numProperty);
        for (i=0;i<numProperty;i++) {
            buf.GetToken(propertyTok,len);
            if (propertyTok != TRPG_LABEL_PROPERTY) throw 1;
            buf.PushLimit(len);
            property.Reset();
            status = property.Read(buf);
            buf.PopLimit();
            if (!status) throw 1;
            //properties[i] = property;
            AddProperty(property);
        }
    }
    catch (...)
    {
        return false;
    }

    return isValid();
}



// ****************** Label implementation

trpgLabel::trpgLabel()
{
    Reset();
}
trpgLabel::~trpgLabel()
{
}

void trpgLabel::Reset()
{
    propertyId = -1;
    text = "";
    alignment = Left;
    tabSize = 8;
    scale = 1.0;
    thickness = 0.0;
    desc = "";
    url = "";
    location.x = 0;
    location.y = 0;
    location.z = 0;
    supports.resize(0);
}

bool trpgLabel::isValid() const
{
    if (text.empty() || propertyId == -1)
        return false;

    return true;
}

void trpgLabel::SetProperty(int id)
{
    propertyId = id;
}

int trpgLabel::GetProperty() const
{
    return propertyId;
}



void trpgLabel::SetText(const std::string &inText)
{
    text = inText;
}

const std::string *trpgLabel::GetText() const
{
    return &text;
}

void trpgLabel::SetAlignment(AlignmentType inType)
{
    alignment = inType;
}

trpgLabel::AlignmentType trpgLabel::GetAlignment() const
{
    return alignment;
}

void trpgLabel::SetTab(int size)
{
    tabSize = size;
}

int trpgLabel::GetTab(void) const
{
    return tabSize;
}

void trpgLabel::SetScale(float32 inScale)
{
    scale = inScale;
}

float32 trpgLabel::GetScale(void) const
{
    return scale;
}

void trpgLabel::SetThickness(float32 inThickness)
{
    thickness = inThickness;
}

float32 trpgLabel::GetThickness(void) const
{
    return thickness;
}

void trpgLabel::SetDesc(const std::string &inText)
{
    desc = inText;
}

const std::string *trpgLabel::GetDesc() const
{
    return &desc;
}


void trpgLabel::SetURL(const std::string &inText)
{
    url = inText;
}

const std::string *trpgLabel::GetURL() const
{
    return &url;
}

void trpgLabel::SetLocation(const trpg3dPoint &pt)
{
    location = pt;
}

const trpg3dPoint& trpgLabel::GetLocation() const
{
    return location;
}

void trpgLabel::AddSupport(const trpg3dPoint &pt)
{
    supports.push_back(pt);
}

const std::vector<trpg3dPoint> *trpgLabel::GetSupports() const
{
    return &supports;
}

bool trpgLabel::Write(trpgWriteBuffer &buf)
{
    unsigned int i;

    buf.Begin(TRPG_LABEL);
    buf.Add(propertyId);
    buf.Add(text);
    buf.Add(alignment);
    buf.Add(tabSize);
    buf.Add(scale);
    buf.Add(thickness);
    buf.Add(desc);
    buf.Add(url);
    buf.Add(location);
    buf.Add((int)supports.size());
    for (i=0;i<supports.size();i++)
        buf.Add(supports[i]);
    buf.End();

    return true;
}

bool trpgLabel::Read(trpgReadBuffer &buf)
{
    int numSupport,i;
    trpg3dPoint support;
    int iVal;

    try
    {
        buf.Get(iVal);
        propertyId = iVal;
        buf.Get(text);
        buf.Get(iVal);
        alignment = (AlignmentType)iVal;
        buf.Get(tabSize);
        buf.Get(scale);
        buf.Get(thickness);
        buf.Get(desc);
        buf.Get(url);
        buf.Get(location);
        buf.Get(numSupport);
        if (numSupport < 0) throw 1;
        for (i=0;i<numSupport;i++) {
            buf.Get(support);
            supports.push_back(support);
        }
    }
    catch (...)
    {
        return false;
    }

    return isValid();
}
