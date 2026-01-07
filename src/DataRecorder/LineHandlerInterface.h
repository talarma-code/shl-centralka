#pragma once
#include "GlobalTypes.h"

class LineHandlerInterface {
public:
    virtual void handle(const StaticString192 &line) = 0;
};
