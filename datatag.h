#ifndef _DATATAG_H_
#define _DATATAG_H_

#include <stdint.h>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

class Tag {
public:
    Tag();
    //Tag(uint8_t address);
    
private:
    uint8_t devAddr;
};

#endif /* _DATATAG_H_ */
