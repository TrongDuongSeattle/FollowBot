// Compatibility shim for older Arduino libraries that include <WProgram.h>
// Modern Arduino cores use Arduino.h. Placing this file in the project's
// include/ directory ensures PlatformIO finds it first and keeps third-party
// headers unchanged.

#pragma once
#include <Arduino.h>
