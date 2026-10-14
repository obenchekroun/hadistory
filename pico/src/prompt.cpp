#include "prompt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pico/rand.h"

// Embedded by CMake from prompts/prompts.txt, NUL terminated
extern "C" const char hadistory_prompts_txt[];

static const char *const PROMPT_INCIPIT =
    "Crée un texte issu d'une page d'un livre illustré pour enfant. Ce texte doit faire maximum 60 mots. "
    "Il doit suivre le thème suivant : ";
static const char *const PROMPT_EXCIPIT =
    "Sois créatif. Inclus une fin heureuse. Pas de titre.";
const char *const PROMPT_IMAGE_INCIPIT =
    "Crée une illustration pour l'histoire suivante. Evite une illustration en cases de bande dessinée, "
    "crée une seule image pour toute l'histoire. N'inclut pas de texte dans l'image générée. "
    "Utilise un style graphique ghibli, comic book ou livre d'enfant, au hasard. Soit créatif. "
    "Voici l'histoire : ";

static const char *const BRACKETS[] = { "{}", "[]", "()" };   // reverse of the Python list, as parse_multiple_brackets does

// "12:text" -> weight 12 and text; otherwise weight 1
static int weight_of(const char *s, size_t len, const char **text) {
    size_t i = 0;
    while (i < len && isdigit((unsigned char)s[i])) i++;
    if (i > 0 && i < len && s[i] == ':') {
        *text = s + i + 1;
        return atoi(s);
    }
    *text = s;
    return 1;
}

// Picks one option of "a|2:b|c" with weights, copies it into out
static void pick_option(const char *s, size_t len, char *out, size_t out_size) {
    int total = 0;
    const char *p = s, *end = s + len;
    while (p <= end) {
        const char *bar = (const char *)memchr(p, '|', end - p);
        size_t olen = bar ? (size_t)(bar - p) : (size_t)(end - p);
        const char *text;
        total += weight_of(p, olen, &text);
        if (!bar) break;
        p = bar + 1;
    }
    int r = total > 0 ? (int)(get_rand_32() % (uint32_t)total) : 0;
    p = s;
    while (p <= end) {
        const char *bar = (const char *)memchr(p, '|', end - p);
        size_t olen = bar ? (size_t)(bar - p) : (size_t)(end - p);
        const char *text;
        int w = weight_of(p, olen, &text);
        if (r < w || !bar) {
            size_t tlen = olen - (size_t)(text - p);
            if (tlen >= out_size) tlen = out_size - 1;
            memcpy(out, text, tlen);
            out[tlen] = '\0';
            return;
        }
        r -= w;
        p = bar + 1;
    }
    out[0] = '\0';
}

// Replaces every "<open>...<close>" group in text by one of its options, repeatedly
static void resolve_brackets(char *text, size_t size, char open, char close) {
    for (;;) {
        char *o = strchr(text, open);
        if (!o) return;
        char *c = strchr(o + 1, close);
        if (!c) return;
        char choice[256];
        pick_option(o + 1, (size_t)(c - o - 1), choice, sizeof choice);
        char tail[512];
        snprintf(tail, sizeof tail, "%s", c + 1);
        size_t head = (size_t)(o - text);
        snprintf(text + head, size - head, "%s%s", choice, tail);
    }
}

static void collapse_spaces(char *text) {
    char *w = text;
    bool space = false;
    for (char *r = text; *r; r++) {
        if (*r == ' ' || *r == '\t') {
            if (!space) *w++ = ' ';
            space = true;
        } else {
            *w++ = *r;
            space = false;
        }
    }
    *w = '\0';
}

bool prompt_create(char *out, size_t out_size) {
    // Weighted random line: total weight first, then walk again to the chosen one
    int total = 0;
    for (const char *p = hadistory_prompts_txt; *p; ) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        if (len > 0 && p[len - 1] == '\r') len--;
        const char *text;
        if (len > 0) total += weight_of(p, len, &text);
        if (!nl) break;
        p = nl + 1;
    }
    if (total == 0) {
        puts("No lines to parse found in prompts.txt");
        return false;
    }
    int r = (int)(get_rand_32() % (uint32_t)total);
    char line[512] = "";
    for (const char *p = hadistory_prompts_txt; *p; ) {
        const char *nl = strchr(p, '\n');
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        if (len > 0 && p[len - 1] == '\r') len--;
        if (len > 0) {
            const char *text;
            int w = weight_of(p, len, &text);
            if (r < w) {
                size_t tlen = len - (size_t)(text - p);
                if (tlen >= sizeof line) tlen = sizeof line - 1;
                memcpy(line, text, tlen);
                line[tlen] = '\0';
                break;
            }
            r -= w;
        }
        if (!nl) break;
        p = nl + 1;
    }

    for (const char *pair : BRACKETS) {
        resolve_brackets(line, sizeof line, pair[0], pair[1]);
    }
    collapse_spaces(line);

    snprintf(out, out_size, "%s%s %s", PROMPT_INCIPIT, line, PROMPT_EXCIPIT);
    return true;
}
