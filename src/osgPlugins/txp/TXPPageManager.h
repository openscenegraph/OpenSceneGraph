#ifndef __TXPPAGEMANAGER_H_
#define __TXPPAGEMANAGER_H_

#include "trpage_sys.h"
#include "trpage_read.h"
#include "trpage_managers.h"

#include <osg/Referenced>

namespace txp
{
    class TXPPageManager : public trpgPageManager, public osg::Referenced
    {
    public:
        TXPPageManager();

    protected:
        virtual ~TXPPageManager();

    };

} // namespace

#endif // __TXPPAGEMANAGER_H_
