#undef NDEBUG
#include <cassert>
#include <stdexcept>

#include "Example.h"      // the library under test; include/ comes from oop

static_assert(Example::limit > 0, "the invariant below assumes a positive limit");

int main() {
    Example e;
    assert(e.value() == 0);

    e.add(7);
    assert(e.value() == 7);
    e.g();
    e.add(-7);
    assert(e.value() == 0);

    bool threw = false;
    try {
        e.add(-1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "add() must refuse to go below 0");
    assert(e.value() == 0 && "a failed call leaves the object untouched");

    threw = false;
    try {
        e.add(Example::limit + 1);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "add() must refuse to go above limit");

    threw = false;
    try {
        Example broken{Example::limit + 1};
        (void) broken;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw && "the constructor must not hand out a broken object either");

    return 0;
}
