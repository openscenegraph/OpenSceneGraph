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

#include <stdlib.h>
#include <stdio.h>

/* trpage_parse.cpp
   This source file contains methods for the trpgr_Parser and trpgr_Token classes.
   trpgr_Parser is the main class.  It parses the basic structure of paging archive
   data out of Read Buffers.  You should not need to change this.
   If you want to parse data out of a different structure instead, look at
   subclassing trpgReadBuffer and replacing its virtual methods.  That's what
   trpgMemReadBuffer is doing.

   This file also contains the implementation of trpgSceneParser().
   That class implements a set of callbacks for handling the Pushes and Pops
   in an archive.  You fill in the Start/EndChildren callbacks and register
   for the rest of the tokens that you want.
*/

#include <trpage_read.h>

/* ***************************
    Paging token callback structure
   ***************************
   */
trpgr_Token::trpgr_Token()
{
    cb = NULL;
    destroy = true;
}
trpgr_Token::~trpgr_Token()
{
}
trpgr_Token::trpgr_Token(int in_tok,trpgr_Callback *in_cb,bool in_dest)
{
    init(in_tok,in_cb,in_dest);
}
void trpgr_Token::init(int in_tok,trpgr_Callback *in_cb,bool in_dest)
{
    Token = in_tok;
    cb = in_cb;
    destroy = in_dest;
}
// Destruct
// Destroy our callback if appropriate
void trpgr_Token::Destruct()
{
    if (cb && destroy)
        delete cb;
    cb = NULL;
    destroy = true;
}

/* ***************************
   Paging parser implementation.
   ***************************
   */

// Constructor
trpgr_Parser::trpgr_Parser()
{
    lastObject = NULL;
}
trpgr_Parser::~trpgr_Parser()
{
}

// Validity check
bool trpgr_Parser::isValid() const
{
    return true;
}

// Add Callback
// Make the given callback object current for the given token.
void trpgr_Parser::AddCallback(trpgToken tok,trpgr_Callback *cb,bool in_dest)
{
    RemoveCallback(tok);

    tokenMap[tok] = trpgr_Token(tok,cb,in_dest);
}

// Callback used as writeable wrapper
class WriteWrapper : public trpgr_Callback {
public:
    WriteWrapper(trpgReadWriteable *in_wr)  { wr = in_wr; };
    void *Parse(trpgToken,trpgReadBuffer &buf) {
        if (wr->Read(buf))
            return wr;
        else
            return NULL;
    }
protected:
    trpgReadWriteable *wr;
};

// Add Callback (writeable)
// Build a wrapper around a trpgWriteable so it can read itself
void trpgr_Parser::AddCallback(trpgToken tok,trpgReadWriteable *wr)
{
    AddCallback(tok,new WriteWrapper(wr),true);
}

// Get the claaback associated with a token, will return 0 if none
const trpgr_Callback *trpgr_Parser::GetCallback(trpgToken tok) const
{
    tok_map::const_iterator iter = tokenMap.find(tok);
    if(iter != tokenMap.end())
        return iter->second.cb;
    else
        return 0;
}
trpgr_Callback *trpgr_Parser::GetCallback(trpgToken tok)
{
    tok_map::iterator iter = tokenMap.find(tok);
    if(iter != tokenMap.end())
        return iter->second.cb;
    else
        return 0;
}


// Remove Callback
void trpgr_Parser::RemoveCallback(trpgToken tok)
{
    tokenMap.erase(tok);
}

// Set Default Callback
// This gets called for all tokens we don't understand
void trpgr_Parser::SetDefaultCallback(trpgr_Callback *cb,bool in_dest)
{
    defCb.Destruct();
    defCb.init(-1,cb,in_dest);
}

/* Token Is Valid
   Checks if something *could be* a token.
   Doesn't necessarily mean that it is.
*/
bool trpgr_Parser::TokenIsValid(trpgToken tok)
{
    if (tok < 0)
        return false;

    return true;
}

/* Parse Buffer
   This runs through the given buffer parsing token sets until
   it (1) runs out of buffer or (2) fails.
   Note: Good place to return an exception, but keep it simple for now.
*/
bool trpgr_Parser::Parse(trpgReadBuffer &buf)
{
    bool ret = true;

    try
    {
        while (!buf.isEmpty())
        {
            /* We're expecting the following
            Token    (int32)
            Length  (int32)
            Data    (variable)
            */
            trpgToken tok;
            int32 len;
            if (!buf.Get(tok))  throw 1;
            // Push and Pop are special - no data
            if (tok != TRPG_PUSH && tok != TRPG_POP)
            {
                if (!buf.Get(len)) throw 1;
                if (!TokenIsValid(tok))  throw 1;
                if (len < 0)  throw 1;
                // Limit what we're reading to the length of this
                buf.PushLimit(len);
            }

            // Call our token handler for this one
            try
            {
                const trpgr_Token *tcb = NULL;
                tok_map::const_iterator p = tokenMap.find(tok);
                if (p != tokenMap.end())
                    tcb = &(*p).second;
                if (!tcb)
                    // No such token, call the default
                    tcb = &defCb;

                // Run the callback
                if (tcb->cb)
                {
                    void *ret = tcb->cb->Parse(tok,buf);
                    // Note: Do something with the return value
                    lastObject = ret;
                }
            }
            catch (...)
            {
                // Don't want to screw up the limit stack
            }
            // No limit to worry about with push and pop
            if (tok != TRPG_PUSH && tok != TRPG_POP)
            {
                buf.SkipToLimit();
                buf.PopLimit();
            }
        }
    }
    catch (...)
    {
        // Failed to parse.
        ret = false;
    }

    return ret;
}

/*    ****************
  Scene Parser
  ****************
  */
// Helper - callback for Push
class trpgSceneHelperPush : public trpgr_Callback
{
public:
    trpgSceneHelperPush(trpgSceneParser *in_parse)
    { parse = in_parse; };

    void *Parse(trpgToken /*tok*/,trpgReadBuffer& /*buf*/)
    {
        // Call the start children callback
        parse->StartChildren(parse->lastObject);
        parse->parents.push_back(parse->lastObject);
        return (void *)1;
    }
protected:
    trpgSceneParser *parse;
};

// Helper - callback for Pop
class trpgSceneHelperPop : public trpgr_Callback
{
public:
    trpgSceneHelperPop(trpgSceneParser *in_parse)
    { parse = in_parse; };
    void *Parse(trpgToken /*tok*/,trpgReadBuffer& /*buf*/)
    {
        // Make sure we don't have an extra pop
        if (parse->parents.size() == 0)
            // Note: let someone know about the extra pop
            return NULL;
        // Call the end children callback
        int len = parse->parents.size();
        parse->EndChildren(parse->parents[len-1]);
        parse->parents.resize(len-1);
        return (void *)1;
    }
protected:
    trpgSceneParser *parse;
};

// Helper - default callback
// Puts a 1 on the parent stack
// Note: Need to use this fact above
class trpgSceneHelperDefault : public trpgr_Callback
{
public:
    trpgSceneHelperDefault(trpgSceneParser *in_parse) { parse = in_parse; }
    void *Parse(trpgToken /*tok*/,trpgReadBuffer& /*buf*/)
    {
        // Absorb it quietly
        return (void *)1;
    }
protected:
    trpgSceneParser *parse;
};

trpgSceneParser::trpgSceneParser()
{
    // Register for Push and Pop
    AddCallback(TRPG_PUSH,new trpgSceneHelperPush(this));
    AddCallback(TRPG_POP,new trpgSceneHelperPop(this));

    // Register for default
    SetDefaultCallback(new trpgSceneHelperDefault(this));
}
trpgSceneParser::~trpgSceneParser()
{
}
