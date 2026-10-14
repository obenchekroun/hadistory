/* Story prompt generation, port of create_prompt() and helpers in pi/hadistory.py:
 * a random line of prompts/prompts.txt (optional "N:" weight prefix), nested
 * bracket choices "(a|b)", "[a|b]", "{a|b}" resolved at random, wrapped between
 * the French incipit and excipit. prompts.txt is embedded in the firmware at build time.
 */
#ifndef HADISTORY_PROMPT_H
#define HADISTORY_PROMPT_H

#include <stddef.h>

extern const char *const PROMPT_IMAGE_INCIPIT;   // prefix for the illustration request

// Builds the full story prompt into out. Returns false when prompts.txt has no usable line.
bool prompt_create(char *out, size_t out_size);

#endif
