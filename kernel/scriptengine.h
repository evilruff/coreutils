#pragma once

#include <string>
#include <map>

#include "classutils.h"
#include "shareddata.h"

class ScriptCoreData : public SharedData {
public:
    
    ScriptCoreData() = default;

    ScriptCoreData(const ScriptCoreData & other) :
        SharedData(other), ident(other.ident), isValid(other.isValid), bufferSize(other.bufferSize), result(other.result), command(other.command) {};

    ~ScriptCoreData() = default;

    std::string             ident;
    std::string             command;
    std::string             result;

    bool                    isValid = false;

    int                     bufferSize = 1024;
};

class ScriptCore {
public:
    SHARED_DATA_CLASS_DECLARE(ScriptCore, ScriptCoreData, d);

    ScriptCore(const char * command, const char * ident = nullptr) {
        d = new ScriptCoreData;
        d->command = command;

        if (ident)
            d->ident    = ident;

        d->isValid  = true;
    }

    bool            isValid()  const              {  return d->isValid;     };
    std::string     ident()    const              {  return d->ident;       };
    std::string     command() const               {  return d->command;     };
    void            setReadBufferSize(int limit)  { d->bufferSize = limit;  };
    int             readBufferSize() const        { return d->bufferSize;   };
    std::string     result() const                { return d->result;       };

    bool            execute();  

protected:

  
};

