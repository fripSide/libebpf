#include <string.h>  //XXX: for strlen, strcpy
#include <stdarg.h>
#include <stdlib.h>

#define __NOIST __attribute__((no_instrument_function))

static __NOIST char *strrev(char *s)
{
    char *t = s;
    char *u = s;

    if (!s || (*s == '\0'))
        return s;

    /* u points to the last char in the string */
    while (*(u + 1) != '\0')
        u++;

    /* t moves forward, u moves backward */
    while (t < u) {
        char tmp = *t;
        *t++ = *u;
        *u-- = tmp;
    }

    return s;
}

__NOIST void strpad(char *buf, char pad_val, int count)
{
    buf[count--] = '\0';
    for (; count >= 0; count--)
        buf[count] = pad_val;
}

__NOIST char *itoa_base(int value, char *buf, int base)
{
    const char base36[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    int i = 0;
    int isneg = 0;
    unsigned int val = value;

    if (!buf || (base > 36))
        return NULL;

    if ((value < 0) && (base == 10)) {
        val = abs(value);
        isneg = 1;
    }

    do {
        buf[i++] = base36[val % base];
        val /= base;
    } while (val);

    if (isneg)
        buf[i++] = '-';
    buf[i] = '\0';

    return strrev(buf);
}

__NOIST int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    size_t str_pos = 0; /* position in the buffer str */
    char itoa_buf[16]; /* contains a 32bit integer in decimal, plus null char */
    char pad_char;
    int pad_count = 0;

    for (unsigned int i = 0; (i < strlen(format)) && (str_pos < size); i++) {
        if (format[i] == '%') {
            if (format[i + 1] == '%') {
                str[str_pos++] = '%';
                i++;
                continue;
            }

            /* format '%6d': value is blank-padded */
            if ((format[i + 1] >= '0') && (format[i + 1] <= '9') &&
                (format[i + 2] == 'd')) {
                pad_char = ' ';
                pad_count = format[i + 1] - '0';
                i += 1;
                goto print_dec;
            }

            /* format '%06d' or '% 6d': value is 0- or blank-padded */
            if ((format[i + 1] == ' ') || (format[i + 1] == '0')) {
                pad_char = format[i + 1];
                pad_count = format[i + 2] - '0';
                i += 2;
            }

            /* conversion modifier */
            switch (format[i + 1]) {
            case 'p':
                strcpy(&str[str_pos], "0x");
                str_pos += 2;
                pad_char = '0';
                pad_count = 8;
                goto print_hex;
            case 'f': // ignore
                break;
            case 'd':
            print_dec:
                itoa_base(va_arg(ap, unsigned int), itoa_buf, 10);
                goto print_num;
            case 'x':
            print_hex:
                itoa_base(va_arg(ap, unsigned int), itoa_buf, 16);
            print_num:;
                int itoa_buf_len = strlen(itoa_buf);
                if (pad_count) {
                    pad_count -= itoa_buf_len;
                    strpad(&str[str_pos], pad_char, pad_count);
                    str_pos += pad_count;
                }
                strcpy(&str[str_pos], itoa_buf);
                str_pos += itoa_buf_len;
                break;
            case 's':;
                char *str_varg = va_arg(ap, char *);
                strcpy(&str[str_pos], str_varg);
                str_pos += strlen(str_varg);
                break;
            default:
                goto ordinary_char;
            }
            i++;
        } else {
        ordinary_char:
            str[str_pos++] = format[i];
        }
    }
    str[str_pos] = '\0';

    return str_pos;
}

__NOIST int vsprintf(char *str, const char *format, va_list ap)
{
    return vsnprintf(str, 1000, format, ap);
}

__NOIST int sprintf(char *str, const char *format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = vsprintf(str, format, ap);
    va_end(ap);

    return retval;
}

__NOIST int snprintf(char *str, size_t size, const char *format, ...)
{
    int retval;
    va_list ap;

    va_start(ap, format);
    retval = vsnprintf(str, size, format, ap);
    va_end(ap);

    return retval;
}