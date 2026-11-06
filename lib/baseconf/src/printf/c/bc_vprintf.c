#include <baseconf/printf.h>
#include <baseconf/console.h>
#include <limits.h>
#include "../../console/internal.h"
#include "../../text/internal.h"
#include "../../video/internal.h"
#include "../internal.h"

static int bc__va_int(va_list *args) {
    return va_arg(*args, int);
}
static unsigned int bc__va_uint(va_list *args) {
    return va_arg(*args, unsigned int);
}
static const char *bc__va_string(va_list *args) {
    return va_arg(*args, const char *);
}

static int bc__emit(char c, int count) {
    if (bc_console_putc(c) != BC_OK) return -1;
    return count + 1;
}

/* The caller stops at a format delimiter or at the end of its input. */
static int bc__emit_text(const char **source, uint8_t limit, uint8_t stop, int count) {
    const char *text = *source;
#ifdef BASECONF_VIDEO_TEXT_GRAPHICS
    const bc_font_t *font = bc_console_active_font;
    if (font && bc_video_draw_surface_state.mode < BC_VIDEO_ATM_TEXT_80) {
        uint8_t code = (uint8_t)*text;
        if (code < 32 && code != '\t') {
            *source = text + 1;
            bc__console_control(code);
            return count + 1;
        }
        uint8_t column = _console_x;
        uint8_t row = _console_y;
        /* Short fields use the character path to avoid the row setup cost. */
        if (limit >= 4 && column < _console_w && row < _console_h && count <= INT_MAX - UINT8_MAX) {
            uint8_t room = _console_w - column;
            uint8_t length = 0;
            if (limit > room) limit = room;
            while (length < limit) {
                code = (uint8_t)text[length];
                if (code < 32 || code > 127 || code == stop) break;
                ++length;
            }
            if (length >= 4) {
                if (bc__text_draw_font(column, row, text, font, length) != BC_OK) return -1;
                *source = text + length;
                if (length == room)
                    bc__console_newline();
                else
                    _console_x = column + length;
                return count + length;
            }
        }
    }
#else
    (void)limit;
    (void)stop;
#endif
    *source = text + 1;
    return bc__emit(*text, count);
}

static int bc__emit_field(const char *text, uint8_t length, uint8_t width, uint8_t left,
                          uint8_t zero, uint8_t negative, int count) {
    uint8_t padding = width > length + negative ? (uint8_t)(width - length - negative) : 0;
    const char *end = text + length;
    uint8_t i;
    if (!left && !zero)
        for (i = 0; i < padding; ++i) {
            count = bc__emit(' ', count);
            if (count < 0) return -1;
        }
    if (negative) {
        count = bc__emit('-', count);
        if (count < 0) return -1;
    }
    if (!left && zero)
        for (i = 0; i < padding; ++i) {
            count = bc__emit('0', count);
            if (count < 0) return -1;
        }
    while (text != end) {
        count = bc__emit_text(&text, (uint8_t)(end - text), 0, count);
        if (count < 0) return -1;
    }
    if (left)
        for (i = 0; i < padding; ++i) {
            count = bc__emit(' ', count);
            if (count < 0) return -1;
        }
    return count;
}

int bc_vprintf(const char *format, va_list args) {
    int count = 0;
    if (!format) return -1;
    while (*format) {
        uint8_t width = 0, left = 0, zero = 0;
        char spec;
        if (*format != '%') {
            count = bc__emit_text(&format, UINT8_MAX, '%', count);
            if (count < 0) return -1;
            continue;
        }
        ++format;
        if (*format == '%') {
            ++format;
            count = bc__emit('%', count);
            if (count < 0) return -1;
            continue;
        }
        if (*format == '-') {
            left = 1;
            ++format;
        }
        if (*format == '0') {
            zero = 1;
            ++format;
        }
        while (*format >= '0' && *format <= '9') {
            uint8_t digit = (uint8_t)(*format - '0');
            if (width > 25u || (width == 25u && digit > 5u))
                width = 255u;
            else
                width = (uint8_t)(width * 10u + digit);
            ++format;
        }
        spec = *format;
        if (!spec) {
            count = bc__emit('%', count);
            if (count < 0) return -1;
            break;
        }
        ++format;
        if (spec == 'c') {
            char value = (char)bc__va_int(&args);
            count = bc__emit_field(&value, 1u, width, left, zero, 0u, count);
        } else if (spec == 's') {
            const char *value = bc__va_string(&args);
            uint8_t length = 0;
            if (!value) value = "";
            while (value[length] && length != 255u) ++length;
            count = bc__emit_field(value, length, width, left, 0u, 0u, count);
            if (count < 0) return -1;
            if (length == 255u) {
                value += 255u;
                while (*value) {
                    count = bc__emit_text(&value, UINT8_MAX, 0, count);
                    if (count < 0) return -1;
                }
            }
        } else if (spec == 'd' || spec == 'i') {
            int16_t signed_value = (int16_t)bc__va_int(&args);
            uint8_t negative = (uint8_t)(signed_value < 0);
            uint16_t magnitude =
                negative ? (uint16_t)(0u - (uint16_t)signed_value) : (uint16_t)signed_value;
            char digits[5];
            char *end = digits + sizeof(digits);
            uint8_t length = bc__printf_unsigned_string(magnitude, 10u, 0u, end);
            count = bc__emit_field(end - length, length, width, left, zero, negative, count);
        } else if (spec == 'u' || spec == 'x' || spec == 'X') {
            uint16_t value = (uint16_t)bc__va_uint(&args);
            char digits[5];
            char *end = digits + sizeof(digits);
            uint8_t length = bc__printf_unsigned_string(value, (spec == 'u') ? 10u : 16u,
                                                        (uint8_t)(spec == 'X'), end);
            count = bc__emit_field(end - length, length, width, left, zero, 0u, count);
        } else {
            count = bc__emit('%', count);
            if (count < 0) return -1;
            count = bc__emit(spec, count);
        }
        if (count < 0) return -1;
    }
    return count;
}
