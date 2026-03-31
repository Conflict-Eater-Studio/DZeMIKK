#ifndef TUL_PBL_DZEMIKK_TRANSITION_H
#define TUL_PBL_DZEMIKK_TRANSITION_H
#include <functional>
#include <string>
struct Transition {
    std::string targetState;
    std::function<bool()> condition;
};
#endif // TUL_PBL_DZEMIKK_TRANSITION_H
