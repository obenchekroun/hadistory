/* Just enough JSON for the OpenAI API: escaping a string into a request body,
 * and pulling one string value out of a response. */
#ifndef HADISTORY_JSON_H
#define HADISTORY_JSON_H

#include <stddef.h>
#include <stdbool.h>

// Writes the JSON-escaped form of `in` (without the surrounding quotes) into out.
// Returns the length written (truncated to out_size - 1).
size_t json_escape(const char *in, char *out, size_t out_size);

// Finds the first `"key":"value"` in json and copies the unescaped value into out.
bool json_extract_string(const char *json, size_t json_len, const char *key, char *out, size_t out_size);

#endif
