#include <osg/State>
#include <osg/Notify>

#include <osg/GLU>

using namespace osg;

State::State()
{
    _contextID = 0;
}

State::~State()
{
}

void State::reset()
{
/*
    for(ModeMap::iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        ModeStack& ms = mitr->second;
        ms.valueVec.clear();
        ms.last_applied_value = !ms.global_default_value;
        ms.changed = true;
    }        
*/

    _modeMap.clear();
    _modeMap[GL_DEPTH_TEST].global_default_value = true;

    // go through all active StateAttribute's, applying where appropriate.
    for(AttributeMap::iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        AttributeStack& as = aitr->second;
        as.attributeVec.clear();
        as.last_applied_attribute = NULL;
        as.changed = true;
    }        

//    _attributeMap.clear();

    _drawStateStack.clear();
}

void State::pushStateSet(const StateSet* dstate)
{
    _drawStateStack.push_back(dstate);
    if (dstate)
    {
        // iterator through all OpenGL modes in incomming StateSet
        // for each GLMode entry push it to the back of the appropriate
        // mode stack, taking into consideration current override status.
        const StateSet::ModeList& ds_modeList = dstate->getModeList();
        for(StateSet::ModeList::const_iterator mitr=ds_modeList.begin();
            mitr!=ds_modeList.end();
            ++mitr)
        {
            // get the mode stack for incomming GLmode {mitr->first}.
            ModeStack& ms = _modeMap[mitr->first];
            if (ms.valueVec.empty())
            {
                // first pair so simply push incomming pair to back.
                ms.valueVec.push_back(mitr->second);
            }
            else if (ms.valueVec.back() & StateAttribute::OVERRIDE) // check the existing override flag 
            {
                // push existing back since override keeps the previoius value.
                ms.valueVec.push_back(ms.valueVec.back());
            }
            else 
            {
                // no override on so simply push incomming pair to back.
                ms.valueVec.push_back(mitr->second);
            }
            ms.changed = true;
        }
        
        // iterator through all StateAttribute's in incomming StateSet
        // for each Type entry push it to the back of the appropriate
        // attribute stack, taking into consideration current override status.
        const StateSet::AttributeList& ds_attributeList = dstate->getAttributeList();
        for(StateSet::AttributeList::const_iterator aitr=ds_attributeList.begin();
            aitr!=ds_attributeList.end();
            ++aitr)
        {
            // get the attribute stack for incomming type {aitr->first}.
            AttributeStack& as = _attributeMap[aitr->first];
            if (as.attributeVec.empty())
            {
                // first pair so simply push incomming pair to back.
                as.attributeVec.push_back(
                    AttributePair(aitr->second.first.get(),aitr->second.second));
            }
            else if (as.attributeVec.back().second & StateAttribute::OVERRIDE) // check the existing override flag 
            {
                // push existing back since override keeps the previoius value.
                as.attributeVec.push_back(as.attributeVec.back());
            }
            else 
            {
                // no override on so simply push incomming pair to back.
                as.attributeVec.push_back(
                    AttributePair(aitr->second.first.get(),aitr->second.second));
            }
            as.changed = true;
        }

    }
}

void State::popStateSet()
{
    if (_drawStateStack.empty()) return;
    
    const StateSet* dstate = _drawStateStack.back().get();

    if (dstate)
    {

        // iterator through all OpenGL modes in incomming StateSet
        // for each GLMode entry pop_back of the appropriate
        // mode stack.
        const StateSet::ModeList& ds_modeList = dstate->getModeList();
        for(StateSet::ModeList::const_iterator mitr=ds_modeList.begin();
            mitr!=ds_modeList.end();
            ++mitr)
        {
            // get the mode stack for incomming GLmode {mitr->first}.
            ModeStack& ms = _modeMap[mitr->first];
            if (!ms.valueVec.empty())
            {
                ms.valueVec.pop_back();
            }
            ms.changed = true;
        }
        
        // iterator through all StateAttribute's in incomming StateSet
        // for each Type entry pop_back of the appropriate
        // attribute stack.
        const StateSet::AttributeList& ds_attributeList = dstate->getAttributeList();
        for(StateSet::AttributeList::const_iterator aitr=ds_attributeList.begin();
            aitr!=ds_attributeList.end();
            ++aitr)
        {
            // get the attribute stack for incomming type {aitr->first}.
            AttributeStack& as = _attributeMap[aitr->first];
            if (!as.attributeVec.empty())
            {
                as.attributeVec.pop_back();
            }
            as.changed = true;
        }
    }
    
    // remove the top draw state from the stack.
    _drawStateStack.pop_back();
}

void State::captureCurrentState(StateSet& stateset) const
{
    // empty the stateset first.
    stateset.setAllToInherit();
    
    for(ModeMap::const_iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        // note GLMode = mitr->first
        const ModeStack& ms = mitr->second;
        if (!ms.valueVec.empty())
        {
            stateset.setMode(mitr->first,ms.valueVec.back());
        }
    }        

    for(AttributeMap::const_iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        const AttributeStack& as = aitr->second;
        if (!as.attributeVec.empty())
        {
            stateset.setAttribute(const_cast<StateAttribute*>(as.attributeVec.back().first));
        }
    }        

}


void State::apply(const StateSet* dstate)
{
    // equivilant to:
    //pushStateSet(dstate);
    //apply();
    //popStateSet();

    if (dstate)
    {

        // then handle mode changes.
        {
            const StateSet::ModeList& ds_modeList = dstate->getModeList();

            StateSet::ModeList::const_iterator ds_mitr = ds_modeList.begin();
            ModeMap::iterator this_mitr=_modeMap.begin();

            while (this_mitr!=_modeMap.end() && ds_mitr!=ds_modeList.end())
            {
                if (this_mitr->first<ds_mitr->first)
                {

                    // note GLMode = this_mitr->first
                    ModeStack& ms = this_mitr->second;
                    if (ms.changed)
                    {
                        ms.changed = false;
                        if (!ms.valueVec.empty())
                        {
                            bool new_value = ms.valueVec.back() & StateAttribute::ON;
                            apply_mode(this_mitr->first,new_value,ms);
                        }
                        else
                        {
                            // assume default of disabled.
                            apply_mode(this_mitr->first,ms.global_default_value,ms);

                        }
            
                    }

                    ++this_mitr;

                }
                else if (ds_mitr->first<this_mitr->first)
                {

                    // ds_mitr->first is a new mode, therefore 
                    // need to insert a new mode entry for ds_mistr->first.
                    ModeStack& ms = _modeMap[ds_mitr->first];

                    bool new_value = ds_mitr->second & StateAttribute::ON;
                    apply_mode(ds_mitr->first,new_value,ms);

                    // will need to disable this mode on next apply so set it to changed.
                    ms.changed = true;

                    ++ds_mitr;

                }
                else
                {
                    // this_mitr & ds_mitr refer to the same mode, check the overide
                    // if any otherwise just apply the incomming mode.

                    ModeStack& ms = this_mitr->second;

                    if (!ms.valueVec.empty() && ms.valueVec.back() & StateAttribute::OVERRIDE)
                    {
                        // override is on, there just treat as a normal apply on modes.

                        if (ms.changed)
                        {
                            ms.changed = false;
                            bool new_value = ms.valueVec.back() & StateAttribute::ON;
                            apply_mode(this_mitr->first,new_value,ms);
        
                        }
                    }
                    else
                    {
                        // no override on or no previous entry, therefore consider incomming mode.
                        bool new_value = ds_mitr->second & StateAttribute::ON;
                        if (apply_mode(ds_mitr->first,new_value,ms))
                        {
                            ms.changed = true;
                        }
                    }
                    
                    ++this_mitr;
                    ++ds_mitr;
                }
            }

            // iterator over the remaining state modes to apply any previous changes.
            for(;
                this_mitr!=_modeMap.end();
                ++this_mitr)
            {
                // note GLMode = this_mitr->first
                ModeStack& ms = this_mitr->second;
                if (ms.changed)
                {
                    ms.changed = false;
                    if (!ms.valueVec.empty())
                    {
                        bool new_value = ms.valueVec.back() & StateAttribute::ON;
                        apply_mode(this_mitr->first,new_value,ms);
                    }
                    else
                    {
                        // assume default of disabled.
                        apply_mode(this_mitr->first,ms.global_default_value,ms);

                    }
            
                }
            }        

            // iterator over the remaining incomming modes to apply any new mode.
            for(;
                ds_mitr!=ds_modeList.end();
                ++ds_mitr)
            {
                ModeStack& ms = _modeMap[ds_mitr->first];

                bool new_value = ds_mitr->second & StateAttribute::ON;
                apply_mode(ds_mitr->first,new_value,ms);

                // will need to disable this mode on next apply so set it to changed.
                ms.changed = true;
            }
        }



        // first handle attribute changes
        {
            const StateSet::AttributeList& ds_attributeList = dstate->getAttributeList();
            StateSet::AttributeList::const_iterator ds_aitr=ds_attributeList.begin();

            AttributeMap::iterator this_aitr=_attributeMap.begin();

            while (this_aitr!=_attributeMap.end() && ds_aitr!=ds_attributeList.end())
            {
                if (this_aitr->first<ds_aitr->first)
                {

                    // note attribute type = this_aitr->first
                    AttributeStack& as = this_aitr->second;
                    if (as.changed)
                    {
                        as.changed = false;
                        if (!as.attributeVec.empty())
                        {
                            const StateAttribute* new_attr = as.attributeVec.back().first;
                            apply_attribute(new_attr,as);
                        }
                        else
                        {
                            apply_global_default_attribute(as);
                        }
                    }

                    ++this_aitr;

                }
                else if (ds_aitr->first<this_aitr->first)
                {

                    // ds_mitr->first is a new attribute, therefore 
                    // need to insert a new attribute entry for ds_aistr->first.
                    AttributeStack& as = _attributeMap[ds_aitr->first];

                    const StateAttribute* new_attr = ds_aitr->second.first.get();
                    apply_attribute(new_attr,as);

                    // will need to disable this mode on next apply so set it to changed.
                    as.changed = true;

                    ++ds_aitr;

                }
                else
                {
                    // this_mitr & ds_mitr refer to the same mode, check the overide
                    // if any otherwise just apply the incomming mode.

                    AttributeStack& as = this_aitr->second;

                    if (!as.attributeVec.empty() && as.attributeVec.back().second)
                    {
                        // override is os, there just treat as a normal apply on modes.

                        if (as.changed)
                        {
                            as.changed = false;
                            const StateAttribute* new_attr = as.attributeVec.back().first;
                            apply_attribute(new_attr,as);
                        }
                    }
                    else
                    {
                        // no override on or no previous entry, therefore consider incomming mode.
                        const StateAttribute* new_attr = ds_aitr->second.first.get();
                        if (apply_attribute(new_attr,as))
                        {
                            as.changed = true;
                        }
                    }
                    
                    ++this_aitr;
                    ++ds_aitr;
                }
            }

            // iterator over the remaining state modes to apply any previous changes.
            for(;
                this_aitr!=_attributeMap.end();
                ++this_aitr)
            {
                // note attribute type = this_aitr->first
                AttributeStack& as = this_aitr->second;
                if (as.changed)
                {
                    as.changed = false;
                    if (!as.attributeVec.empty())
                    {
                        const StateAttribute* new_attr = as.attributeVec.back().first;
                        apply_attribute(new_attr,as);
                    }
                    else
                    {
                        apply_global_default_attribute(as);
                    }
                }
            }        

            // iterator over the remaining incomming modes to apply any new mode.
            for(;
                ds_aitr!=ds_attributeList.end();
                ++ds_aitr)
            {
                // ds_mitr->first is a new attribute, therefore 
                // need to insert a new attribute entry for ds_aistr->first.
                AttributeStack& as = _attributeMap[ds_aitr->first];

                const StateAttribute* new_attr = ds_aitr->second.first.get();
                apply_attribute(new_attr,as);

                // will need to update this attribute on next apply so set it to changed.
                as.changed = true;
            }
        }



    }
    else
    {
        // no incomming stateset, so simply apply state.
        apply();
    }

}

void State::apply()
{

    // go through all active OpenGL modes, enabling/disable where
    // appropriate.
    for(ModeMap::iterator mitr=_modeMap.begin();
        mitr!=_modeMap.end();
        ++mitr)
    {
        // note GLMode = mitr->first
        ModeStack& ms = mitr->second;
        if (ms.changed)
        {
            ms.changed = false;
            if (!ms.valueVec.empty())
            {
                bool new_value = ms.valueVec.back() & StateAttribute::ON;
                apply_mode(mitr->first,new_value,ms);
            }
            else
            {
                // assume default of disabled.
                apply_mode(mitr->first,ms.global_default_value,ms);
            }
            
        }
    }        


    // go through all active StateAttribute's, applying where appropriate.
    for(AttributeMap::iterator aitr=_attributeMap.begin();
        aitr!=_attributeMap.end();
        ++aitr)
    {
        AttributeStack& as = aitr->second;
        if (as.changed)
        {
            as.changed = false;
            if (!as.attributeVec.empty())
            {
                const StateAttribute* new_attr = as.attributeVec.back().first;
                apply_attribute(new_attr,as);
            }
            else
            {
                apply_global_default_attribute(as);
            }
            
        }
    }        
    
}

/** mode has been set externally, update state to reflect this setting.*/
void State::have_applied(const StateAttribute::GLMode mode,const StateAttribute::GLModeValue value)
{
    ModeStack& ms = _modeMap[mode];

    ms.last_applied_value = value & StateAttribute::ON;

    // will need to disable this mode on next apply so set it to changed.
    ms.changed = true;    
}

/** attribute has been applied externally, update state to reflect this setting.*/
void State::have_applied(const StateAttribute* attribute)
{
    if (attribute)
    {
        AttributeStack& as = _attributeMap[attribute->getType()];

        as.last_applied_attribute = attribute;

        // will need to update this attribute on next apply so set it to changed.
        as.changed = true;
    }
}

