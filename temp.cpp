#include "src/Key.hpp"
int main() { auto k = KeyEvent::Character(97); return k.IsCharacter(97) ? 0 : 1; }
